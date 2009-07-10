#ifndef __ATMEGA88__H
#define __ATMEGA88__H 1

#include "atmega668base.h"

class AvrDevice_atmega88:public AvrDevice_atmega668base {
public:
    AvrDevice_atmega88() : AvrDevice_atmega668base(1024, 8*1024, 512) {}
    ~AvrDevice_atmega88() {} 
} ;
// AvrDevice_atmega88
#endif  // __ATMEGA88__H
