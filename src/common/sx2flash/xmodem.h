#ifndef XMODEH_H
#define XMODEH_H

#define XMODEM_ACK 6
#define XMODEM_NAK 21
#define XMODEM_SOH 1
#define XMODEM_EOT 4

#ifndef XMODEM_TIMEOUT
#  define XMODEM_TIMEOUT 2000 // Millisenconds
#elif XMODEM_TIMEOUT <= 0
# error XMODEM_TIMEOUT must be positive
#endif

#define XMODEM_DATA_SIZE 128
#define XMODEM_PACKET_SIZE (XMODEM_DATA_SIZE + 4)

void xmodem_loop(void);

#endif // XMODEM_H