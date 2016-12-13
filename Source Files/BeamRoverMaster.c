#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/portpins.h>
#include <avr/pgmspace.h>

//FreeRTOS include files
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
// Keypad Functions
#include "keypad.h"
// 16x2 LCD Functions
#include "io.h"


#define		SS1			0x01
#define		SS1_LOW		0xEF
#define		SS1_HIGH	0x10

// System periods
int sm_period = 100; // 100 ms

//Transmitted Data
unsigned char data = 0x00;

//variables for use
unsigned char button_press = 0x00;
unsigned char curr_state = 0x00;
unsigned char greatest_sensor = 0;
uint16_t greatest_reading = 0;
uint16_t reading1 = 0;
uint16_t reading2 = 0;
uint16_t reading3 = 0;
uint16_t reading4 = 0;
uint16_t cal_reading1 = 0;
uint16_t cal_reading2 = 0;
uint16_t cal_reading3 = 0;
uint16_t cal_reading4 = 0;
unsigned char total = 0;


void adc_init()
{
	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

void SPI_MasterInit(void) {
	// Set DDRB to have MOSI, SCK, and SS as outputs; MISO as keypad_input
	DDRB = 0xB0; // 0b1011 0000
	PORTB |= 0x40;  // set keypad_input high
	
	// Set SPCR register to enable SPI, enable master, and use SCK frequency of fosc/16
	SPCR = 0x51; // 0b0101 0001
	
	// Make sure to enable global interrupts on SREG register
	SREG |= 0x80; // 0b1000 0000
}


uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with ’7′ will always keep the value
	// of ‘ch’ between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
	
	// start single convertion
	// write ’1′ to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes ’0′ again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}

void SPI_MasterTransmit(unsigned char cData)
{
	// Give SPDR data to be transmitted
	SPDR = cData;
	
	// Set SS low ie pin B4
	PORTB &= 0xEF;
	
	while(!(SPSR & (1<<SPIF))); // Wait for transmission to complete
	
	// Set SS high
	PORTB |= 0x10;
}


// Master LED Task
enum master_keypad_state {keypad_init, keypad_input} keypad_state;

void master_keypad_init(){
	keypad_state = keypad_init;
}

void master_keypad_tick(){
	//Actions
	switch(keypad_state){
		case keypad_init:
			curr_state = 0x00;
			total = 0;
			cal_reading1 = adc_read(0);
			cal_reading2 = adc_read(1);
			cal_reading3 = adc_read(2);
			cal_reading4 = adc_read(3);
			break;
		case keypad_input:
			button_press = GetKeypadKey();
			if( button_press != '\0')
			{ // if button pressed, returns non '\0''
				while(GetKeypadKey() != '\0'); // Wait to release button
				if(curr_state == 0x00) //if in main menu
				{
					if(button_press == '*')
					{
						data = 0x8F;
						curr_state = 0x01;
						SPI_MasterTransmit(data);
					}
				}
				else if(curr_state == 0x01)
				{
					if(button_press == 'A')
					{
						data = 0x8A;
						curr_state = 0x0A;
						SPI_MasterTransmit(data);
					}
					else if(button_press == 'B')
					{
						data = 0x8B;
						curr_state = 0x0B;
						SPI_MasterTransmit(data);
					}
					else if(button_press == 'C')
					{
						data = 0x8C;
						curr_state = 0x0C;
						SPI_MasterTransmit(data);
					}
				}
				else if(curr_state == 0x0A || curr_state == 0x0B || curr_state == 0x0C)
				{
					if(button_press == '*')
					{
						data = 0x8F;
						curr_state = 0x00;
						SPI_MasterTransmit(data);
					}
				}
			}
			else if(curr_state == 0x0A || curr_state == 0x0B)
			{
				greatest_reading = 2000;
				greatest_sensor = 0x00;
				reading1 = 0;
				reading2 = 0;
				reading3 = 0;
				reading4 = 0;
				
				reading1 = adc_read(0);
				reading2 = adc_read(1);
				reading3 = adc_read(2);
				reading4 = adc_read(3);
				
				if((reading1 < reading2) && (reading1 < cal_reading1))
				{
					greatest_reading = reading1;
					greatest_sensor = 0x01;
				}
				else if((reading2 < reading1) && (reading2 < cal_reading2))
				{
					greatest_reading = reading2;
					greatest_sensor = 0x02;
				}
				
				if((reading3 < greatest_reading) && (reading3 < cal_reading3))
				{
					greatest_reading = reading3;
					greatest_sensor = 0x03;
				}
				
				if((reading4 < greatest_reading) && (reading4 < cal_reading4))
				{
					greatest_reading = reading4;
					greatest_sensor = 0x04;
				}
				
				if(greatest_reading > 0)
				{
					data = greatest_sensor;
					SPI_MasterTransmit(data);
					if(greatest_sensor == 0x01)
					{
						PORTD = 0x01;
					}
					else if(greatest_sensor == 0x02)
					{
						PORTD = 0x02;
					}
					else if(greatest_sensor == 0x03)
					{
						PORTD = 0x03;
					}
					else if(greatest_sensor == 0x04)
					{
						PORTD = 0x04;
					}
				}
			}
			break;
		default:
			//data = 0x00;
			break;
	}
	//Transitions
	switch(keypad_state){
		case keypad_init:
			keypad_state = keypad_input;
			break;
		case keypad_input:
			break;
		default:
			keypad_state = keypad_init;
			break;
	}
}

void master_keypad_SecTask()
{
	master_keypad_init();
	for(;;)
	{
		master_keypad_tick();
		vTaskDelay(sm_period); // Period 100 ms
	}
}

void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(master_keypad_SecTask, (signed portCHAR *)"master_keypad_SecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}


int main(void)
{
	// Key Pad Setup, pin 1 close to star, pin 8 close to D
	DDRC = 0xF0; PORTC = 0x0F; // pin1 -> C4, pin2 -> C5, pin3 -> C6, pin4 -> C7, pin5 -> C0, pin6 -> C1, pin7 -> C2, pin8 -> C3
	DDRD = 0xFF; PORTD = 0x00;
	// Initialize Master SPI
	SPI_MasterInit();
	adc_init();
	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();
	
	return 0;
}


