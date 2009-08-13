/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2009 Onno Kortmann
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
#include <algorithm>
#include <fstream>
#include <sstream>
#include "helper.h"
#include "traceval.h"
#include "avrerror.h"
#include "systemclock.h"

using namespace std;

TraceValue::TraceValue(size_t bits,
                       const std::string &__name,
                       const int __index,
                       void *_shadow) :
    b(bits),
    _name(__name),
    _index(__index),
    shadow(_shadow),
    v(0xaffeaffe),
    f(0),
    _written(false),
    _enabled(false) {}

size_t TraceValue::bits() const { return b; }

std::string TraceValue::name() const {
    if (index()>=0)
        return _name+int2str(index());
    else return _name;
}

std::string TraceValue::barename() const { return _name; }

int TraceValue::index() const { return _index; }
    
unsigned TraceValue::value() const { return v; }

bool TraceValue::enabled() const { return _enabled; }

void TraceValue::enable() { _enabled=true; }

void TraceValue::change(unsigned val) {
    // this is mostly the same as write, but dosn't set WRITE nor _written flag!
    if ((v != val) || !_written) {
        f |= CHANGE;
        v = val;
    }
}

void TraceValue::write(unsigned val) {
    if ((v != val) || !_written) {
        f |= CHANGE;
        v = val;
    }
    f |= WRITE;
    _written = true;
}

void TraceValue::read() {
    f |= READ;
}

bool TraceValue::written() const { return _written;  }

void TraceValue::set_written() {
    _written=true;
}

void TraceValue::set_written(unsigned val) {
    _written=true;
    v = val;
}

TraceValue::Atype TraceValue::flags() const { return (Atype)f; }

void TraceValue::cycle() {
    if (shadow) {
        unsigned nv;
        switch (b) {
        case 1:
            nv=*(bool*)shadow; break;
        case 8:
            nv=*(uint8_t*)shadow; break;
        case 16:
            nv=*(uint16_t*)shadow; break;
        case 32:
            nv=*(uint32_t*)shadow; break;
        default:
            avr_error("Internal error: Unsupported number of bits in TraceValue::cycle().");
        }
        if (v!=nv) {
            f|=CHANGE;
            _written=true; // FIXME: This detection can fail!
            v=nv;
        }
    }
}

void TraceValue::dump(Dumper &d) {
    if (f&READ) {
        d.markRead(this);
        if (!_written)
            d.markReadUnknown(this);
    } 
    if (f&WRITE) {
        d.markWrite(this);
    }
    if (f&CHANGE) {
        d.markChange(this);
    }
    f=0;
}

void TraceValueRegister::_tvr_registerTraceValues(TraceValueRegister *r) {
    // check for duplicate names
    // register this sub-register
}

void TraceValueRegister::RegisterTraceValue(TraceValue *t) {
    // check for duplicate names
    // register this TraceValue
}

TraceValue* TraceValueRegister::GetTraceValueByName(const std::string &name) {
  avr_error("method GetTraceValueByName is unimplemented yet");
}

WarnUnknown::WarnUnknown(AvrDevice *_core) : core(_core) {}

void WarnUnknown::markReadUnknown(const TraceValue *t) {
    cerr << "READ-before-WRITE for value " << t->name()
         << " at time " << SystemClock::Instance().getCurrentTime()
         << ", PC=0x" << hex << 2*core->PC << dec << endl;
}
bool WarnUnknown::enabled(const TraceValue *t) const {
    return true;
}

void DumpVCD::valout(const TraceValue *v) {
    osbuffer << 'b';
    if (v->written()) {
        unsigned val=v->value();
        for (int i=v->bits()-1; i>=0; i--) 
            osbuffer << ((val&(1<<i)) ? '1' : '0');
    } else {
        for (int i=0; i < v->bits(); i++)
            osbuffer << 'x';
    }
}

void DumpVCD::flushbuffer(void) {
    if(changesWritten) {
        *os << osbuffer.str();
        changesWritten = false;
    }
    osbuffer.str("");
}

DumpVCD::DumpVCD(ostream *_os,
                 const std::string &_tscale,
                 const bool rstrobes,
                 const bool wstrobes) :
    os(_os),
    tscale(_tscale),
    rs(rstrobes),
    ws(wstrobes),
    changesWritten(false) {}

void DumpVCD::setActiveSignals(const TraceSet &act) {
    tv=act;
    unsigned n=0;
    for (TraceSet::const_iterator i=act.begin();
         i!=act.end(); i++) {
        if (id2num.find(*i)!=id2num.end())
            avr_error("Trace value would be twice in VCD list.");
        id2num[*i]=n++;
    }
}

