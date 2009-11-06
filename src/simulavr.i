%module simulavr

%include std_string.i

%{
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
#include "ui/scope.h"
    
SystemClock &GetSystemClock() { return SystemClock::Instance(); }
%}

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
%include "ui/scope.h"

void setVerbose(int value);
void setMessageOnBadAccess(int value);
void StartTrace(const char*);
SystemClock &GetSystemClock();
