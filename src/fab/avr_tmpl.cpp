/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph		
 * Copyright (C) 2008 Onno Kortmann
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
 * THIS FILE HAS BEEN AUTOMATICALLY GENERATED FROM avr_tmpl.cpp.
 *                     --- DO NOT EDIT MANUALLY! ---
 */
\#include "avr_$(part).h"

## FIXME: Include only those parts which are needed!
\#include "irqsystem.h"
\#include "avrfactory.h"
\#include "hweeprom.h"
\#include "hwstack.h"
\#include "hwport.h"
\#include "hwwado.h"
\#include "hwtimer.h"
\#include "hwtimer01irq.h"
\#include "hwspi.h"
\#include "ioregs.h"
\#include "hwuart.h"
\#include "hwsreg.h"

AVR_REGISTER($(part), AVR_$(part));

AVR_$part::~AVR_$(part)() {}

AVR_$part::AVR_$(part)() : AvrDevice($io_size,
				   $iram_size,
				   $eram_size,
				   $flash_size) {
    irqSystem = new HWIrqSystem(this, $irq_vec_size); 

#include "eeprom_tmpl.cpp"
    
#if $iram_size>0
    stack = new HWStack(this, Sram, $stack.ceil);
#else
    stack = new HWStack(this, new ThreeLevelStack(this), 0x06);
#end if

    wado			= new HWWado(this);
    rw[$io["WDTCR"].addr]	= & wado->wdtcr_reg;
    
    prescaler	= new HWPrescaler(this);
    mcucr	= new HWMcucr(this); //FIXME! MCUCR reg!
    rw[$io["SREG"].addr]= new RWSreg(this, status);

#if "SPL" in $io
    rw[$io["SPL"].addr]= & stack->spl_reg;
#end if
#if "SPH" in $io
    rw[$io["SPH"].addr]= & stack->sph_reg;
#end if    
    

    
#include "port_tmpl.cpp"

#include "timer_tmpl.cpp"

#include "spi_tmpl.cpp"

#include "usart_tmpl.cpp"    
    Reset();
}


