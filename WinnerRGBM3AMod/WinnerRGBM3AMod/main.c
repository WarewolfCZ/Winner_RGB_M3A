/*
 * WinnerRGBM3AMod.c
 *
 * Created: 25.12.2019 15:36:18
 * Author : WarewolfCZ
 */ 

#define VERSION "1.0.0"

#define DEBUG_ENABLED 1

#define F_CPU 8000000UL // 8 MHz
#define BAUD 19200       // define speed
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1) // set baud rate value for UBRR
#define DEFAULT_LED_TIMER_THRESHOLD 40
#define EEPROM_SETTINGS_ADDRESS 0x00
#define ANALOG_INPUTS_COUNT 6
#define OUTPUTS_COUNT 4
#define INPUT_COLD 0
#define INPUT_WARM 1

#define OUTPUT_POWER 3
#define OUTPUT_MODE 2
#define OUTPUT_INCREASE 1
#define OUTPUT_DECREASE 0

#define LEVEL_1_THRESHOLD 90 // 0 to 135
#define LEVEL_2_THRESHOLD 250 // 135 to 315
#define LEVEL_3_THRESHOLD 600 // 315 to 665
#define LEVEL_4_THRESHOLD 800 // 665 to 840
#define LEVEL_5_THRESHOLD 930 // 840 to 965

#define LEVEL_0 0
#define LEVEL_1 1
#define LEVEL_2 2
#define LEVEL_3 3
#define LEVEL_4 4
#define LEVEL_5 5

#define SETTINGS_COLD ((uint8_t)(settings >> 8))
#define SETTINGS_WARM ((uint8_t)(settings & 0xFF))

#define POWER_DIFFERS ((inputs[INPUT_COLD] == LEVEL_0 && inputs[INPUT_WARM] == LEVEL_0 && (SETTINGS_COLD != LEVEL_0 || SETTINGS_WARM != LEVEL_0)) || ((inputs[INPUT_COLD] != LEVEL_0 || inputs[INPUT_WARM] != LEVEL_0) && SETTINGS_COLD == LEVEL_0 && SETTINGS_WARM == LEVEL_0))
#define MODE_DIFFERS (\
 (inputs[INPUT_COLD] == LEVEL_0 && SETTINGS_COLD != LEVEL_0) ||\
 (inputs[INPUT_WARM] == LEVEL_0 && SETTINGS_WARM != LEVEL_0) ||\
 (inputs[INPUT_COLD] != LEVEL_0 && SETTINGS_COLD == LEVEL_0) ||\
 (inputs[INPUT_WARM] != LEVEL_0 && SETTINGS_WARM == LEVEL_0)\
)
#define BRIGHTNESS_DIFFERS (inputs[INPUT_COLD] != SETTINGS_COLD || inputs[INPUT_WARM] != SETTINGS_WARM)

#define COLD_BRIGHTNESS_LOWER (inputs[INPUT_COLD] < SETTINGS_COLD)
#define WARM_BRIGHTNESS_LOWER (inputs[INPUT_WARM] < SETTINGS_WARM)

#define COLD_BRIGHTNESS_HIGHER (inputs[INPUT_COLD] > SETTINGS_COLD)
#define WARM_BRIGHTNESS_HIGHER (inputs[INPUT_WARM] > SETTINGS_WARM)

#define BRIGHTNESS_LOWER COLD_BRIGHTNESS_LOWER || WARM_BRIGHTNESS_LOWER
#define BRIGHTNESS_HIGHER COLD_BRIGHTNESS_HIGHER || WARM_BRIGHTNESS_HIGHER

#define TOUCH_DELAY 110

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <avr/eeprom.h>

uint16_t settings, pin;
uint8_t selectedInput = 0;
uint16_t analogInputs[ANALOG_INPUTS_COUNT];
uint8_t inputs[ANALOG_INPUTS_COUNT];
uint8_t bufferIndex = 0;
uint8_t bufferLimit = 50;
char *uartBuffer;

