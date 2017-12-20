#ifndef F_CPU
#define F_CPU 16000000UL		// Use 16MHz as default
#endif

#include <stdio.h>
#include <avr/io.h>
#include "uart.h"

#ifndef BAUD
#define BAUD 9600				// Default BAUD rate 9600
#endif

#define MYUBRR (((F_CPU / (BAUD * 16UL))) - 1)

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE); //redirect uart_putchar() to stdio
static FILE mystdin = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ); //redirect uart_getchar() to stdio

void UARTinit (void)
{
    //USART Baud rate: 9600, 8data, 1stop
    UBRR0H = MYUBRR >> 8;
    UBRR0L = MYUBRR;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    
    stdout = &mystdout; //required by printf
    stdin  = &mystdin; //required by scanf
}

//redirect stdout to UART
int uart_putchar(char c, FILE *stream) {	
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

//redirect stdin to UART
int uart_getchar(FILE *stream) {
	loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
	return UDR0;
}
