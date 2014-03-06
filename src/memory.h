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

#ifndef MEMORY
#define MEMORY

#include <string>
#include <map>

#include "decoder.h"
#include "avrmalloc.h"

//! Hold a memory block and symbol informations.
/*!  Memory class to hold memory content and symbol informations to map symbols
  to addresses and vice versa. */
class Memory {
    protected:
      
        unsigned int size; /*!< allocated size (in bytes) of myMemory */
        
    public:

        unsigned char *myMemory; /*!< THE memory block content itself */
        
        /*! address to symbol map */
        std::multimap<unsigned int, std::string> sym;
        
        /*! Creates the memory block
          
          @param size the memory block size */
        Memory(int size);
        
        /*! Destructor, frees myMemory */
        virtual ~Memory() { avr_free(myMemory); }
        
        /*! Return string with symbols found at address
        
          Seeks for symbols, which are registered for the given address. If the
          address isn't equal to a symbol address, but before the next one, then
          a offset to symbol address will be added. Returns a empty string, if
          nothing is found. (in case of no given symbols!)
          @param add the given address
          @return a string with all found symbols, concatenated by ',' */
        std::string GetSymbolAtAddress(unsigned int add);
        
        /*! Returns the address for a symbol
        
          If the given string is a hex string, the hex value will be converted
          and returned. If the symbol isn't found, program aborts.
          @param s the symbol string or hex string
          @return address for symbol or value of hex string
          
          \todo if the symbol isn't found, it aborts with a message. Maybe it
          should raise a exeption to handle this on the caller side? */
        unsigned int GetAddressAtSymbol(const std::string &s);
        
        /*! Add the (address, symbol) pair
        
          @param p a std::pair with address and symbol string */
        void AddSymbol(std::pair<unsigned int, std::string> p) { sym.insert(p); }
        
        /*! Returns the size in bytes of memory block */
        unsigned int GetSize() { return size; }
        
        /*! Write memory data to memory */
        virtual void WriteMem(const unsigned char*, unsigned int offset, unsigned int size) = 0;
};

//! Hold data memory block and symbol informations.
/*! Data class is a derived class from Memory, at the moment it's only used for
  hold symbols informations about data. NO memory space is allocated! myMemory
  will be initialized to NULL! */
class Data : public Memory {
    public:
        /*! Creates the data memory block */
        Data(): Memory(0) {}
        void WriteMem(const unsigned char*, unsigned int offset, unsigned int size) {}
};

#endif
