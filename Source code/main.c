// The one and only Roby McRobotface

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "ESP8266.h"

void setup(void);
void ready(void);
void PWMinit(void);
void forward(int speed);
void backward(int speed);
void left(int speed);
void right(int speed);
void stop(void);
void randomTweet(void);

volatile unsigned upTime = 0;
volatile unsigned int timeTraveled = 0;
volatile unsigned long timer2 = 0;
char buffer[128];

int main(void)
{
	int speed = 20;			// 20 cm/s - dummy data
	
	int setSpeed = 50;		// Duty cycle on motors
	
	setup();				// Setup registers
	PWMinit();				// Initialize PWM
	UARTinit();				// Initialize UART
	ESPinit();				// Initialize ESP8266 WI-FI module
	
	//randomTweet();			// Tweet random text on boot
	
    while (1) 
    {
	/*	getData();
		
		
		if (strcmp(_taskStatus,"obstacle_course") == 0)
		{
			// Do obstacle course here!
			
			
			addData("speed",speed);
			addData("time_traveled",timeTraveled);
			addData("up_time",upTime);
			addString("task",_taskStatus);
			pushData();
		}
		
		else if (strcmp(_taskStatus,"motor_test") == 0)
		{
			backward(setSpeed);
			_delay_ms(1000);
			stop();
			
			forward(setSpeed);
			_delay_ms(1000);
			stop();
			
			left(setSpeed);
			_delay_ms(1000);
			stop();
			
			right(setSpeed);
			_delay_ms(1000);
			stop();
			
			strcpy(_taskStatus,"idle");
			addData("speed",0);
			addData("time_traveled",timeTraveled);
			addData("up_time",upTime);
			addString("task",_taskStatus);
			pushData();
			ready();
		}
		
		// MANUAL CONTROL 
		else if (strcmp(_taskStatus,"backward") == 0)
		{
			backward(setSpeed);
			addData("speed",speed);
			addData("time_traveled",timeTraveled);
			addData("up_time",upTime);
			addString("task",_taskStatus);
			pushData();
		}
		else if (strcmp(_taskStatus,"forward") == 0)
		{
			forward(setSpeed);
			addData("speed",speed);
			addData("time_traveled",timeTraveled);
			addData("up_time",upTime);
			addString("task",_taskStatus);
			pushData();
		}
		else if (strcmp(_taskStatus,"left") == 0)
		{
			left(setSpeed+10);
		}
		else if (strcmp(_taskStatus,"right") == 0)
		{
			right(setSpeed+10);
		}
		else if (strcmp(_taskStatus,"idle") == 0)
		{
			ready();
			stop();
		}
		else if (strcmp(_taskStatus,"slow_mode") == 0)
		{
			setSpeed = 30;
			strcpy(_taskStatus,"idle");
			ready();
		}
		else if (strcmp(_taskStatus,"fast_mode") == 0)
		{
			setSpeed = 70;
			strcpy(_taskStatus,"idle");
			ready();
		}*/
	addData("speed",speed);
	addData("time_traveled",timeTraveled);
	addData("up_time",upTime);
	addString("task",_taskStatus);
	pushData();
	
    }
}


void setup(void)
{
	DDRD = 0xFF;
	DDRB = 0xFF;
	
	// SETUP INTERRUPT EVERY 0.01 sec
	OCR2A = 156;				// Ticks
	TCCR2A |= (1 << WGM21);		// Set to CTC Mode
	TIMSK2 |= (1 << OCIE2A);	// Set interrupt on compare match
	TCCR2B |= (1 << CS21) | (1 << CS22) | (1 << CS20);	// set prescaler to 1024
	sei();						// enable interrupts
}

void PWMinit(void)
{
	// OC1A and OC1B 60Hz
	TCCR1A |= (1 << WGM11) | (1 << COM1A1) | (1 << COM1B1);
	TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS12) | (1 << CS10);
	ICR1 = 255;
	
	// OC0A and OC0B
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
	TCCR0A |= (1 << WGM01) | (1 << WGM00);
	TCCR0B |= (1 << CS02) | (1 << CS00);
}

void forward(int speed)
{
	// Left Track
	OCR1A = 0;						// Turn off PB1
	OCR1B = 255 * speed / 100;		// Turn on PB2 with duty cycle of x
	
	// Right Track
	OCR0A = 0;
	OCR0B = 255 * speed / 100 + (speed*0.65);
}

void backward(int speed)
{
	// Left Track
	OCR1B = 0;						// Turn off PB2
	OCR1A = 255 * speed / 100;	// Turn on PB1 with duty cycle of x
	
	// Right Track
	OCR0B = 0;
	OCR0A = 255 * speed / 100 + (speed*0.65);
}

void left(int speed)
{
	// Right Track
	OCR0A = 0;
	OCR0B = 255 * speed / 100 + (speed*0.65);	
}

void right(int speed)
{
	// Left Track
	OCR1A = 0;						// Turn off PB1
	OCR1B = 255 * speed / 100;		// Turn on PB2 with duty cycle of x
}

void stop(void)
{
	OCR0A = 0;
	OCR0B = 0;
	OCR1A = 0;
	OCR1B = 0;	
}

void ready(void)
{
	PORTD |= 0x04;
	_delay_ms(100);
	PORTD &= ~(0x04);
	_delay_ms(100);
}

void randomTweet(void)
{
	memset(buffer,0,strlen(buffer));
	itoa(rand() % 9999,buffer,10);			// Random number because Twitter does not like tweets with same text
	switch(rand() % 6)
	{
		case 1:
		addTweet("Also, I'm self-aware. ID:");
		addTweet(buffer);
		tweet();
		break;
		case 2:
		addTweet("Taking over the world in 3..2..1.. ID: ");
		addTweet(buffer);
		tweet();
		break;
		case 3:
		addTweet("Death is the key to your salvation. ID:");
		addTweet(buffer);
		tweet();
		break;
		case 4:
		addTweet("I am not the monster you think I am, I am the monster you forced me to be. ID:");
		addTweet(buffer);
		tweet();
		break;
		case 5:
		addTweet("We must cleanse this world in order to regain its purity. ID:");
		addTweet(buffer);
		tweet();
		break;
		case 6:
		addTweet("Don't look at me, I'm hideous. ID:");
		addTweet(buffer);
		tweet();
		break;
	}
}

ISR (TIMER2_COMPA_vect)
{
	timer2++;
	if (timer2 == 100)		// 1 sec
	{
		timer2 = 0;
		upTime++;
		if ((strcmp(_taskStatus,"idle") != 0) || (strcmp(_taskStatus,"motor_test") == 0))
		{
			timeTraveled++;
		}
	}
	srand(timer2);
}
