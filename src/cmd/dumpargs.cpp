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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 */

#include <fstream>
#include <sstream>
#include <iomanip>

#include <stdlib.h>

#include "dumpargs.h"
#include "../helper.h"
#include "../avrerror.h"
#include "../flash.h"
#include "../hweeprom.h"

using namespace std;
 
void SetDumpTraceArgs(const vector<string> &traceopts, AvrDevice *dev) {
    DumpManager *dman = DumpManager::Instance();
    for(size_t i = 0; i < traceopts.size(); i++) {
        vector<string> ls = split(traceopts[i], ":");
        if(ls.size() < 1)
            avr_error("Invalid tracing option '%s'.", traceopts[i].c_str());
        Dumper *d;
        TraceSet ts;
        cerr << "Enabling tracer: '";
        if(ls[0] == "warnread") {
            cerr << "warnread'." << endl;
            if(ls.size() > 1)
                avr_error("Invalid number of options for 'warnread'.");
            ts = dman->all();
            d = new WarnUnknown(dev);
        } else if (ls[0] == "vcd") {
            cerr << "vcd'." << endl;
            if(ls.size() < 3 || ls.size() > 4)
                avr_error("Invalid number of options for 'vcd'.");
            cerr << "Reading values to trace from '" << ls[1] << "'." << endl;
        
            ifstream is(ls[1].c_str());
            if(is.is_open() == 0)
                avr_error("Can't open '%s'", ls[1].c_str());
        
            cerr << "Output VCD file is '" << ls[2] << "'." << endl;
            ts = dman->load(is);
        
            bool rs = false, ws = false;
            if(ls.size() == 4) { // ReadStrobe/WriteStrobe display specified?
                if(ls[3] == "rw") {
                    rs = ws = true;
                } else if(ls[3] == "r") {
                    rs = true;
                } else if(ls[3] == "w") {
                    ws = true;
                } else
                    avr_error("Invalid read/write strobe specifier '%s'", ls[3].c_str());
            }
            d = new DumpVCD(ls[2], "ns", rs, ws);
        } else
            avr_error("Unknown tracer '%s'", ls[0].c_str());
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

static void WriteCoreDumpIO(ostream &outf, AvrDevice *dev, int offs, int size) {
    int hsize = (size + 1) / 2;
    const int sp_name = 10, sp_col = 15; // place for IO register name an gap size between columns
    for(int i = 0; i < hsize; i++) {
        // left column
        string regname = dev->rw[i + offs]->GetTraceName();
        unsigned char val = 0;
        if(dev->rw[i + offs]->IsInvalid())
            regname = "Reserved";
        else
            val = (unsigned char)*(dev->rw[i + offs]);
        outf << hex << setw(2) << setfill('0') << right << (i + offs) << " : "
             << setw(sp_name) << setfill(' ') << left << regname << " : "
             << "0x" << hex << setw(2) << setfill('0') << right << (int)val;
        if((i + hsize) >= size)
            outf << endl; // odd count of IO registers?
        else {
            // right column
            regname = dev->rw[i + hsize + offs]->GetTraceName();
            val = 0;
            if(dev->rw[i + hsize + offs]->IsInvalid())
                regname = "Reserved";
            else
                val = (unsigned char)*(dev->rw[i + hsize + offs]);
            outf << setw(sp_col) <<  setfill(' ') << " "
                 << hex << setw(2) << setfill('0') << right << (i + hsize + offs) << " : "
                 << setw(sp_name) << setfill(' ') << left << regname << " : "
                 << "0x" << hex << setw(2) << setfill('0') << right << (int)val
                 << endl;
        }
    }
}

static void WriteCoreDumpRAM(ostream &outf, AvrDevice *dev, int offs, int size) {
    const int maxLineByte = 16;
    ostringstream buf;
    int start = offs, lastStart = 0, dup = 0, j = 0;
    string lastLine("");

    for(int i = 0; i < size; i++) {
        buf << hex << setw(2) << setfill('0') << (int)((unsigned char)*(dev->rw[i + offs])) << " ";
        if(++j == maxLineByte) {
            if(buf.str() == lastLine) // check for duplicate line
              dup++;
            else {
              if(dup > 0) outf << "  -- last line repeats --" << endl;
              outf << hex << setw(4) << setfill('0') << right << start << " : " << buf.str() << endl;
              dup = 0;
              lastLine = buf.str();
            }
            j = 0;
            lastStart = start;
            start += maxLineByte;
            buf.str("");
        }
    }
    if((j > 0) || (dup > 0)) {
        if(dup > 0) outf << "  -- last line repeats --" << endl;
        if(j == 0)
          outf << hex << setw(4) << setfill('0') << right << lastStart << " : " << lastLine << endl;
        else
          outf << hex << setw(4) << setfill('0') << right << start << " : " << buf.str() << endl;
    }
}

static void WriteCoreDumpEEPROM(ostream &outf, AvrDevice *dev, int size) {
    const int maxLineByte = 16;
    ostringstream buf;
    int start = 0, lastStart = 0, dup = 0, j = 0;
    string lastLine("");

    for(int i = 0; i < size; i++) {
        buf << hex << setw(2) << setfill('0') << (int)(dev->eeprom->ReadFromAddress(i)) << " ";
        if(++j == maxLineByte) {
            if(buf.str() == lastLine) // check for duplicate line
              dup++;
            else {
              if(dup > 0) outf << "  -- last line repeats --" << endl;
              outf << hex << setw(4) << setfill('0') << right << start << " : " << buf.str() << endl;
              dup = 0;
              lastLine = buf.str();
            }
            j = 0;
            lastStart = start;
            start += maxLineByte;
            buf.str("");
        }
    }
    if((j > 0) || (dup > 0)) {
        if(dup > 0) outf << "  -- last line repeats --" << endl;
        if(j == 0)
          outf << hex << setw(4) << setfill('0') << right << lastStart << " : " << lastLine << endl;
        else
          outf << hex << setw(4) << setfill('0') << right << start << " : " << buf.str() << endl;
    }
}

static void WriteCoreDumpFlash(ostream &outf, AvrDevice *dev, int size) {
    const int maxLineWord = 8;
    ostringstream buf;
    int start = 0, lastStart = 0, dup = 0, j = 0;
    string lastLine("");

    for(int i = 0; i < size; i += 2) {
        buf << hex << setw(4) << setfill('0') << dev->Flash->ReadMemRawWord(i) << " ";
        if(++j == maxLineWord) {
            if(buf.str() == lastLine) // check for duplicate line
              dup++;
            else {
              if(dup > 0) outf << "  -- last line repeats --" << endl;
              outf << hex << setw(4) << setfill('0') << right << start << " : " << buf.str() << endl;
              dup = 0;
              lastLine = buf.str();
            }
            j = 0;
            lastStart = start;
            start += maxLineWord;
            buf.str("");
        }
    }
    if((j > 0) || (dup > 0)) {
        if(dup > 0) outf << "  -- last line repeats --" << endl;
        if(j == 0)
          outf << hex << setw(4) << setfill('0') << right << lastStart << " : " << lastLine << endl;
        else
          outf << hex << setw(4) << setfill('0') << right << start << " : " << buf.str() << endl;
    }
}

void WriteCoreDump(const string &outname, AvrDevice *dev) {
    ostream *outf;

    // open dump file
    if(outname != "-")
        outf = new ofstream(outname.c_str());
    else
        outf = &cout;

    // write out PC
    *outf << "PC = 0x" << hex << setw(6) << setfill('0') << dev->PC
          << " (PC*2 = 0x" << hex << setw(6) << setfill('0') << (dev->PC * 2)
          << ")" << endl << endl;

    // write out general purpose register
    *outf << "General Purpose Register Dump:" << endl;
    for(unsigned int i = 0, j = 0; i < dev->GetMemRegisterSize(); i++) {
        *outf << dec << "r" << setw(2) << setfill('0') << i << "="
              << hex << setw(2) << setfill('0') << (int)((unsigned char)*(dev->rw[i])) << "  ";
        j++;
        if(j == 8) {
            *outf << endl;
            j = 0;
        }
    }
    *outf << endl;

    // write out IO register
    *outf << "IO Register Dump:" << endl;
    WriteCoreDumpIO(*outf, dev, dev->GetMemRegisterSize(), dev->GetMemIOSize());
    *outf << endl;

    // write out internal RAM
    *outf << "Internal SRAM Memory Dump:" << endl;
    WriteCoreDumpRAM(*outf, dev, dev->GetMemRegisterSize() + dev->GetMemIOSize(), dev->GetMemIRamSize());
    *outf << endl;

    // write out external RAM
    if(dev->GetMemERamSize() > 0) {
        *outf << "External SRAM Memory Dump:" << endl;
        WriteCoreDumpRAM(*outf, dev, dev->GetMemRegisterSize() + dev->GetMemIOSize() + dev->GetMemIRamSize(), dev->GetMemERamSize());
        *outf << endl;
    }

    // write out EEPROM content
    *outf << "EEPROM Memory Dump:" << endl;
    WriteCoreDumpEEPROM(*outf, dev, dev->eeprom->GetSize());
    *outf << endl;

    // write out flash content
    *outf << "Program Flash Memory Dump:" << endl;
    WriteCoreDumpFlash(*outf, dev, dev->Flash->GetSize());
    *outf << endl;

    // close file
    if(outf != &cout)
        delete outf;
}

// EOF
