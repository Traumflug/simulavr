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

#ifndef MYSOCKET_INCLUDED
#define MYSOCKET_INCLUDED

/*! \todo The implementation of Socket has to be cleaned. In the moment,
  the Socket implementation for MingW acts only as client, the unix implementation
  could act also as server, but is this necessary? */
  
#include <string>
#include "config.h"

#if defined(_MSC_VER) || defined(HAVE_SYS_MINGW)
#include <winsock2.h>
#undef GetCurrentTime  // defined by winbase.h, clashes with SystemClock::GetCurrentTime()
#include <sys/types.h>
#define ssize_t size_t
#else
#include <unistd.h>
#endif

class Socket {
    
    private:
#     if defined(_MSC_VER) || defined(HAVE_SYS_MINGW)
        static void Start();
        static void End();
        static int socketCount;
        SOCKET _socket;
#     else
        int sock, conn;
        void OpenSocket(int port);
#     endif
        
    public:
        Socket(int port);
        ~Socket();
        ssize_t Read(std::string &a);
        void Write(const std::string &s); 
        ssize_t Poll();

        void Write(const char *in) {
            std::string a(in);
            Write(a);
        }
};


#endif

