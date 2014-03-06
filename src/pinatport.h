/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003 Klaus Rudolph
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

#ifndef PINATPORT
#define PINATPORT

class HWPort;
class Pin;

class PinAtPort {
    protected:
        HWPort *port;
        unsigned char pinNo;

    public:
        PinAtPort(); //dummy only used by admux !!!!!
        PinAtPort( HWPort *p, unsigned char pn);
        void SetPort(bool val);
        void SetDdr(bool val); 

        void SetAlternateDdr(bool val);
        void SetUseAlternateDdr(bool val);

        void SetAlternatePort(bool val); 
        void SetUseAlternatePort(bool val); 

        void SetUseAlternatePortIfDdrSet(bool val);

        bool GetPort();
        bool GetDdr(); 
        bool GetAlternateDdr();
        bool GetUseAlterateDdr(); 
        bool GetAlternatePort(); 
        bool GetUseAlternatePort(); 
        bool GetUseAlternatePortIfDdrSet(); 
        Pin& GetPin();

        operator bool(); 
        float GetAnalogValue(float vcc); //!< Get pin analog voltage level

    protected:
        void SetVal( unsigned char *adr, bool val);
};

#endif
