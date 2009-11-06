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
    for (mi=begin(); mi!=end(); mi++) {
        mi->second->trace_on=trace_on;
    }
} 


void SystemClock::Add(SimulationMember *dev) {
    insert( pair<SystemClockOffset, SimulationMember*>(currentTime,dev));
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

int SystemClock::Step(bool &untilCoreStepFinished) { //0-> return also if cpu in waitstate 1-> return if cpu is really finished
    int res=0; //returns the state from a core step. Needed by gdb-server to wathc for breakpoints
    SystemClockOffset nextStepIn_ns;

    static vector<SimulationMember*>::iterator ami;
    static vector<SimulationMember*>::iterator amiEnd;

    SimulationMember *core;
    if (begin()!=end()) {
        core=begin()->second;
        currentTime=begin()->first; 
        erase(begin());
        if (core->trace_on) traceOut << DecLong(currentTime)<<" ";
        res=core->Step(untilCoreStepFinished, &nextStepIn_ns);
        if (nextStepIn_ns==0) { //insert the next step behind the following!
            SystemClock::iterator ii=begin();
            //ii++;
            if (ii  != end() ) {
                nextStepIn_ns=1+ (ii->first); 
            }
        } else if(nextStepIn_ns>0) {
            nextStepIn_ns+=currentTime;
        }
        //erase(begin());

        //if nextStepIn_ns is < 0 remove the entry from the list and do not reenter it 
        if (nextStepIn_ns>0) {
            insert (pair<SystemClockOffset, SimulationMember*>(nextStepIn_ns, core));
        } 

        amiEnd= asyncMembers.end();
        for (ami= asyncMembers.begin(); ami!=amiEnd ; ami++) {
            bool untilCoreStepFinished=false;
            (*ami)->Step(untilCoreStepFinished,0);
        }
    }

    return res;
}

void SystemClock::Rescedule( SimulationMember *sm, SystemClockOffset newTime) {
    iterator ii;

    for ( ii=begin(); ii!=end(); ii++) {
        if ( ii->second== sm) {
            erase(ii); 
            break;
        }
    }

    insert (pair<SystemClockOffset, SimulationMember*>(newTime+currentTime+1, sm));
}



volatile int breakMessage=0;

void OnBreak(int s) 
{
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    breakMessage=1;
}

void SystemClock::stop() {
    breakMessage=true;
}

void SystemClock::Endless() {
    int steps=0;
    signal(SIGINT, OnBreak);
    signal(SIGTERM, OnBreak);

    while( breakMessage==0) {
        steps++;
        bool untilCoreStepFinished=false;
        Step(untilCoreStepFinished);
    }

    cout << "SystemClock::Endless stopped" << endl;
    cout << "number of cpu cycles simulated: " << dec << steps << endl;
    Application::GetInstance()->PrintResults();
}


void SystemClock::Run(SystemClockOffset maxRunTime) {
    int steps=0;
    signal(SIGINT, OnBreak);
    signal(SIGTERM, OnBreak);

    while( breakMessage==0 && (SystemClock::Instance().GetCurrentTime() < maxRunTime)) {
        steps++;
        bool untilCoreStepFinished=false;
        Step(untilCoreStepFinished);
    }

    cout << endl << "Ran too long.  Terminated after " << maxRunTime << " simulated nanoseconds." << endl;

    Application::GetInstance()->PrintResults();
}

SystemClock& SystemClock::Instance() {
    static SystemClock obj;
    return obj;
}



SystemClockOffset SystemClock::GetCurrentTime() { return currentTime; }

