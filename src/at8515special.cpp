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
 */
#include "at8515special.h"
#include "rwmem.h"

AvrDevice_at90s8515special::AvrDevice_at90s8515special():
AvrDevice_at90s8515() { 
    //	status= new HWSreg();	
    rw[0x20]= new RWWriteToPipe("../../rbout");
    Reset();
}

AvrDevice_at90s8515special::~AvrDevice_at90s8515special() {}

unsigned char AvrDevice_at90s8515special::GetRampz(){ 
    cout << "Rampz is not a valid Register in at8515!" ;
    return 0;
}
void AvrDevice_at90s8515special::SetRampz(unsigned char){cout << "Rampz is not a valid Register in at8515!" ;}
