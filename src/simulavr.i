%module simulavr

%include std_string.i

%{
#include "config.h"
#include "systemclocktypes.h"
#include "traceval.h"
#include "avrdevice.h"
#include "avrfactory.h"
#include "at8515.h"
#include "atmega128.h"
#include "at4433.h"
#include "systemclock.h"
#include "ui/ui.h"
#include "hardware.h"
#include "pin.h"
#include "ui/extpin.h"
#include "net.h"
#include "cmd/gdb.h"
#include "ui/keyboard.h"
#include "ui/lcd.h"
#include "ui/serialrx.h"
#include "ui/serialtx.h"
#include "spisrc.h"
#include "spisink.h"
#include "adcpin.h"
#include "pinmon.h"
#include "rwmem.h"
#include "specialmem.h"
#include "ui/scope.h"
#include "avrerror.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#   pragma message ("If link fails because of missing pythonXY_d.lib then")
#   pragma message ("you need to edit simulavr_wrap.cxx and")
#   pragma message ("find #include <Python.h> and replace with #include <io.h>,")
#   pragma message ("and #undef _DEBUG and #include <Python.h>, and #define _DEBUG")
#endif

SystemClock &GetSystemClock() { return SystemClock::Instance(); }
%}

%include "std_vector.i"

namespace std {
   %template(DWordVector) vector<dword>;
};

%immutable HWStack::m_ThreadList;

%include "systemclocktypes.h"
%include "simulationmember.h"
%include "externaltype.h"
%include "ui/mysocket.h"
%include "pinnotify.h"
%include "traceval.h"
%include "avrdevice.h"
%include "avrfactory.h"
%include "at8515.h"
%include "atmega128.h"
%include "at4433.h"
%include "systemclock.h"
%include "ui/ui.h"
%include "hardware.h"
%include "pin.h"

%extend Pin {
    void SetOutState(int s) {
        $self->outState = (Pin::T_Pinstate)s;
    }
}

%include "config.h"
%include "ui/extpin.h"
%include "net.h"
%include "cmd/gdb.h"
%include "ui/keyboard.h"
%include "ui/lcd.h"
%include "ui/serialrx.h"
%include "ui/serialtx.h"
%include "spisrc.h"
%include "spisink.h"
%include "adcpin.h"
%include "pinmon.h"
%include "rwmem.h"
%include "specialmem.h"
%include "ui/scope.h"
%include "avrerror.h"

SystemClock &GetSystemClock();
