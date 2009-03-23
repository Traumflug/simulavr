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

#ifndef RWMEM
#define RWMEM

#include "avrdevice.h"
#include <iostream>
#include <fstream>
#include <iostream>

/*
 * All here defined types are used to simulate the 
 * read write address space. This means also registers
 * io-data space, internal and external sram
 */


class RWMemoryMembers{
    protected:
        AvrDevice *core;

    public:
        RWMemoryMembers(AvrDevice *c): core(c) {}

        virtual unsigned char operator=(unsigned char val) =0;
        virtual operator unsigned char() const =0 ;
        void operator=(const RWMemoryMembers &mm);
        virtual ~RWMemoryMembers(){};
};

/* the following class have one byte own memory and can be used for
 * registers sram and registers 
 */
class RWMemoryWithOwnMemory: public RWMemoryMembers {
    protected: 
        unsigned char value;

    public:
        RWMemoryWithOwnMemory(AvrDevice *c):RWMemoryMembers(c) {
            value=0;
        }

        unsigned char operator=(unsigned char val);
        operator unsigned char() const;
};

class AvrDevice;

class CPURegister: public RWMemoryWithOwnMemory {
    unsigned int myNumber;

    public:
    CPURegister(AvrDevice *c, unsigned int number): RWMemoryWithOwnMemory(c), myNumber(number){}

    unsigned char operator=(unsigned char val);
    operator unsigned char() const;
};


class IRam: public RWMemoryWithOwnMemory {
    unsigned int myAddress;
    public:
    IRam(AvrDevice *c, unsigned int number):RWMemoryWithOwnMemory(c), myAddress(number) { }
    unsigned char operator=(unsigned char val); 
    operator unsigned char() const;
};

//TODO this Ram must be connected to the special io register for controlling ext ram!
class ERam: public RWMemoryWithOwnMemory {
    unsigned int myAddress;
    public:
    ERam(AvrDevice *c, unsigned int number): RWMemoryWithOwnMemory(c), myAddress(number) { }
    unsigned char operator=(unsigned char val);
};

class NotAvailableIo: public RWMemoryMembers {
    unsigned int myAddress;
    public:
    NotAvailableIo(AvrDevice* c, unsigned int number):RWMemoryMembers(c), myAddress(number) { }

    unsigned char operator=(unsigned char val); 
    operator unsigned char() const;
};

class RWReserved: public RWMemoryMembers {
    unsigned int myAddress;
    public:
        RWReserved(AvrDevice* c, unsigned int number):RWMemoryMembers(c), myAddress(number) { }
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};



class MemoryOffsets {
    protected:
        unsigned int myOffset;
        RWMemoryMembers **rwHandler;

    public:
        MemoryOffsets(unsigned int offset, RWMemoryMembers **rw):rwHandler(rw){
            myOffset=offset;
        }

        RWMemoryMembers &operator[](unsigned int externOffset) const;


};


//;-------------------------------------------------------
#include <fstream>
using namespace std;
class RWWriteToPipe: public RWMemoryMembers {
    protected:
        ofstream ofs;
        ostream &os;
        string pipeName;

    public:
        RWWriteToPipe(AvrDevice *c, const string &name)
           : RWMemoryMembers(c), os((name=="-")?std::cout:ofs), pipeName(name) 
         {
         if( name != "-" )ofs.open(name.c_str());
         }
        virtual ~RWWriteToPipe() {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

//We need a ifstream pointer because all the "virtual operator unsigned char" functions are defined const.
//if "is" is member not pointer a read from "is" will modify the object which is not "const".
//To solve this problem we handle only a pointer to a file.... this is not really a const call, I know!

class RWReadFromPipe: public RWMemoryMembers {
    protected:
        mutable istream &is;
        mutable ifstream ifs;
        string pipeName;

    public:
        RWReadFromPipe(AvrDevice *c, const string &name)
           : RWMemoryMembers(c), is((name=="-")?std::cin:ifs), pipeName(name) 
         {
         if( name != "-" )ifs.open(name.c_str());
         }
        virtual ~RWReadFromPipe() {}
        virtual unsigned char operator=(unsigned char) ;
        virtual operator unsigned char() const;
};


// Exit the simulator magic address 
class RWExit: public RWMemoryMembers {

    public:
        RWExit(AvrDevice *c)
          : RWMemoryMembers(c) {}

        virtual ~RWExit() {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

// Abort the simulator magic address 
class RWAbort: public RWMemoryMembers {

    public:
        RWAbort(AvrDevice *c)
          : RWMemoryMembers(c) {}

        virtual ~RWAbort() {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

#endif
