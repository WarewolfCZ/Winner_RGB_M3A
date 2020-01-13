/*
 * WinnerRGBM3AMod.c
 *
 * Created: 25.12.2019 15:36:18
 * Author : WarewolfCZ
 */ 

#define VERSION "1.0.0"

#define F_CPU 8000000UL // 8 MHz
#define BAUD 19200       // define speed
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1) // set baud rate value for UBRR
#define DEFAULT_LED_TIMER_THRESHOLD 40
#define EEPROM_SETTINGS_ADDRESS 0x00
#define EEPROM_SETTINGS_DEBUG 0x04
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

#define TOUCH_DELAY 150

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
//#include <stdio.h>
#include <string.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

uint16_t settings;
uint8_t pin[4];
uint8_t debugMode;
uint8_t selectedInput = 0;
uint16_t analogInputs[ANALOG_INPUTS_COUNT];
uint8_t inputs[ANALOG_INPUTS_COUNT];
uint8_t bufferIndex = 0;
uint8_t bufferLimit = 9;
char uartBuffer[10];


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

/************************************************************************/
/* Send string from program memory                                      */
/************************************************************************/
void uart_prg_str_transmit(const char str[]) {

    uint8_t c;
    while (0 != (c = pgm_read_byte(str++))) {
        uart_transmit(c);
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
    if (debugMode) uart_prg_str_transmit(PSTR("Soft reset\r\n"));
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
    if (debugMode) uart_prg_str_transmit(PSTR("power button touch\r\n"));
    PORTB |= (1 << PINB3);
    _delay_ms(200);
    PORTB &= ~(1 << PINB3);
    _delay_ms(TOUCH_DELAY);
}

void touchMode() {
    if (debugMode) uart_prg_str_transmit(PSTR("mode button touch\r\n"));
    PORTB |= (1 << PINB2);
    _delay_ms(TOUCH_DELAY);
    PORTB &= ~(1 << PINB2);
    _delay_ms(TOUCH_DELAY);
}

void touchIncrease() {
    if (debugMode) uart_prg_str_transmit(PSTR("increase button touch\r\n"));
    PORTB |= (1 << PINB1);
    _delay_ms(TOUCH_DELAY);
    PORTB &= ~(1 << PINB1);
    _delay_ms(TOUCH_DELAY);
}

void touchDecrease() {
    if (debugMode) uart_prg_str_transmit(PSTR("decrease button touch\r\n"));
    PORTB |= (1 << PINB0); // set PB0 to HIGH
    _delay_ms(TOUCH_DELAY);
    PORTB &= ~(1 << PINB0); // set PB0 to LOW
    _delay_ms(TOUCH_DELAY);
}

uint16_t validateSettings() {
    if (settings == 0xFFFF || SETTINGS_COLD > LEVEL_5 || SETTINGS_WARM > LEVEL_5 || 
    (SETTINGS_COLD != 0 && SETTINGS_WARM != 0 && SETTINGS_COLD != SETTINGS_WARM)) {
        if (debugMode) uart_prg_str_transmit(PSTR("Clearing invalid settings\r\n"));
        settings = 0;
    }
    return settings;
}

void updateSettings(uint16_t settings) {
    uint16_t oldValue = eeprom_read_word(EEPROM_SETTINGS_ADDRESS);
    settings = validateSettings();
    eeprom_update_word(EEPROM_SETTINGS_ADDRESS, settings);
    if (settings != oldValue) {
        if (debugMode) {
            uart_prg_str_transmit(PSTR("Settings written to EEPROM: 0x0"));
            uart_transmit(SETTINGS_COLD + '0');
            uart_transmit('0');
            uart_transmit(SETTINGS_WARM + '0');
            uart_prg_str_transmit(PSTR("\r\n"));
        }
    }
}

uint16_t readSettings() {
    settings = eeprom_read_word(EEPROM_SETTINGS_ADDRESS);
    if (settings == 0xFFFF || SETTINGS_COLD > LEVEL_5 || SETTINGS_WARM > LEVEL_5) {
        uart_prg_str_transmit(PSTR("Initializing EEPROM\r\n"));
        settings = 0;
        updateSettings(settings);
    }
    return settings;
}


void sendLoadedSettings() {
    uart_prg_str_transmit(PSTR("Settings loaded from EEPROM: 0x0"));
    uart_transmit(SETTINGS_COLD + '0');
    uart_transmit('0');
    uart_transmit(SETTINGS_WARM + '0');
    uart_prg_str_transmit(PSTR("\r\n"));
}

void updateDebugMode(uint8_t debugMode) {
    if (debugMode > 0) debugMode = 1;
    else debugMode = 0;
    uint8_t oldValue = eeprom_read_byte(EEPROM_SETTINGS_DEBUG);
    eeprom_update_byte(EEPROM_SETTINGS_DEBUG, debugMode);
    if (debugMode != oldValue) {
        uart_prg_str_transmit(PSTR("Debug mode written to EEPROM: 0x"));
        uart_transmit(debugMode + '0');
        uart_prg_str_transmit(PSTR("\r\n"));
    }
}

uint16_t readDebugMode() {
    debugMode = eeprom_read_byte(EEPROM_SETTINGS_DEBUG);
    if (debugMode == 0xFF) {
        uart_prg_str_transmit(PSTR("Initializing debugMode mode value\r\n"));
        debugMode = 0;
        updateDebugMode(debugMode);
    }
    return debugMode;
}

void sendLoadedDebugMode() {
    uart_prg_str_transmit(PSTR("Debug mode loaded from EEPROM: 0x0"));
    uart_transmit(debugMode + '0');
    uart_prg_str_transmit(PSTR("\r\n"));
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
    uart_prg_str_transmit(PSTR("STATUS:"));
    itoa(analogInputs[0], msg, 10);
    uart_str_transmit(msg);
    uart_transmit(',');
    itoa(analogInputs[1], msg, 10);
    uart_str_transmit(msg);
    uart_transmit(',');

    itoa(inputs[INPUT_COLD], msg, 10);
    uart_str_transmit(msg);
    uart_transmit(',');
    itoa(inputs[INPUT_WARM], msg, 10);
    uart_str_transmit(msg);
    uart_transmit(',');

    itoa(SETTINGS_COLD, msg, 10);
    uart_str_transmit(msg);
    uart_transmit(',');
    itoa(SETTINGS_WARM, msg, 10);
    uart_str_transmit(msg);

    uart_prg_str_transmit(PSTR("\r\n"));

    if (debugMode) {
        uart_prg_str_transmit(PSTR("Buffer status: index: "));
        itoa(bufferIndex, msg, 10);
        uart_str_transmit(msg);
        uart_prg_str_transmit(PSTR(", limit: "));
        itoa(bufferLimit, msg, 10);
        uart_str_transmit(msg);
        uart_prg_str_transmit(PSTR(", buffer content: "));
        uart_str_transmit(uartBuffer);
        uart_prg_str_transmit(PSTR("\r\n"));
    }
}


void setup() {

    readInputs();
    uint8_t counter = 0;
    _delay_ms(500);
    validateSettings();
    while (POWER_DIFFERS) {
        if (debugMode) uart_prg_str_transmit(PSTR("Power status differs\r\n"));
        if (debugMode) sendStatus();
        touchPower();
        readInputs();
        counter++;
        if (counter > 10) soft_reset();
    }
    counter = 0;
    _delay_ms(50);
    while (MODE_DIFFERS) {
        if (debugMode) uart_prg_str_transmit(PSTR("Mode differs\r\n"));
        if (debugMode) sendStatus();
        touchMode();
        readInputs();
        counter++;
        if (counter > 10) soft_reset();
    }
    _delay_ms(50);
    counter = 0;
    while (BRIGHTNESS_DIFFERS) {
        if (debugMode) uart_prg_str_transmit(PSTR("Brightness differs\r\n"));
        if (debugMode) sendStatus();
        counter = 0;
        while (BRIGHTNESS_LOWER) {
            touchIncrease();
            if (debugMode) sendStatus();
            readInputs();
            counter++;
            if (counter > 10) soft_reset();
        }
        counter = 0;
        while (BRIGHTNESS_HIGHER) {
            touchDecrease();
            if (debugMode) sendStatus();
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
	uart_init();

    uint16_t statusCounter = 0;
    uint16_t settingsCounter = 0;
    ADCSRA |= (1 << ADSC);  // start ADC conversion
    readSettings();
    readDebugMode();
    uart_prg_str_transmit(PSTR("======= Winner RGB M3A Lamp mod v"));
    uart_prg_str_transmit(PSTR(VERSION));
    uart_prg_str_transmit(PSTR(" by WarewolfCZ =======\r\n"));
    uart_prg_str_transmit(PSTR("Supported commands:\r\n"));
    uart_prg_str_transmit(PSTR("SETXY = set light levels, X = cold Y = warm, X and Y in interval <0,5>\r\n"));
    uart_prg_str_transmit(PSTR("PINXXXX = set Bluetooth PIN\r\n"));
    uart_prg_str_transmit(PSTR("STATUS = request status line\r\n"));
    uart_prg_str_transmit(PSTR("DEBUG = toggle debug mode\r\n"));
    uart_prg_str_transmit(PSTR("Commands are separated by new line\r\n\r\n"));

    if (debugMode) sendLoadedSettings();
    if (debugMode) sendLoadedDebugMode();
    _delay_ms(50);
    setup();
    uart_prg_str_transmit(PSTR("Setup complete\r\n"));

	while (1) { 
        if (UCSRA & (1<<RXC)) {
            char* data = uart_str_receive();
            if (data != NULL && data[0] != 0x00) {
                if (debugMode)  {
                    uart_prg_str_transmit(PSTR("RECEIVED:"));
                    uart_str_transmit(data);
                    uart_prg_str_transmit(PSTR("\r\n"));
                }

                if (strncasecmp(data, "SET", 3) == 0 && strlen(data) >= 5) {
                    uint8_t cold = ((data[3] - '0') % (LEVEL_5 + 1));
                    uint8_t warm = ((data[4] - '0') % (LEVEL_5 + 1));
                    if (cold != warm && cold != 0 && warm != 0) { // invalid combination, set both to same value
                        uart_prg_str_transmit(PSTR("SET_INVALID:"));
                    } else {
                        settings = cold << 8 | warm;
                        uart_prg_str_transmit(PSTR("SET_OK:"));
                    }
                    uart_transmit((unsigned char) cold + '0');
                    uart_transmit((unsigned char) warm + '0');
                    uart_prg_str_transmit(PSTR("\r\n"));
                    updateSettings(settings);
                    setup();
                } else if (strncasecmp(data, "PIN", 3) == 0 && strlen(data) >= 7) {
                    pin[0] = ((data[3] - '0') % 10);
                    pin[1] = ((data[4] - '0') % 10);
                    pin[2] = ((data[5] - '0') % 10);
                    pin[3] = ((data[6] - '0') % 10);
                    uart_prg_str_transmit(PSTR("PIN_OK:"));
                    uart_transmit((unsigned char) (pin[0] + '0'));
                    uart_transmit((unsigned char) (pin[1] + '0'));
                    uart_transmit((unsigned char) (pin[2] + '0'));
                    uart_transmit((unsigned char) (pin[3] + '0'));
                    uart_prg_str_transmit(PSTR("\r\n"));
                    setup();
                } else if (strncasecmp(data, "STATUS", 6) == 0) {
                    sendStatus();
                } else if (strncasecmp(data, "DEBUG", 5) == 0) {
                    if (debugMode > 0) debugMode = 0;
                    else debugMode = 1;
                    uart_prg_str_transmit(PSTR("DEBUG_OK\r\n"));
                    updateDebugMode(debugMode);
                } else {
                    uart_prg_str_transmit(PSTR("CMD_INVALID\r\n"));
                }
            }
        }
        readInputs();
        statusCounter++;
        statusCounter = statusCounter % 9000;
        settingsCounter++;
        settingsCounter = settingsCounter % 1000;
        
        if (MODE_DIFFERS || BRIGHTNESS_DIFFERS) {    
            settings = (((uint16_t) inputs[INPUT_COLD]) << 8) | inputs[INPUT_WARM];
        }
        
        if (debugMode && statusCounter == 0) {
            sendStatus();
        }
        if (settingsCounter == 0) {
            updateSettings(settings);
        }
	}
}
