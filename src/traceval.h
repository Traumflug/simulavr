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
#ifndef traceval_h
#define traceval_h

#include <stdint.h>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>

/* TODO, notes:

   ===========================================================   
   Goals for tracing functionality for values in the simulator
   ===========================================================
   
   - being dumped into a file every time they changed (classical hardware
   register change use)

   - being dumped into a file every time they have been write-accessed
   (register with 'write-will-cause-something' functionality

   - enable statistics counters for accesses (such as EEPROM)
   
   - for eeproms, it should also be possible in particular to have an easy
   tracing for whole blocks of memory
   
   - statistics on the number of accesses on the FLASH can then be used for
   profiling and on the RAM for stack size checking etc.

      ... this leads to ...   
   ============================
   List of tracers to implement
   ============================
   - VCD dumping on changing a value. VCD dumping of the writes AND the READS
   itself as some kind of visible strobes may also be of interest!
   
   - Human-readable dumping of a simulavrxx trace, with register changes in a
   column next to the current opcode line etc. (much like the current tracing,
   but much cleaner in the code). Human-readable dumping should also say
   whether there has been a read-access or a write-access and whether the
   value changed during write!
   
   Also the interface should be able to warn when there is a read-access
   before a write access (e.g. read access of SRAM in unknown state).

   =====================   
   Ideas for further use
   =====================

   - memory array access profiling

   - 'wear map' for eeproms

   - For RAM, map with addresses that have been written at all
   
   - For flash, give out a map of read access counters (write-acesses later on
   for boot loaders??) for each flash address.  A separate tool could then be
   used to merge the output of avr-objdump -S and this to produce nice
   per-line profiling statistics. */

class Dumper;

/*! Abstract interface for traceable values.
  Traced values can be written (marking it with a WRITE flag
  and if the value changed also a CHANGE flag. If the traced value
  has been written once, it is marked 'written()' for the whole simulation.
  They can also be read, marking the READ flag.

  For values where no accessors for read and write can be intercepted,
  it is also possible to use the cycle() method (activated when the
  traceval is initialized with a shadow ptr !=0), which will then simply
  update the state of the value during the cycle() method by comparing
  it with the internal state. This does not allow to trace read and write
  accesses, but all state changes will still be represented in the output file.
  This is helpful for e.g. tracing the hidden shadow states in various
  parts of the AVR hardware, such as the timer double buffers.
  */
class TraceValue {
    
    public:
        //! Generate a new unitialized trace value of width bits
        TraceValue(size_t bits,
                   const std::string &_name,
                   const int __index=-1,
                   const void* shadow=0);
        virtual ~TraceValue() {}

        //! Give number of bits for this value. Max 32.
        size_t bits() const;
    
        //! Gives the saved shadow value for this trace value.
        /*! Note that the shadow value does not necessarily reflect
          the *current* value of the traced variable.
        */
        unsigned value() const;
        
        //! Give name (fully qualified), including the index appended if it is >=0
        std::string name() const;
    
        //! Gives the name without the index
        std::string barename() const;
    
        //! Gives the index of this member in a memory field (or -1)
        int index() const;

        //! Possible access types for a trace value
        enum Atype {
        READ=1, // true if a READ access has been logged
        WRITE=2,
        CHANGE=4
        };
    
        /*! Enabled? All operations should be skipped if a trace value is not
          enabled. */
        bool enabled() const;
    
        //! Enable tracing
        void enable();
        
        
        //! Log a change on this value
        void change(unsigned val);
        void change(unsigned val, unsigned mask);
        //! Log a write access on this value
        void write(unsigned val);
        //! Log a read access
        void read();
        
    
        /*! Gives true if this value has been written at one point during the
          simulation. */
        bool written() const;
    
        /*! Just set the written flag for tracevalues which are automatically
          initialized (IO registers etc.) */
        void set_written();
        
        /*! Just set the written flag for tracevalues which are automatically
          initialized (IO registers etc.)
          
          This method sets not only the _written flag, it set's also the saved
          value for detecting changes. */
        void set_written(unsigned val);
        
        //! Gives the current set of flag readings
        Atype flags() const;
        
