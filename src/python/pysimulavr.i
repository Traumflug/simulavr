%module pysimulavr

%{
  #include "systemclocktypes.h"
  #include "avrdevice.h"
  #include "systemclock.h"
  #include "hardware.h"
  #include "pin.h"
  #include "net.h"
  #include "trace.h"
  #include "gdb.h"
  #include "rwmem.h"
  #include "avrfactory.h"
  #include "memory.h"
  #include "flash.h"
  #include "rwmem.h"
  #include "breakpoint.h"
  #include "global.h"
%}

%include "std_vector.i"

namespace std {
   %template(DWordVector) vector<dword>;
};

%include "types.h"
%include "systemclocktypes.h"
%include "simulationmember.h"
%include "externaltype.h"
%include "pinnotify.h"
%include "avrdevice.h"

%extend AvrDevice {
  unsigned char getRWMem(unsigned a) {
    return (unsigned char)*($self->rw[a]);
  }
  void setRWMem(unsigned a, unsigned char v) {
    *($self->rw[a]) = v;
  }
}

%include "systemclock.h"

%extend SystemClock {
  int Step() {
    bool untilCoreStepFinished = false;
    return $self->Step(untilCoreStepFinished);
  }
  void ResetClock(void) {
    //$self->asyncMembers.clear(); // isn't reachable, because protected and no
                                 // clear method available
    $self->clear();
    $self->IncrTime(-$self->GetCurrentTime()); // Quickfix! Needs a real time reset!
  }
}

%include "hardware.h"
%include "pin.h"

%extend Pin {
  char toChar() {
    char c = *$self;
    return c;
  }
}

%include "net.h"
%include "gdb.h"
%include "rwmem.h"
%include "avrfactory.h"
%include "memory.h"

%extend Memory {
  unsigned int GetAddressAtSymbol(const char *cs) {
    std::string s = std::string(cs);
    return $self->GetAddressAtSymbol(s);
  }
}

%include "flash.h"
%include "rwmem.h"
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

// EOF
