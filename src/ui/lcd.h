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
 *
 *  $Id$
 */

#ifndef LCD_H_INCLUDED
#define LCD_H_INCLUDED


#include <fstream>
#include <string>

#include "systemclocktypes.h"
#include "simulationmember.h"
//#include "hardware.h"
#include "ui.h"
#include "pin.h"

typedef enum {
        IDLE,
        POWER_ON,     // First State after Reset
        PWR_AFTER_FS1,// After first Function Set Cmd no Busy Flag
        PWR_AFTER_FS2,// After second Function Set Cmd no Busy Flag
        PWR_ON_FINISH,// After third Function Set Cmd no Busy Flag. After the next CMD BF is valid
        CMDEXEC       // Executing any command after init
    } t_myState;


/** Simulates a HD44780 character-LCD controller with a 4 bit interface.
 * This HD-controller is boring slow :-) like some original.
 */
class Lcd : public SimulationMember {
    protected:
        UserInterface *ui;
        std::string name;
        unsigned char myPortValue;
        std::map<std::string, Pin*> allPins;
        Pin d0;
        Pin d1;
        Pin d2;
        Pin d3;

        Pin enable;
        Pin readWrite;
        Pin commandData;

        unsigned int CmdExecTime_ns; // Command Execution Time
        t_myState myState;           // LCD State-Event machine
        char      myd3;              // internal D3


        int merke_x;
        int merke_y;

        void LcdWriteData(unsigned char data);
        unsigned int  LcdWriteCommand(unsigned char command);

        std::ofstream debugOut;
        void SendCursorPosition();

        unsigned char lastPortValue;
        int readLow;
        unsigned char command;
        int enableOld;

    public:
        virtual int Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns=0);
        //Lcd(UserInterface *ui, const string &name, const string &baseWindow);
        Lcd(UserInterface *ui, const char *name, const char *baseWindow);
        virtual ~Lcd();
        Pin *GetPin(const char *name);
};

#endif
