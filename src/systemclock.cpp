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

#include "systemclocktypes.h"
#include "systemclock.h"
#include "simulationmember.h"
#include "helper.h"
#include "application.h"
#include "avrdevice.h"
#include "avrerror.h"

#include "signal.h"
#include <assert.h>

using namespace std;

template<typename Key, typename Value>
MinHeap<Key, Value>::MinHeap()
{
    this->reserve(10);  // vector would free&malloc when we keep inserting and removing only 1 element.
}

template<typename Key, typename Value>
void MinHeap<Key, Value>::RemoveMinimum()
{
    assert(!this->empty());
    Key k = this->back().first;
    Value v = this->back().second;
    RemoveMinimumAndInsert(k, v);
    this->pop_back();
}

template<typename Key, typename Value>
bool MinHeap<Key, Value>::ContainsValue(Value v) const
{
    for(unsigned i = 0; i < this->size(); i++)
    {
        std::pair<Key,Value> item = (*this)[i];
        if(item.second == v)
            return true;
    }
    return false;
}

template<typename Key, typename Value>
void MinHeap<Key, Value>::InsertInternal(Key k, Value v, unsigned pos)
{
    for(unsigned i = pos;;) {
        unsigned parent = i/2;
        if(parent == 0 || (*this)[parent-1].first <= k) {
            (*this)[i-1].first = k;
            (*this)[i-1].second = v;
            return;
        }
        Key k_temp = (*this)[parent-1].first;
        Value v_temp = (*this)[parent-1].second;
        (*this)[i-1].first = k_temp;
        (*this)[i-1].second = v_temp;
        i = parent;
    }
}

template<typename Key, typename Value>
void MinHeap<Key, Value>::RemoveAtPositionAndInsertInternal(Key k, Value v, unsigned pos)
{
    assert(pos < this->size());
    unsigned i = pos + 1;
    for(;;) {
        unsigned left = 2*i;
        unsigned right = 2*i + 1;
        unsigned smallest = i;
        if(left-1 < this->size() && (*this)[left-1].first < k)
            smallest = left;
        if(right-1 < this->size() && (*this)[right-1].first < k && (*this)[right-1].first < (*this)[left-1].first)
            smallest = right;
        if(smallest == i) {
            (*this)[smallest-1].first = k;
            (*this)[smallest-1].second = v;
            return;
        }
        Key k_temp = (*this)[smallest-1].first;
        Value v_temp = (*this)[smallest-1].second;
        (*this)[i-1].first = k_temp;
        (*this)[i-1].second = v_temp;
        i = smallest;
    }
}

SystemClock::SystemClock() { 
    static int no = 0;
    currentTime = 0; 
    no++;
    if(no > 1)
        avr_error("Crazy problem: Second instance of SystemClock created!");
}

void SystemClock::SetTraceModeForAllMembers(int trace_on) {
    MinHeap<SystemClockOffset, SimulationMember *>::iterator mi;
    for(mi = syncMembers.begin(); mi != syncMembers.end(); mi++)
    {
        AvrDevice* core = dynamic_cast<AvrDevice*>( mi->second );
        if(core != NULL)
            core->trace_on = trace_on;
    }
} 

void SystemClock::Add(SimulationMember *dev) {
    syncMembers.Insert(currentTime, dev);
}

void SystemClock::AddAsyncMember(SimulationMember *dev) {
    asyncMembers.push_back(dev);
}

volatile bool breakMessage = false;

