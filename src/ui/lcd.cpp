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

#include <iostream>
#include <fstream>

#include "lcd.h"
#include "pinatport.h"

using namespace std;

// "Bit-Values for the control lines of the LCD
#define ENABLE 16
#define READWRITE 32
#define COMMANDDATA 64

int lcdStartLine []={0, 0x40, 0x14, 0x20};
// Exec Times for HD44780 5V version! At 2.7V the first time rises from 15 to 40ms!
//                 Power-on, 1st CMD, 2nd Cmd, 3rd Cmd, typical, Ret-Home
//static int Power_onTimes[]={15000000, 4100000,  100000,   37000, 1520000};
static int Power_onTimes[]={1500000, 410000,  10000,   3700, 152000};

void Lcd::LcdWriteData(unsigned char data) {
   ostringstream os;
   os << name << " WriteChar " << merke_x+1 << " " << merke_y << " " << (unsigned int)data << endl;
   //    cerr << "LcdWriteData: "<< (unsigned int)data << endl;
   ui->Write(os.str());

   merke_x++;
   SendCursorPosition();
}

void Lcd::SendCursorPosition() {
   ostringstream os;
   os << name << " MoveCursor " << merke_x << " " << merke_y << " " <<  endl;
   ui->Write(os.str());
}

unsigned int  Lcd::LcdWriteCommand(unsigned char command) {
   bool lerr = false;
   if (command >= 0x80 ) { //goto

      int line;
      int value=command-0x80;
      if (value >= 0x54) {line=3; value-=0x54;} else {
         if (value >= 0x40) {line=1; value-=0x40;} else {
            if (value >= 0x14) {line=2; value-=0x14;} else {
               line=0; }}}

      merke_x=value;
      merke_y=line;

      merke_x++;
      SendCursorPosition();

      return Power_onTimes[3];
   }

   if (command >= 0x40) { //Set Character Generator Address
      cerr << "Not supported LCD command: Set Character Generator Address " << endl;
      return Power_onTimes[3];
   }
   if (command >= 0x20) { //Function Set
      if ((command & 0x10)) {
         cerr << "Not supported LCD command: Set 8 Bit Interface ";
         lerr = true;
      }
      if ((command & 0x04)) {
         cerr << "Not supported LCD command: 5*10 char. size";
         lerr = true;
      }
      if (lerr == true) {
         cerr << endl;
      }
      return Power_onTimes[3];
   }

   if (command >= 0x10) { //Cursor or Display shift
      command &= 0x0c; // Mask S/L, R/L Bits, ignore rest
      switch (command) {
         case 0:
            merke_x--;
            break;
         case 4:
            merke_x++;
            break;
         case 8:
         case 0x0c:
            cerr << "Not supported LCD command: Display shift left or right" << endl;
            break;
         default:
            break;
      }
      return Power_onTimes[3];
   }

   if (command >= 8) { //Display on / off
      if (command != 0x0e) {// E = Display on, Cursor on, Cursor Blink off
         cerr << "Not supported LCD command: Display off / Cursor off / Cursor Blink" << endl;
      }
      return Power_onTimes[3];
   }

   if (command >= 4) { //Set Entry Mode
      if (command != 6) {// 6 = Increment, Cursor movement
         cerr << "Not supported LCD command: Set Entry Mode" << endl;
      }
      return Power_onTimes[3];
   }

   if (command >= 2) { //Return Home
      merke_x=0;
      merke_y=0;
      SendCursorPosition();
      return Power_onTimes[4];
   }

   if (command==1) { //clear
      for (merke_y=3; merke_y>=0; merke_y--) {
         for ( merke_x=0; merke_x<=19; ) {
            LcdWriteData(' ');
         }
      }
      merke_x=0;
      merke_y=0;
      SendCursorPosition();
      return Power_onTimes[4];
   }
   return 0;  // Should not come here! Added to avoid warning
}

t_myState setInitNext(unsigned char command,t_myState myState,unsigned int  *CmdExecTime_ns) {
   if (command == 0x30) { // Valid command for the first 3 commands
      switch (myState) {
         case POWER_ON:     // First State after Reset
            myState = PWR_AFTER_FS1;
            *CmdExecTime_ns = Power_onTimes[0]; //
            break;
         case PWR_AFTER_FS1:// After first Function Set Cmd no Busy Flag
            myState = PWR_AFTER_FS2;
            *CmdExecTime_ns = Power_onTimes[1]; //
            break;
         case PWR_AFTER_FS2:// After second Function Set Cmd no Busy Flag
            myState = PWR_ON_FINISH;
            *CmdExecTime_ns = Power_onTimes[2]; //
            break;
         default: // stop warnings
            break;
      }
   } else {
      cerr << "LCD-Init: Waiting for Function Set Command. Received: 0x" << hex << (unsigned int) command << " Dismissed!" <<endl;
   }
   return myState;
}


