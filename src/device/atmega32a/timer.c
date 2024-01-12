#include "f_cpu.h"

#include <sx2flash/timer.h>

#include <avr/io.h>

#if (F_CPU >> 20) == 0
# define CLOCK_TICK 1
#else
# define CLOCK_TICK (F_CPU >> 20)
#endif

void timer_start(uint16_t durationMillis) {
    timer_stop();
    uint16_t d = 65536 - durationMillis * CLOCK_TICK;
    TCNT1H = d >> 8;
    TCNT1L = d & 0xFF;
    TCCR1B = _BV(CS12) | _BV(CS10);
}

void timer_stop(void) {
    TCCR1B = 0;
    TCNT1H = 0;
    TCNT1L = 0;
    TIFR = 0xFF;
}

bool timer_elapsed(void) {
    return TIFR & _BV(TOV1);
}