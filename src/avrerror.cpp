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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 */

/**
   \file avrerror.c
   \brief Functions for printing messages, warnings and errors.

   This module provides output printing facilities. */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "avrerror.h"

#if MACRO_DOCUMENTATION

/** \brief Print an ordinary message to stdout. */
#define avr_message(fmt, args...) private_avr_message(__FILE__, __LINE__, fmt, ## args)

/** \brief Print a warning message to stderr. */
#define avr_warning(fmt, args...) private_avr_warning(__FILE__, __LINE__, fmt, ## args)

/** \brief Print an error message to stderr and terminate program. */
#define avr_error(fmt, args...)   private_avr_error(__FILE__, __LINE__, fmt, ## args)

#else /* Not Documentation */

void private_avr_message( char *file, int line, char *fmt, ... )
{
    va_list ap;
    char    ffmt[128];

    snprintf(ffmt, sizeof(ffmt), "MESSAGE: file %s: line %d: %s", 
             file, line, fmt );
    ffmt[127] = '\0';

    va_start(ap, fmt);
    vfprintf(stdout, ffmt, ap);
    va_end(ap);
}

void private_avr_warning( char *file, int line, char *fmt, ... )
{
    va_list ap;
    char    ffmt[128];

    snprintf(ffmt, sizeof(ffmt), "WARNING: file %s: line %d: %s", 
             file, line, fmt );
    ffmt[127] = '\0';

    va_start(ap, fmt);
    vfprintf(stderr, ffmt, ap);
    va_end(ap);
}

void private_avr_error( char *file, int line, char *fmt, ... )
{
    va_list ap;
    char    ffmt[128];

    snprintf(ffmt, sizeof(ffmt), "\nERROR: file %s: line %d: %s\n\n", 
             file, line, fmt );
    ffmt[127] = '\0';

    va_start(ap, fmt);
    vfprintf(stderr, ffmt, ap);
    va_end(ap);

    exit(1);                    /* exit instead of abort */
}

#endif /* Not documenation */