int Lcd::Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns) {
   //static unsigned char lastPortValue=0;
   //static int readLow = 0;
   //static unsigned char command=0;
   static unsigned int lcnt = 0;

   if (CmdExecTime_ns > 0){
      CmdExecTime_ns --;
   } else {
      CmdExecTime_ns = 0;  // Safty exit
      myd3='L';
   }
   if (lastPortValue!= myPortValue) {
      lastPortValue=myPortValue;
      /*
      debugOut << "Changed LCD Port Value to new value: " << hex << (unsigned int ) myPortValue << endl;
      debugOut << "Data: " << hex << (unsigned int) (myPortValue&0x0f) << endl;
      debugOut << "Flags: "  ;

      if ( myPortValue& ENABLE) debugOut << "ENABLE " ;
      else debugOut << "------ ";

      if (myPortValue& READWRITE) debugOut << "READ ";
      else debugOut << "WRITE ";

      if (myPortValue&COMMANDDATA) debugOut << "COMMAND" << endl;
      else debugOut <<"DATA   " << endl;
      */
      //static int enableOld=0;
      if (enableOld!= ( myPortValue& ENABLE)) {
         enableOld= ( myPortValue& ENABLE);
         d3='t';  // Set output to tristate
         if (myPortValue& ENABLE) { //we are now new enabled!
            if ((myPortValue& READWRITE)==0) { //write
               if (CmdExecTime_ns >= 1000) { // 1uS we ignore
                  cerr << "LCD busy for another " << CmdExecTime_ns /1000 << "us" << endl;
                  // Trace file entry?
               }
               if ( readLow ==0 ) { //write HIGH
                  command= (myPortValue&0x0f)<<4;
                  switch (myState) {
                     case POWER_ON:
                     case PWR_AFTER_FS1:
                     case PWR_AFTER_FS2:
                        // During startup only 8 Bit commands are send to the LCD and are excepted!
                        cerr << lcnt << " Got new 8Bit value data: 0x";
                        cerr.setf(ios::hex);
                        cerr << (unsigned int)command << endl;
                        cerr.unsetf(ios::hex);
                        myState = setInitNext((command&0xf0),myState, &CmdExecTime_ns);
                        lcnt++;
                        myd3='L';
                        readLow=0;
                        break;
                     case PWR_ON_FINISH:  // Now we need a line length definition as 8Bit command!!
                        if ((command&0xf0) == 0x20) { // Valid command for the first 3 commands
                           CmdExecTime_ns = Power_onTimes[3]; // Ohhh Ohhh
                           myState = CMDEXEC;
                           myd3='H';
                        } else {
                           cerr << "LCD-Init: Waiting for Function Set Command with 4 Bit I/F. Received: 0x" << hex << (unsigned int)command << " Dismissed!" <<endl;
                        }
                        break;
                     default:
                        readLow=1;
                        break;
                  }
               } else {            //write Low
                  readLow=0;
                  command|=(myPortValue&0x0f);
                  //                        cerr << lcnt << " Got new value data:" << (unsigned int)command << endl;
                  lcnt++;
                  myd3='H';
                  if ((myPortValue&COMMANDDATA)) { //comand
                     //                            debugOut << "Got new value data:" << hex << (unsigned int)command << endl;
                     LcdWriteData (command);
                     CmdExecTime_ns = Power_onTimes[3];
                  } else {
                     //                            debugOut << "Got new value command:" << hex << (unsigned int)command << endl;
                     /* Handle the first 3 states before the normal operation */
                     switch (myState) {
                        case POWER_ON:
                        case PWR_AFTER_FS1:
                        case PWR_AFTER_FS2:
                           myd3='L';
                           myState = setInitNext((command&0xf0),myState, &CmdExecTime_ns);
                           break;
                        case PWR_ON_FINISH:  // Now we need a line length definition
                           cerr << "LCD-Init: I/F set to not not supported 8 Bit mode! Received: 0x" << hex << (unsigned int)command << " Dismissed!" <<endl;
                           break;
                        case IDLE:
                        case CMDEXEC:
                           CmdExecTime_ns = LcdWriteCommand(command);
                           myState = CMDEXEC;
                           break;
                     }
                  }
               }
            } else { // read
               if ((myPortValue&COMMANDDATA)) { //comand
                  cerr << "LCD-Read: Read data not supported " <<  endl;
               }else {
                  d3=myd3;
                  if ((CmdExecTime_ns == 0) && (myState>=PWR_ON_FINISH)) {
                     myState=IDLE;
                  }
               }
            }
         }
      }
   }
   if(timeToNextStepIn_ns!=0) *timeToNextStepIn_ns=0; //call as fast as possible
   return 0;
}

//Lcd::Lcd(UserInterface *_ui, const string &_name, const string &baseWindow):
Lcd::Lcd(UserInterface *_ui, const char *_name, const char *baseWindow):
   ui(_ui), name(_name),
   d0( &myPortValue, 1),
   d1( &myPortValue, 2),
   d2( &myPortValue, 4),
   d3( &myPortValue, 8),
   enable( &myPortValue, ENABLE),
   readWrite( &myPortValue, READWRITE),
   commandData( &myPortValue, COMMANDDATA)
                                          //    ,debugOut("./curses")
{
   lastPortValue=0;
   readLow=0;
   command=0;
   enableOld=0;

   allPins["d0"]=&d0;
   allPins["d1"]=&d1;
   allPins["d2"]=&d2;
   allPins["d3"]=&d3;

   allPins["e"]=&enable;
   allPins["r"]=&readWrite;
   allPins["c"]=&commandData;

   myPortValue=0;

   myState = POWER_ON;
   CmdExecTime_ns = Power_onTimes[0]; // 15ms = 15.000us = 15.000.000ns
   myd3='L';  // Set the default level of the Busy flag

   merke_x=0;
   merke_y=0;

   //setup the corrosponding ui in tcl from here
   ostringstream os;
   os << "create Lcd " << name  << " " << baseWindow << " " << " 20 4" << endl;
   ui->Write(os.str());
}

Lcd::~Lcd() { }
Pin *Lcd::GetPin(const char *name) {
   return allPins[name];
}

