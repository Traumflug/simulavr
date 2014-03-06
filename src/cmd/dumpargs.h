/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph
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

#ifndef DUMPARGS_H
#define DUMPARGS_H

#include <iostream>
#include <vector>

#include "../avrdevice.h"
#include "../traceval.h"

//! Parse given trace options and open Dumper
extern void SetDumpTraceArgs(const std::vector<std::string> &traceopts, AvrDevice *dev);

//! Write out registered trace values for device to file or stdout
extern void ShowRegisteredTraceValues(const std::string &outname);

//! Write out core dump file (for analysis)
extern void WriteCoreDump(const std::string &outname, AvrDevice *dev);

#endif
// EOF
