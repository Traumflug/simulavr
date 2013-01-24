/*
 *  $Id$
 */

/* Set cpu specific attributes in this file */

#define    ATMEGA128    100
#define    ATMEGA163     90
#define    AT90S8515     70
#define    AT90S4434     60
#define    AT90S4433     50
#define    AT90S2313     20

// Change this line to set the device type   <*********** USER
//  See choices above.
#define DEVICE_NAME  ATMEGA128
// Define crystal frequency as 10 x MHz, i.e. 7.37 = 73, etc.  <********** USER
#define CPU_XTL  40
// Define RUN LED port / pin    <********** USER
#define RUNLED_PORT  PORTD
#define RUNLED_BIT   5
#define RUNLED_ON()    sbi(RUNLED_PORT, RUNLED_BIT)
#define RUNLED_OFF()   cbi(RUNLED_PORT, RUNLED_BIT)
#define TESTPIN_PORT PORTB
#define TESTPIN_BIT  1
#define TESTPIN_ON()   sbi(TESTPIN_PORT, TESTPIN_BIT)
#define TESTPIN_OFF()  cbi(TESTPIN_PORT, TESTPIN_BIT)

/* Maps GCC-AVR  variables to standard data types  */

typedef unsigned char  BOOLEAN;
typedef unsigned char  CHARU;   /* Unsigned  8 bit quantity  */
typedef signed char    CHARS;   /* Signed    8 bit quantity  */
typedef unsigned int   INT16U;  /* Unsigned 16 bit quantity  */
typedef int            INT16S;  /* Signed   16 bit quantity   */
typedef unsigned long  INT32U;  /* Unsigned 32 bit quantity  */
typedef long           INT32S;  /* Signed   32 bit quantity  */
typedef float          FP32;    /* Single precision floating point  */
typedef double         FP64;    /* Double precision floating point  */

// Function prototypes
void run_led(INT16U, INT16U);
void test_pin(void);
void msleep(INT16U ms);
void putBCD(INT16S X, CHARU length, CHARU TrailingSpace);
//void putchar(CHARU c);
void putstr(CHARU *s);
//CHARU bit_num(CHARU x);
//CHARU subt_rollover(CHARU x, CHARU y);
void print_hexbyte(unsigned char i);


#define ONETENTH_MS  10*(1570/CPU_XTL)
#define CRLF()  putchar(13); putchar(10)

/* Macros to Standardize register names amongst various devices
   This table is only partially complete - I add items as
   required.  The Device names listed cover most, but not all AVR variants.
   Use only the listed names */

#if DEVICE_NAME == ATMEGA128
   #define UART_BAUD_REG        UBRR0L
   #define UART_BAUD_REG_H      UBRR0H
   #define UART_CONTROL_REG     UCSR0B
   #define UART_STATUS_REG      UCSR0A
   #define UART_RCV_INT_VECTOR  USART0_RX_vect
   #define UART_REG_EMPTY_INT_VECTOR  USART0_UDRE_vect
   #define UART_DATA_REG        UDR0
#elif DEVICE_NAME == ATMEGA163
   #define UART_BAUD_REG        UBRR
   #define UART_CONTROL_REG     UCSRB
   #define UART_STATUS_REG      UCSRA
   #define UART_RCV_INT_VECTOR  SIG_UART_RECV
   #define UART_REG_EMPTY_INT_VECTOR  SIG_UART_DATA
   #define UART_DATA_REG        UDR
#elif DEVICE_NAME == AT90S2313
   #define UART_BAUD_REG        UBRR
   #define UART_CONTROL_REG     UCR
   #define UART_STATUS_REG      USR
   #define UART_RCV_INT_VECTOR  SIG_UART_RECV
   #define UART_REG_EMPTY_INT_VECTOR  SIG_UART_DATA
   #define UART_DATA_REG        UDR
#elif DEVICE_NAME == AT90S4433
   #define UART_BAUD_REG        UBRRL
   #define UART_CONTROL_REG     UCSRB
   #define UART_STATUS_REG      UCSRA
   #define UART_RCV_INT_VECTOR  SIG_UART_RECV
   #define UART_REG_EMPTY_INT_VECTOR  SIG_UART_DATA
   #define UART_DATA_REG        UDR
#elif DEVICE_NAME == AT90S4434
   #define UART_BAUD_REG        UBRR
   #define UART_CONTROL_REG     UCR
   #define UART_STATUS_REG      USR
   #define UART_RCV_INT_VECTOR  SIG_UART_RECV
   #define UART_REG_EMPTY_INT_VECTOR  SIG_UART_DATA
   #define UART_DATA_REG        UDR
#elif DEVICE_NAME == AT90S8515
   #define UART_BAUD_REG        UBRR
   #define UART_CONTROL_REG     UCR
   #define UART_STATUS_REG      USR
   #define UART_RCV_INT_VECTOR  SIG_UART_RECV
   #define UART_REG_EMPTY_INT_VECTOR  SIG_UART_DATA
   #define UART_DATA_REG        UDR
#else
   // Defaults to AT90S2313
   #define UART_BAUD_REG        UBRR
   #define UART_CONTROL_REG     UCR
   #define UART_STATUS_REG      USR
   #define UART_RCV_INT_VECTOR  SIG_UART_RECV
   #define UART_REG_EMPTY_INT_VECTOR  SIG_UART_DATA
   #define UART_DATA_REG        UDR
#endif

typedef enum
    {
    #if   CPU_XTL == 160
       BAUD115K = 8, BAUD76K = 12, BAUD57K = 16,
    	BAUD38K = 25, BAUD19K = 51, BAUD14K = 68,
    	BAUD9600 = 103, BAUD4800 = 207, BAUD2400 = 416,
    #elif CPU_XTL == 147
       BAUD115K = 7, BAUD76K = 11, BAUD57K = 15,
    	BAUD38K = 23, BAUD19K = 47, BAUD14K = 63,
    	BAUD9600 = 95, BAUD4800 = 191, BAUD2400 = 383,
    #elif CPU_XTL == 80
       BAUD115K = 3, BAUD76K = 6, BAUD57K = 8,
    	BAUD38K = 12, BAUD19K = 25, BAUD14K = 34,
    	BAUD9600 = 51, BAUD4800 = 103, BAUD2400 = 207,
    	BAUD1200 = 414, BAUD600 = 818, BAUD300 = 1636
    #elif CPU_XTL == 73
       BAUD115K = 3, BAUD76K = 5, BAUD57K = 7,
       BAUD38K = 11, BAUD19K = 23, BAUD14K = 31,
    	BAUD9600 = 47, BAUD4800 = 95, BAUD2400 = 191
    #elif CPU_XTL == 40
       BAUD115K = 1, BAUD76K = 2, BAUD57K = 3,
       BAUD38K = 6, BAUD19K = 12, BAUD14K = 16,
    	BAUD9600 = 25, BAUD4800 = 51, BAUD2400 = 103
    #endif
    } BaudRate;

void setbaud(BaudRate);




