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

using namespace std;

SystemClock::SystemClock() { 
    static int no = 0;
    currentTime = 0; 
    no++;
    if(no > 1)
        avr_error("Crazy problem: Second instance of SystemClock created!");
}

void SystemClock::SetTraceModeForAllMembers(int trace_on) {
    iterator mi;
    for(mi = begin(); mi != end(); mi++)
        mi->second->trace_on = trace_on;
} 


void SystemClock::Add(SimulationMember *dev) {
    insert(pair<SystemClockOffset, SimulationMember*>(currentTime, dev));
}

void SystemClock::AddAsyncMember(SimulationMember *dev) {
    asyncMembers.push_back(dev);
}

int SystemClock::Step(bool &untilCoreStepFinished) {
    //0-> return also if cpu in waitstate 1-> return if cpu is really finished
    int res = 0; // returns the state from a core step. Needed by gdb-server to
                 // watch for breakpoints
    SystemClockOffset nextStepIn_ns = -1;

    static vector<SimulationMember*>::iterator ami;
    static vector<SimulationMember*>::iterator amiEnd;

    SimulationMember *core;
    
    if(begin() != end()) {
        // take simulation member and current simulation time from time table
        core = begin()->second;
        currentTime = begin()->first; 
        erase(begin());
        
        if(core->trace_on)
            traceOut << DecLong(currentTime) << " ";
        
        // do a step on simulation member
        res = core->Step(untilCoreStepFinished, &nextStepIn_ns);
        
        
        if(nextStepIn_ns == 0) { // insert the next step behind the following!
            SystemClock::iterator ii = begin();
            nextStepIn_ns = 1 + ((ii != end()) ? ii->first : currentTime);
        } else if(nextStepIn_ns > 0) // or on offset, given back from Step call
            nextStepIn_ns += currentTime;
        // if nextStepIn_ns is < 0, it means, that this simulation member will not
        // be called anymore!
        
        if(nextStepIn_ns > 0)
            insert(pair<SystemClockOffset, SimulationMember*>(nextStepIn_ns, core));

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
    iterator ii;

    for(ii=begin(); ii != end(); ii++) {
        if(ii->second == sm) {
            erase(ii); 
            break;
        }
    }

    insert(pair<SystemClockOffset, SimulationMember*>(newTime+currentTime+1, sm));
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
    clear();
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

