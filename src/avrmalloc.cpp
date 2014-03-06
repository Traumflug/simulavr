/*
 *
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

/**
   \brief Memory Management Functions.

   This module provides facilities for managing memory.

   There is no need to check the returned values from any of these
   functions. Any memory allocation failure is considered fatal and the
   program is terminated.

   We want to wrap all functions that allocate memory. This way we can
   add secret code to track memory usage and debug memory leaks if we 
   want. Right now, I don't want to ;). */

#include <stdlib.h>
#include <string.h>

#include "avrerror.h"
#include "avrmalloc.h"

/* These macros are only here for documentation purposes. */

#if MACRO_DOCUMENTATION

/** \brief Macro for allocating memory.
    \param type  The C type of the memory to allocate.
    \param count Allocate enough memory hold count types.

    This macro is just a wrapper for avr_malloc() and should be used to avoid
    the repetitive task of casting the returned pointer. */

#define avr_new(type, count)          \
    ((type *) avr_malloc ((unsigned) sizeof (type) * (count)))

/** \brief Macro for allocating memory and initializing it to zero.
    \param type  The C type of the memory to allocate.
    \param count Allocate enough memory hold count types.

    This macro is just a wrapper for avr_malloc0() and should be used to avoid
    the repetitive task of casting the returned pointer. */

#define avr_new0(type, count)         \
    ((type *) avr_malloc0 ((unsigned) sizeof (type) * (count)))

/** \brief Macro for allocating memory.
    \param type  The C type of the memory to allocate.
    \param mem   Pointer to existing memory.
    \param count Allocate enough memory hold count types.

    This macro is just a wrapper for avr_malloc() and should be used to avoid
    the repetitive task of casting the returned pointer. */

#define avr_renew(type, mem, count)   \
   ((type *) avr_realloc (mem, (unsigned) sizeof (type) * (count)))

#endif /* MACRO_DOCUMENTATION */

/** \brief Allocate memory and initialize to zero.

    Use the avr_new() macro instead of this function.

    There is no need to check the returned value, since this function will
    terminate the program if the memory allocation fails.

    No memory is allocated if passed a size of zero. */

void *avr_malloc(size_t size)
{
    if (size)
    {
        void *ptr;
        ptr = malloc( size );
        if (ptr)
            return ptr;

        avr_error( "malloc failed" );
    }
    return NULL;
}

/** \brief Allocate memory and initialize to zero.

    Use the avr_new0() macro instead of this function.

    There is no need to check the returned value, since this function will
    terminate the program if the memory allocation fails.

    No memory is allocated if passed a size of zero. */

void *avr_malloc0(size_t size)
{
    if (size)
    {
        void *ptr;
        ptr = calloc( 1, size );
        if (ptr)
            return ptr;

        avr_error( "malloc0 failed" );
    }
    return NULL;
}

/** \brief Wrapper for realloc().

    Resizes and possibly allocates more memory for an existing memory block.

    Use the avr_renew() macro instead of this function.

    There is no need to check the returned value, since this function will
    terminate the program if the memory allocation fails.

    No memory is allocated if passed a size of zero. */

void *avr_realloc(void *ptr, size_t size)
{
    if (size)
    {
        ptr = realloc( ptr, size );
        if (ptr)
            return ptr;

        avr_error( "realloc failed\n" );
    }
    return NULL;
}

/** \brief Wrapper for strdup().

    Returns a copy of the passed in string. The returned copy must be
    free'd.

    There is no need to check the returned value, since this function will
    terminate the program if the memory allocation fails.

    It is safe to pass a NULL pointer. No memory is allocated if a NULL is
    passed. */

char *avr_strdup(const char *s)
{
    if (s)
    {
        char *ptr;
        ptr = strdup(s);
        if (ptr)
            return ptr;

        avr_error( "strdup failed" );
    }
    return NULL;
}

/** \brief Free malloc'd memory.

    It is safe to pass a null pointer to this function. */

void avr_free(void *ptr)
{
    if (ptr)
        free(ptr);
}

