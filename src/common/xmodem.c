#include <sx2flash/xmodem.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <sx2flash/uart.h>
#include <sx2flash/timer.h>
#include <sx2flash/ihex.h>


static uint8_t next_xmodem_seq_num(uint8_t seq) {
    seq++;
    return seq + !seq;    
}

static bool packet_process(const uint8_t *data) {
    static ihex_record_status_t last_ihex_status = IHEX_RECORD_STATUS_OK;

    if (data == NULL) {
        // Check if we have processed an END record. If not,
        // the transfer is not complete.
        return last_ihex_status == IHEX_RECORD_STATUS_END;
    }
    if (last_ihex_status == IHEX_RECORD_STATUS_END) {
        // We do not accept more data after and END record
        return false;
    }
    for (size_t sz = XMODEM_DATA_SIZE; sz > 0; ) {
        size_t used_sz = sz;
        last_ihex_status = ihex_process_record(data, &used_sz);
        switch (last_ihex_status) {
        case IHEX_RECORD_STATUS_PARSE_ERROR:
            return false;
        case IHEX_RECORD_STATUS_END:
            return true;
        case IHEX_RECORD_STATUS_OK:
            sz -= used_sz;
            data += used_sz;
            break;
        }
    }
    return true;
}


void xmodem_loop(void) {
    // We store only the data portion plus the checksum, but not the header
    static uint8_t packet_data[XMODEM_PACKET_SIZE - 3];

    uint8_t next_ack = XMODEM_NAK;
    uint8_t seq_num = 1;
    bool first_packet = true;
    for (;;) {
        uart_send(next_ack);
        timer_start(XMODEM_TIMEOUT);
        while (!uart_can_read() && !timer_elapsed());
        if (timer_elapsed()) {
            if (first_packet) {
                break;
            }
            continue;
        }
        first_packet = false;
        const uint8_t data = uart_receive();
        switch (data) {
        case XMODEM_EOT:
            next_ack = packet_process(NULL) ? XMODEM_ACK : XMODEM_NAK;
            uart_send(next_ack);
            while (next_ack != XMODEM_ACK);
            return;
        default:
            const uint8_t pkt_seq = uart_receive();
            const uint8_t pkt_seq_inv = uart_receive();
            for (size_t i = 0; i < sizeof packet_data; i++) {
                packet_data[i] = uart_receive();
            }
            if (data != XMODEM_SOH || 255 - pkt_seq != pkt_seq_inv || pkt_seq != seq_num) {
                next_ack = XMODEM_NAK;
                continue;
            }
            next_ack = packet_process(packet_data) ? XMODEM_ACK : XMODEM_NAK;
            if (next_ack == XMODEM_ACK) {
                seq_num = next_xmodem_seq_num(seq_num);
            }
            break;
        }
    }
}