        //! Called at least once for each cycle if this trace value is activated
        /*! This may check for updates to an underlying referenced value etc.
          and update the flags accordingly. */
        virtual void cycle();
        
        /*! Dump the state or state change somewhere. This also resets the current
          flags. */
        virtual void dump(Dumper &d);
        
        /*! Give back VCD coding of a bit */
        virtual char VcdBit(int bitNo) const;

    protected:
        //! Clear all access flags
        void clear_flags();
        friend class TraceKeeper;
        
    private:
        std::string _name;
    
        int _index;
    
        //! number of bits
        const unsigned b;
    
        //! shadow reg, if used
        const void *shadow;
    
        //! The value itself
        unsigned v;
        //! accesses since last dump/clearflagsd
        int f;
        /*! Initialized to zero upon creation and any logged write will make this
          true. */
        bool _written;
    
        //! Tracing of this value enabled at all?
        /*! Note that it must additionally be enabled in the particular
          Dumper. */
        bool _enabled;
};

class TraceValueOutput: public TraceValue {

    public:
        /*! Generate a new uninitialized trace value of pin output driver */
        TraceValueOutput(const std::string &_name): TraceValue(1, _name) {}

        /*! Give back VCD coding of pin output driver  */
        virtual char VcdBit(int bitNo) const;

};

class AvrDevice;
class TraceValueRegister;

typedef std::vector<TraceValue*> TraceSet;

/*! Generic interface for a trace value processor */
class Dumper {
    
    public:
        /*! Called with the set of all active signals,
          after they've been specified. */
        virtual void setActiveSignals(const TraceSet &act) {}
    
        //! Called before start of tracing
        virtual void start() {}
        //! Called after stopping tracing
        virtual void stop() {}
    
        //! Called for each cycle before dumping the values
        virtual void cycle() {}
        
        /*! Called when a traced value has been read (as long as it supports read
          logging!) */
        virtual void markRead(const TraceValue *t) {}
        /*! Called for all values which are read before they have been written. */
        virtual void markReadUnknown(const TraceValue *t) {}
        
        /*! Called when a traced value has been written (as long as it supports
          write logging!) */
        virtual void markWrite(const TraceValue *t) {}
        /*! Called when the value has changed. This is mainly used for values which
          do not have READ/WRITE notification by checking for changes after
          each clock cycle. All writes changing something also appear as a change.*/
        virtual void markChange(const TraceValue *t) {}
    
        //! Destructor, called for all dumpers at the very end of the run
        /*! Should close files etc. */
        virtual ~Dumper() {}
    
        //! Returns true iff tracing a particular value is enabled
        /*! FIXME: For a lot of values to trace,
          checking enabled() each time by doing find on a map()
          could be slow. Here is potential for more optimization! */
        virtual bool enabled(const TraceValue *t) const=0;
};

/*! Very simple dumper which will simply warn on unknown read
  accesses on stderr. */
class WarnUnknown : public Dumper {
    
    public:
        WarnUnknown(AvrDevice *core);
        void markReadUnknown(const TraceValue *t);
        bool enabled(const TraceValue *t) const;
        
    private:
        AvrDevice *core;
};

/*! Produces value change dump files. */
class DumpVCD : public Dumper {
    
    public:
        //! Create tracer with time scale tscale and output os
        DumpVCD(std::ostream *os, const std::string &tscale = "ns",
            const bool rstrobes = false, const bool wstrobes = false);
        
        //! Create tracer with time scale tscale for output file name
        DumpVCD(const std::string &name, const std::string &tscale = "ns",
            const bool rstrobes = false, const bool wstrobes = false);
        
        void setActiveSignals(const TraceSet &act);
    
        //! Writes header stuff and the initial state
        void start();
    
        //! Writes a last time marker
        void stop();
    
        //! Writes next clock cycle and resets all RS and WS states
        void cycle();
    
        /*! Iff rstrobes is true, this will mark reads on a special
          R-strobe signal line. */
        void markRead(const TraceValue *t);
    
        /*! Iff wstrobes is true, this will mark writes on a special
          W-strobe signal line .*/
        void markWrite(const TraceValue *t);
        
        /*! This will produce a change in the value CHANGE dump file :-) */
        void markChange(const TraceValue *t);
    
