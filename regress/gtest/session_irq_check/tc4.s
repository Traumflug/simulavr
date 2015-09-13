#include <avr/io.h>
#include <avr/interrupt.h>

#undef _SFR_IO8
#define _SFR_IO8(x) (x)
#undef _SFR_IO16
#define _SFR_IO16(x) (x)


.global main
main:
    ldi r18, 0x00          ; we check later for other value
    ldi r19, 0x00          ; we check later for other value
    ldi r20, 0x00          ; we check later for other value

    ldi r16, 0xff          ; all pins output
    out DDRB, r16          ; all pins output
    ldi r16, 0x01          ; one pin on
    out PORTB, r16         ; initial only 1 pin 

    sei                    ; enable global irqs
    ldi r16, (1<<UDRIE)    ; lets generate a usart data register empty irq
    out UCSRB, r16         ; fire hardware irq, I flag is already set
    cli                    ; but there is one more instruction to do
                           ; and cli should prevent going to irq after the command has
                           ; executed!

; because the irq should NOT be fired, we see the result of the next instruction
; after test finsihed    
    ldi r18, 0x01          ; 0x01 must be found in R10 after test

; now we do some additional tests
    sei                    ; now the irq should be enabled again
    ldi r19, 0x02          ; this command should be executed after a sei

; but the following instructions should NOT be executed
    ldi r20, 0x03          ; so the old value 0x00 should be here
    ret                    ; this will result in going back to ctors_end -> exit

.global USART_UDRE_vect
USART_UDRE_vect:
    ldi r17, 0xff;         ; if we run into irq, we will see 0xff 
    out PORTB, r17

.global stopsim
stopsim:
   rjmp USART_UDRE_vect


