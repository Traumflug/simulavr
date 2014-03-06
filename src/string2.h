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

#ifndef __String2_h
#define __String2_h

bool StringToUnsignedChar (
  const char     *s,
  unsigned char  *n,
  char          **endptr,
  int             base
);

bool StringToLong (
  const char  *s,
  long        *n,
  char       **endptr,
  int          base
);

bool StringToUnsignedLong (
  const char     *s,
  unsigned long  *n,
  char          **endptr,
  int             base
);

bool StringToLongLong (
  const char  *s,
  long long   *n,
  char       **endptr,
  int          base
);

bool StringToUnsignedLongLong (
  const char           *s,
  unsigned long long   *n,
  char                **endptr,
  int                   base
);

#endif