        bool enabled(const TraceValue *t) const;
        ~DumpVCD();
        
    private:
        TraceSet tv;
        std::map<const TraceValue*, size_t> id2num;
        const std::string tscale;
        const bool rs, ws;
        bool changesWritten;
        
        // list of signals marked last cycle
        std::vector<int> marked;
        std::ostream *os;
    
        // buffer for change data
        std::stringstream osbuffer;
        
        void valout(const TraceValue *v);
        
        //! writes content of osbuffer to os and empty osbuffer afterwards
        void flushbuffer(void);
};

/*! Manages all active Dumper instances for a given AvrDevice.
  It also manages all trace values and sets them active as necessary.
  */
class DumpManager {
    
    public:
        //! Singleton class access.
        static DumpManager* Instance(void);
        
        //! Reset DumpManager instance (e.g. delete available instance)
        static void Reset(void);

        //! Tell DumpManager, that we have only one device
        /*! In normal simulavr application we have only one device aka processor.
         But it's possible to make a simulation with 2 or more devices together.
         For that, we have to assign a unique name for every device to identify
         a device and to get a unique namespace in trace output.
         
         This method has to be called *before* the device instance will be
         created in single device application! Default is multi device application. */
         void SetSingleDeviceApp(void);
         
        /*! Add a dumper to the list. The vector vals
          contains all the values this dumper should trace. */
        void addDumper(Dumper *dump, const TraceSet &vals);
    
        /*! Start processing on all dumpers. They will be stopped when stopApplication
          method will be called or the dump manager gets destroyed. */
        void start();
    
        //! Stop processing on all dumpers and removes it from dumpers list
        void stopApplication(void);
        
        /*! Process one AVR clock cycle. Must be done after the AVR did all
          processing so that changed values etc. can be collected. */
        void cycle();
    
        //! Destroys the DumpManager instance and shut down all dumpers
        ~DumpManager() { stopApplication(); }
    
        /*! Write a list of all tracing value names into the given
          output stream. */
        void save(std::ostream &os) const;
        
        /*! Load a list of tracing values from the given input stream.
          Checks whether the values are part of the set of traceable
          values.
          @return TraceSet with found TraceValue's */
        TraceSet load(std::istream &is);

        /*! Load a list of tracing values from the given input string.
          Checks whether the values are part of the set of traceable
          values.
          @return TraceSet with found TraceValue's */
        TraceSet load(const std::string &istr);

        /*! Gives all available tracers as a set. */
        const TraceSet& all();
        
    private:
        friend class TraceValueRegister;
        friend class AvrDevice;
        
        //! Private instance constructor
        DumpManager();
        
        //! append a unique device name to a string
        void appendDeviceName(std::string &s);
        
        //! Add a device to devicelist
        void registerAvrDevice(AvrDevice* dev);
        
        //! Remove a device from devicelist
        void unregisterAvrDevice(AvrDevice* dev);
        
        //! detach all devices
        void detachAvrDevices();

        //! Seek value by name in all devices
        TraceValue* seekValueByName(const std::string &name);
        
        //! Flag, if we use only one device, e.g. assign no device name
        bool singleDeviceApp;
        
        //! Set of active tracing values
        TraceSet active;
        //! Set of all traceable values (placeholder instance for all() method)
        TraceSet _all;
        
        //! All dumpers, which we want to use
        std::vector<Dumper*> dumps;
        
        //! Device list
        std::vector<AvrDevice*> devices;

        static int _devidx;
        static DumpManager *_instance;
};

//! Build a register for TraceValue's
/*! This is used by DumpManager to find TraceValues by name */
class TraceValueRegister {
    
    private:
        typedef std::map<std::string*, TraceValue*> valmap_t; //!< type of values map
        typedef std::map<std::string*, TraceValueRegister*> regmap_t; //!< type of subregisters map
        
        std::string _tvr_scopename; //!< the scope name itself
        std::string _tvr_scopeprefix; //!< the prefix scope for a TraceValue name
        valmap_t _tvr_values; //!< the registered TraceValue's
        regmap_t _tvr_registers; //!< the sub-registers
        
        //! Registers a TraceValueRegister for this register, build a hierarchy
        void _tvr_registerTraceValues(TraceValueRegister *r);
        
