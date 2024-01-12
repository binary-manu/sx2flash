#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

void uart_init(void);
void uart_deinit(void);
uint8_t uart_receive(void);
void uart_send(uint8_t v);
bool uart_can_read(void);

#endif // UART_H