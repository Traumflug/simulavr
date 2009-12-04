%module(directors="1") pysimulavr

%{

  #include "systemclocktypes.h"
  #include "avrdevice.h"
  #include "systemclock.h"
  #include "hardware.h"
  #include "externaltype.h"
  #include "pin.h"
  #include "net.h"
  #include "rwmem.h"
  #include "hwsreg.h"
  #include "avrfactory.h"
  #include "memory.h"
  #include "flash.h"
  #include "hweeprom.h"
  #include "breakpoint.h"
  #include "global.h"
  #include "avrerror.h"
  #include "pysimulationmember.h"
  #include "hwport.h"
  
  // to get devices registered (automatically on linux, but necessary on windows)
  #include "atmega128.h"
  #include "at4433.h"
  #include "at8515.h"
  #include "atmega668base.h"
  #include "atmega16_32.h"
  #include "attiny2313.h"
  
%}

%include "std_vector.i"
%include "std_iostream.i"
%include "std_sstream.i"

namespace std {
   %template(DWordVector) vector<dword>;
};

%exception {
  try {
    $action
  } catch(char const* s) {
    PyErr_SetString(PyExc_SystemError, s);
    return NULL;
  } catch(int i) {
    PyErr_Format(PyExc_RuntimeError, "%d", i);
    return NULL;
  }
}

%include "types.h"
%include "systemclocktypes.h"
%include "simulationmember.h"

%feature("director") PySimulationMember;
%include "pysimulationmember.h"

%include "externaltype.h"
%include "pinnotify.h"
%include "traceval.h"
%include "avrdevice.h"

%extend DumpManager {
  void addDumpVCD(const std::string &vcdname,
                  const std::string &istr,
                  const std::string &timebase,
                  const bool rstrobe,
                  const bool wstrobe) {
    DumpVCD *d = new DumpVCD(vcdname, timebase, rstrobe, wstrobe);
    $self->addDumper(d, $self->load(istr));
  }
}

%extend AvrDevice {
  unsigned char getRWMem(unsigned a) {
    if(a >= $self->GetMemTotalSize())
      return (unsigned char)0;
    return (unsigned char)*($self->rw[a]);
  }
  bool setRWMem(unsigned a, unsigned char v) {
    if(a >= $self->GetMemTotalSize())
      return false;
    *($self->rw[a]) = v;
    return true;
  }
  bool replaceRWMemCell(unsigned a, RWMemoryMember* c) {
    if(a >= $self->GetMemTotalSize())
      return false;
    $self->rw[a] = c;
    return true;
  }
  RWMemoryMember* getRWMemCell(unsigned a) {
    if(a >= $self->GetMemTotalSize())
      return NULL;
    return $self->rw[a];
  }
}

%include "systemclock.h"

%extend SystemClock {
  int Step() {
    bool untilCoreStepFinished = false;
    return $self->Step(untilCoreStepFinished);
  }
}

%feature("director") Hardware;
%include "hardware.h"
%include "hwport.h"

%feature("director") Pin;
%include "pin.h"

%extend Pin {
  char toChar() {
    char c = *$self;
    return c;
  }
  void SetPin(const char c) {
    *$self = c;
  }
}

%include "net.h"

%feature("director") RWMemoryMember;
%include "rwmem.h"

%include "hwsreg.h"
%extend RWSreg {
  unsigned char GetValue(void) {
    unsigned char v = *$self;
    return v;
  }
  void SetValue(unsigned char v) {
    RWMemoryMember* m = $self;
    *m = v;
  }
}

%include "avrfactory.h"
%include "memory.h"

%extend Memory {
  unsigned char GetMemory(unsigned int a) {
    if(a < $self->GetSize())
      return $self->myMemory[a];
    return 0;
  }
  void PyWriteMem(char *src, unsigned int offset, unsigned int secSize) {
    $self->WriteMem((unsigned char *)src, offset, secSize);
  }
}

%include "flash.h"
%include "hweeprom.h"
%include "breakpoint.h"

%extend Breakpoints {
  void RemoveBreakpoint(unsigned bp) {
    Breakpoints::iterator ii;
    if ((ii = find($self->begin(), $self->end(), bp)) != $self->end()) $self->erase(ii);
  }
  void AddBreakpoint(unsigned bp) {
    $self->push_back(bp);
  }
}

%include "global.h"
%include "avrerror.h"

// to get devices registered (automatically on linux, but necessary on windows)
%include "atmega128.h"
%include "at4433.h"
%include "at8515.h"
%include "atmega668base.h"
%include "atmega16_32.h"
%include "attiny2313.h"

// EOF
