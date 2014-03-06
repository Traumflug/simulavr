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
{
#if $has_timer1
    timer01irq	= new HWTimer01Irq(this,
				   irqSystem,
				   $irq_bysrc["TIMER1 CAPT"].addr,
				   $irq_bysrc["TIMER1 COMPA"].addr,
				   $irq_bysrc["TIMER1 COMPB"].addr,
				   $irq_bysrc["TIMER1 OVF"].addr,
				   $irq_bysrc["TIMER0 OVF"].addr
				   );
    timer01irq->TOIE1=$timer1_timsk["TOIE1"];
    timer01irq->OCIE1A=$timer1_timsk["OCIE1A"];
    timer01irq->OCIE1B=$timer1_timsk["OCIE1B"];
#if "TICIE1"    in $timer1_timsk
    timer01irq->TICIE1=$timer1_timsk["TICIE1"];
#else
    timer01irq->TICIE1=$timer1_timsk["ICIE1"];
#end if    
#if "TOIE0" in $timer1_timsk
    timer01irq->TOIE0=$timer1_timsk["TOIE0"];
#end if    

    timer01irq->TOV1=$timer1_tifr["TOV1"];
    timer01irq->OCF1A=$timer1_tifr["OCF1A"];
    timer01irq->OCF1B=$timer1_tifr["OCF1B"];
    timer01irq->ICF1=$timer1_tifr["ICF1"];
#if "TOV0" in $timer1_tifr    
    timer01irq->TOV0=$timer1_tifr["TOV0"];
#end if    
    
    timer1	= new HWTimer1(this,
			       prescaler,
			       timer01irq,
			       PinAtPort(port$pkg_portlet["T1"],$pkg_portbit["T1"]),
			       PinAtPort(port$pkg_portlet["OC1A"],$pkg_portbit["OC1A"]),
			       PinAtPort(port$pkg_portlet["OC1B"],$pkg_portbit["OC1B"]),
			       PinAtPort(port$pkg_portlet["ICP"],$pkg_portbit["ICP"]));

    rw[$io["TIMSK"].addr]= & timer01irq->timsk_reg;
    rw[$io["TIFR"].addr]= & timer01irq->tifr_reg;
    rw[$io["TCCR1A"].addr]= & timer1->tccr1a_reg;
    rw[$io["TCCR1B"].addr]= & timer1->tccr1b_reg;
    rw[$io["TCNT1H"].addr]= & timer1->tcnt1h_reg;
    rw[$io["TCNT1L"].addr]= & timer1->tcnt1l_reg;
    rw[$io["OCR1AH"].addr]= & timer1->ocr1ah_reg;
    rw[$io["OCR1AL"].addr]= & timer1->ocr1al_reg;

    rw[$io["ICR1H"].addr]= & timer1->icr1h_reg;
    rw[$io["ICR1L"].addr]= & timer1->icr1l_reg;
#end if						
}
