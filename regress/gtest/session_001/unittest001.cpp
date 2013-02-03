#include <iostream>
using namespace std;

#include "gtest.h"

#include "avrdevice.h"
#include "atmega16_32.h"
#include "systemclock.h"

TEST( EXAMPLE_SESSION_001, TEST1)
{
    AvrDevice *dev1= new AvrDevice_atmega32;
    dev1->Load("session_001/avr_code.atmega32.o");
    dev1->SetClockFreq(136);    // 7.3728
    dev1->RegisterTerminationSymbol("myexit");
    SystemClock::Instance().Add(dev1);
    SystemClock::Instance().Endless(); // should break if myexit is reached

    // Read out Register 31
   EXPECT_EQ(0x40, (unsigned char)(*(dev1->rw[16])))<< "Register contains wrong content" << endl;
   EXPECT_EQ(0x41, (unsigned char)(*(dev1->rw[17])))<< "Register contains wrong content" << endl;
   EXPECT_EQ(0x51, (unsigned char)(*(dev1->rw[18])))<< "Register contains wrong content" << endl;
   EXPECT_EQ(0x10, (unsigned char)(*(dev1->rw[19])))<< "Register contains wrong content" << endl;


}