    protected:
        //! Get the count of all TraceValues, that are registered here and descending
        virtual size_t _tvr_getValuesCount(void);
        
        //! Insert all TraceValues into TraceSet, that registered here and descending
        virtual void _tvr_insertTraceValuesToSet(TraceSet &t);
        
    public:
        //! Create a TraceValueRegister, with a scope prefix built on parent scope + name
        TraceValueRegister(TraceValueRegister *parent, const std::string &name):
            _tvr_scopename(name),
            _tvr_scopeprefix(parent->GetTraceValuePrefix() + name + ".")
        {
            parent->_tvr_registerTraceValues(this);
        }
        //! Create a TraceValueRegister, with a empty scope name, single device application
        TraceValueRegister():
            _tvr_scopename(""),
            _tvr_scopeprefix("")
        {
            DumpManager::Instance()->appendDeviceName(_tvr_scopename);
            if(_tvr_scopename.length() > 0)
                _tvr_scopeprefix += _tvr_scopename + ".";
        }
        virtual ~TraceValueRegister();
        
        //! Returns the scope prefix
        const std::string GetTraceValuePrefix(void) { return _tvr_scopeprefix; }
        //! Returns the scope name
        const std::string GetScopeName(void) { return _tvr_scopename; }
        //! Registers a TraceValue for this register
        void RegisterTraceValue(TraceValue *t);
        //! Unregisters a TraceValue, remove it from register
        void UnregisterTraceValue(TraceValue *t);
        //! Get a here registered TraceValueRegister by it's name
        TraceValueRegister* GetScopeGroupByName(const std::string &name);
        //! Get a here registered TraceValue by it's name
        virtual TraceValue* GetTraceValueByName(const std::string &name);
        //! Seek for a TraceValueRegister by it's name
        TraceValueRegister* FindScopeGroupByName(const std::string &name);
        //! Seek for a TraceValue by it's name
        TraceValue* FindTraceValueByName(const std::string &name);
        //! Get all here registered TraceValue's only (not with descending values)
        TraceSet* GetAllTraceValues(void);
        //! Get all here registered TraceValue's with descending values
        TraceSet* GetAllTraceValuesRecursive(void);
};

/*! TraceValueRegister for CORE group to hold also RAM groups */
class TraceValueCoreRegister: public TraceValueRegister {
  
    private:
        typedef std::map<std::string*, TraceSet*> setmap_t; //!< type of TraceSet map
        
        setmap_t _tvr_valset; //!< the registered TraceValue's

        //! helper function to split up into name an number tail
        size_t _tvr_numberindex(const std::string &str);
        
    protected:
        //! Get the count of all TraceValues, that are registered here and descending
        /*! This includes here also values in _tvr_valset! */
        virtual size_t _tvr_getValuesCount(void);
        
        //! Insert all TraceValues into TraceSet, that registered here and descending
        /*! This includes here also values in _tvr_valset! */
        virtual void _tvr_insertTraceValuesToSet(TraceSet &t);
        
    public:
        //! Create a TraceValueCoreRegister instance
        TraceValueCoreRegister(TraceValueRegister *parent);
        
        ~TraceValueCoreRegister();
        
        //! Registers a TraceValue for this register
        void RegisterTraceSetValue(TraceValue *t, const std::string &name, const size_t size);
        //! Get a here registered TraceValue by it's name
        virtual TraceValue* GetTraceValueByName(const std::string &name);
};

//! Register a directly traced bool value
/*! \return pointer to the new registered TraceValue */
TraceValue *trace_direct(TraceValueRegister *t, const std::string &name, const bool *val);

//! Register a directly traced byte value
/*! \return pointer to the new registered TraceValue */
TraceValue* trace_direct(TraceValueRegister *t, const std::string &name, const uint8_t *val);

//! Register a directly traced 16bit word value
/*! \return pointer to the new registered TraceValue */
TraceValue* trace_direct(TraceValueRegister *t, const std::string &name, const uint16_t *val);

//! Register a directly traced 32bit word value
/*! \return pointer to the new registered TraceValue */
TraceValue* trace_direct(TraceValueRegister *t, const std::string &name, const uint32_t *val);

#endif
