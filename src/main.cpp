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
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
//#include <signal.h>

#include <map>

#include <sstream>
#include <iostream>
#include <string>
using namespace std;

#include "config.h"

#include "global.h"
#include "avrdevice.h"
#include "at8515.h"
#include "at8515special.h"
#include "atmega128.h"
#include "at4433.h"
#include "gdb.h"
#include "ui.h"
#include "systemclock.h"
#include "lcd.h"
#include "keyboard.h"
#include "trace.h"
#include "scope.h"

#define PRG_WISH "/usr/bin/wish"

//test only!

int main(int argc, char *argv[]) {
    int c;
    bool gdbserver_flag=0;
    string filename("unknown");
    string devicename("unknown");
    string tracefilename("unknown");
    int global_gdbserver_port    = 1212;
    int global_gdb_debug         = 0;
    bool globalWaitForGdbConnection=true; //please wait for gdb connection
    int userinterface_flag=0;
    unsigned long long fcpu=4000000;
    UserInterface *ui;

    global_trace_on=0;


    while (1) {
        //int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"file", 1, 0, 'f'},
            {"device", 1, 0, 'd'},
            {"gdbserver", 1, 0, 'g'},
            {"trace", 1, 0, 't'},
            {"version",0,0,'v'},
            {"nogdbwait",0,0,'n'},
            {"cpufrequence", 1,0,'F'},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, "f:d:gGd:p:t:uxyzhvniF:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'F':
                fcpu=strtoll(optarg, NULL, 10);
                cout << "Running with CPU frequence: " << fcpu<<endl;
                break;

            case 'u':
                cout << "Run with User Interface at Port 7777" << endl;
                userinterface_flag=1;
                break;

            case 'f':
                cout << "File to load " << optarg << endl;
                filename=optarg;
                break;

            case 'd':
                cout << "Device to simulate " << optarg << endl;
                devicename=optarg;
                break;

            case 'g':
                cout << "Running as gdb-server" << endl;
                gdbserver_flag=1;
                break;

            case 'G':
                cout << "Running with debug informations from gdbserver" << endl;
                global_gdb_debug = 1;
                gdbserver_flag=1;
                break;

            case 'p':
                cout << "Running NOT on default port, use instead: " << global_gdbserver_port << endl;
                global_gdbserver_port    = atoi(optarg);
                break;

            case 't':
                cout << "Running in Trace Mode" << endl;
                StartTrace(optarg);
                break;

            case 'v':
                {
                    cout << "Simulavr++ " << VERSION << endl;
                    cout << "See documentation for copyright and distribution terms" << endl;
                    cout << endl;
                    exit(0);
                }
                break;

            case 'n':
                { 
                    cout << "We will NOT wait for a gdb connection, simulation starts now!" << endl;
                    globalWaitForGdbConnection=false;
                }
                break;


            default:
                cout << "AVR-Simulator" << endl;
                cout << "-u                           run with user interface for external pin handling at port 7777" << endl;
                cout << "-f --file <name>             load elf-file <name> for simulation in simulated target" << endl; 
                cout << "-d --device <device name>    simulate <device name> " << endl;
                cout << "-g --gdbserver               running as gdb-server" << endl;
                cout << "-G                           running as gdb-server and write debug info for gdb-connection" << endl;                             
                cout << "-p  <port>                   change <port> for gdb server to port" << endl;
                cout << "-t --trace <file name>       enable trace outputs into <file name>" << endl;
                cout << "-n --nogdbwait               do not wait for gdb connection" << endl;
                cout << "-F --cpufrequence            set the cpu frequence to <Hz> " << endl;
                cout << endl;
                cout << "Supported devices:" << endl;
                cout << "at90s8515" << endl;
                cout << "at90s4433" << endl;
                cout << "atmega128" << endl;
                cout << endl;

                exit(0);

        }
    }

    /* now we create the device */
    AvrDevice *dev1;
    if (devicename=="unknown") {
        dev1= new AvrDevice_at90s8515;
    } else if (devicename=="at90s8515") {
        dev1= new AvrDevice_at90s8515;
    } else if (devicename=="atmega128") {
        dev1= new AvrDevice_atmega128;
    } else if (devicename=="at90s4433") {
        dev1= new AvrDevice_at90s4433;
    } else {
        cerr << "Not a valid device" << endl;
        exit(0);
    }



    if (filename != "unknown" ) {
        dev1->Load(filename.c_str());
    }

    if (userinterface_flag==1) {
        ui=new UserInterface(7777); //if not gdb, the ui will be master controller :-)
    }
    
    //dev1->SetClockFreq(250); //4 Mhz for dummy
    dev1->SetClockFreq(1000000000/fcpu);

    if (global_trace_on) dev1->trace_on=1;

    if (gdbserver_flag==0) {
        SystemClock::Instance().Add(dev1);
        while (1) { //for ever
            bool untilCoreStepFinished=false;
            SystemClock::Instance().Step(untilCoreStepFinished);
        }
    } else { //gdb should be activated
        cout << "Going to gdb..." << endl;
        GdbServer gdb1(dev1, global_gdbserver_port, global_gdb_debug, globalWaitForGdbConnection);
        SystemClock::Instance().Add(&gdb1);
        SystemClock::Instance().Endless();
    } 
}

