## Copyright (C) 2009 Onno Kortmann <onno@gmx.net>
##  
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##  
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##  
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
##  
##FIXME: any other eeprom types?
##FIXME#2: using right eeprom type?
{
#if "EE_RDY" in $irq_bysrc
	eeprom = new HWMegaEeprom(this, irqSystem, $eeprom_size, $irq_bysrc["EE_RDY"].addr);
#else
	eeprom = new HWEeprom(this, irqSystem, $eeprom_size);
#end if    
#if "EEAR" in $io
	rw[$io["EEAR"].addr]= & eeprom->eearl_reg;
	rw[$io["EEDR"].addr]= & eeprom->eedr_reg;
	rw[$io["EECR"].addr]= & eeprom->eecr_reg;
#end if
}
