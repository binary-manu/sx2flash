.global spm_handler

; void spm_handler(uint8_t spmcr, uint16_t r1r0, uint16_t z);
; spmcr =     r24
; r1r0  = r23:r22
; z     = r21:r20

#include <avr/io.h>

#define io(ioloc) ((ioloc) - 32)

spm_handler:
    mov     r0,         r22
    mov     r1,         r23
    mov     zl,         r20
    mov     zh,         r21
    out     io(SPMCR),  r24
    spm
wait_for_spm_done:
    in      r25,        io(SPMCR)
    sbrc    r25,        0
    rjmp    wait_for_spm_done
    clr     r1
    ret
