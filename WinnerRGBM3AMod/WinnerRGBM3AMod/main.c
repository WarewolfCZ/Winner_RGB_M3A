/*
 * WinnerRGBM3AMod.c
 *
 * Created: 25.12.2019 15:36:18
 * Author : WarewolfCZ
 */ 

#define F_CPU 16000000UL // 8 MHz
#define BAUD 19200       // define baud
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1) // set baud rate value for UBRR
#define DEFAULT_LED_TIMER_THRESHOLD 40

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <stdbool.h>

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

char* uart_str_receive(int limit) {
	char ch;
	char *result = malloc(limit * sizeof(char) + 1);
	int i;
	for (i = 0; i < limit && ch != '\n'; i++) {
		ch = uart_receive();
		result[i] = ch;
	}
	result[i] = 0x00;
	return result;
}

void wdt_init(void) {
	MCUSR = 0;
	wdt_disable(); //disable watchdog
}

void input_init() {
	PORTD |= (1 << PIND2);    // turn On the Pull-up
	PORTD |= (1 << PIND3);    // turn On the Pull-up
}

void adc_init() {
    ADCSRA |= (1 << ADEN); // enable ADC
    ADMUX &= ~((1 << REFS1) | (1 << REFS0)); // select voltage reference - AREF
    ADMUX |= 0; // select pin ADC0 as analog input
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1); // use a prescaler of 64
}

void touch(bool outputs[4]) {
    if (outputs[0]) {
        outputs[0] = false;
        uart_str_transmit("button1 touch\r\n");
        PORTD |= (1 << PIND4); // set PD4 to HIGH
        _delay_ms(200);
        PORTD &= ~(1 << PIND4); // set PD4 to LOW
    }
    if (outputs[1]) {
        outputs[1] = false;
        uart_str_transmit("button2 touch\r\n");
        PORTD |= (1 << PIND5);
        _delay_ms(200);
        PORTD &= ~(1 << PIND5);
    }
    if (outputs[2]) {
        outputs[2] = false;
        uart_str_transmit("button3 touch\r\n");
        PORTD |= (1 << PIND6);
        _delay_ms(200);
        PORTD &= ~(1 << PIND6);
    }
    if (outputs[3]) {
        outputs[3] = false;
        uart_str_transmit("button4 touch\r\n");
        PORTD |= (1 << PIND7);
        _delay_ms(200);
        PORTD &= ~(1 << PIND7);
    }
}

int main(void) {
	wdt_init(); // important
    DDRD |= 1 << PIND4; // PIN4 of portD as OUTPUT
    DDRD |= 1 << PIND5; // PIN5 of portD as OUTPUT
	DDRD |= 1 << PIND6; // PIN6 of portD as OUTPUT
	DDRD |= 1 << PIND7; // PIN7 of portD as OUTPUT
    PORTD &= ~(1 << PIND4); // set PD4 to LOW
    PORTD &= ~(1 << PIND5); // set PD5 to LOW
	PORTD &= ~(1 << PIND6); // set PD6 to LOW
	PORTD &= ~(1 << PIND7); // set PD7 to LOW
	
	input_init();
    adc_init();
	//sei();                    // turn on interrupts
	uart_init();
	uart_str_transmit("Lamp mod Initialized\r\n");
    uint16_t value = 0;
    uint8_t adcInput = 0;
    uint16_t inputs[6];
    bool outputs[4];
    outputs[0] = false;
    outputs[1] = false;
    outputs[2] = false;
    outputs[3] = false;
    uint8_t counter = 0;
    char string[256];
    ADCSRA |= (1 << ADSC);  // start ADC conversion
	while (1) {
        
        _delay_ms(1);
        if ((ADCSRA & (1 << ADSC)) == 0) { // conversion complete
            //PORTD |= (1 << PIND7); // LED on
            value = ADCL;
            value |= (ADCH<<8);
            inputs[adcInput] = value;
            adcInput++;
            adcInput = adcInput % 6;
            ADMUX = (((1 << REFS1) | (1 << REFS0)) & 0xFF) + adcInput; // select next pin 
            ADCSRA |= (1 << ADSC);  // start conversion
            
        }
        counter++;
        counter = counter % 100;
        if (adcInput == 0) {
            if (inputs[0] < 50) {
                outputs[0] = true;
            }
            if (inputs[1] < 50) {
                outputs[1] = true;
            }
            if (inputs[2] < 50) {
                outputs[2] = true;
            }
            if (inputs[3] < 50) {
                outputs[3] = true;
            }
            touch(outputs);
        }

        if (counter == 0) {
            memset(string, 0, sizeof(string[0])*256); // clear string
            sprintf(string,"0:%d,\t1:%d,\t2:%d,\t3:%d,\t4:%d,\t5:%d\r\n", inputs[0], inputs[1], inputs[2], inputs[3], inputs[4], inputs[5]);
            uart_str_transmit(string);
           // PORTD &= ~(1 << PIND7); // LED off
        }

        
	}
}
