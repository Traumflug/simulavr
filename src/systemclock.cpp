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
#include "systemclock.h"
#include "simulationmember.h"
#include "helper.h"
#include "trace.h"
#include "application.h"

#include "signal.h"

SystemClock::SystemClock() { currentTime=0; }

void SystemClock::Add(SimulationMember *dev) {
    insert( pair<unsigned long long, SimulationMember*>(0,dev));
}

void SystemClock::AddAsyncMember( SimulationMember *dev) {
    asyncMembers.push_back(dev);
}

//TODO
//In multiple core simulations which uses also gdb with single stepping we need a other solution to fit
//the time accurate behaviour. Currently on a single step from gdb the simulation runs until the
//command is completly executed which is not! correct. Some commands need up to 4 cycles and teh actual implementation
//do up to 4 steps for one step so the other cores run slower then in normal operation. 
//this is not a problem today because we are not able to run multiple cores with gdb but this will
//be implementated later.
//So this version is only made for running the regression tests and stepping in gdb. Normal operation/simulation
//is not affected.

int SystemClock::Step(int untilCoreStepFinished) { //0-> return also if cpu in waitstate 1-> return if cpu is really finished
    int res=0; //returns the state from a core step. Needed by gdb-server to wathc for breakpoints
    unsigned long long nextStepIn_ns;

    static vector<SimulationMember*>::iterator ami;
    static vector<SimulationMember*>::iterator amiEnd;

    SimulationMember *core;
    if (begin()!=end()) {
        core=begin()->second;
        currentTime=begin()->first; 
        if (trace_on) traceOut << DecLong(currentTime)<<" ";
        res=core->Step(untilCoreStepFinished, &nextStepIn_ns);
        if (nextStepIn_ns==0) { //insert the next step behind the following!
            SystemClock::iterator ii=begin();
            ii++;
            if (ii  != end() ) {
                nextStepIn_ns=1+ (ii->first); 
            }
        } else {
            nextStepIn_ns+=currentTime;
        }
        erase(begin());
        insert (pair<unsigned long long, SimulationMember*>(nextStepIn_ns, core));

        amiEnd= asyncMembers.end();
        for (ami= asyncMembers.begin(); ami!=amiEnd ; ami++) {
            (*ami)->Step(0,0);
        }
    }

    return res;
}

void SystemClock::Rescedule( SimulationMember *sm, unsigned long long newTime) {
    iterator ii;

    for ( ii=begin(); ii!=end(); ii++) {
        if ( ii->second== sm) {
            erase(ii); 
            break;
        }
    }

    insert (pair<unsigned long long, SimulationMember*>(newTime+currentTime+1, sm));
}



int breakMessage=0;

void OnBreak(int s) 
{
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    breakMessage=1;
}



void SystemClock::Endless() {
    int steps=0;
    signal(SIGINT, OnBreak);
    signal(SIGTERM, OnBreak);

#ifdef PROF
    for (unsigned long long tt=0; tt<1000000LL; tt++) {
        Step(0);
        if (breakMessage!=0) break;
    }
#else
    cout << "normal loop" << endl;
    while( breakMessage==0) {
        steps++;
        Step(0);
    }
#endif
    cout << "SystemClock::Endless stopped" << endl;
    cout << "number of cpu cycles simulated: " << dec << steps << endl;
    Application::GetInstance()->PrintResults();
}

SystemClock& SystemClock::Instance() {
    static SystemClock obj;
    return obj;
}



unsigned long long SystemClock::GetCurrentTime() { return currentTime; }

