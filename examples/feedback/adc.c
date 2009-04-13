/*
 *   $Id$
 */

#include <avr/io.h>

unsigned char getAdc(unsigned char channel)
{
  unsigned char adcVal;
  
  //  ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(ADLAR) | channel;
  //  for internal reference
  ADMUX = _BV(ADLAR) | channel; //for AREF
  ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

  ADCSRA |= _BV(ADEN);
  ADCSRA |= _BV(ADSC);

  while (bit_is_set(ADCSRA, ADSC))
      /**/;

  adcVal = ADCH;
  ADCSRA &= ~_BV(ADEN);

  return adcVal;
}

