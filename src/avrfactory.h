/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph       
 * Copyright (C) 2007 Onno Kortmann
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

#ifndef AVRFACTORY
#define AVRFACTORY

#include <string>

class AvrDevice;

//! Produces AVR devices
/*! Factory for producing AVR devices according to a configuration string.
  This decouples the instantiation of concrete AVR devices from the code
  using them. It is helpful to remove all direct references to particular
  AVR devices in the code which uses them, such as in main.cpp. */
class AvrFactory {
    
    public:
        typedef AvrDevice*(*AvrDeviceCreator)();
    
        /*! Produces an AVR device according to the configuration string.
          Right now, the configuration string is simply the full name of the AVR
          device, like AT90S4433 or ATMEGA128.
          */
        AvrDevice* makeDevice(const char *config);
    
        //! Singleton class access. 
        static AvrFactory& instance();
        
        /*! Gives a list of all supported devices, which can be supplied
          to makeDevice() as is. */
        static std::string supportedDevices();
    
        //! Register a creation static method with the factory
        static void reg(const std::string name,
                        AvrDeviceCreator create);
        
    private:
        AvrFactory() {}
        //! map of registered AVR devices
        std::map<std::string, AvrFactory::AvrDeviceCreator> devmap;
};

/*! Macro to be used to register an AVR device with the AvrFactory.
  For a usage example, see atmega128.cpp. */
#define AVR_REGISTER(name, class) \
    struct AVRFactoryEntryMaker_ ## name { \
        public: \
            static AvrDevice *create_one() { \
                return new class; \
            } \
            AVRFactoryEntryMaker_ ## name() { \
                AvrFactory::reg(#name, create_one); \
            } \
    }; \
    AVRFactoryEntryMaker_ ## name maker_ ##name;

#endif
