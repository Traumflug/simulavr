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

#include "systemclocktypes.h"
#include "systemclock.h"
#include "simulationmember.h"
#include "helper.h"
#include "application.h"
#include "avrerror.h"

#include "signal.h"
#include <assert.h>

using namespace std;


template<typename Key, typename Value>
MinHeap<Key, Value>::MinHeap()
{
	reserve(10);  // vector would free&malloc when we keep inserting and removing only 1 element.
}

template<typename Key, typename Value>
void MinHeap<Key, Value>::RemoveMinimum()
{
	assert(!empty());
	Key k = back().first;
	Value v = back().second;
	RemoveMinimumAndInsert(k, v);
	pop_back();
}

template<typename Key, typename Value>
bool MinHeap<Key, Value>::ContainsValue(Value v) const
{
	for(unsigned i = 0; i < size(); i++)
	{
		std::pair<Key,Value> item = (*this)[i];
		if(item.second == v)
			return true;
	}
	return false;
}

template<typename Key, typename Value>
void MinHeap<Key, Value>::Insert(Key k, Value v)
{
	resize(size()+1);
	for(unsigned i = size();;) {
		unsigned parent = i/2;
		if(parent == 0 || (*this)[parent-1].first < k) {
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
void MinHeap<Key, Value>::RemoveMinimumAndInsert(Key k, Value v)
{
	assert(!empty());
	unsigned i = 1;
	for(;;) {
		unsigned left = 2*i;
		unsigned right = 2*i + 1;
		unsigned smallest = i;
		if(left-1 < size() && (*this)[left-1].first < k)
			smallest = left;
		if(right-1 < size() && (*this)[right-1].first < k)
			smallest = right;
		if(smallest == i) {
			(*this)[smallest-1].first = k;
			(*this)[smallest-1].second = v;
			return;
		}
		Key k_temp = (*this)[smallest-1].first;
		Value v_temp = (*this)[smallest-1].second;
		(*this)[smallest-1].first = k;
		(*this)[smallest-1].second = v;
		k = k_temp;
		v = v_temp;
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
    for(auto mi = syncMembers.begin(); mi != syncMembers.end(); mi++)
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

int SystemClock::Step(bool &untilCoreStepFinished) {
    //0-> return also if cpu in waitstate 1-> return if cpu is really finished
    int res = 0; // returns the state from a core step. Needed by gdb-server to
                 // watch for breakpoints

    static vector<SimulationMember*>::iterator ami;
    static vector<SimulationMember*>::iterator amiEnd;

    if(syncMembers.begin() != syncMembers.end()) {
        // take simulation member and current simulation time from time table
        SimulationMember * core = syncMembers.begin()->second;
        currentTime = syncMembers.begin()->first;
        SystemClockOffset nextStepIn_ns = -1;
        
        // do a step on simulation member
        res = core->Step(untilCoreStepFinished, &nextStepIn_ns);
        
        if(nextStepIn_ns == 0) { // insert the next step behind the following!
            nextStepIn_ns = 1 + (syncMembers.IsEmpty() ? currentTime : syncMembers.front().first);
        } else if(nextStepIn_ns > 0)
            nextStepIn_ns += currentTime;
        // if nextStepIn_ns is < 0, it means, that this simulation member will not
        // be called anymore!
        
        if(nextStepIn_ns > 0)
            syncMembers.RemoveMinimumAndInsert(nextStepIn_ns, core);
		else
			syncMembers.RemoveMinimum();

        // handle async simulation members
        amiEnd = asyncMembers.end();
        for(ami = asyncMembers.begin(); ami != amiEnd; ami++) {
            bool untilCoreStepFinished = false;
            (*ami)->Step(untilCoreStepFinished, 0);
        }
    }

    return res;
}

void SystemClock::Rescedule(SimulationMember *sm, SystemClockOffset newTime) {
    MinHeap<SystemClockOffset, SimulationMember *>::iterator ii;

    for(ii=syncMembers.begin(); ii != syncMembers.end(); ii++) {
        if(ii->second == sm) {
            syncMembers.erase(ii); 
            break;
        }
    }

    syncMembers.Insert(newTime+currentTime+1, sm);
}

volatile int breakMessage = false;

void OnBreak(int s) {
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    breakMessage = true;
}

void SystemClock::stop() {
    breakMessage = true;
}

void SystemClock::ResetClock(void) {
    asyncMembers.clear();
    syncMembers.clear();
    currentTime = 0;
}

void SystemClock::Endless() {
    int steps = 0;
    
    signal(SIGINT, OnBreak);
    signal(SIGTERM, OnBreak);

    while(breakMessage == false) {
        steps++;
        bool untilCoreStepFinished = false;
        Step(untilCoreStepFinished);
    }

    cout << "SystemClock::Endless stopped" << endl;
    cout << "number of cpu cycles simulated: " << dec << steps << endl;
    Application::GetInstance()->PrintResults();
}


void SystemClock::Run(SystemClockOffset maxRunTime) {
    int steps = 0;
    
    signal(SIGINT, OnBreak);
    signal(SIGTERM, OnBreak);

    while((breakMessage== false) &&
          (SystemClock::Instance().GetCurrentTime() < maxRunTime)) {
        steps++;
        bool untilCoreStepFinished =false;
        Step(untilCoreStepFinished);
    }

    cout << endl << "Ran too long.  Terminated after " << maxRunTime;
    cout << " simulated nanoseconds." << endl;

    Application::GetInstance()->PrintResults();
}

int SystemClock::RunTimeRange(SystemClockOffset timeRange) {
    int res = 0;
    bool untilCoreStepFinished;
    
    signal(SIGINT, OnBreak);
    signal(SIGTERM, OnBreak);
    
    timeRange += SystemClock::Instance().GetCurrentTime();
    while((breakMessage == false) && (SystemClock::Instance().GetCurrentTime() < timeRange)) {
        untilCoreStepFinished = false;
        res = Step(untilCoreStepFinished);
        if(res != 0)
            break;
    }
    
    return res;
}

SystemClock& SystemClock::Instance() {
    static SystemClock obj;
    return obj;
}

