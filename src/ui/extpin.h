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

#ifndef EXTPIN
#define EXTPIN

#include <string>

#include "ui.h"
#include "externaltype.h"
#include "pin.h"
#include "pinnotify.h"

/*! "external" pin, used for connecting UI to pins */
class ExtPin: public Pin, public ExternalType {

    protected:
        UserInterface *ui;   //!< ptr to UI
        std::string extName; //!< identifier for UI access

    public:
        /*! creates an ExtPin instance
          @param ps pin status
          @param _ui pointer to UI instance
          @param _extName identifier used for UI access
          @param baseWindow window identifier from UI window */
        ExtPin(T_Pinstate ps, UserInterface *_ui, const char *_extName, const char *baseWindow);
        
        /*! Receives a external value from UI
          @param s value string */
        void SetNewValueFromUi(const std::string& s);
        
        //Pin &operator= (unsigned char);

        /*! Send new pin status to UI
          @param p pin, for which status change is to send */
        void SetInState(const Pin& p);
};

/*! "external" analog pin, used for connecting UI to pins */
class ExtAnalogPin: public Pin, public ExternalType {
  
    protected:
        UserInterface *ui;   //!< ptr to UI
        std::string extName; //!< identifier for UI access

    public:
        /*! creates an ExtAnalogPin instance
          @param startval initial analog value
          @param _ui pointer to UI instance
          @param _extName identifier used for UI access
          @param baseWindow window identifier from UI window */
        ExtAnalogPin(unsigned int startval, UserInterface *_ui, const char *_extName, const char* baseWindow); 

        /*! Receives a external value from UI
          @param s value string */
        void SetNewValueFromUi(const std::string &);

        //Pin &operator= (unsigned char);

        /*! Send new pin status to UI
          @param p pin, for which status change is to send */
        void SetInState(const Pin& p);
};

#endif // EXTPIN
