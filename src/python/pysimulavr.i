%module(directors="1") pysimulavr

%{

  #include "systemclocktypes.h"
  #include "avrdevice.h"
  #include "systemclock.h"
  #include "hardware.h"
  #include "externaltype.h"
  #include "irqsystem.h"
  #include "pin.h"
  #include "net.h"
  #include "rwmem.h"
  #include "hwsreg.h"
  #include "avrfactory.h"
  #include "avrreadelf.h"
  #include "memory.h"
  #include "flash.h"
  #include "hweeprom.h"
  #include "avrerror.h"
  #include "pysimulationmember.h"
  #include "hwport.h"
  #include "hwstack.h"
  
  // to get devices registered (automatically on linux, but necessary on windows)
  #include "atmega128.h"
  #include "at4433.h"
  #include "at8515.h"
  #include "atmega668base.h"
  #include "atmega16_32.h"
  #include "attiny2313.h"
  #include "attiny25_45_85.h"
  
%}

%include "std_vector.i"
%include "std_iostream.i"
%include "std_sstream.i"
%include "types.h"

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

%include "systemclocktypes.h"
%include "simulationmember.h"

%feature("director") PySimulationMember;
%include "pysimulationmember.h"

%include "externaltype.h"
%include "pinnotify.h"
%include "traceval.h"
%include "irqsystem.h"
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
  // getRWMem and setRWMem are deprecated, don't use it in new code!
  unsigned char getRWMem(unsigned a) { return $self->GetRWMem(a); }
  bool setRWMem(unsigned a, unsigned char v) { return $self->SetRWMem(a, v); }
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
%include "hwstack.h"

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

%extend Breakpoints {
  void RemoveBreakpoint(unsigned bp) {
    Breakpoints::iterator ii;
    if ((ii = find($self->begin(), $self->end(), bp)) != $self->end()) $self->erase(ii);
  }
  void AddBreakpoint(unsigned bp) {
    $self->push_back(bp);
  }
}

%include "avrerror.h"

// to get devices registered (automatically on linux, but necessary on windows)
%include "atmega128.h"
%include "at4433.h"
%include "at8515.h"
%include "atmega668base.h"
%include "atmega16_32.h"
%include "attiny2313.h"
%include "attiny25_45_85.h"

// EOF