// function to initialize UART
void uart_init (void) {
	UBRRH = (BAUDRATE>>8);                      // shift the register right by 8 bits
	UBRRL = BAUDRATE;                           // set baud rate
	UCSRB|= (1<<TXEN)|(1<<RXEN);                // enable receiver and transmitter
	UCSRC|= (1<<URSEL)|(1<<UPM1)|(1<<UCSZ0)|(1<<UCSZ1);   // 8bit data format, even parity
	
	DDRD |= 1 << PIND1;//pin1 of portD as OUTPUT
	DDRD &= ~(1 << PIND0);//pin0 of portD as INPUT
	PORTD |= 1 << PIND0; // turn on the pull-up

    uartBuffer = malloc(bufferLimit * sizeof(char) + 1);
    memset(uartBuffer, 0, sizeof(uartBuffer[0])*bufferLimit); // clear buffer
}

// function to send data
void uart_transmit (unsigned char data) {
	while ( !( UCSRA & (1<<UDRE)) );              // wait while register is free
	UDR = data;                                   // load data in the register
}

// function to receive data
unsigned char uart_receive (void) {
	while(!(UCSRA & (1<<RXC)));                   // wait while data is being received
	return UDR;                                   // return 8-bit data
}

void uart_str_transmit(char* str) {
	int i = 0;
	
	while (str[i] != 0x00) {
		uart_transmit(str[i]);
		i++;
	}
}

char* flushBuffer() {
    uartBuffer[bufferIndex] = 0x00;
    bufferIndex = 0;
    return uartBuffer;
}

char* uart_str_receive() {
    if (bufferIndex == 0) {
        memset(uartBuffer, 0, sizeof(uartBuffer[0])*bufferLimit); // clear buffer
    }
	if (bufferIndex < bufferLimit) {
		unsigned char receivedChar = uart_receive();
        if (receivedChar == '\n' || receivedChar == '\r') {
            return flushBuffer();
        } else {
		    uartBuffer[bufferIndex] = receivedChar;
            bufferIndex++;
            //uart_transmit(receivedChar);
        }
	} else {
        return flushBuffer();
    }
	return NULL;
}

void wdt_init(void) {
	MCUSR = 0;
	wdt_disable(); //disable watchdog
}

void soft_reset() {
    if (DEBUG_ENABLED) uart_str_transmit("Soft reset\r\n");
    // enable watchdog and let it reset the controller
    wdt_enable(WDTO_15MS);
    for(;;){}
}

void adc_init() {
    ADCSRA |= (1 << ADEN); // enable ADC
    ADMUX &= ~((1 << REFS1) | (1 << REFS0)); // select voltage reference - AREF
    ADMUX |= 0; // select pin ADC0 as analog input
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1); // use a prescaler of 64
}

void output_init() {
    DDRB |= 1 << PINB0; // PIN0 of portB as OUTPUT
    DDRB |= 1 << PINB1; // PIN1 of portB as OUTPUT
    DDRB |= 1 << PINB2; // PIN3 of portB as OUTPUT
    DDRB |= 1 << PINB3; // PIN3 of portB as OUTPUT
    PORTB &= ~(1 << PINB0); // set PB0 to LOW
    PORTB &= ~(1 << PINB1); // set PB1 to LOW
    PORTB &= ~(1 << PINB2); // set PB2 to LOW
    PORTB &= ~(1 << PINB3); // set PB3 to LOW
}

void touchPower() {
    if (DEBUG_ENABLED) uart_str_transmit("power button touch\r\n");
    PORTB |= (1 << PINB3);
    _delay_ms(200);
    PORTB &= ~(1 << PINB3);
    _delay_ms(TOUCH_DELAY);
}

void touchMode() {
    if (DEBUG_ENABLED) uart_str_transmit("mode button touch\r\n");
    PORTB |= (1 << PINB2);
    _delay_ms(TOUCH_DELAY);
    PORTB &= ~(1 << PINB2);
    _delay_ms(TOUCH_DELAY);
}

void touchIncrease() {
    if (DEBUG_ENABLED) uart_str_transmit("increase button touch\r\n");
    PORTB |= (1 << PINB1);
    _delay_ms(TOUCH_DELAY);
    PORTB &= ~(1 << PINB1);
    _delay_ms(TOUCH_DELAY);
}

void touchDecrease() {
    if (DEBUG_ENABLED) uart_str_transmit("decrease button touch\r\n");
    PORTB |= (1 << PINB0); // set PB0 to HIGH
    _delay_ms(TOUCH_DELAY);
    PORTB &= ~(1 << PINB0); // set PB0 to LOW
    _delay_ms(TOUCH_DELAY);
}

