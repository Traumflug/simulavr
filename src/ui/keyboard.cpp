/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph		
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 */

#include <map>
#include <sstream>
#include <stdlib.h>

#include "../avrerror.h"
#include "ui.h"   
using namespace std;

map<int,int> xToNumber;
map<int, const int* > keynumberToScancode1;
map<int, const int* > keynumberToScancode2;
map<int, const int* > keynumberToScancode3;



#include "keyboard.h"

int Keyboard::Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns) {

    //static string scanString("");  //now we use the buffer
    //unsigned char actualChar;

    static enum {
        IDLE,
        WRITE_CHANGE_DATA,
        WRITE_CHANGE_CLOCK_LOW, 
        WRITE_CHANGE_CLOCK_HIGH,
        READ
    } myState = IDLE;

    //static unsigned char lastPortValue=0;

    switch (myState ) {
        case IDLE:
            {
                if (bufferWriteIndex!=bufferReadIndex) {
                    myState=WRITE_CHANGE_DATA;
                    actualChar= buffer[bufferReadIndex];
                    bufferReadIndex=(bufferReadIndex+1)&(KBD_BUFFER_SIZE-1);
                }
            }
            break;

        case WRITE_CHANGE_DATA:
            {
                bool d;
                static bool parity;


                if (bitCnt==0) { //write start bit
                    d=0;
                    parity=0;
                } else if ((bitCnt >0 ) && ( bitCnt <=8 )) {
                    d= (((actualChar)>>(bitCnt-1)))  & 0x01;
                } else if (bitCnt == 9 ) {
                    d= parity;
                } else {
                    d= 1; //stop bit
                }

                if (d!=0) {
                    data= 'H';
                } else {
                    data='L';
                }

                parity^=d;

                *timeToNextStepIn_ns=10000;

                bitCnt++;

                if(bitCnt>11) {
                    bitCnt=0; //frame End
                    *timeToNextStepIn_ns=50000;
                    myState= IDLE;
                } else {
                    *timeToNextStepIn_ns=10000;
                    myState=WRITE_CHANGE_CLOCK_LOW;
                }
            }
            break;


        case WRITE_CHANGE_CLOCK_LOW:
            clk='L';
            *timeToNextStepIn_ns=30000;
            myState=WRITE_CHANGE_CLOCK_HIGH;
            break;

        case WRITE_CHANGE_CLOCK_HIGH:
            clk='H';
            *timeToNextStepIn_ns=40000;
            myState=WRITE_CHANGE_DATA;
            break;



        case READ:
            {
                //cout << "Read State for Keybd not implemented" << endl;
            }

            break;

        default:
            avr_error("Default state in kbd ????????????????????");
    } //end switch myState

    if (lastPortValue!= myPortValue) {
        lastPortValue=myPortValue;
        /* 
        debugOut << "Changed LCD Port Value to new value: " << hex << (unsigned int ) myPortValue << endl;
        debugOut << "Data: " << hex << (unsigned int) (myPortValue&0x0f) << endl;
        debugOut << "Flags: "  ; 

        if ( myPortValue& 16) debugOut << "ENABLE " ; 
        else debugOut << "------ ";

        if (myPortValue& 32) debugOut << "READ ";
        else debugOut << "WRITE ";

        if (myPortValue&64) debugOut << "COMMAND" << endl;
        else debugOut <<"DATA   " << endl;

        static int enableOld=0;
        if (enableOld!= ( myPortValue& 16)) {
        enableOld= ( myPortValue& 16);

        if (myPortValue& 16) { //we are now new enabled! 
        if ((myPortValue& 32)==0) { //write
        if ( readLow ==0 ) { //write HIGH
        command= (myPortValue&0x0f)<<4;
        readLow=1;
        } else {            //write Low
        readLow=0;
        command|=(myPortValue&0x0f);
        if ((myPortValue&64)) { //comand
        //debugOut << "Got new value data:" << hex << (unsigned int)command << endl;
        LcdWriteData(command);
        } else {
        //debugOut << "Got new value command:" << hex << (unsigned int)command << endl;
        LcdWriteCommand(command);
        }
        }
        }
        }
        }



*/
    }
    if(timeToNextStepIn_ns!=0) *timeToNextStepIn_ns=myClockFreq; //call as fast as possible
    return 0;
}
Keyboard::Keyboard(UserInterface *ui, const char *name, const char *baseWindow):
    clk( &myPortValue, 1),
    data( &myPortValue, 2),
bufferWriteIndex(0), bufferReadIndex(0)
{
    myPortValue=0;
    bitCnt=0;

    //ncurses setup
    ostringstream os;
    os << "create Kbd " << name  << " .x " <<  endl;
    ui->Write(os.str());

    ui->AddExternalType(name, this);
    lastPortValue=0;
#include "keytrans.h"
}

Keyboard::~Keyboard() { }

void Keyboard::SetClockFreq(SystemClockOffset f){
    myClockFreq=f;
}

int Keyboard::InsertScanCodeToBuffer( unsigned char scan) {
    if (((bufferWriteIndex+1)&(KBD_BUFFER_SIZE-1)) == bufferReadIndex) return -1; //buffer is full
    buffer[bufferWriteIndex]= scan; 
    bufferWriteIndex=(bufferWriteIndex+1)&(KBD_BUFFER_SIZE-1);

    return 0;
}

void Keyboard::InsertMakeCodeToBuffer( int key) {
    int keynum= xToNumber[key];
    if (keynum==0) return;

    //we act only for scancode 2 here :-)

    const int *scan= keynumberToScancode2[keynum];
    do {
        if (*scan==0xffff) return ; //last scan code 
        if ( InsertScanCodeToBuffer((unsigned char)(*scan)) < 0) return ; //buffer is full if <0 is returned
        scan++;

    } while (1);
}

void Keyboard::InsertBreakCodeToBuffer( int key) {
    int sendF0Pos=0;

    int keynum= xToNumber[key];
    if (keynum==0) return;

    const int *scan= keynumberToScancode2[keynum];
    if (*scan==0x00e0) { //extended Code!
        sendF0Pos=1;
    }

    do {
        if (*scan==0xffff) return ; //last scan code 
        if (sendF0Pos==0) {
            if ( InsertScanCodeToBuffer((unsigned char)(0xf0)) < 0) return ; //send break char
            sendF0Pos--; //dont send it again
        } else {
            if ( InsertScanCodeToBuffer((unsigned char)(*scan)) < 0) return ; //buffer is full if <0 is returned
            scan++;
            sendF0Pos--;
        }

    } while (1);
}

void Keyboard::SetNewValueFromUi(const string& s) {
    switch (s[0]) {
        case 'B':
            InsertBreakCodeToBuffer(atoi(s.substr(1).c_str()));
            break;

        case 'M':
            InsertMakeCodeToBuffer(atoi(s.substr(1).c_str()));
            break;

        default:
            cerr << "Unknown message for kbd-handler received from gui :-(" << endl;
    }
}

