/*
 * WinnerRGBM3AMod.c
 *
 * Created: 25.12.2019 15:36:18
 * Author : WarewolfCZ
 */ 

#define F_CPU 8000000UL // 8 MHz
#define BAUD 19200       // define baud
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

#define SETTINGS_INPUT_COLD ((uint8_t)(settings >> 8))
#define SETTINGS_INPUT_WARM ((uint8_t)(settings & 0xFF))

#define POWER_DIFFERS ((inputs[INPUT_COLD] == LEVEL_0 && inputs[INPUT_WARM] == LEVEL_0 && (SETTINGS_INPUT_COLD != LEVEL_0 || SETTINGS_INPUT_WARM != LEVEL_0)) || ((inputs[INPUT_COLD] != LEVEL_0 || inputs[INPUT_WARM] != LEVEL_0) && SETTINGS_INPUT_COLD == LEVEL_0 && SETTINGS_INPUT_WARM == LEVEL_0))
#define MODE_DIFFERS (\
 (inputs[INPUT_COLD] == LEVEL_0 && SETTINGS_INPUT_COLD != LEVEL_0) ||\
 (inputs[INPUT_WARM] == LEVEL_0 && SETTINGS_INPUT_WARM != LEVEL_0) ||\
 (inputs[INPUT_COLD] != LEVEL_0 && SETTINGS_INPUT_COLD == LEVEL_0) ||\
 (inputs[INPUT_WARM] != LEVEL_0 && SETTINGS_INPUT_WARM == LEVEL_0)\
)
#define BRIGHTNESS_DIFFERS (inputs[INPUT_COLD] != SETTINGS_INPUT_COLD || inputs[INPUT_WARM] != SETTINGS_INPUT_WARM)

#define COLD_BRIGHTNESS_LOWER (inputs[INPUT_COLD] < SETTINGS_INPUT_COLD)
#define WARM_BRIGHTNESS_LOWER (inputs[INPUT_WARM] < SETTINGS_INPUT_WARM)

#define COLD_BRIGHTNESS_HIGHER (inputs[INPUT_COLD] > SETTINGS_INPUT_COLD)
#define WARM_BRIGHTNESS_HIGHER (inputs[INPUT_WARM] > SETTINGS_INPUT_WARM)

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

uint16_t settings;
uint8_t selectedInput = 0;
uint16_t analogInputs[ANALOG_INPUTS_COUNT];
uint8_t inputs[ANALOG_INPUTS_COUNT];
uint8_t bufferIndex = 0;
uint8_t bufferLimit = 150;
char *uartBuffer;
char lastChar = 0;

void soft_reset() {
	// enable watchdog and let it reset the controller
	wdt_enable(WDTO_15MS);
	for(;;){}
}