void validateSettings() {
    if (settings == 0xFFFF || SETTINGS_COLD > LEVEL_5 || SETTINGS_WARM > LEVEL_5 || 
    (SETTINGS_COLD != 0 && SETTINGS_WARM != 0 && SETTINGS_COLD != SETTINGS_WARM)) {
        uart_str_transmit("Clearing invalid settings\r\n");
        settings = 0;
    }
}

void updateSettings(uint16_t settings) {
    uint16_t oldValue = eeprom_read_word(EEPROM_SETTINGS_ADDRESS);
    validateSettings();
    eeprom_update_word(EEPROM_SETTINGS_ADDRESS, settings);
    if (settings != oldValue) {
        char msg[50];
        if (DEBUG_ENABLED) sprintf(msg,"Settings written to EEPROM: 0x%04X\r\n", settings);
        uart_str_transmit(msg);
    }
}

uint16_t readSettings() {
    settings = eeprom_read_word(EEPROM_SETTINGS_ADDRESS);
    if (settings == 0xFFFF || SETTINGS_COLD > LEVEL_5 || SETTINGS_WARM > LEVEL_5) {
        uart_str_transmit("Initializing EEPROM\r\n");
        settings = 0;
        updateSettings(settings);
    }
    return settings;
}

uint8_t getInputLevel(uint16_t input) {
    if (input < LEVEL_1_THRESHOLD) {
        return LEVEL_0;
    } else if (input < LEVEL_2_THRESHOLD) {
        return LEVEL_1;
    } else if (input < LEVEL_3_THRESHOLD) {
        return LEVEL_2;
    } else if (input < LEVEL_4_THRESHOLD) {
        return LEVEL_3;
    } else if (input < LEVEL_5_THRESHOLD) {
        return LEVEL_4;
    } else {
        return LEVEL_5;
    }
}

void readInputs() {
    for (int i = 0; i < ANALOG_INPUTS_COUNT; i++) {
        ADMUX = (((1 << REFS1) | (1 << REFS0)) & 0xFF) + selectedInput; // select input pin
        ADCSRA |= (1 << ADSC);  // start conversion
        while (ADCSRA & (1<<ADSC)); //wait for conversion to finish
        uint16_t value = 0;
        value = ADCL;
        value |= (ADCH << 8);
        analogInputs[selectedInput] = value;
        inputs[selectedInput] = getInputLevel(value);
        selectedInput++;
        selectedInput = selectedInput % ANALOG_INPUTS_COUNT;
    }
}

void sendStatus() {
    char msg[5];
    uart_str_transmit("STATUS:");
    itoa(analogInputs[0], msg, 10);
    uart_transmit(*msg);
    uart_transmit(',');
    itoa(analogInputs[1], msg, 10);
    uart_transmit(*msg);
    uart_transmit(',');
    itoa(analogInputs[2], msg, 10);
    uart_transmit(*msg);
    uart_transmit(',');
    itoa(analogInputs[3], msg, 10);
    uart_transmit(*msg);
    uart_transmit(',');
    itoa(analogInputs[4], msg, 10);
    uart_transmit(*msg);
    uart_transmit(',');
    itoa(analogInputs[5], msg, 10);
    uart_transmit(*msg);
    uart_transmit(',');

    itoa(inputs[INPUT_COLD], msg, 10);
    uart_transmit(*msg);
    uart_transmit(',');
    itoa(inputs[INPUT_WARM], msg, 10);
    uart_transmit(*msg);
    uart_transmit(',');

    itoa(SETTINGS_COLD, msg, 10);
    uart_transmit(*msg);
    uart_transmit(',');
    itoa(SETTINGS_WARM, msg, 10);
    uart_transmit(*msg);

    uart_str_transmit("\r\n");

    if (DEBUG_ENABLED) {
        char msg3[120];
        sprintf(msg3,"Buffer status: index: %d, limit: %d, buffer: %s\r\n", bufferIndex, bufferLimit, uartBuffer);
        uart_str_transmit(msg3);
    }
}

void sendLoadedSettings() {
    char msg[50];
    sprintf(msg,"Settings loaded from EEPROM: 0x%04X\r\n", settings);
    uart_str_transmit(msg);
}

