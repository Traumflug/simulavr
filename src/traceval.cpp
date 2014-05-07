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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 */
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "helper.h"
#include "traceval.h"
#include "avrdevice.h"
#include "avrerror.h"
#include "systemclock.h"

using namespace std;

TraceValue::TraceValue(size_t bits,
                       const std::string &__name,
                       const int __index,
                       const void *_shadow) :
    _name(__name),
    _index(__index),
    b(bits),
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

void TraceValue::change(unsigned val, unsigned mask) {
    // this is mostly the same as write, but dosn't set WRITE nor _written flag!
    if (((v & mask) != (val & mask)) || !_written) {
        f |= CHANGE;
        v = (v & ~mask) | (val & mask);
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
            nv = *(const bool*) shadow; break;
        case 8:
            nv = *(const uint8_t*) shadow; break;
        case 16:
            nv = *(const uint16_t*) shadow; break;
        case 32:
            nv = *(const uint32_t*) shadow; break;
        default:
            avr_error("Internal error: Unsupported number of bits in TraceValue::cycle().");
            break;
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

char TraceValue::VcdBit(int bitNo) const {
    if (_written)
        return (v & (1 << bitNo)) ? '1' : '0';
    else
        return 'x';
}

char TraceValueOutput::VcdBit(int bitNo) const {
    unsigned val = value();
    if(written()) {
        if(val == Pin::TRISTATE)
            return 'z';
        if((val == Pin::HIGH) || (val == Pin::PULLUP))
            return '1';
        if(val == Pin::LOW)
            return '0';
    }
    return 'x';
}

TraceValueRegister::~TraceValueRegister() {
    for (valmap_t::iterator i = _tvr_values.begin(); i != _tvr_values.end(); i++)
        delete i->first;
    _tvr_values.clear();
    for (regmap_t::iterator i = _tvr_registers.begin(); i != _tvr_registers.end(); i++)
        delete i->first;
    _tvr_registers.clear();
}

void TraceValueRegister::_tvr_registerTraceValues(TraceValueRegister *r) {
    string n = r->GetScopeName();
    if(GetScopeGroupByName(n) == NULL) {
        string *s = new string(n);
        pair<string*, TraceValueRegister*> v(s, r);
        _tvr_registers.insert(v);
    } else
        avr_error("duplicate name '%s', another TraceValueRegister child is already registered", n.c_str());
}

size_t TraceValueRegister::_tvr_getValuesCount(void) {
    size_t cnt = _tvr_values.size();
    for (regmap_t::iterator i = _tvr_registers.begin(); i != _tvr_registers.end(); i++)
        cnt += (i->second)->_tvr_getValuesCount();
    return cnt;
}

void TraceValueRegister::_tvr_insertTraceValuesToSet(TraceSet &t) {
    for (valmap_t::iterator i = _tvr_values.begin(); i != _tvr_values.end(); i++)
        t.push_back(i->second);
    for (regmap_t::iterator i = _tvr_registers.begin(); i != _tvr_registers.end(); i++)
        (i->second)->_tvr_insertTraceValuesToSet(t);
}

void TraceValueRegister::RegisterTraceValue(TraceValue *t) {
    // check for duplicate names and the right prefix
    string p = t->name();
    size_t idx = _tvr_scopeprefix.length();
    if((p.length() <= idx) || (p.substr(0, idx) != _tvr_scopeprefix))
        avr_error("add TraceValue denied: wrong prefix: '%s', scope is '%s'",
                  p.c_str(), _tvr_scopeprefix.c_str());
    string n = p.substr(idx);
    if(n.find('.') != string::npos)
        avr_error("add TraceValue denied: wrong name: '%s', scope is '%s'",
                  n.c_str(), _tvr_scopeprefix.c_str());
    // register this TraceValue
    if(GetTraceValueByName(n) == NULL) {
        string *s = new string(n);
        pair<string*, TraceValue*> v(s, t);
        _tvr_values.insert(v);
    } else
        avr_error("add TraceValue denied: name found: '%s'", n.c_str());
}

void TraceValueRegister::UnregisterTraceValue(TraceValue *t) {
    size_t idx = _tvr_scopeprefix.length();
    string n = t->name().substr(idx);
    for (valmap_t::iterator i = _tvr_values.begin(); i != _tvr_values.end(); i++) {
        if(n == *(i->first)) {
            _tvr_values.erase(i);
            break;
        }
    }
}

TraceValueRegister* TraceValueRegister::GetScopeGroupByName(const std::string &name) {
    for (regmap_t::iterator i = _tvr_registers.begin(); i != _tvr_registers.end(); i++) {
        if(name == *(i->first))
            return i->second;
    }
    return NULL;
}

TraceValue* TraceValueRegister::GetTraceValueByName(const std::string &name) {
    for (valmap_t::iterator i = _tvr_values.begin(); i != _tvr_values.end(); i++) {
        if(name == *(i->first))
            return i->second;
    }
    return NULL;
}

TraceValueRegister* TraceValueRegister::FindScopeGroupByName(const std::string &name) {
    size_t idx = name.find('.');
    if(idx != 0 && idx != string::npos) {
        TraceValueRegister *r = GetScopeGroupByName(name.substr(0, idx));
        if(r == NULL)
            return NULL;
        else
            return r->FindScopeGroupByName(name.substr(idx + 1));
    } else
        return GetScopeGroupByName(name);
}

TraceValue* TraceValueRegister::FindTraceValueByName(const std::string &name) {
    size_t idx = name.find('.');
    if(idx != 0 && idx != string::npos) {
        TraceValueRegister *r = GetScopeGroupByName(name.substr(0, idx));
        if(r == NULL)
            return NULL;
        else
            return r->FindTraceValueByName(name.substr(idx + 1));
    } else
        return GetTraceValueByName(name);
}

TraceSet* TraceValueRegister::GetAllTraceValues(void) {
    TraceSet* result = new TraceSet;
    result->reserve(_tvr_values.size());
    for (valmap_t::iterator i = _tvr_values.begin(); i != _tvr_values.end(); i++)
        result->push_back(i->second);
    return result;
}

TraceSet* TraceValueRegister::GetAllTraceValuesRecursive(void) {
    TraceSet* result = new TraceSet;
    result->reserve(_tvr_getValuesCount());
    _tvr_insertTraceValuesToSet(*result);
    return result;
}

TraceValueCoreRegister::TraceValueCoreRegister(TraceValueRegister *parent):
    TraceValueRegister(parent, "CORE") {}

void TraceValueCoreRegister::RegisterTraceSetValue(TraceValue *t, const std::string &name, const size_t size) {
    // seek TraceSet
    TraceSet *set = NULL;
    for(setmap_t::iterator i = _tvr_valset.begin(); i != _tvr_valset.end(); i++) {
        if(name == *(i->first)) {
            set = i->second;
            break;
        }
    }
    // create TraceSet, if not found
    if(set == NULL) {
        set = new TraceSet(size, NULL);
        string *s = new string(name);
        pair<string*, TraceSet*> v(s, set);
        _tvr_valset.insert(v);
    }
    // set TraceValue to set[idx]
    (*set)[t->index()] = t;
}

TraceValue* TraceValueCoreRegister::GetTraceValueByName(const std::string &name) {
    TraceValue *res = TraceValueRegister::GetTraceValueByName(name);
    if(res == NULL) {
        size_t idx = _tvr_numberindex(name);
        if(idx != string::npos) {
            // name + number found, check name and index value
            string n = name.substr(0, idx);
            int v = atoi(name.substr(idx).c_str());
            for(setmap_t::iterator i = _tvr_valset.begin(); i != _tvr_valset.end(); i++) {
                if(n == *(i->first)) {
                    TraceSet *set = i->second;
                    if(v < (int)set->size())
                        res = (*set)[v];
                    break;
                }
            }
        }
    }
    return res;
}

TraceValueCoreRegister::~TraceValueCoreRegister() {
    for(setmap_t::iterator i = _tvr_valset.begin(); i != _tvr_valset.end(); i++)
        delete i->second;
}

size_t TraceValueCoreRegister::_tvr_getValuesCount(void) {
    size_t cnt = TraceValueRegister::_tvr_getValuesCount();
    // now count too values in _tvr_valset
    for(setmap_t::iterator i = _tvr_valset.begin(); i != _tvr_valset.end(); i++)
        cnt += i->second->size();
    return cnt;
}

void TraceValueCoreRegister::_tvr_insertTraceValuesToSet(TraceSet &t) {
    TraceValueRegister::_tvr_insertTraceValuesToSet(t);
    // now insert also all values from _tvr_valset
    for(setmap_t::iterator i = _tvr_valset.begin(); i != _tvr_valset.end(); i++) {
        TraceSet* s = i->second;
        for(TraceSet::iterator j = s->begin(); j != s->end(); j++)
            t.push_back(*j);
    }
}

size_t TraceValueCoreRegister::_tvr_numberindex(const std::string &str) {
    size_t l = str.size(), i;
    // start from end of string to the beginning ...
    for(i = l - 1; i >= 0; i--) {
        char c = str[i];
        // check, if number sign
        if(c < '0' || c > '9') {
            i++;
            break;
        }
    }
    if(i == l)
        i = string::npos;
    return i;
}

WarnUnknown::WarnUnknown(AvrDevice *_core) : core(_core) {}

void WarnUnknown::markReadUnknown(const TraceValue *t) {
    cerr << "READ-before-WRITE for value " << t->name()
         << " at time " << SystemClock::Instance().GetCurrentTime()
         << ", PC=0x" << hex << 2*core->PC << dec << endl;
}
bool WarnUnknown::enabled(const TraceValue *t) const {
    return true;
}

void DumpVCD::valout(const TraceValue *v) {
    osbuffer << 'b';
    for (int i = v->bits()-1; i >= 0; i--)
        osbuffer << v->VcdBit(i);

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
    tscale(_tscale),
    rs(rstrobes),
    ws(wstrobes),
    changesWritten(false),
    os(_os)
{}

DumpVCD::DumpVCD(const std::string &_name,
                 const std::string &_tscale,
                 const bool rstrobes,
                 const bool wstrobes) :
    tscale(_tscale),
    rs(rstrobes),
    ws(wstrobes),
    changesWritten(false),
    os(new ofstream(_name.c_str()))
{}

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
    SystemClockOffset clock=SystemClock::Instance().GetCurrentTime();
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
    SystemClockOffset clock=SystemClock::Instance().GetCurrentTime();
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

int DumpManager::_devidx = 0;
DumpManager *::DumpManager::_instance = NULL;

DumpManager* DumpManager::Instance(void) {
    if(_instance == NULL)
        _instance = new DumpManager();
    return _instance;
}

void DumpManager::Reset(void) {
    if(_instance) {
        _instance->detachAvrDevices();
        delete _instance;
    }
    _instance = NULL;
    _devidx = 0;
}

DumpManager::DumpManager() {
    singleDeviceApp = false;
}

void DumpManager::appendDeviceName(std::string &s) {
    _devidx++;
    if(singleDeviceApp && _devidx > 1)
        avr_error("Can't create device name twice, because it's a single device application");
    if(!singleDeviceApp)
        s += "Dev" + int2str(_devidx);
}

void DumpManager::registerAvrDevice(AvrDevice* dev) {
    devices.push_back(dev);
}

void DumpManager::unregisterAvrDevice(AvrDevice* dev) {
    vector<AvrDevice*> dl;
    for(vector<AvrDevice*>::iterator i = devices.begin(); i != devices.end(); i++) {
        AvrDevice* d = *i;
        if(d != dev)
            dl.push_back(d);
    }
    devices.swap(dl);
}

void DumpManager::detachAvrDevices() {
    vector<AvrDevice*> dl;

    for(vector<AvrDevice*>::iterator i = devices.begin(); i != devices.end(); i++) {
        AvrDevice* d = *i;
        d->detachDumpManager();
    }
}

TraceValue* DumpManager::seekValueByName(const std::string &name) {
    if(singleDeviceApp) {
        if(devices.size() == 0)
            return NULL;
        return devices[0]->FindTraceValueByName(name);
    } else {
        size_t idx = name.find('.');
        if(idx == 0 || idx == string::npos)
            return NULL;
        for(vector<AvrDevice*>::iterator i = devices.begin(); i != devices.end(); i++) {
            if((*i)->GetScopeName() == name.substr(0, idx)) {
                return (*i)->FindTraceValueByName(name.substr(idx + 1));
            }
        }
        return NULL;
    }
}

void DumpManager::SetSingleDeviceApp(void) {
    if(devices.size() > 0)
        avr_error("method SetSingleDeviceApp must be called before devices are added to DumpManager");
    singleDeviceApp = true;
}

void DumpManager::addDumper(Dumper *dump, const TraceSet &vals) {
    // enable values and insert into active list, if not there
    for(TraceSet::const_iterator i = vals.begin(); i != vals.end(); i++) {
        (*i)->enable();
        if(find(active.begin(), active.end(), *i) == active.end())
            active.push_back(*i);
    }
    
    // check, if dumper exists in dumps list
    if(find(dumps.begin(), dumps.end(), dump) != dumps.end())
        avr_error("Internal error: Dumper already registered.");
    // set active signals for dumper
    dump->setActiveSignals(vals);
    // and insert dumper in dumps list
    dumps.push_back(dump);
}

const TraceSet& DumpManager::all() {
    TraceSet* s;
    
    // clear list
    _all.clear();
    
    // over all registered devices
    for(vector<AvrDevice*>::const_iterator d = devices.begin(); d != devices.end(); d++) {
        // all values from device
        s = (*d)->GetAllTraceValuesRecursive();
        // change allocated vector size
        _all.reserve(_all.size() + s->size());
        // append all values from device list to result list
        for(TraceSet::const_iterator i = s->begin(); i != s->end(); i++)
            _all.push_back(*i);
        delete s;
    }
    
    // return resulting list
    return _all;
}

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

void DumpManager::stopApplication(void) {
    for(size_t i = 0; i < dumps.size(); i++) {
        dumps[i]->stop(); // inform dumper to stop output
        delete dumps[i];
    }
    dumps.clear();
}

void DumpManager::save(ostream &os) const {
    TraceSet* s;
    for(vector<AvrDevice*>::const_iterator d = devices.begin(); d != devices.end(); d++) {
        s = (*d)->GetAllTraceValuesRecursive();
        for(TraceSet::const_iterator i = s->begin(); i != s->end(); i++) {
            TraceValue& tv = *(*i);
            if (tv.index() >= 0) {
                TraceSet::const_iterator j = i;
                int c = tv.index();
                
                while(((*i)->barename() == tv.barename()) &&
                      (c == (*i)->index())) {
                    i++;
                    c++;
                }
                i--; c--;
                
                if(c) {
                    os << "| " << tv.barename() << ' ' 
                       << tv.index() << " .. "
                       << (*i)->index() << '\n';
                } else {
                    os << "+ " << (*i)->name() << '\n';
                }
            } else
                os << "+ " << (*i)->name() << '\n';
        }
        delete s;
    }
}

TraceSet DumpManager::load(istream &is) {
    TraceSet res;
    
    while(!is.eof()) {
        // read line from stream and split it up
        string l = readline(is);
        vector<string> ls = split(l);
        
        // empty line or to short?
        if(ls.size() < 2) continue;
        
        if(ls[0] == "+") {
            // single value, get name
            string n = ls[1];
            // seek value
            TraceValue *t = seekValueByName(n);
            if(t == NULL)
                avr_error("TraceValue '%s' is not known.", n.c_str());
            // insert to list
            res.push_back(t);
        } else if(ls[0] == "|") {
            // value range?
            if(ls[3] != "..")
                avr_error("'..' expected between range limits.");
            // get name and range values
            string bn = ls[1];
            size_t min = atoi(ls[2].c_str());
            size_t max = atoi(ls[4].c_str());
            // seek for all values in range
            for(size_t i = min; i <= max; i++) {
                string n = ls[1] + int2str(i);
                TraceValue *t = seekValueByName(n);
                if(t == NULL)
                    avr_error("While constructing range with '%s', TraceValue is not known.", n.c_str());
                // insert to list
                res.push_back(t);
            }
        } else if(ls[0][0] != '#')
            // not a comment, then it's an error
            avr_error("Invalid trace value specifier '%s'.", ls[0].c_str());
    }
    return res;
}

TraceSet DumpManager::load(const string &istr) {
    istringstream is(istr.c_str());
    return load(is);
}

TraceValue* trace_direct(TraceValueRegister *t, const std::string &name, const bool *val) {
    TraceValue *tv=new TraceValue(1, t->GetTraceValuePrefix() + name,
                                  -1, val);
    t->RegisterTraceValue(tv);
    return tv;
}

TraceValue* trace_direct(TraceValueRegister *t, const std::string &name, const uint8_t*val) {
    TraceValue* tv=new TraceValue(8, t->GetTraceValuePrefix() + name,
                                  -1, val);
    t->RegisterTraceValue(tv);
    return tv;
}

TraceValue* trace_direct(TraceValueRegister *t, const std::string &name, const uint16_t*val) {
    TraceValue* tv=new TraceValue(16, t->GetTraceValuePrefix() + name,
                                  -1, val);
    t->RegisterTraceValue(tv);
    return tv;
}

TraceValue* trace_direct(TraceValueRegister *t, const std::string &name, const uint32_t*val) {
    TraceValue* tv=new TraceValue(32, t->GetTraceValuePrefix() + name,
                                  -1, val);
    t->RegisterTraceValue(tv);
    return tv;
}

