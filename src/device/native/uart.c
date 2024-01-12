#include <sx2flash/uart.h>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>

#include <sx2flash/util.h>

#define ME "native uart"

void uart_init(void) {
}

void uart_deinit(void) {
}

uint8_t uart_receive(void) {
    uint8_t t;
    when_true(read(STDIN_FILENO, &t, sizeof t) < 0, quit);
    return t;
quit:
    perror(ME " read");
    exit(EXIT_FAILURE);
}

void uart_send(uint8_t v) {
    when_true(write(STDOUT_FILENO, &v, sizeof v) < 0, quit);
    return;
quit:
    perror(ME " write");
    exit(EXIT_FAILURE);
}

bool uart_can_read(void) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    struct timeval pollTime = {0};
    when_true(select(STDIN_FILENO + 1, &fds, 0, 0, &pollTime) < 0, quit);
    return FD_ISSET(STDIN_FILENO, &fds);

quit:
    perror(ME " poll");
    exit(EXIT_FAILURE);
}