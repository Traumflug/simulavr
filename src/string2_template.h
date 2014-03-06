/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2009   Joel Sherrill
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 */

/*
 * This file is designed to be included multiple times to instantiate
 * it and should NOT be protected against multiple inclusions.
 */

#ifndef STRING_TO_TYPE
  #error "STRING_TO_TYPE not defined"
#endif

#ifndef STRING_TO_NAME
  #error "STRING_TO_NAME not defined"
#endif

#ifndef STRING_TO_METHOD
  #error "STRING_TO_METHOD not defined"
#endif

#ifndef STRING_TO_MAX
  #error "STRING_TO_MAX not defined"
#endif

bool STRING_TO_NAME (
  const char      *s,
  STRING_TO_TYPE  *n,
  char           **endptr,
  int              base
)
{
  unsigned long long result;
  char           *end;

  if ( !n )
    return false;

  errno = 0;
  *n    = 0;

  result = STRING_TO_METHOD( s, &end, base );

  if ( endptr )
    *endptr = end; 

  /* nothing was converted */
  if ( end == s )
    return false;

  /* there was a conversion error */
  if ( (result == 0) && errno )
    return false;

  /* there was an overflow */
  if ( (result == LONG_MAX) && (errno == ERANGE))
    return false;

  /* does not fit into target type */
  if ( result > STRING_TO_MAX )
	  return false;

#ifdef STRING_TO_MIN
  /* there was an underflow */
  if ( (result == STRING_TO_MIN) && (errno == ERANGE))
    return false;
#endif

  *n = (STRING_TO_TYPE) result;
  return true;
}

