#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

void timer_start(uint16_t durationMillisecs);
void timer_stop(void);
bool timer_elapsed(void);

#endif // TIMER_H