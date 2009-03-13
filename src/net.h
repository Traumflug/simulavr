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

#ifndef NET
#define NET

#include <vector>
using namespace std;

//class Pin;
#include "pin.h"

class NetInterface {
    public:
        virtual bool CalcNet()=0;
        virtual ~NetInterface();

    private:
        // this function could delete the Net itself
        // if the net is a MirrorNet. Because of this
        // not really "normal" behaviour we have to take
        // care that this function is only used if the
        // user knows about this wanted "side effect"
        // Only allow Pin::RegisterNet to use the Delete 
        // function. Pin::RegisterNet has signed a
        // special agreemend with me :-)
        virtual void Delete(Pin*)=0; 

        friend void Pin::RegisterNet(Net*);
};

class Net:
#ifndef SWIG
    public vector <Pin *>,
#endif
    public NetInterface 
{
    public:
        void Add(Pin *p);
        void Delete(Pin *p);
        bool CalcNet();
        ~Net();
};

class MirrorNet: public NetInterface {
    protected:
        Pin *p;

    public:
        MirrorNet(Pin *p);
        bool CalcNet(); //only give in out-state to in-state  
        void Delete(Pin *p);
    private: //Do never allow that MirrorNet becomes an automatic object,
             //because Delete kills the this pointer which is never possible
             //for auto objects
        ~MirrorNet();
};
#endif