void DumpVCD::start() {
    *os <<
        "$version\n"
        "\tSimulavr VCD dump file generator\n"
        "$end\n";
    
    *os << "$timescale 1" << tscale << " $end\n";
    typedef TraceSet::iterator iter;
    unsigned n=0;
    for (iter i=tv.begin();
         i!=tv.end(); i++) {
        string s=(*i)->name();

        /* find last dot in string as divider
           between name of the variable and the module string. */
        int ld;
        for (ld=s.size()-1; ld>0; ld--)
            if (s[ld]=='.') break;
    
        *os << "$scope module " << s.substr(0, ld) << " $end\n";
        *os << "$var wire " << (*i)->bits() << ' ' << n*(1+rs+ws) << ' ' << s.substr(ld+1, s.size()-1) << " $end\n";
        if (rs)
            *os << "$var wire 1 " << n*(1+rs+ws)+1 << ' ' << s.substr(ld+1, s.size()-1)+"_R" << " $end\n";
        if (ws)
            *os << "$var wire 1 " << n*(1+rs+ws)+1+rs << ' ' << s.substr(ld+1, s.size()-1)+"_W" << " $end\n";
        *os << "$upscope $end\n";
        n++;
    }
    *os << "$enddefinitions $end\n";

    // mark initial state
    changesWritten = true;
    osbuffer << "#0\n$dumpvars\n";
    n=0;
    for (iter i=tv.begin();
         i!=tv.end(); i++) {
        valout(*i);
        osbuffer << ' ' << n*(1+rs+ws) << '\n';
        // reset RS, WS
        if (rs) {
            osbuffer << "0" << n*(1+rs+ws)+1 << "\n";
        }
        if (ws) {
            if (rs)
                osbuffer << "0" << n*(1+rs+ws)+2 << "\n";
            else
                osbuffer << "0" << n*(1+rs+ws)+1 << "\n";
        }
        n++;
    }
    osbuffer << "$end\n";
    flushbuffer();
}

void DumpVCD::cycle() {
    // flush the buffer
    flushbuffer();
    
    // write new time marker to buffer
    SystemClockOffset clock=SystemClock::Instance().getCurrentTime();
    osbuffer << "#" << clock << '\n';

    // reset RS, WS states
    for (size_t i=0; i<marked.size(); i++)
        osbuffer << "0" << marked[i] << "\n";
    if(marked.size())
        changesWritten = true;
    marked.clear();
}

void DumpVCD::stop() {
    // flush the buffer
    flushbuffer();
    
    // write a last time marker to report end of dump
    SystemClockOffset clock=SystemClock::Instance().getCurrentTime();
    *os << "#" << clock << '\n';
    
    os->flush(); // flush stream
}

void DumpVCD::markRead(const TraceValue *t) {
    if (rs) {
        // mark read cycle
        osbuffer << "1" << id2num[t]*(1+rs+ws)+1 << "\n";
        changesWritten = true;
        // mark to disable @ next cycle
        marked.push_back(id2num[t]*(1+rs+ws)+1);
    }
}

void DumpVCD::markWrite(const TraceValue *t) {
    if (ws) {
        osbuffer << "1" << id2num[t]*(1+rs+ws)+1+rs << "\n";
        changesWritten = true;
        marked.push_back(id2num[t]*(1+rs+ws)+1+rs);
    }
}

void DumpVCD::markChange(const TraceValue *t) {
    valout(t);
    osbuffer << " " << id2num[t]*(1+rs+ws) << "\n";
    changesWritten = true;
}

bool DumpVCD::enabled(const TraceValue *t) const {
    return id2num.find(t)!=id2num.end();
}

DumpVCD::~DumpVCD() { delete os; }

DumpManager::DumpManager(AvrDevice *_core) : core(_core) {}

void DumpManager::regTrace(TraceValue *tv) {
    if (all_map.find(tv->name())!=all_map.end())
        avr_error(("Internal error: Tracevalue '"+tv->name()+"' already registered.").c_str());
    all_map[tv->name()]=tv;

    /*FIXME: Note that the order in which the values come from _all
      is important for clean output of the list of traceabe values!
      The container should be ordered! */
    _all.push_back(tv);
}

