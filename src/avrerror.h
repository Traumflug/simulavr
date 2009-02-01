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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 */

#ifndef SIM_AVRERROR_H
#define SIM_AVRERROR_H

/* FIXME: TRoth 2002-02-23 : '## args' is gcc specific. If porting to another
   compiler, this will have to be handled. Although, I beleive the C99
   standard added this to precompiler. */

/* If these macros are changed, update the documentation in avrerror.c */

#define avr_message(fmt, args...) private_avr_message(__FILE__, __LINE__, fmt, ## args)
#define avr_warning(fmt, args...) private_avr_warning(__FILE__, __LINE__, fmt, ## args)
#define avr_error(fmt, args...)   private_avr_error(__FILE__, __LINE__, fmt, ## args)

extern void private_avr_message   ( const char *file, int line, const char *fmt, ... );
extern void private_avr_warning   ( const char *file, int line, const char *fmt, ... );
extern void private_avr_error     ( const char *file, int line, const char *fmt, ... );

#endif /* SIM_AVRERROR_H */
