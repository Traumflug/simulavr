#include <avr/interrupt.h>

#include <avr/io.h>
#include <avr/eeprom.h>

volatile unsigned char in_loop = 0;
volatile unsigned char complete = 1;
volatile unsigned long eep_value = 0;

typedef struct {
    unsigned char dummy; // Addr. 0, don't use
    unsigned char byte1;
    unsigned short word1;
    unsigned long long1;
} eep_t;
eep_t EEMEM eep = { 0, 0x33, 0x55AA, 0xDEADBEEF };

int main(void) {

    do {
        in_loop = 1;
        switch(complete) {
            case 0:
                // wait till eeprom is ready again (for write operation)
                if(eeprom_is_ready())
                     complete = 1;
            case 1:
                break;
            case 2:
                eep_value = eeprom_read_byte(&(eep.byte1));
                complete = 1;
                break;
            case 3:
                eep_value = eeprom_read_word((uint16_t *)&(eep.word1));
                complete = 1;
                break;
            case 4:
                eep_value = eeprom_read_dword(&(eep.long1));
                complete = 1;
                break;
            case 5:
                eeprom_write_byte(&(eep.byte1), 0x66);
                complete = 0;
                break;
            case 6:
                eeprom_write_word((uint16_t *)&(eep.word1), 0x1234);
                complete = 0;
                break;
            case 7:
                eeprom_write_dword(&(eep.long1), 0x55AAAA55);
                complete = 0;
                break;
        }
    } while(1); // do forever
}

// EOF