// function to initialize UART
void uart_init (void) {
	UBRRH = (BAUDRATE>>8);                      // shift the register right by 8 bits
	UBRRL = BAUDRATE;                           // set baud rate
	UCSRB|= (1<<TXEN)|(1<<RXEN);                // enable receiver and transmitter
	UCSRC|= (1<<URSEL)|(1<<UPM1)|(1<<UCSZ0)|(1<<UCSZ1);   // 8bit data format, even parity
	
	DDRD |= 1 << PIND1;//pin1 of portD as OUTPUT
	DDRD &= ~(1 << PIND0);//pin0 of portD as INPUT
	PORTD |= 1 << PIND0; // turn on the pull-up
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

char* uart_str_receive() {
    if (bufferIndex == 0) {
	    uartBuffer = malloc(bufferLimit * sizeof(char) + 1);
        lastChar = 0;
    }
	if (bufferIndex < bufferLimit && lastChar != '\n' && lastChar != '\r') {
		lastChar = uart_receive();
		uartBuffer[bufferIndex] = lastChar;
        bufferIndex++;
	} else {
        uartBuffer[bufferIndex] = 0x00;
        lastChar = 0;
        bufferIndex = 0;
        return uartBuffer;
    }
	return NULL;
}

void wdt_init(void) {
	MCUSR = 0;
	wdt_disable(); //disable watchdog
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
    uart_str_transmit("power button touch\r\n");
    PORTB |= (1 << PINB3);
    _delay_ms(200);
    PORTB &= ~(1 << PINB3);
    _delay_ms(TOUCH_DELAY);
}

void touchMode() {
    uart_str_transmit("mode button touch\r\n");
    PORTB |= (1 << PINB2);
    _delay_ms(TOUCH_DELAY);
    PORTB &= ~(1 << PINB2);
    _delay_ms(TOUCH_DELAY);
}

void touchIncrease() {
    uart_str_transmit("increase button touch\r\n");
    PORTB |= (1 << PINB1);
    _delay_ms(TOUCH_DELAY);
    PORTB &= ~(1 << PINB1);
    _delay_ms(TOUCH_DELAY);
}

void touchDecrease() {
    uart_str_transmit("decrease button touch\r\n");
    PORTB |= (1 << PINB0); // set PB0 to HIGH
    _delay_ms(TOUCH_DELAY);
    PORTB &= ~(1 << PINB0); // set PB0 to LOW
    _delay_ms(TOUCH_DELAY);
}

void updateSettings(uint16_t settings) {
    uint16_t oldValue = eeprom_read_word(EEPROM_SETTINGS_ADDRESS);
    eeprom_update_word(EEPROM_SETTINGS_ADDRESS, settings);
    if (settings != oldValue) {
        uart_str_transmit("Settings written to EEPROM\r\n");
    }
}

uint16_t readSettings() {
    settings = eeprom_read_word(EEPROM_SETTINGS_ADDRESS);
    if (settings == 0xFFFF || SETTINGS_INPUT_COLD > LEVEL_5 || SETTINGS_INPUT_WARM > LEVEL_5) {
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
        //PORTB |= (1 << PINB3); // LED on
        value = ADCL;
        value |= (ADCH << 8);
        analogInputs[selectedInput] = value;
        inputs[selectedInput] = getInputLevel(value);
        selectedInput++;
        selectedInput = selectedInput % ANALOG_INPUTS_COUNT;
    }
}

void sendStatus() {
    char string[256];
    memset(string, 0, sizeof(string[0])*256); // clear string
    sprintf(string,"0:%d,\t1:%d,\t2:%d,\t3:%d,\t4:%d,\t5:%d\tc:%u\tw:%u\tpwrdiff:%u\tsett: 0x%04X\r\n", analogInputs[0], analogInputs[1], analogInputs[2], analogInputs[3], analogInputs[4], analogInputs[5], inputs[INPUT_COLD], inputs[INPUT_WARM], POWER_DIFFERS, settings);
    uart_str_transmit(string);
}

void sendLoadedSettings() {
    char string[256];
    memset(string, 0, sizeof(string[0])*256); // clear string
    sprintf(string,"Settings loaded from EEPROM: 0x%04X\r\n", settings);
    uart_str_transmit(string);
}

int main(void) {
	wdt_init(); // disable watchdog
    output_init();
    adc_init();
	//sei();    // turn on interrupts
	uart_init();

    char string[256];
    memset(string, 0, sizeof(string[0])*256); // clear string
    uint8_t counter = 0;
    ADCSRA |= (1 << ADSC);  // start ADC conversion
    readSettings();
    uart_str_transmit("Lamp mod starting\r\n");
    sendLoadedSettings();

    readInputs();

    _delay_ms(500);
    while (POWER_DIFFERS) {
        uart_str_transmit("Power status differs\r\n");
        sendStatus();
        touchPower();
        readInputs();
        counter++;
        if (counter > 10) soft_reset();
    }
    counter = 0;
    _delay_ms(50);
    while (MODE_DIFFERS) {
        uart_str_transmit("Mode differs\r\n");
        sendStatus();
        touchMode();
        readInputs();
        counter++;
        if (counter > 10) soft_reset();
    }
    _delay_ms(50);
    counter = 0;
    while (BRIGHTNESS_DIFFERS) {
        uart_str_transmit("Brightness differs\r\n");
        sendStatus();
        counter = 0;
        while (BRIGHTNESS_LOWER) {
            touchIncrease();
            sendStatus();
            readInputs();
            counter++;
            if (counter > 10) soft_reset();
        }
        counter = 0;
        while (BRIGHTNESS_HIGHER) {
            touchDecrease();
            sendStatus();
            readInputs();
            counter++;
            if (counter > 10) soft_reset();
        }
    }
    uart_str_transmit("Lamp mod setup complete\r\n");

	while (1) { 
        _delay_ms(1);
        if (UCSRA & (1<<RXC)) {
            char* data = uart_str_receive();
            if (data != NULL) {
                uart_str_transmit("Received: ");
                uart_str_transmit(data);
                uart_str_transmit("\n");
            }
        }
        readInputs();
        counter++;
        counter = counter % 250;
        
        if (MODE_DIFFERS || BRIGHTNESS_DIFFERS) {    
            sendStatus();
            settings = (((uint16_t) inputs[INPUT_COLD]) << 8) | inputs[INPUT_WARM];
        }

        if (counter == 0) {
            sendStatus();
            updateSettings(settings);
           // PORTB &= ~(1 << PINB3); // LED off
        }
	}
}
