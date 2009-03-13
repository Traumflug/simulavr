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

#ifndef SOCKET
#define SOCKET

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <unistd.h>

#include <string>
#include <iostream>
#include <map>
#include <sstream>

using namespace std;

class sockbuf 
#ifndef SWIG
: public streambuf
#endif
{
    protected:
        int *conn;

    public: 
        sockbuf(int *c) {conn=c;}

    protected:
        virtual int overflow(int c){
            char dummy =c&0xff;
            ::write( *conn, &dummy, 1);
            return 0;
        }
};

class sockstream
#ifndef SWIG
:public ostream 
#endif 
{

    private:
        sockbuf buffer; 

    public: 
        sockstream(int *c): ostream(&buffer), buffer(c){}
};

class Pin;
class ExternalType;

#include <sys/poll.h>

class Socket: public sockstream {
    protected:
        int sock, conn;
        void OpenSocket(int port);

    public:
        Socket(int port);
        ~Socket();
        ssize_t Read( string &a);
        void Write(const string &s); 
        ssize_t Poll();

        void Write(const char *in) {
            string a(in);
            Write(a);
        }
};

#endif

