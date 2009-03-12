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
#include "flash.h"
#include "avrdevice.h"
#include "avrfactory.h"
#include "gdb.h"
#include "ui.h"
#include "systemclock.h"
#include "lcd.h"
#include "keyboard.h"
#include "trace.h"
#include "scope.h"

const char Usage[] = 
"AVR-Simulator Version " VERSION "\n"
"-u                    run with user interface for external pin\n"
"                      handling at port 7777\n"
"-f --file <name>      load elf-file <name> for simulation in simulated target\n"
"-d --device <name>    simulate device <name> \n"
"-g --gdbserver        run as gdb-server\n"
"-G                    run as gdb-server and write debug info for gdb-connection\n"
"-m  <nanoseconds>     maximum run time of <nanoseconds>\n"
"-p  <port>            use <port> for gdb server\n"
"-t --trace <file>     enable trace outputs to <file>\n"
"-n --nogdbwait        do not wait for gdb connection\n"
"-F --cpufrequency     set the cpu frequency to <Hz> \n"
"-W --writetopipe <offset>,<file>\n"
"                      add a special pipe register to device at\n"
"                      IO-Offset and opens <file> for writing\n"
"-R --readfrompipe <offset>,<file>\n"
"                      add a special pipe register to device at IO-offset\n"
"                      and opens <file> for reading\n"
"-V --verbose          output some hints to console\n"
"-T --terminate <label> or <address>\n"
"                      stops simulation if PC runs on <label> or <address>\n"
"-B --breakpoint <label> or <address>\n"
"                      same as -T for backward compatibility\n"
"\n"
"Supported devices:\n"
"  at90s4433\n"
"  at90s8515\n"
"  atmega128\n"
"\n";

            
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
   unsigned long long maxRunTime=0;
   UserInterface *ui;

   global_trace_on=0;

   unsigned int writeToPipeOffset=0x20;
   unsigned int readFromPipeOffset=0x21;
   string readFromPipeFileName="";
   string writeToPipeFileName="";

   vector<string> terminationArgs;

   while (1) {
      //int this_option_optind = optind ? optind : 1;
      int option_index = 0;
      char *dummy;
      static struct option long_options[] = {
         {"file", 1, 0, 'f'},
         {"device", 1, 0, 'd'},
         {"gdbserver", 1, 0, 'g'},
         {"maxruntime", 1,0,'m'},
         {"nogdbwait",0,0,'n'},
         {"trace", 1, 0, 't'},
         {"version",0,0,'v'},
         {"cpufrequency", 1,0,'F'},
         {"readfrompipe", 1,0,'R'},
         {"writetopipe", 1,0,'W'},
         {"verbose", 0,0,'V'},
         {"terminate",1,0,'T'},
         {"breakpoint",1,0,'B'},
         {0, 0, 0, 0}
      };

      c = getopt_long (argc, argv, "f:d:gGm:p:t:uxyzhvniF:R:W:VT:B:", long_options, &option_index);
      if (c == -1)
         break;

      switch (c) {
         case 'B':
         case 'T':
            terminationArgs.push_back(optarg);
            break;

         case 'V':
            global_verbose_on=1;
            break;

         case 'R': //read from pipe 
            readFromPipeOffset=strtoul( optarg, &dummy, 16);
            dummy++; //position behind the "," or any other delimiter for the offset
            readFromPipeFileName=dummy;
            break;

         case 'W': //write to pipe
            writeToPipeOffset=strtoul( optarg, &dummy, 16);
            dummy++; //position behind the "," or any other delimiter for the offset
            writeToPipeFileName=dummy;
            break;

         case 'F':
            fcpu=strtoll(optarg, NULL, 10);
            if (global_verbose_on) cout << "Running with CPU frequency: " << fcpu << endl;
            break;

         case 'm':
            maxRunTime=strtoll(optarg, NULL, 10);
            if (global_verbose_on)
               cout << "Maximum Run Time: " << maxRunTime << endl;
            break;

         case 'u':
            if (global_verbose_on) cout << "Run with User Interface at Port 7777" << endl;
            userinterface_flag=1;
            break;

         case 'f':
            if (global_verbose_on) cout << "File to load: " << optarg << endl;
            filename=optarg;
            break;

         case 'd':
            if (global_verbose_on) cout << "Device to simulate: " << optarg << endl;
            devicename=optarg;
            break;

         case 'g':
            if (global_verbose_on) cout << "Running as gdb-server" << endl;
            gdbserver_flag=1;
            break;

         case 'G':
            if (global_verbose_on) cout << "Running with debug information from gdbserver" << endl;
            global_gdb_debug = 1;
            gdbserver_flag=1;
            break;

         case 'p':
            if (global_verbose_on) cout << "Running on port: " << optarg << endl;
            global_gdbserver_port = atoi(optarg);
            break;

         case 't':
            if (global_verbose_on) cout << "Running in Trace Mode" << endl;
            StartTrace(optarg);
            break;

         case 'v':
            cout << "SimulAVR " << VERSION << endl;
            cout << "See documentation for copyright and distribution terms" << endl;
            cout << endl;
            exit(0);
            break;

         case 'n':
            cout << "We will NOT wait for a gdb connection, simulation starts now!" << endl;
            globalWaitForGdbConnection=false;
            break;


         default:
            cout << Usage;
            exit(0);

      }
   }

   /* now we create the device */
   AvrDevice *dev1=AvrFactory::instance().makeDevice(devicename);

   //if we want to insert some special "pipe" Registers we could do this here:
   if (readFromPipeFileName!="") {
      if (global_verbose_on) cout << "Add ReadFromPipe-Register at 0x" << hex << readFromPipeOffset << " and read from file: " << readFromPipeFileName << endl;
      dev1->ReplaceIoRegister(readFromPipeOffset, new RWReadFromPipe(dev1, readFromPipeFileName));
   }

   if (writeToPipeFileName!="") {
      if (global_verbose_on) cout << "Add WriteToPipe-Register at 0x" << hex << writeToPipeOffset << " and write to file: " << writeToPipeFileName << endl;
      dev1->ReplaceIoRegister(writeToPipeOffset, new RWWriteToPipe(dev1, writeToPipeFileName));
   }



   if (filename != "unknown" ) {
      dev1->Load(filename.c_str());
   }

   //if we have a file we can check out for termination lines.
   vector<string>::iterator ii;
   for (ii=terminationArgs.begin(); ii!=terminationArgs.end(); ii++) {
      if (global_verbose_on) cout <<*ii<<endl;
      unsigned int epa=dev1->Flash->GetAddressAtSymbol(*ii);
      dev1->EP.push_back(epa);
   }

   if (userinterface_flag==1) {
      ui=new UserInterface(7777); //if not gdb, the ui will be master controller :-)
   }

   dev1->SetClockFreq(1000000000/fcpu);

   if (global_trace_on) dev1->trace_on=1;

   if (gdbserver_flag==0) {
      SystemClock::Instance().Add(dev1);
      if (maxRunTime == 0) {
         SystemClock::Instance().Endless();
      } else {                                           // limited
         SystemClock::Instance().Run(maxRunTime);
      }
   } else { //gdb should be activated
      cout << "Going to gdb..." << endl;
      GdbServer gdb1(dev1, global_gdbserver_port, global_gdb_debug, globalWaitForGdbConnection);
      SystemClock::Instance().Add(&gdb1);
      SystemClock::Instance().Endless();
   } 
}

