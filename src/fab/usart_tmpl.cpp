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
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
##  
{
#if $has_usart
    ## FIXME: USART != UART !!!!

#if "USART, RX" in $irq_bysrc
	usart= new HWUart(this,
			  irqSystem,
			  PinAtPort(port$pkg_portlet["TXD"],$pkg_portbit["TXD"]),
			  PinAtPort(port$pkg_portlet["RXD"],$pkg_portbit["RXD"]),
			  $irq_bysrc["USART, RX"].addr,
			  $irq_bysrc["USART, UDRE"].addr,
			  $irq_bysrc["USART, TX"].addr);
#else
	usart= new HWUart(this,
			  irqSystem,
			  PinAtPort(port$pkg_portlet["TXD"],$pkg_portbit["TXD"]),
			  PinAtPort(port$pkg_portlet["RXD"],$pkg_portbit["RXD"]),
			  $irq_bysrc["USART, RXC"].addr,
			  $irq_bysrc["USART, UDRE"].addr,
			  $irq_bysrc["USART, TXC"].addr);
#endif    


    rw[$io["UDR"].addr]= new RWUdr(this, usart);
    rw[$io["UCSRA"].addr]= new RWUsr(this, usart);
    rw[$io["UCSRB"].addr]= new RWUcr(this, usart);
    rw[$io["UBRRL"].addr]= new RWUbrr(this, usart);
#endif						
}
