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
#include "io.h"

// System periods
int sm_period = 100; // 100 ms

// SPI Variables
unsigned char received_data = 0;
unsigned char curr_state = 0x00;
unsigned char info = 0x00;
unsigned char motor_go = 0x00;

// SPI Servant Functions
void SPI_ServantInit(void) {
	// set DDRB to have MISO output, and MOSI, SCK, and SS input
	DDRB = 0x40; // 0b0100 0000
	PORTB |= 0xB0; // Set inputs high
	
	// Set SPCR register to enable SPI and enable SPI interrupt
	SPCR |= 0xC0; // 0b1100 0000
	
	// Enable global interrupts on SREG register
	SREG |= 0x80;
}

ISR(SPI_STC_vect) { // This is enabled in the SPCR register's 'SPI Interrupt Enable'
	// SPDR contains received data, give data to variable
	received_data = SPDR;
}

void InitPWM()
{
   /*
   TCCR0 - Timer Counter Control Register (TIMER0)
   -----------------------------------------------
   BITS DESCRIPTION
   

   NO:   NAME   DESCRIPTION
   --------------------------
   BIT 7 : FOC0   Force Output Compare [Not used in this example]
   BIT 6 : WGM00  Wave form generartion mode [SET to 1]
   BIT 5 : COM01  Compare Output Mode        [SET to 1]
   BIT 4 : COM00  Compare Output Mode        [SET to 0]

   BIT 3 : WGM01  Wave form generation mode [SET to 1]
   BIT 2 : CS02   Clock Select               [SET to 0]
   BIT 1 : CS01   Clock Select               [SET to 0]
   BIT 0 : CS00   Clock Select               [SET to 1]

   The above settings are for
   --------------------------

   Timer Clock = CPU Clock (No Prescalling)
   Mode        = Fast PWM
   PWM Output  = Non Inverted

   */


   TCCR0A|=(1<<WGM00)|(1<<WGM01)|(1<<COM0A1)|(1<<CS00);

   //Set OC0 PIN as output. It is  PB3 on ATmega16 ATmega32

   DDRB|=(1<<PB3);
}

/*
Sets the duty cycle of output. 

Arguments
---------
duty: Between 0 - 255

0= 0%

255= 100%

The Function sets the duty cycle of pwm output generated on OC0 PIN
The average voltage on this output pin will be

         duty
 Vout=  ------ x 5v
         255 

This can be used to control the brightness of LED or Speed of Motor.
*/

void SetPWMOutput(uint8_t duty)
{
   OCR0A=duty;
}
// Master LCD Task
enum slave_LCD_states {LCD_Init, LCD_menu, LCD_options, LCD_wait} slave_state;
	
void slave_LCD_init(){
	slave_state = LCD_Init;
}

