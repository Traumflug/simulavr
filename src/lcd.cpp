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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 */
#include <iostream>
#include <fstream>

#include "lcd.h"

int lcdStartLine []={0, 0x40, 0x14, 0x20};

void Lcd::LcdWriteData(unsigned char data) { 
    ostringstream os;
    os << name << " WriteChar " << merke_x+1 << " " << merke_y << " " << (unsigned int)data << endl;
    ui->Write(os.str());

    merke_x++;
    SendCursorPosition();
}

void Lcd::SendCursorPosition() {
        ostringstream os;
        os << name << " MoveCursor " << merke_x << " " << merke_y << " " <<  endl;
        ui->Write(os.str());
}

void Lcd::LcdWriteCommand(unsigned char command) {
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

        return;
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
        return;
    }       
} 

int Lcd::Step(bool trueHwStep, unsigned long long *timeToNextStepIn_ns) {
    static unsigned char lastPortValue=0;

    static int readLow = 0;

    static unsigned char command=0;

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
        */ 
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



    }
    if(timeToNextStepIn_ns!=0) *timeToNextStepIn_ns=0; //call as fast as possible
    return 0; 
}

Lcd::Lcd(UserInterface *_ui, const string &_name, const string &baseWindow): 
ui(_ui), name(_name),
    d0( &myPortValue, 1),
    d1( &myPortValue, 2),
    d2( &myPortValue, 4),
    d3( &myPortValue, 8),
    enable( &myPortValue, 16),
    readWrite( &myPortValue, 32),
commandData( &myPortValue, 64)
    //debugOut("./curses")
{
    allPins["d0"]=&d0;
    allPins["d1"]=&d1;
    allPins["d2"]=&d2;
    allPins["d3"]=&d3;

    allPins["e"]=&enable;
    allPins["r"]=&readWrite;
    allPins["c"]=&commandData;

    myPortValue=0;


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

