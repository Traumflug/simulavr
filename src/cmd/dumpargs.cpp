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

#include <fstream>

#include <stdlib.h>

#include "dumpargs.h"
#include "helper.h"

using namespace std;
 
void SetDumpTraceArgs(const vector<string> &traceopts, AvrDevice *dev) {
    DumpManager *dman = DumpManager::Instance();
    for(size_t i = 0; i < traceopts.size(); i++) {
        vector<string> ls = split(traceopts[i], ":");
        if(ls.size() < 1) {
            cerr << "Invalid tracing option '" << traceopts[i] << "'.";
            exit(1);
        }
        Dumper *d;
        TraceSet ts;
        cerr << "Enabling tracer: '";
        if(ls[0] == "warnread") {
            cerr << "warnread'." << endl;
            if(ls.size() > 1) {
                cerr << "Invalid number of options for 'warnread'.";
                exit(1);
            }
            ts = dman->all();
            d = new WarnUnknown(dev);
        } else if (ls[0] == "vcd") {
            cerr << "vcd'." << endl;
            if(ls.size() < 3 || ls.size() > 4) {
                cerr << "Invalid number of options for 'vcd'.";
                exit(1);
            }
            cerr << "Reading values to trace from '" << ls[1] << "'." << endl;
        
            ifstream is(ls[1].c_str());
            if(is.is_open() == 0) {
                cerr << "Can't open '" << ls[1] << "'.";
                exit(1);
            }
        
            cerr << "Output VCD file is '" << ls[2] << "'." << endl;
            ofstream *out = new ofstream(ls[2].c_str());
            ts = dman->load(is);
        
            bool rs = false, ws = false;
            if(ls.size() == 4) { // ReadStrobe/WriteStrobe display specified?
                if(ls[3] == "rw") {
                    rs = ws = true;
                } else if(ls[3] == "r") {
                    rs = true;
                } else if(ls[3] == "w") {
                    ws = true;
                } else {
                    cerr << "Invalid read/write strobe specifier '" << ls[3]
                         << "'." << endl;
                    exit(1);
                }
            }
            d = new DumpVCD(out, "ns", rs, ws);
        } else {
            cerr << "Unknown tracer '" << ls[0] <<  "'." << endl;
            exit(1);
        }
        dman->addDumper(d, ts);
    }
}
 
void ShowRegisteredTraceValues(const string &outname) {
    cerr << "Dumping traceable values to ";
    if(outname != "-")
        cerr << "'" << outname << "'." << endl;
    else
        cerr << "stdout." << endl;
    
    ostream *outf;
    if(outname != "-")
        outf = new ofstream(outname.c_str());
    else
        outf = &cout;
   
    DumpManager::Instance()->save(*outf);
    
    if(outf != &cout)
        delete outf;
}

// EOF
