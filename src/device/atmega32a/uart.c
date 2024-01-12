#include "f_cpu.h"

#include <sx2flash/uart.h>

#include <avr/io.h>

#ifndef BAUD_RATE
#  define BAUD_RATE 9600
#elif BAUD_RATE <= 0
#  error BAUD_RATE must be positive
#endif

#define UBRR_VALUE ((F_CPU + 4l * BAUD_RATE) / (8l * BAUD_RATE) - 1)
#if UBRR_VALUE < 0 || UBRR_VALUE > 32767
#  error The chosen pair (F_CPU,BAUD_RATE) is not supported
#endif

void uart_init(void) {
    UCSRA = _BV(U2X);
    UCSRB = _BV(RXEN) | _BV(TXEN);
    UBRRH = UBRR_VALUE >> 8;
    UBRRL = UBRR_VALUE & 0xFF;
}

void uart_deinit(void) {
    UCSRA = _BV(TXC);
    UCSRB = 0;
    UBRRL = 0;
    UBRRH = 0;
}

uint8_t uart_receive(void) {
    while (!uart_can_read());
    return UDR;
}

void uart_send(uint8_t v) {
    UDR = v;
    while (!(UCSRA & _BV(TXC)));
    UCSRA |= _BV(TXC);
}

bool uart_can_read(void) {
    return UCSRA & _BV(RXC);
}