%module simulavr
%{
#include "avrdevice.h"
#include "at8515.h"
#include "atmega128.h"
#include "at8515special.h"
#include "at4433.h"
#include "systemclock.h"
#include "ui.h"
#include "hardware.h"
#include "pin.h"
#include "net.h"
#include "trace.h"
#include "gdb.h"
%}

%include "avrdevice.h"
%include "at8515.h"
%include "atmega128.h"
%include "at8515special.h"
%include "at4433.h"
%include "systemclock.h"
%include "ui.h"
%include "hardware.h"
%include "pin.h"
%include "net.h"
%include "gdb.h"
void StartTrace(const char*);





