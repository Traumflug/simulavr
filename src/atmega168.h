#ifndef __ATMEGA168__H
#define __ATMEGA168__H 1

#include "atmega668base.h"

class AvrDevice_atmega168:public AvrDevice_atmega668base {
public:
    AvrDevice_atmega168() : AvrDevice_atmega668base(1024, 16*1024, 512) {}
    ~AvrDevice_atmega168() {} 
} ;
// AvrDevice_atmega168
#endif  // __ATMEGA168__H
