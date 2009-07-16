%module simulavr

%include std_string.i

%{
#include "systemclocktypes.h"
#include "avrdevice.h"
#include "avrfactory.h"
#include "at8515.h"
#include "atmega128.h"
#include "at4433.h"
#include "systemclock.h"
#include "ui.h"
#include "hardware.h"
#include "pin.h"
#include "net.h"
#include "trace.h"
#include "cmd/gdb.h"
#include "keyboard.h"
#include "lcd.h"
#include "serialrx.h"
#include "serialtx.h"
#include "spisrc.h"
#include "spisink.h"
#include "adcpin.h"
#include "pinmon.h"
#include "rwmem.h"
#include "scope.h"
#include "traceval.h"
    
SystemClock &GetSystemClock() { return SystemClock::Instance(); }
%}

%include "systemclocktypes.h"
%include "simulationmember.h"
%include "externaltype.h"
%include "mysocket.h"
%include "pinnotify.h"
%include "avrdevice.h"
%include "avrfactory.h"
%include "at8515.h"
%include "atmega128.h"
%include "at4433.h"
%include "systemclock.h"
%include "ui.h"
%include "hardware.h"
%include "pin.h"
%include "net.h"
%include "cmd/gdb.h"
%include "keyboard.h"
%include "lcd.h"
%include "serialrx.h"
%include "serialtx.h"
%include "spisrc.h"
%include "spisink.h"
%include "adcpin.h"
%include "pinmon.h"
%include "rwmem.h"
%include "scope.h"
%include "traceval.h"

void setVerbose(int value);
void setMessageOnBadAccess(int value);
void StartTrace(const char*);
SystemClock &GetSystemClock();
