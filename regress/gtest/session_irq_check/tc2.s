#include <avr/io.h>
#include <avr/interrupt.h>

#undef _SFR_IO8
#define _SFR_IO8(x) (x)
#undef _SFR_IO16
#define _SFR_IO16(x) (x)


.global main
main:
    ldi r16, 0xff          ; all pins output
    out DDRB, r16          ; all pins output
    ldi r16, 0x01          ; one pin on
    out PORTB, r16         ; initial only 1 pin 

    ldi r16, (1<<UDRIE)    ; lets generate a usart data register empty irq
    out UCSRB, r16         ; fire hardware irq, but i flag is not set
    ldi r17, 0x55

    sei                    ; now the irq is pending
    out PORTB, r17         ; if this instruction is executed, we will see 0x55 on portb!

; normaly never execute any of the following instructions, we caught into irq handler
    ldi r17, 0x0f;         ; if irq will not be executed or comes back
    out PORTB, r17         ; we see 0x0f on port b which should not happen
    ret                    ; this will result in going back to ctors_end -> exit

.global USART_UDRE_vect
USART_UDRE_vect:
.global stopsim
stopsim:
   rjmp USART_UDRE_vect


