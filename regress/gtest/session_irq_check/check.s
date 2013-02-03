#include <avr/io.h>
#include <avr/interrupt.h>

#undef _SFR_IO8
#define _SFR_IO8(x) (x)
#undef _SFR_IO16
#define _SFR_IO16(x) (x)


.global main
main:
    in r11, SPH            ; save stack pointer

    ldi r16, 0xff
    out SPH, r16           ; kill stack pointer

    sei                    ; enable interrupts, attention: stack is not in place!
    ldi r16, (1<<UDRIE)    ; lets generate a usart data register empty irq
    out UCSRB, r16         ; which should exactly now take place BUT ->
    out SPH, r11           ; the avr should do one more instruction after irq vector raise! always! 
    ret                    ; this will result in going back to ctors_end -> exit

.global USART_UDRE_vect
USART_UDRE_vect:
    ldi r16, ~(1<<UDRIE)
    out UCSRB, r16

.global stopsim
stopsim:
    reti


