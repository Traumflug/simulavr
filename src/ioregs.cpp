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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 *
 *  $Id$
 */

#include "ioregs.h"
#include "trace.h"
unsigned int HWMcucr::CpuCycle(){return 0;}

unsigned char RWRampz::operator=(unsigned char val) { if (core->trace_on) trioaccess("Rampz",val);ad->SetRampz(val); return val; } 
RWRampz::operator unsigned char() const { return ad->GetRampz(); } 


unsigned char RWMcucr::operator=(unsigned char val) { if (core->trace_on) trioaccess("Mcucr",val);hwMcucr->SetMcucr(val); hwExtIrq->SetMcucrCopy(val); return val; } 

RWMcucr::operator unsigned char() const { return hwMcucr->GetMcucr(); } 
