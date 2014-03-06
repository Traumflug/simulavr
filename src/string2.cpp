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

#include <errno.h>
#include <stdlib.h>
#include <limits.h>

#ifdef _MSC_VER
#  define strtoll _strtoi64
#  define strtoull _strtoui64
#endif

/*
 *  Instantiate an error checking wrapper for strtol (unsigned char)
 */
#undef STRING_TO_TYPE
#undef STRING_TO_NAME
#undef STRING_TO_METHOD
#undef STRING_TO_MAX
#undef STRING_TO_MIN
#define STRING_TO_TYPE unsigned char
#define STRING_TO_NAME StringToUnsignedChar
#define STRING_TO_METHOD strtoul
#define STRING_TO_MAX 255
#include "string2_template.h"

/*
 *  Instantiate an error checking wrapper for strtol (long)
 */
#undef STRING_TO_TYPE
#undef STRING_TO_NAME
#undef STRING_TO_METHOD
#undef STRING_TO_MAX
#undef STRING_TO_MIN
#define STRING_TO_TYPE long int
#define STRING_TO_NAME StringToLong
#define STRING_TO_METHOD strtol
#define STRING_TO_MIN LONG_MIN
#define STRING_TO_MAX LONG_MAX
#include "string2_template.h"

/*
 *  Instantiate an error checking wrapper for strtoul (unsigned long)
 */
#undef STRING_TO_TYPE
#undef STRING_TO_NAME
#undef STRING_TO_METHOD
#undef STRING_TO_MAX
#undef STRING_TO_MIN
#define STRING_TO_TYPE unsigned long int
#define STRING_TO_NAME StringToUnsignedLong
#define STRING_TO_METHOD strtoul
#define STRING_TO_MAX ULONG_MAX
#include "string2_template.h"

/*
 *  Instantiate an error checking wrapper for strtoll (long long)
 */
#undef STRING_TO_TYPE
#undef STRING_TO_NAME
#undef STRING_TO_METHOD
#undef STRING_TO_MAX
#undef STRING_TO_MIN
#define STRING_TO_TYPE long long
#define STRING_TO_NAME StringToLongLong
#define STRING_TO_METHOD strtoll
#define STRING_TO_MIN LLONG_MIN
#define STRING_TO_MAX LLONG_MAX
#include "string2_template.h"

/*
 *  Instantiate an error checking wrapper for strtoull (unsigned long long)
 */
#undef STRING_TO_TYPE
#undef STRING_TO_NAME
#undef STRING_TO_METHOD
#undef STRING_TO_MAX
#undef STRING_TO_MIN
#define STRING_TO_TYPE unsigned long long
#define STRING_TO_NAME StringToUnsignedLongLong
#define STRING_TO_METHOD strtoull
#define STRING_TO_MAX ULLONG_MAX
#include "string2_template.h"

