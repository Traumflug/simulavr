#include <iostream>
using namespace std;

#include "gtest.h"

#include "avrdevice.h"
#include "atmega128.h"
#include "systemclock.h"

#include "pin.h"

TEST( SESSION_IO_PIN, OPEN_DRAIN1 )
{
    AvrDevice *dev1= new AvrDevice_atmega128;
    dev1->Load("session_io_pin/tc1.atmega128.o");
    dev1->SetClockFreq(136);    // 7.3728
    dev1->RegisterTerminationSymbol("stopsim");
    SystemClock::Instance().Add(dev1);

    Net net;

    OpenDrain driveOpenDrain( dev1->GetPin("B2"));
    net.Add( &driveOpenDrain );

    net.Add( dev1->GetPin("B3"));    // read behind the open drain transistor circuit

    Pin extPullUp(Pin::PULLUP);  //create a pull up
    net.Add( &extPullUp );

    // set all other pins to defined value
    Net net2;
    net2.Add( dev1->GetPin("B0"));
    net2.Add( dev1->GetPin("B1"));
    net2.Add( dev1->GetPin("B4"));
    net2.Add( dev1->GetPin("B5"));
    net2.Add( dev1->GetPin("B6"));
    net2.Add( dev1->GetPin("B7"));

    Pin extPullDown(Pin::PULLDOWN); // create a pull down
    net2.Add( &extPullDown);


    SystemClock::Instance().Endless(); 

    EXPECT_EQ(0x08, (unsigned char)(*(dev1->rw[17]))) << "wrong value read back from PORTB R17" << endl;
    EXPECT_EQ(0x04, (unsigned char)(*(dev1->rw[18]))) << "wrong value read back from PORTB R18" << endl;
}
