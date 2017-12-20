#ifndef STDIO_UART_H_
#define STDIO_UART_H_

void UARTinit(void);      // initializes IO - call in main!
int uart_putchar(char c, FILE *stream); //UART to stdio (output) function
int uart_getchar(FILE *stream); //UART to stdio (input) function

#endif /* STDIO_UART_H_ */