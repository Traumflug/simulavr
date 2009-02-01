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

void private_avr_message( const char *file, int line, const char *fmt, ... )
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

void private_avr_warning( const char *file, int line, const char *fmt, ... )
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

void private_avr_error( const char *file, int line, const char *fmt, ... )
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

