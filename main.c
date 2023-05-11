/*
 * Sequencer Code.c
 *
 * Created: 4/4/2023 11:44:11 PM
 * Author : angel
 */ 

char String[25];
#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)

#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <math.h>

float ovfNum = 0;
int channel = 1;
float floatADC = 0.0;
int swCount = 122; //244counts/sec --> switch every 0.5s (**CHANGE THIS TO CHANGE THE SPEED OF CYCLING)

double logWithBase(double base, double x);
double equCalc(double x);
double harmonicChoose();


void UART_init(void){
	//Set Baud rate
	UBRR0H = (unsigned char)(BAUD_PRESCALER>>8);
	UBRR0L = (unsigned char)BAUD_PRESCALER;
	
	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	
	//Set frame format: 2 stop bits, 8 data bits
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); //8 data bits
	UCSR0C |= (1<<USBS0); //2 stop bits
}

void UART_send(unsigned char data) {
	//while for empty transmit buffer
	while(!(UCSR0A & (1<<UDRE0)));
	//put data into buffer and send data
	UDR0 = data;
}

void UART_putstring(char* StringPtr) {
	while(*StringPtr != 0x00){
		UART_send(*StringPtr);
		StringPtr++;
	}
}

void Initialize() {
	// Disable global interrupts
	cli();
	
	// Set PD6 to output (base freq)
	DDRD |= (1<<DDD6);
	
	//set PB3 to output (user freq)
	DDRB |= (1<<DDB3);
	
	//Timer0 SETUP (base freq)
	//Set Timer 0 clock to be internal div by 1024
	//15.625kHz timer clock, 1 tick = (1 / 15.625k) sec
	TCCR0B &= ~(1<<CS00);
	TCCR0B &= ~(1<<CS01);
	TCCR0B |= (1<<CS02);
	//Set timer 0 to Fast PWM
	TCCR0A |= (1<<WGM00);
	TCCR0A |= (1<<WGM01);
	TCCR0B |= (1<<WGM02);
	//toggle OC0A on compare match
	TCCR0A |= (1<<COM0A0);
	
	//Timer2 SETUP (user freq)
	//Set Timer 2 clock to be internal div by 1024
	//15.625kHz timer clock, 1 tick = (1 / 15.625k) sec
	TCCR2B &= ~(1<<CS20);
	TCCR2B |= (1<<CS21);
	TCCR2B |= (1<<CS22);
	//Set timer 2 to Fast PWM
	TCCR2A |= (1<<WGM20);
	TCCR2A |= (1<<WGM21);
	TCCR2B |= (1<<WGM22);
	//toggle OC0A on compare match
	TCCR2A |= (1<<COM2A0);
		
	//Timer1 SETUP (sequencer)
	//not prescale, equals internal 16MHz
	TCCR1B |= (1<<CS10);
	TCCR1B &= ~(1<<CS11);
	TCCR1B &= ~(1<<CS12);
	//overflow interrupt
	TIMSK1 |= (1<<TOIE1);
	
	// Set PB0 to input (user input, least sig bit)
	DDRB &= ~(1<<DDB0);
	//Set PB1 to input (user input, most sig bit)
	DDRB &= ~(1<<DDB1);
	
	//SETUP FOR ADC
	//clear power reduction for ADC
	PRR &= ~(1<<PRADC);
	//Select Vref = AVcc
	ADMUX |= (1<<REFS0);
	ADMUX &= ~(1<<REFS1);
	//set the ADC CLock div by 128
	//16M/1288=125kHz
	ADCSRA |= (1<<ADPS0);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS2);
	//select channel 0
	ADMUX &= ~(1<<MUX0);
	ADMUX &= ~(1<<MUX1);
	ADMUX &= ~(1<<MUX2);
	ADMUX &= ~(1<<MUX3);
	//set to auto trigger
	ADCSRA |= (1<<ADATE);
	//set to free running
	ADCSRB &= ~(1<<ADTS0);
	ADCSRB &= ~(1<<ADTS1);
	ADCSRB &= ~(1<<ADTS2);
	//disable digital input buffer on ADC pin
	DIDR0 |= (1<<ADC0D);
	//enable ADC
	ADCSRA |= (1<<ADEN);
	//start conversion
	ADCSRA |= (1<<ADSC);
	
	sei();
}

//SEQUENCER: overflow counter which handles cycling through A0 to A3
ISR(TIMER1_OVF_vect) {
	ovfNum++; //(16M / 2^16 bits) = 244.1 counts/sec 
	if(ovfNum >= swCount) {
		ovfNum = 0;
		contF();
		logFU();
		if (channel == 0) {
			channel++;
			//A0
			ADMUX &= ~(1<<MUX0);
			ADMUX &= ~(1<<MUX1);
			ADMUX &= ~(1<<MUX2);
			ADMUX &= ~(1<<MUX3);
		} else if (channel == 1) {
			channel++;
			//A1
			ADMUX |= (1<<MUX0);
			ADMUX &= ~(1<<MUX1);
			ADMUX &= ~(1<<MUX2);
			ADMUX &= ~(1<<MUX3);
		} else if (channel == 2) {
			channel++;
			//A2
			ADMUX &= ~(1<<MUX0);
			ADMUX |= (1<<MUX1);
			ADMUX &= ~(1<<MUX2);
			ADMUX &= ~(1<<MUX3);
		} else {
			channel = 0;
			//A3
			ADMUX |= (1<<MUX0);
			ADMUX |= (1<<MUX1);
			ADMUX &= ~(1<<MUX2);
			ADMUX &= ~(1<<MUX3);
		}	
	}
}

double contOCR0A() {
	floatADC = ADC * 1.0;
	//ADC range -- 0 to 1023
	//desired range -- 41 to 255
	//base = 1.03
	return logWithBase(1.034, (double) (floatADC + 1.0)) + 46;
}

double customCont(double scaler) {
	double old = contOCR0A();
	double equed = equCalc(old);
	double scaled = equed * scaler;
	return equCalc(scaled);
}

double equCalc(double x) {
	double a = 30530.173;
	double b = -1;
	return a * pow(x, b);
}

double logWithBase(double base, double x) {
	return log(x) / log(base);
}

void contF() {
	OCR0A = contOCR0A();
	//y = ax^b
	//a = 30530.173
	//b = -0.9969
	//x = OCR0A
	//
	//OCR0A = 42; for freq = 726Hz
	
}

double harmonicChoose() {
	if (~PINB & (1<<PINB1) && ~PINB & (1<<PINB0)) {
		return 1;
	} else if (~PINB & (1<<PINB1) && PINB & (1<<PINB0)) {
		return 1.2;
	} else if (PINB & (1<<PINB1) && ~PINB & (1<<PINB0)) {
		return 1.5;
	} else { //else if (PINB & (1<<PINB1) && PINB & (1<<PINB0))
		return 2;
	}
}

void logFU() {
	//double real = contOCR0A();
	//OCR2A = real * (3/2);
	//1.5 = fifth
	//1.2 = m3
	//2 = octave
	OCR2A = customCont(harmonicChoose());
}

int main(void)
{
	UART_init();
	Initialize();

    while (1) 
    {
    }
}