/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002  Theodore A. Roth
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

#ifndef SIM_AVRMALLOC_H
#define SIM_AVRMALLOC_H

/* Macros for allocating new memory. Automatically performs type cast. If
   these definitions are changed, the documentation in avrmalloc.c must be
   changed. */

/** \brief Macro for allocating memory.
    \param type  The C type of the memory to allocate.
    \param count Allocate enough memory hold count types.

    This macro is just a wrapper for avr_malloc() and should be used to avoid
    the repetitive task of casting the returned pointer. */

#define avr_new(type, count)          \
    ((type *) avr_malloc ((unsigned) sizeof (type) * (count)))

#define avr_new0(type, count)         \
    ((type *) avr_malloc0 ((unsigned) sizeof (type) * (count)))

#define avr_renew(type, mem, count)   \
    ((type *) avr_realloc (mem, (unsigned) sizeof (type) * (count)))

/*
 * Malloc and free wrappers. Perform sanity checks for you.
 */

extern void *avr_malloc      (size_t size);
extern void *avr_malloc0     (size_t size);
extern void *avr_realloc     (void *ptr, size_t size);
extern char *avr_strdup      (const char *s);
extern void  avr_free        (void *ptr);

#endif /* SIM_AVRMALLOC_H */
