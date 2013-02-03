#include <avr/io.h>
#include <avr/interrupt.h>

#undef _SFR_IO8
#define _SFR_IO8(x) (x)
#undef _SFR_IO16
#define _SFR_IO16(x) (x)

#define USED_OUT_PIN 0x04           // B2
#define USED_IN_PIN  0x08           // B3

; the pins should be be connected as follows:
; USED_OUTPUT_PIN should drive a open drain transistor circuit
; The open drain output is connected to an external pull up
; the USED_IN_PIN should rout the open drain result, which is simply inverse to the USED_OUT_PIN
.global main
main:
    ldi r16, USED_OUT_PIN          ; only one output pin 
    out DDRB, r16                  ; all pins output

    ldi r16, 0x00
    out PORTB, r16

    nop                             ; in real device e need one instruction before reading the result back

    in r17, PINB                    ; should have USED_IN_PN high and USED_OUT_PIN low -> 0x08 -> r17

    ldi r16, USED_OUT_PIN
    out PORTB, r16

    nop

    in r18, PINB                    ; should have USED_IN_PIN low and USED_OUT_PIN high -> 0x04 -> r18

stopsim:
    nop

endless:
    rjmp  endless





