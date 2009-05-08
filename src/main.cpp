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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

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
#include "string2.h"

const char *SplitOffsetFile(
  const char    *arg,
  const char    *name,
  int            base,
  unsigned long *offset
)
{
  char *end;

  if (!StringToUnsignedLong(arg, offset, &end, base)) {
    cerr << name << " offset is not a number" << endl;
    exit(1);
  }
  //position behind the "," or any other delimiter for the offset
  if (!*end) {
    cerr << name << " argument ends before filename" << endl;
    exit(1);
  }
  if (*end != ',') {
    cerr << name << " argument does not have comma before filename" << endl;
    exit(1);
  }
  ++end;
  if (!*end) {
    cerr << name << " argument has comma but no filename" << endl;
    exit(1);
  }

  return end;
}

const char Usage[] = 
"AVR-Simulator Version " VERSION "\n"
"-u                    run with user interface for external pin\n"
"                      handling at port 7777\n"
"-f --file <name>      load elf-file <name> for simulation in simulated target\n"
"-d --device <name>    simulate device <name> \n"
"-g --gdbserver        run as gdb-server\n"
"-G                    run as gdb-server and write debug info for gdb-connection\n"
"-m  <nanoseconds>     maximum run time of <nanoseconds>\n"
"-M                    disable messages for bad I/O and memory references\n"
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
"-a --writetoabort <offset>\n"
"                      add a special register at IO-offset\n"
"                      which aborts simulator run\n"
"-e --writetoexit <offset>\n"
"                      add a special register at IO-offset\n"
"                      which exits simulator run\n"
"-V --verbose          output some hints to console\n"
"-T --terminate <label> or <address>\n"
"                      stops simulation if PC runs on <label> or <address>\n"
"-B --breakpoint <label> or <address>\n"
"                      same as -T for backward compatibility\n"
"\n"
"Supported devices:\n"
"  at90s4433\n"
"  at90s8515\n"
"  atmega48\n"
"  atmega128\n"
"\n";

