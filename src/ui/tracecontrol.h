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

#ifndef __TRACECONTROL__
#define __TRACECONTROL__

#include <string>

using namespace std;

#include "externaltype.h"
#include "ui.h"

class TraceControl: public ExternalType
{
   protected:
      UserInterface *ui;   //!< ptr to UI
      std::string extName; //!< identifier for UI access
      AvrDevice* dev;

   public:
      TraceControl(UserInterface *_ui, AvrDevice* _dev, const char *_extName, const char *baseWindow):
         ui( _ui),
         extName( _extName),
         dev(_dev)
      {
         ostringstream os;
         os << "create TraceControl "  << _extName << " " << baseWindow << " " << endl;
         ui->Write(os.str());
         ui->AddExternalType(extName, this);
      }

      void SetNewValueFromUi(const string& s) 
      {
         if ( s == "1" )
         {
            dev->trace_on = 1;
         }
         else
         {
            dev->trace_on = 0;
         }
      }
};

#endif