void setup() {

    readInputs();
    uint8_t counter = 0;
    _delay_ms(500);
    validateSettings();
    while (POWER_DIFFERS) {
        if (DEBUG_ENABLED) uart_str_transmit("Power status differs\r\n");
        if (DEBUG_ENABLED) sendStatus();
        touchPower();
        readInputs();
        counter++;
        if (counter > 10) soft_reset();
    }
    counter = 0;
    _delay_ms(50);
    while (MODE_DIFFERS) {
        if (DEBUG_ENABLED) uart_str_transmit("Mode differs\r\n");
        if (DEBUG_ENABLED) sendStatus();
        touchMode();
        readInputs();
        counter++;
        if (counter > 10) soft_reset();
    }
    _delay_ms(50);
    counter = 0;
    while (BRIGHTNESS_DIFFERS) {
        if (DEBUG_ENABLED) uart_str_transmit("Brightness differs\r\n");
        if (DEBUG_ENABLED) sendStatus();
        counter = 0;
        while (BRIGHTNESS_LOWER) {
            touchIncrease();
            if (DEBUG_ENABLED) sendStatus();
            readInputs();
            counter++;
            if (counter > 10) soft_reset();
        }
        counter = 0;
        while (BRIGHTNESS_HIGHER) {
            touchDecrease();
            if (DEBUG_ENABLED) sendStatus();
            readInputs();
            counter++;
            if (counter > 10) soft_reset();
        }
    }
}


int main(void) {
	wdt_init(); // disable watchdog
    output_init();
    adc_init();
	//sei();    // turn on interrupts
	uart_init();

    uint32_t counter = 0;
    ADCSRA |= (1 << ADSC);  // start ADC conversion
    readSettings();
    uart_str_transmit("\r\n======= Winner RGB M3A Lamp mod v");
    uart_str_transmit(VERSION);
    uart_str_transmit(" by WarewolfCZ =======\n\r\n");

    uart_str_transmit("Supported commands:\r\n");
    uart_str_transmit("CMD:XY = set light levels, X = cold Y = warm, X and Y in interval <0,5>\r\n");
    uart_str_transmit("PIN:1234 = set Bluetooth PIN\r\n");
    uart_str_transmit("STATUS = request status line\r\n");
    uart_str_transmit("Commands are separated by new line\r\n\r\n");

    if (DEBUG_ENABLED) sendLoadedSettings();
    _delay_ms(50);
    setup();
    uart_str_transmit("Setup complete\r\n");

	while (1) { 
        //_delay_ms(1);
        if (UCSRA & (1<<RXC)) {
            char* data = uart_str_receive();
            if (data != NULL && data[0] != 0x00) {
                if (DEBUG_ENABLED)  {
                    uart_str_transmit("RECEIVED:");
                    uart_str_transmit(data);
                    uart_str_transmit("\r\n");
                }

                if (strncasecmp(data, "SET:", 4) == 0 && strlen(data) >= 6) {
                    uint8_t cold = ((data[4] - '0') % (LEVEL_5 + 1));
                    uint8_t warm = ((data[5] - '0') % (LEVEL_5 + 1));
                    if (cold != warm && cold != 0 && warm != 0) { // invalid combination, set both to same value
                        uart_str_transmit("SET_INVALID:");
                    } else {
                        settings = cold << 8 | warm;
                        uart_str_transmit("SET_OK:");
                    }
                    uart_transmit((unsigned char) cold + '0');
                    uart_transmit((unsigned char) warm + '0');
                    uart_str_transmit("\r\n");
                    setup();
                } else if (strncasecmp(data, "PIN:", 4) == 0 && strlen(data) >= 8) {
                    pin = ((data[4] - '0') % 10) << 8 | ((data[5] - '0') % 10);
                    uart_str_transmit("PIN_OK:");
                    uart_transmit((unsigned char) (pin >> 8) + '0');
                    uart_transmit((unsigned char) ((pin & 0xFF) + '0'));
                    uart_str_transmit("\r\n");
                    setup();
                } else if (strncasecmp(data, "STATUS", 6) == 0 && strlen(data) >= 6) {
                     sendStatus();
                } else {
                    uart_str_transmit("CMD_INVALID\r\n");
                }
            }
        }
        readInputs();
        counter++;
        counter = counter % 25000;
        
        if (MODE_DIFFERS || BRIGHTNESS_DIFFERS) {    
            settings = (((uint16_t) inputs[INPUT_COLD]) << 8) | inputs[INPUT_WARM];
            updateSettings(settings);
        }
        
        if (counter == 0) {
            if (DEBUG_ENABLED) sendStatus();
            updateSettings(settings);
        }
	}
}
