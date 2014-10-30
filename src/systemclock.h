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

#ifndef SYSTEMCLOCK
#define SYSTEMCLOCK

#include <map>
#include <vector>

#include "systemclocktypes.h"

class SimulationMember;

/** A heap data structure optimized for obtaining Value of the smallest Key.
    Example MinHeap<SystemClockOffset, SimulationMember*>. */
template<typename Key, typename Value>
class MinHeap : public std::vector<std::pair<Key,Value> >
{
public:
    MinHeap();
    bool IsEmpty() const { return this->empty(); }
    Key GetMinimumKey() const { return this->front().first; }
    Value GetMinimumValue() const { return this->front().second; };
    void RemoveMinimum();
    bool ContainsValue(Value v) const;
    void Insert(Key k, Value v) {
        this->resize(this->size()+1);
        InsertInternal(k, v, this->size());
    }
    void RemoveMinimumAndInsert(Key k, Value v) {
        RemoveAtPositionAndInsertInternal(k, v, 0);
    }
    void RemoveAtPositionAndInsert(Key k, Value v, unsigned pos) {
        if(k < (*this)[pos-1].first)
            InsertInternal(k, v, pos);
        else
            RemoveAtPositionAndInsertInternal(k, v, pos);
    }
protected:
    // These are internal because a bad value of `pos' could violate the binary heap invariant.
    void InsertInternal(Key k, Value v, unsigned pos);
    void RemoveAtPositionAndInsertInternal(Key k, Value v, unsigned pos);
};

//! Class to store and manage the central simulation time
/*! This acts as a time table, a simulation member gets a place on this ordered
    table, where it should be called next time, the placement depends on the
    results of Step method call Step on this simulation member.
    
    \todo In multiple core simulations which uses also gdb with single stepping
    we need a other solution to fit the time accurate behaviour. Currently on a
    single step from gdb the simulation runs until the command is completly
    executed which is NOT correct. Some commands need up to 4 cycles and the
    actual implementation do up to 4 steps for one step so the other cores run
    slower then in normal operation. This is not a problem today because we are
    not able to run multiple cores with gdb but this will be implementated later.
    So this version is only made for running the regression tests and stepping
    in gdb. Normal operation/simulation is not affected. (taken over from
    systemclock.cpp, but to check!) */
class SystemClock
{
    private:
        SystemClock(); //!< Do not this constructor from application code!
        SystemClock(const SystemClock &); //!< Do not this constructor from application code!

    protected:
        SystemClockOffset currentTime;  //!< time in [ns] since start of simulation
        MinHeap<SystemClockOffset, SimulationMember *> syncMembers;  //!< earliest first
        std::vector<SimulationMember*> asyncMembers; //!< List of asynchron working simulation members, will be called every step!
        
    public:
        //! Returns the current simulation time
        SystemClockOffset GetCurrentTime() const { return currentTime; }
        //! Set the simulation time to a dedicated value
        /*! Attention! Use this method with care, if you don't want crazy results */
        void SetCurrentTime(SystemClockOffset of) { currentTime = of; }
        //! Increments the current simulation time with a offset
        /*! Attention! Use this method with care, if you don't want crazy results */
        void IncrTime(SystemClockOffset of) { currentTime += of; }
        //! Add a simulation member (normally a device)
        void Add(SimulationMember *dev);
        //! Add a async simulation member, this will be called every simulation step.
        void AddAsyncMember(SimulationMember *dev);
        //! Process one simulation step
        int Step(bool &untilCoreStepFinished);
        //! Run simulation endless till SIGINT or SIGTERM signal, return the number of CPU cycles
        long Endless();
        //! Run simulation till given time is arrived or signal is cached
        long Run(SystemClockOffset maxRunTime);
        //! Like Run method, but stops on breakpoint or after given time offset
        long RunTimeRange(SystemClockOffset timeRange);
        //! Returns the central SystemClock instance for the application
        /*! There will be only one instance on a application! */
        static SystemClock& Instance();
        //! Moves the given simulation member to a new place in time table
        /*! The next time, simulation member will be called, is calculated as a
            given offset to current simulation time + 1.
            
            \todo This method is possibly obsolete! */
        void Reschedule(SimulationMember *sm, SystemClockOffset newTime);
        //! Switches trace mode for all current found simulation members
        void SetTraceModeForAllMembers(int trace_on);
        //! Stop Run/Endless or Step asynchronously
        void Stop();
        //! Resets the simulation time and clears table for simulation members and async simulation members
        void ResetClock(void);
};

#endif
