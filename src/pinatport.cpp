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


#include "pinatport.h"
#include "hwport.h"

#include <iostream>
using namespace std;
PinAtPort::PinAtPort() { 
    cerr << "Dummy Pin At Port" << endl;
}

PinAtPort::PinAtPort( HWPort *p, unsigned char pn)
{
    port=p;
    pinNo=pn;
}

Pin& PinAtPort::GetPin() {
    return port->GetPin(pinNo);
}

void PinAtPort::SetPort(bool val) {
    unsigned char *adr=&port->port;
    SetVal(adr, val);
    port->CalcOutputs();
}

int PinAtPort::GetAnalog() const {
    return port->p[pinNo].GetAnalog();
}


void PinAtPort::SetDdr(bool val) {
    unsigned char *adr=&port->ddr;
    SetVal(adr, val);
    port->CalcOutputs();
}

void PinAtPort::SetAlternateDdr(bool val){
    unsigned char *adr=&port->alternateDdr;
    SetVal(adr, val);
    port->CalcOutputs();
}

void PinAtPort::SetUseAlternateDdr(bool val) {
    unsigned char *adr=&port->useAlternateDdr;
    SetVal(adr, val);
    port->CalcOutputs();
}

void PinAtPort::SetAlternatePort(bool val) {
    unsigned char *adr=&port->alternatePort;
    SetVal(adr, val);
    port->CalcOutputs();
    port->port_reg.hardwareChange(port->alternatePort);
}

void PinAtPort::SetUseAlternatePort(bool val) {
    unsigned char *adr=&port->useAlternatePort;
    SetVal(adr, val);
    port->CalcOutputs();
}

void PinAtPort::SetUseAlternatePortIfDdrSet(bool val) {
    unsigned char *adr=&port->useAlternatePortIfDdrSet;
    SetVal(adr, val);
    port->CalcOutputs();
}

bool PinAtPort::GetPort() {
    return (port->port)>>pinNo;
}

bool PinAtPort::GetDdr() {
    return (port->ddr)>>pinNo;
}

bool PinAtPort::GetAlternateDdr(){
    return (port->alternateDdr)>>pinNo;
}

bool PinAtPort::GetUseAlterateDdr() {
    return (port->useAlternateDdr)>>pinNo;
} 

bool PinAtPort::GetAlternatePort() {
    return (port->alternatePort)>>pinNo;
}

bool PinAtPort::GetUseAlternatePort() {
    return (port->useAlternatePort)>>pinNo;
}

bool PinAtPort::GetUseAlternatePortIfDdrSet() {
    return (port->useAlternatePortIfDdrSet)>>pinNo;
}

PinAtPort::operator bool() {
    return ((port->GetPin())>>pinNo)&0x01;
} //we must use GetPin to recalculate the Pin from p[] array


void PinAtPort::SetVal( unsigned char *adr, bool val) {
    if (val) {
        *adr|=(1<<pinNo);
    } else {
        *adr&=~(1<<pinNo);
    }
}

