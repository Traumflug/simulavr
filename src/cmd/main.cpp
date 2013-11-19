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

#include <iostream>
#include <sstream>
#include <string>
#include <map>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#ifndef _MSC_VER
#  include <getopt.h>
#else
#  include "../getopt/getopt.h"
#  define VERSION "(git-snapshot)"
#endif

#include "config.h"

#include "flash.h"
#include "avrdevice.h"
#include "avrfactory.h"
#include "avrsignature.h"
#include "avrreadelf.h"
#include "gdb.h"
#include "ui/ui.h"
#include "systemclock.h"
#include "ui/lcd.h"
#include "ui/keyboard.h"
#include "traceval.h"
#include "ui/scope.h"
#include "string2.h"
#include "helper.h"
#include "specialmem.h"
#include "irqsystem.h"

#include "dumpargs.h"

const char *SplitOffsetFile(const char *arg,
                            const char *name,
                            int base,
                            unsigned long *offset)
{
    char *end;
    
    if(!StringToUnsignedLong(arg, offset, &end, base)) {
        cerr << name << ": offset is not a number" << endl;
        exit(1);
    }
    //position behind the "," or any other delimiter for the offset
    if(!*end) {
        cerr << name << ": argument ends before filename" << endl;
        exit(1);
    }
    if(*end != ',') {
        cerr << name << ": argument does not have comma before filename" << endl;
        exit(1);
    }
    ++end;
    if(!*end) {
        cerr << name << ": argument has comma but no filename" << endl;
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
    "-g --gdbserver        listen for GDB connection on TCP port defined by -p\n"
    "-G --gdb-debug        listen for GDB connection and write debug info\n"
    "   --gdb-stdin        for use with GDB as 'target remote | ./simulavr'\n"
    "-m  <nanoseconds>     maximum run time of <nanoseconds>\n"
    "-M                    disable messages for bad I/O and memory references\n"
    "-p  <port>            use <port> for gdb server\n"
    "-t --trace <file>     enable trace outputs to <file>\n"
    "-l --linestotrace <number>\n"
    "                      maximum number of lines in each trace file.\n"
    "                      0 means endless. Attention: if you use gdb & trace, please use always 0!\n"
    "-n --nogdbwait        do not wait for gdb connection\n"
    "-F --cpufrequency     set the cpu frequency to <Hz> \n"
    "-s --irqstatistic     prints statistic informations about irq usage after simulation\n"
    "                      is stopped\n"
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
    "-v --verbose          output some hints to console\n"
    "-T --terminate <label> or <address>\n"
    "                      stops simulation if PC runs on <label> or <address>\n"
    "-B --breakpoint <label> or <address>\n"
    "                      same as -T for backward compatibility\n"
    "-c <tracing-option>   Enables a tracer with a set of options. The format for\n"
    "                      <tracing-option> is:\n"
    "                      <tracer>[:further-options ...]\n"
    "-o <trace-value-file> Specifies a file into which all available trace value names\n"
    "                      will be written.\n"
    "-V --version          print out version and exit immediately\n"
    "-h --help             print this help\n"
    "\n";

int main(int argc, char *argv[]) {
    int c;
    bool gdbserver_flag = 0;
    string filename("unknown");
    string devicename("unknown");
    string tracefilename("unknown");
    long global_gdbserver_port = 1212;
    int global_gdb_debug = 0;
    bool globalWaitForGdbConnection = true; //please wait for gdb connection
    int userinterface_flag = 0;
    unsigned long long fcpu = 4000000;
    unsigned long long maxRunTime = 0;
    unsigned long long linestotrace = 1000000;
    UserInterface *ui;
    
    unsigned long writeToPipeOffset = 0x20;
    unsigned long readFromPipeOffset = 0x21;
    unsigned long writeToAbort = 0;
    unsigned long writeToExit = 0;
    string readFromPipeFileName = "";
    string writeToPipeFileName = "";
    
    vector<string> terminationArgs;
    
    vector<string> tracer_opts;
    bool tracer_dump_avail = false;
    string tracer_avail_out;
    
    while (1) {
        //int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"file", 1, 0, 'f'},
            {"device", 1, 0, 'd'},
            {"gdbserver", 0, 0, 'g'},
            {"gdb-debug", 0, 0, 'G'},
            {"debug-gdb", 0, 0, 'G'},
            {"linestotrace", 1, 0, 'l'},
            {"maxruntime", 1, 0, 'm'},
            {"nogdbwait", 0, 0, 'n'},
            {"trace", 1, 0, 't'},
            {"version", 0, 0, 'V'},
            {"cpufrequency", 1, 0, 'F'},
            {"readfrompipe", 1, 0, 'R'},
            {"writetopipe", 1, 0, 'W'},
            {"writetoabort", 1, 0, 'a'},
            {"writetoexit", 1, 0, 'e'},
            {"verbose", 0, 0, 'v'},
            {"terminate", 1, 0, 'T'},
            {"breakpoint", 1, 0, 'B'},
            {"irqstatistic", 0, 0, 's'},
            {"help", 0, 0, 'h'},
            {0, 0, 0, 0}
        };
        
        c = getopt_long(argc, argv, "a:e:f:d:gGm:p:t:uxyzhvnisF:R:W:VT:B:c:o:l:", long_options, &option_index);
        if(c == -1)
            break;
        
        switch(c) {
            case 'B':
            case 'T':
                terminationArgs.push_back(optarg);
                break;
            
            case 'v':
                global_verbose_on = 1;
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
                if(!StringToUnsignedLong(optarg, &writeToAbort, NULL, 16)) {
                    cerr << "writeToAbort is not a number" << endl;
                    exit(1);
                }
                break;
            
            case 'e': // write to exit
                if(!StringToUnsignedLong(optarg, &writeToExit, NULL, 16)) {
                    cerr << "writeToExit is not a number" << endl;
                    exit(1);
                }
                break;
            
            case 'F':
                if(!StringToUnsignedLongLong(optarg, &fcpu, NULL, 10)) {
                    cerr << "frequency is not a number" << endl;
                    exit(1);
                }
                if(fcpu == 0) {
                    cerr << "frequency is zero" << endl;
                    exit(1);
                }
                if(global_verbose_on)
                    printf("Running with CPU frequency: %1.4f MHz (%lld Hz)\n",
                           fcpu/1000000.0, fcpu);
                break;

            case 'l':
                if(!StringToUnsignedLongLong( optarg, &linestotrace, NULL, 10)) {
                    cerr << "linestotrace is not a number" << endl;
                    exit(1);
                }
                break;

            case 'm':
                if(!StringToUnsignedLongLong( optarg, &maxRunTime, NULL, 10)) {
                    cerr << "maxRunTime is not a number" << endl;
                    exit(1);
                }
                if(maxRunTime == 0) {
                    cerr << "maxRunTime is zero" << endl;
                    exit(1);
                }
                avr_message("Maximum Run Time: %lld", maxRunTime);
                break;
            
            case 'u':
                avr_message("Run with User Interface at Port 7777");
                userinterface_flag = 1;
                break;
            
            case 'f':
                avr_message("File to load: %s", optarg);
                filename = optarg;
                break;
            
            case 'd':
                avr_message("Device to simulate: %s", optarg);
                devicename = optarg;
                break;
            
            case 'g':
                avr_message("Running as gdb-server");
                gdbserver_flag = 1;
                break;
            
            case 'G':
                avr_message("Running with debug information from gdbserver");
                global_gdb_debug = 1;
                gdbserver_flag = 1;
                break;
            
            case 'p':
                if(!StringToLong( optarg, &global_gdbserver_port, NULL, 10)) {
                    cerr << "GDB Server Port is not a number" << endl;
                    exit(1);
                }
                avr_message("Running on port: %ld", global_gdbserver_port);
                break;
            
            case 't':
                avr_message("Running in Trace Mode with maximum %lld lines per file",
                            linestotrace);

                sysConHandler.SetTraceFile(optarg, linestotrace);
                break;
            
            case 'V':
                cout << "SimulAVR " << VERSION << endl
                     << "See documentation for copyright and distribution terms" << endl
                     << endl;
                exit(0);
                break;
            
            case 'n':
                cout << "We will NOT wait for a gdb connection, "
                        "simulation starts now!" << endl;
                globalWaitForGdbConnection = false;
                break;
            
            case 'c':
                tracer_opts.push_back(optarg);
                break;
            
            case 'o':
                tracer_dump_avail = true;
                tracer_avail_out = optarg;
                break;
             
            case 's':
                enableIRQStatistic = true;
                break;
            
            default:
                cout << Usage
                     << "Supported devices:" << endl
                     << AvrFactory::supportedDevices() << endl;
                exit(0);
        }
    }
    
    /* get dump manager an inform it, that we have a single device application */
    DumpManager *dman = DumpManager::Instance();
    dman->SetSingleDeviceApp();
    
    /* check, if devicename is given or get it out from elf file, if given */
    unsigned int sig;
    if(devicename == "unknown") {
        // option -d | --device not given
        if(filename != "unknown") {
            // filename given, try to get signature
            sig = ELFGetSignature(filename.c_str());
            if(sig != -1) {
                // signature in elf found, try to get devicename
                std::map<unsigned int, std::string>::iterator cur  = AvrSignatureToNameMap.find(sig);
                if(cur != AvrSignatureToNameMap.end()) {
                    // devicename found
                    devicename = cur->second;
                } else {
                    avr_warning("unknown signature in elf file '%s': 0x%x", filename.c_str(), sig);
                }
            }
        }
    }

    /* now we create the device and set device name and signature */
    AvrDevice *dev1 = AvrFactory::instance().makeDevice(devicename.c_str());
    std::map<std::string, unsigned int>::iterator cur  = AvrNameToSignatureMap.find(devicename);
    if(cur != AvrNameToSignatureMap.end()) {
        // signature found
        sig = cur->second;
    } else {
        avr_warning("signature for device '%s' not found", devicename.c_str());
        sig = -1;
    }
    dev1->SetDeviceNameAndSignature(devicename, sig);
    
    /* We had to wait with dumping the available tracing values
      until the device has been created! */
    if(tracer_dump_avail) {
        ShowRegisteredTraceValues(tracer_avail_out);
        exit(0);
    }
    
    /* handle DumpTrace option */
    SetDumpTraceArgs(tracer_opts, dev1);
    
    if(!gdbserver_flag && filename == "unknown") {
        cerr << "Specify either --file <executable> or --gdbserver (or --gdb-stdin)" << endl;
        exit(1);
    }
    
    //if we want to insert some special "pipe" Registers we could do this here:
    if(readFromPipeFileName != "") {
        avr_message("Add ReadFromPipe-Register at 0x%lx and read from file: %s",
                    readFromPipeOffset, readFromPipeFileName.c_str());
        dev1->ReplaceIoRegister(readFromPipeOffset,
            new RWReadFromFile(dev1, "FREAD", readFromPipeFileName.c_str()));
    }
    
    if(writeToPipeFileName != "") {
        avr_message("Add WriteToPipe-Register at 0x%lx and write to file: %s",
                    writeToPipeOffset, writeToPipeFileName.c_str());
        dev1->ReplaceIoRegister(writeToPipeOffset,
            new RWWriteToFile(dev1, "FWRITE", writeToPipeFileName.c_str()));
    }
    
    if(writeToAbort) {
        avr_message("Add WriteToAbort-Register at 0x%lx", writeToAbort);
        dev1->ReplaceIoRegister(writeToAbort, new RWAbort(dev1, "ABORT"));
    }
    
    if(writeToExit) {
        avr_message("Add WriteToExit-Register at 0x%lx", writeToExit);
        dev1->ReplaceIoRegister(writeToExit, new RWExit(dev1, "EXIT"));
    }
    
    if(filename != "unknown" ) {
        dev1->Load(filename.c_str());
        dev1->Reset(); // reset after load data from file to activate fuses and lockbits
    }
    
    //if we have a file we can check out for termination lines.
    vector<string>::iterator ii;
    for(ii = terminationArgs.begin(); ii != terminationArgs.end(); ii++) {
        avr_message("Termination or Breakpoint Symbol: %s", (*ii).c_str());
        dev1->RegisterTerminationSymbol((*ii).c_str());
    }
    
    //if not gdb, the ui will be master controller :-)
    ui = (userinterface_flag == 1) ? new UserInterface(7777) : NULL;
    
    dev1->SetClockFreq(1000000000 / fcpu); // time base is 1ns!
    
    if(sysConHandler.GetTraceState())
        dev1->trace_on = 1;
    
    dman->start(); // start dump session
    
    if(gdbserver_flag == 0) { // no gdb
        SystemClock::Instance().Add(dev1);
        if(maxRunTime == 0) {
            SystemClock::Instance().Endless();
        } else {                                           // limited
            SystemClock::Instance().Run(maxRunTime);
        }
    } else { // gdb should be activated
        avr_message("Going to gdb...");
        GdbServer gdb1(dev1, global_gdbserver_port, global_gdb_debug, globalWaitForGdbConnection);
        SystemClock::Instance().Add(&gdb1);
        SystemClock::Instance().Endless();
    }
    
    dman->stopApplication(); // stop dump session. Close dump files, if necessary
    
    // delete ui and device
    delete ui;
    delete dev1;
    
    return 0;
}

