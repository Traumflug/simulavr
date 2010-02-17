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

#ifndef SYSTEMCLOCK
#define SYSTEMCLOCK
#include <map>
#include <vector>

#include "avrdevice.h"
#include "systemclocktypes.h"

class SystemClock
#ifndef SWIG
: public std::multimap<SystemClockOffset, SimulationMember *> 
#endif
{
    private:
        SystemClock(); //never !
        SystemClock(const SystemClock &);

    protected:
        SystemClockOffset currentTime; ///< time in [ns] since start of simulation
        std::vector<SimulationMember*> asyncMembers;
        
    public:
        SystemClockOffset GetCurrentTime() const { return currentTime; }
        void IncrTime(SystemClockOffset of) { currentTime+= of; }
        void Add(SimulationMember *dev);
        void AddAsyncMember(SimulationMember *dev);
        int Step(bool &untilCoreStepFinished);
        void Endless();
        void Run(SystemClockOffset maxRunTime);
        int RunTimeRange(SystemClockOffset timeRange);
        static SystemClock& Instance();
        void Rescedule( SimulationMember *sm, SystemClockOffset newTime);
        void SetTraceModeForAllMembers(int trace_on);
        void stop();
        void ResetClock(void);
};

#endif
