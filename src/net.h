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
#ifndef NET
#define NET

#include <vector>
using namespace std;

class Pin;

class NetInterface {
    public:
    virtual bool CalcNet()=0;
    virtual void Delete(Pin*)=0;
    virtual ~NetInterface();
};

class Net: public vector <Pin *>, public NetInterface  {
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
    ~MirrorNet();
};
#endif
