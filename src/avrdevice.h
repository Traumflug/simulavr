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
#ifndef AVRDEVICE
#define AVRDEVICE

#include "simulationmember.h"
#include "pin.h"
#include "net.h"
#include "breakpoint.h"

#include <string>
#include <map>

class AvrFlash;
class HWEeprom;
class HWStack;
class HWWado;
class HWSreg;
class Data;
class HWIrqSystem;
class MemoryOffsets;
class RWMemoryMembers;
class Hardware;

using namespace std;

class AvrDevice: public SimulationMember {
    protected:
        unsigned long clockFreq;
        string actualFilename;
        map < string, Pin *> allPins; 

        //old static vars for Step()
        int cpuCycles;
        unsigned int newIrqPc;
        int noDirectIrqJump;

    public:
        Breakpoints BP;
        word PC;
        int PC_size;
        AvrFlash *Flash;
        HWEeprom *eeprom;
        Data *data;
        HWIrqSystem *irqSystem;

        //RWMemory *rwmem;
        MemoryOffsets *Sram;
        MemoryOffsets *R;
        MemoryOffsets *ioreg;
        RWMemoryMembers **rw;

        HWStack *stack;
        HWSreg *status;
        HWWado *wado;

        vector<Hardware *> hwResetList; 
        vector<Hardware *> hwCycleList; 

        void AddToResetList(Hardware *hw){ hwResetList.push_back(hw) ; }
        void AddToCycleList(Hardware *hw){ hwCycleList.push_back(hw) ; }
        void RemoveFromCycleList(Hardware *hw);
        void Load(const char* n);

        Pin *GetPin(const char *name);

        MemoryOffsets &operator->*(MemoryOffsets *m) { return *m;}

        AvrDevice(unsigned int flashSize, unsigned int IRamSize, unsigned int ERamSize, unsigned int IoSpaceSize);
        int Step(bool, unsigned long long *nextStepIn_ns =0);
        void Reset();
        void SetClockFreq(unsigned long f);

        virtual ~AvrDevice();
        virtual unsigned char GetRampz();
        virtual void SetRampz(unsigned char);

        void RegisterPin(const string &name, Pin *p) {
            allPins.insert(pair<string, Pin*>(name, p));
        }
};


#endif