int SystemClock::Step(bool &untilCoreStepFinished) {
    // 0-> return also if cpu in waitstate 
    // 1-> return if cpu is really finished
    int res = 0; // returns the state from a core step. Needed by gdb-server to
                 // watch for breakpoints

    static vector<SimulationMember*>::iterator ami;
    static vector<SimulationMember*>::iterator amiEnd;

    if(syncMembers.begin() != syncMembers.end()) {
        // take simulation member and current simulation time from time table
        SimulationMember * core = syncMembers.begin()->second;
        currentTime = syncMembers.begin()->first;
        SystemClockOffset nextStepIn_ns = -1;
        
        syncMembers.RemoveMinimum();

        // do a step on simulation member
        int rc = core->Step(untilCoreStepFinished, &nextStepIn_ns);
        if (rc)
            res = rc;

        if(nextStepIn_ns == 0) { // insert the next step behind the following!
            nextStepIn_ns = 1 + (syncMembers.IsEmpty() ? currentTime : syncMembers.front().first);
        } else if(nextStepIn_ns > 0)
            nextStepIn_ns += currentTime;
        // if nextStepIn_ns is < 0, it means, that this simulation member will not
        // be called anymore!
        
        if(nextStepIn_ns > 0)
            syncMembers.Insert(nextStepIn_ns, core);

        // handle async simulation members
        amiEnd = asyncMembers.end();
        for(ami = asyncMembers.begin(); ami != amiEnd; ami++) {
            bool untilCoreStepFinished = false;
            (*ami)->Step(untilCoreStepFinished, 0);
        }
    }

    // honour the stop command
    if (breakMessage)
        return 1;

    return res;
}

void SystemClock::Reschedule(SimulationMember *sm, SystemClockOffset newTime) {

    for(unsigned i = 0; i < syncMembers.size(); i++) {
        if(syncMembers[i].second == sm) {
            syncMembers.RemoveAtPositionAndInsert(newTime+currentTime+1, sm, i);
            return;
        }
    }

    syncMembers.Insert(newTime+currentTime+1, sm);
}

void OnBreak(int s) {
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    breakMessage = true;
}

void SystemClock::Stop() {
    breakMessage = true;
}

void SystemClock::ResetClock(void) {
    breakMessage = false;
    asyncMembers.clear();
    syncMembers.clear();
    currentTime = 0;
}

long SystemClock::Endless() {
    long steps = 0;

    breakMessage = false;        // if we run a second loop, clear break before entering loop
    
    signal(SIGINT, OnBreak);
    signal(SIGTERM, OnBreak);

    while(breakMessage == false) {
        steps++;
        bool untilCoreStepFinished = false;
        Step(untilCoreStepFinished);
    }

    return steps;
}

long SystemClock::Run(SystemClockOffset maxRunTime) {
    long steps = 0;
    
    breakMessage = false;        // if we run a second loop, clear break before entering loop
    
    signal(SIGINT, OnBreak);
    signal(SIGTERM, OnBreak);

    while((breakMessage== false) &&
          (SystemClock::Instance().GetCurrentTime() < maxRunTime)) {
        steps++;
        bool untilCoreStepFinished = false;
        // This breaks at least ATemga644, core->Step() in SystemClock::Step()
        // occasionally returns 1 in normal program flow even without the use
        // of Sleep, Break or whatever causes AvrDevice to return 1.
        //
        //if (Step(untilCoreStepFinished))
        //    break;
        //
        // Breaking commit was d6e3b58358cce6aa35eaf5fcc62c1ff1e139bf06,
        // "patch #7766 Make Step stoppable, print less when used as a library"
        //
        // Let's take the old code (this one line):
        Step(untilCoreStepFinished);
    }

    return steps;
}

long SystemClock::RunTimeRange(SystemClockOffset timeRange) {
    long steps = 0;
    bool untilCoreStepFinished;
    
    breakMessage = false;        // if we run a second loop, clear break before entering loop
    
    signal(SIGINT, OnBreak);
    signal(SIGTERM, OnBreak);
    
    timeRange += SystemClock::Instance().GetCurrentTime();
    while((breakMessage == false) && (SystemClock::Instance().GetCurrentTime() < timeRange)) {
        untilCoreStepFinished = false;
        if (Step(untilCoreStepFinished))
            break;
        steps++;
    }
    
    return steps;
}

SystemClock& SystemClock::Instance() {
    static SystemClock obj;
    return obj;
}
