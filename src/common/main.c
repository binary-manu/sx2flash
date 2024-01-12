#include <sx2flash/xmodem.h>
#include <sx2flash/uart.h>
#include <sx2flash/timer.h>
#include <sx2flash/payload.h>

int main() {
    uart_init();

    xmodem_loop();

    uart_deinit();
    timer_stop();
    jump_to_application();
}