void DumpManager::addDumper(Dumper *dump, const TraceSet &vals) {
    TraceSet s_vals(vals),
        s_all(_all);
    
    sort(s_vals.begin(), s_vals.end());
    sort(s_all.begin(), s_all.end());
    
    TraceSet toomany;
    set_difference(s_vals.begin(), s_vals.end(),
                   s_all.begin(), s_all.end(),
                   insert_iterator<TraceSet>(toomany, toomany.end()));
    if (toomany.size()) {
        TraceSet::iterator i=toomany.begin();
        
        // FIXME: Maybe make this more verbose.
        avr_error(("Value '"+(*i)->name()+"'  not registered.").c_str());
    }
    for (TraceSet::iterator i=s_vals.begin();
         i!=s_vals.end(); i++)
        (*i)->enable();

    TraceSet s_active(active);
    sort(s_active.begin(), s_active.end());

    TraceSet newact;
    set_union(s_vals.begin(), s_vals.end(),
              s_active.begin(), s_active.end(),
              insert_iterator<TraceSet>(newact, newact.end()));
    active.swap(newact);

    if (find(dumps.begin(), dumps.end(), dump)!=dumps.end())
        avr_error("Internal error: Dumper already registered.");

    dump->setActiveSignals(s_vals);

    dumps.push_back(dump);
}

const TraceSet& DumpManager::all() const { return _all; }

void DumpManager::start() {
    for (size_t i=0; i< dumps.size(); i++)
        dumps[i]->start();

}

void DumpManager::cycle() {
    // First, call the Dumpers
    for (size_t i=0; i<dumps.size(); i++)
        dumps[i]->cycle();

    // And then, update the TraceValues and dump them
    for (TraceSet::iterator i=active.begin();
         i!=active.end(); i++) {
        (*i)->cycle();
        for (size_t j=0; j<dumps.size(); j++)
            if (dumps[j]->enabled(*i))
                (*i)->dump(*dumps[j]);
    }
}

DumpManager::~DumpManager() {
    for(size_t i = 0; i < dumps.size(); i++) {
        dumps[i]->stop(); // inform dumper to stop output
        delete dumps[i];
    }
}

void DumpManager::save(ostream &os, const TraceSet &s) const {
    typedef std::vector<TraceValue*>::const_iterator citer;
    for (citer i=s.begin(); i!=s.end(); i++) {
        TraceValue& tv=*(*i);
        if (tv.index()>=0) {
            citer j=i;
            int c=tv.index();
            
            while (((*i)->barename()==tv.barename()) &&
                   (c==(*i)->index())) {
                i++;
                c++;
            }
            i--; c--;
            
            if (c) {
                os << "| " << tv.barename() << ' ' 
                   << tv.index() << " .. "
                   << (*i)->index() << '\n';
            } else {
                os << "+ " << (*i)->name() << '\n';
            }
        } else os << "+ " << (*i)->name() << '\n';
    }
}

std::vector<TraceValue*> DumpManager::load(istream &is) {
    std::vector<TraceValue*> res;
    while (!is.eof()) {
        std::string l=readline(is);
        vector<std::string> ls=split(l);
        if (ls.size()<2) continue;
        if (ls[0]=="+") {
            std::string n=ls[1];
            if (all_map.find(n)==all_map.end()) {
                string msg="TraceValue '"+n+"' is not known.";
                avr_error(msg.c_str());
            }
            res.push_back(all_map[n]);
        } else if (ls[0]=="|") {
            if (ls[3]!="..") {
                avr_error("'..' expected between range limits.");
            }
            std::string bn=ls[1];
            size_t
                min=atoi(ls[2].c_str()),
                max=atoi(ls[4].c_str());

            for (size_t i=min; i <= max; i++) {
                std::string n=ls[1]+int2str(i);
                if (all_map.find(n)==all_map.end()) {
                    string msg="While constructing range with '"+n+
                        "', TraceValue is not known.";
                    avr_error(msg.c_str());
                }
                res.push_back(all_map[n]);
            }
        } else if (ls[0][0]!='#') avr_error(
            ("Invalid trace value specifier '"+ls[0]+"'.").c_str());
    }
    return res;
}

//FIXME
static std::string trgrp="FIXME.UNDEFINED";

void trace_direct(AvrDevice *c, const std::string &name, bool *val) {
    c->dump_manager->regTrace(new TraceValue(1, trgrp+"."+name, -1, val));
}

void trace_direct(AvrDevice *c, const std::string &name, uint8_t *val) {
    c->dump_manager->regTrace(new TraceValue(8, trgrp+"."+name, -1, val));
}
void trace_direct(AvrDevice *c, const std::string &name, uint16_t *val) {
    c->dump_manager->regTrace(new TraceValue(16, trgrp+"."+name, -1, val));
}
void trace_direct(AvrDevice *c, const std::string &name, uint32_t *val) {
    c->dump_manager->regTrace(new TraceValue(32, trgrp+"."+name, -1, val));
}

void set_trace_group_s(const std::string &grp) { trgrp=grp; }