int main(int argc, char *argv[]) {
   int c;
   bool gdbserver_flag=0;
   string filename("unknown");
   string devicename("unknown");
   string tracefilename("unknown");
   long global_gdbserver_port    = 1212;
   int global_gdb_debug         = 0;
   bool globalWaitForGdbConnection=true; //please wait for gdb connection
   int userinterface_flag=0;
   unsigned long long fcpu=4000000;
   unsigned long long maxRunTime=0;
   UserInterface *ui;

   global_trace_on=0;

   unsigned long writeToPipeOffset=0x20;
   unsigned long readFromPipeOffset=0x21;
   unsigned long writeToAbort=0;
   unsigned long writeToExit=0;
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
         {"gdbserver", 0, 0, 'g'},
         {"maxruntime", 1,0,'m'},
         {"nogdbwait",0,0,'n'},
         {"trace", 1, 0, 't'},
         {"version",0,0,'v'},
         {"cpufrequency", 1,0,'F'},
         {"readfrompipe", 1,0,'R'},
         {"writetopipe", 1,0,'W'},
         {"writetoabort", 1,0,'a'},
         {"writetoexit", 1,0,'e'},
         {"verbose", 0,0,'V'},
         {"terminate",1,0,'T'},
         {"breakpoint",1,0,'B'},
         {0, 0, 0, 0}
      };

      c = getopt_long (argc, argv, "a:e:f:d:gGm:Mp:t:uxyzhvniF:R:W:VT:B:", long_options, &option_index);
      if (c == -1)
         break;

      switch (c) {
         case 'B':
         case 'T':
            terminationArgs.push_back(optarg);
            break;

         case 'M':
            global_message_on_bad_access = 0;
            break;

         case 'V':
            global_verbose_on=1;
            break;

         case 'R': //read from pipe 
            readFromPipeFileName = 
              SplitOffsetFile(optarg, "readFromPipe", 16, &readFromPipeOffset);
            break;

         case 'W': //write to pipe
            writeToPipeFileName = 
              SplitOffsetFile(optarg, "writeToPipe", 16, &writeToPipeOffset);
            break;

         case 'a': // write to abort
            if (!StringToUnsignedLong(optarg, &writeToAbort, NULL, 16)) {
              cerr << "writeToAbort is not a number" << endl;
              exit(1);
            }
            break;

         case 'e': // write to exit
            if (!StringToUnsignedLong(optarg, &writeToExit, NULL, 16)) {
              cerr << "writeToExit is not a number" << endl;
              exit(1);
            }
            break;

         case 'F':
            if ( !StringToUnsignedLongLong( optarg, &fcpu, NULL, 10 ) ) {
              cerr << "frequency is not a number" << endl;
              exit(1);
            }
            if ( fcpu == 0 ) {
              cerr << "frequency is zero" << endl;
              exit(1);
            }
            if (global_verbose_on)
               cout << "Running with CPU frequency: " << fcpu << endl;
            break;

         case 'm':
            if ( !StringToUnsignedLongLong( optarg, &maxRunTime, NULL, 10 ) ) {
              cerr << "maxRunTime is not a number" << endl;
              exit(1);
            }
            if ( maxRunTime == 0 ) {
              cerr << "maxRunTime is zero" << endl;
              exit(1);
            }
            if (global_verbose_on)
               cout << "Maximum Run Time: " << maxRunTime << endl;
            break;

         case 'u':
            if (global_verbose_on)
               cout << "Run with User Interface at Port 7777" << endl;
            userinterface_flag=1;
            break;

         case 'f':
            if (global_verbose_on)
               cout << "File to load: " << optarg << endl;
            filename=optarg;
            break;

         case 'd':
            if (global_verbose_on)
               cout << "Device to simulate: " << optarg << endl;
            devicename=optarg;
            break;

         case 'g':
            if (global_verbose_on)
               cout << "Running as gdb-server" << endl;
            gdbserver_flag=1;
            break;

         case 'G':
            if (global_verbose_on)
               cout << "Running with debug information from gdbserver" << endl;
            global_gdb_debug = 1;
            gdbserver_flag=1;
            break;

         case 'p':
            if ( !StringToLong( optarg, &global_gdbserver_port, NULL, 10 ) ) {
              cerr << "GDB Server Port is not a number" << endl;
              exit(1);
            }
            if (global_verbose_on)
               cout << "Running on port: " << optarg << endl;
            break;

         case 't':
            if (global_verbose_on)
               cout << "Running in Trace Mode" << endl;
            StartTrace(optarg);
            break;

         case 'v':
            cout << "SimulAVR " << VERSION << endl
                 << "See documentation for copyright and distribution terms"
                     << endl
                 << endl;
            exit(0);
            break;

         case 'n':
            cout << "We will NOT wait for a gdb connection, "
                    "simulation starts now!" << endl;
            globalWaitForGdbConnection=false;
            break;

         default:
            cout << Usage;
            exit(0);
      }
   }

   if ( !gdbserver_flag && filename == "unknown" ) {
     cerr << "No executable file specified" << endl;
     exit(1);
   }

   /* now we create the device */
   AvrDevice *dev1=AvrFactory::instance().makeDevice(devicename.c_str());

   //if we want to insert some special "pipe" Registers we could do this here:
   if (readFromPipeFileName!="") {
      if (global_verbose_on)
         cout << "Add ReadFromPipe-Register at 0x"
              << hex << readFromPipeOffset
              << " and read from file: " << readFromPipeFileName << endl;
      dev1->ReplaceIoRegister(
        readFromPipeOffset,
        new RWReadFromPipe(dev1, readFromPipeFileName.c_str())
      );
   }

   if (writeToPipeFileName!="") {
      if (global_verbose_on)
         cout << "Add WriteToPipe-Register at 0x" <<
                 hex << writeToPipeOffset <<
                 " and write to file: " << writeToPipeFileName << endl;
      dev1->ReplaceIoRegister(
        writeToPipeOffset,
        new RWWriteToPipe(dev1, writeToPipeFileName.c_str())
      );
   }

   if (writeToAbort) {
      if (global_verbose_on)
        cout << "Add WriteToAbort-Register at 0x" << hex <<
                 writeToAbort << endl;
      dev1->ReplaceIoRegister(writeToAbort, new RWAbort(dev1));
   }

   if (writeToExit) {
      if (global_verbose_on)
        cout << "Add WriteToExit-Register at 0x" << hex << writeToExit << endl;
      dev1->ReplaceIoRegister(writeToExit, new RWExit(dev1));
   }

   if (filename != "unknown" ) {
      dev1->Load(filename.c_str());
   }

   //if we have a file we can check out for termination lines.
   vector<string>::iterator ii;
   for (ii=terminationArgs.begin(); ii!=terminationArgs.end(); ii++) {
      if (global_verbose_on)
         cout << "Termination or Breakpoint Symbol: " << *ii << endl;
      dev1->RegisterTerminationSymbol((*ii).c_str());
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