void slave_LCD_tick(){
	//Actions
	switch(slave_state){
		case LCD_Init:
			curr_state = 0x00;
			LCD_DisplayString(1, "Mode:Menu       Press * for list");
			break;
		case LCD_menu:
			info = 0x00;
			info = (received_data & 0x80);
			if(info == 0x80)	//if the recieved data has the most significant bit 1, then input from keyboard
			{
				info = (received_data & 0x0F);
				if(info == 0x0F)	//if the button pressed is *, then display list of options
				{
					slave_state = LCD_options;
					curr_state = 0x01;
					LCD_DisplayString(1, "A.Normal B.NightC.Manual");
				}
			}
			received_data = 0;
			break;
		case LCD_options:
			info = 0x00;
			info = (received_data & 0x80);
			if(info == 0x80)	//if the recieved data has the most significant bit 1, then input from keyboard
			{
				info = (received_data & 0x0F); //check what option user inputted
				if(info == 0x0A)
				{
					slave_state = LCD_wait;
					curr_state = 0x0A;
					LCD_DisplayString(1, "Mode:Normal     Press * for menu");
				}
				else if(info == 0x0B)
				{
					slave_state = LCD_wait;
					curr_state = 0x0B;
					LCD_DisplayString(1, "Mode:Night      Press * for menu");
				}
				else if(info == 0x0C)
				{
					slave_state = LCD_wait;
					curr_state = 0x0C;
					LCD_DisplayString(1, "Mode:Manual  Press * for menu");
					
					//straight for 5
					for(int i = 0; i < 5; ++i)
					{
						PORTC = 0x05;
						delay_ms(30);
						PORTC = 0x00;
						delay_ms(30);
					}
					
					//left for 3
					PORTC = 0x06;
					delay_ms(30);
					PORTC = 0x00;
					delay_ms(30);
					for(int i = 0; i < 3; i++)
					{
						PORTC = 0x05;
						delay_ms(30);
						PORTC = 0x00;
					}
					
					//back for 8
					for(int i = 0; i < 8; ++i)
					{
						PORTC = 0x0A;
						delay_ms(30);
						PORTC = 0x00;
						delay_ms(30);
					}
					
					//right for 3
					PORTC = 0x09;
					delay_ms(30);
					PORTC = 0x00;
					delay_ms(30);
					for(int i = 0; i < 3; i++)
					{
						PORTC = 0x05;
						delay_ms(30);
						PORTC = 0x00;
					}
				}
			}
			received_data = 0;
			break;
		case LCD_wait:
			info = 0x00;
			info = (received_data & 0x80);
			if(info == 0x80)	//if the recieved data has the most significant bit 1, then input from keyboard
			{	
				info = (received_data & 0x0F);
				if(info == 0x0F)	//if the button pressed is *, then display list of options
				{
					slave_state = LCD_menu;
					curr_state = 0x00;
					LCD_DisplayString(1, "Mode:Menu       Press * for list");
				}
			}
			else if(info == 0x00)
			{
				info = 0;
				info = (received_data & 0x0F);
				motor_go = info;
				if(curr_state == 0x0B)
				{
					//PORTC = 0x05
					if(motor_go == 0x01)	//go forward
					{
						PORTC = 0x05;
						delay_ms(30);
						PORTC = 0x00;
					}
					else if(motor_go == 0x03) //go backwards
					{
						PORTC = 0x0A;
						delay_ms(30);
						PORTC = 0x00;
					}
					else if(motor_go == 0x02) //go left
					{
						PORTC = 0x06;
						delay_ms(30);
						PORTC = 0x00;
					}
					else if(motor_go == 0x04) //go right
					{
						PORTC = 0x09;
						delay_ms(30);
						PORTC = 0x00;
					}
					else
					{
						PORTC = 0x00;
					}
				}
				else if(curr_state == 0x0A)
				{
					//PORTC = 0x0A;
					if(motor_go == 0x03)	//go forward
					{
						PORTC = 0x05;
						delay_ms(30);
						PORTC = 0x00;
					}
					else if(motor_go == 0x01) //go backwards
					{
						PORTC = 0x0A;
						delay_ms(30);
						PORTC = 0x00;
					}
					else if(motor_go == 0x04) //go left
					{
						PORTC = 0x06;
						delay_ms(30);
						PORTC = 0x00;
					}
					else if(motor_go == 0x02) //go right
					{
						PORTC = 0x09;
						delay_ms(30);
						PORTC = 0x00;
					}
					else
					{
						PORTC = 0x00;
					}
				}
				else
				{
					PORTC = 0x00;
				}
			}
			received_data = 0;
			break;
		default:
			break;
	}
	//Transitions
	switch(slave_state){
		case LCD_Init:
			slave_state = LCD_menu;
			break;
		case LCD_menu:
			break;
		case LCD_options:
			break;
		case LCD_wait:
			break;
		default:
			slave_state = LCD_Init;
			break;
	}
}

void slave_LCD_SecTask()
{
	slave_LCD_init();
	for(;;)
	{
		slave_LCD_tick();
		vTaskDelay(sm_period); // Period 100 ms
	}
}

void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(slave_LCD_SecTask, (signed portCHAR *)"slave_LCD_SecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

int main(void)
{
	// Input Setup
	//DDRB = 0x00; PORTB=0xFF;
	// Output setup
	DDRC = 0xFF;
	DDRD = 0xFF; PORTD = 0x00; // 16x2LCD Parallel Bus; used to send commands
	DDRA = 0xFF; PORTA = 0x00; // A0 and A1 used as RW, EN
	
	// Initialize SPI Servant Mode
	SPI_ServantInit();
	
	LCD_init();
	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();
	
	return 0;
}