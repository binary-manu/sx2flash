#include <sx2flash/timer.h>

#include <unistd.h>
#include <signal.h>

void timer_start(uint16_t durationMillis) {
    timer_stop();
    alarm((durationMillis + 999) / 1000);
}

void timer_stop(void) {
    alarm(0);
    sigset_t blockMe;
    sigemptyset(&blockMe);
    sigaddset(&blockMe, SIGALRM);
    sigprocmask(SIG_BLOCK, &blockMe, NULL);
}

bool timer_elapsed(void) {
    sigset_t pending;
    sigpending(&pending);
    return sigismember(&pending, SIGALRM);
}