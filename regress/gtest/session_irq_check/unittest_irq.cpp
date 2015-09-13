#include <iostream>
using namespace std;

#include "gtest.h"

#include "avrdevice.h"
#include "atmega16_32.h"
#include "systemclock.h"

TEST( SESSION_IRQ, IRQ1 )
{
   AvrDevice *dev1= new AvrDevice_atmega32;
   dev1->Load("session_irq_check/check.atmega32.o");
   dev1->SetClockFreq(136);    // 7.3728
   dev1->RegisterTerminationSymbol("stopsim");
   SystemClock::Instance().Add(dev1);
   SystemClock::Instance().Endless(); 

   EXPECT_EQ(0x08, (unsigned char)(*(dev1->rw[0x5e]))) << "Wrong Stack pointer SPH found" << endl;
   EXPECT_EQ(0x5b, (unsigned char)(*(dev1->rw[0x5d]))) << "Wrong Stack pointer SPL found" << endl;
}

TEST( SESSION_IRQ, TC1)
{
   AvrDevice *dev1= new AvrDevice_atmega32;
   dev1->Load("session_irq_check/tc1.atmega32.o");
   dev1->SetClockFreq(136);    // 7.3728
   dev1->RegisterTerminationSymbol("stopsim");
   dev1->RegisterTerminationSymbol("exit");
   SystemClock::Instance().Add(dev1);
   SystemClock::Instance().Endless();

   EXPECT_EQ( 0x55, (unsigned char)(*(dev1->rw[0x38]))) << "Wrong PORTB value" << endl;
}
TEST( SESSION_IRQ, TC2)
{
   AvrDevice *dev1= new AvrDevice_atmega32;
   dev1->Load("session_irq_check/tc2.atmega32.o");
   dev1->SetClockFreq(136);    // 7.3728
   dev1->RegisterTerminationSymbol("stopsim");
   dev1->RegisterTerminationSymbol("exit");
   SystemClock::Instance().Add(dev1);
   SystemClock::Instance().Endless();

   EXPECT_EQ( 0x55, (unsigned char)(*(dev1->rw[0x38]))) << "Wrong PORTB value" << endl;
}

TEST( SESSION_IRQ, TC3)
{
   AvrDevice *dev1= new AvrDevice_atmega32;
   dev1->Load("session_irq_check/tc3.atmega32.o");
   dev1->SetClockFreq(136);    // 7.3728
   dev1->RegisterTerminationSymbol("stopsim");
   dev1->RegisterTerminationSymbol("exit");
   SystemClock::Instance().Add(dev1);
   SystemClock::Instance().Endless();

   EXPECT_EQ( 0x55, (unsigned char)(*(dev1->rw[0x38]))) << "Wrong PORTB value" << endl;
}

TEST( SESSION_IRQ, TC4)
{
   AvrDevice *dev1= new AvrDevice_atmega32;
   dev1->Load("session_irq_check/tc4.atmega32.o");
   dev1->SetClockFreq(136);    // 7.3728
   dev1->RegisterTerminationSymbol("stopsim");
   dev1->RegisterTerminationSymbol("exit");
   SystemClock::Instance().Add(dev1);
   SystemClock::Instance().Endless();

   EXPECT_EQ( 0xff, (unsigned char)(*(dev1->rw[0x38]))) << "Wrong PORTB value" << endl;
   EXPECT_EQ( 0x01, (unsigned char)(*(dev1->rw[18]  ))) << "Wrong value in R18" << endl;
   EXPECT_EQ( 0x02, (unsigned char)(*(dev1->rw[19]  ))) << "Wrong value in R19" << endl;
   EXPECT_EQ( 0x00, (unsigned char)(*(dev1->rw[20]  ))) << "Wrong value in R20" << endl;
}
