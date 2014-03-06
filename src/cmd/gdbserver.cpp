/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003  Theodore A. Roth, Klaus Rudolph      
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
using namespace std;

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#include "avrmalloc.h"
#include "avrerror.h"
#include "types.h"
#include "systemclock.h"

/* only for compilation ... later to be removed */
#include "avrdevice.h"
#include "avrdevice_impl.h"
#include "gdb.h"

#ifdef _MSC_VER
#  define snprintf _snprintf
#endif

#ifndef DOXYGEN /* have doxygen system ignore this. */
enum {
    MAX_READ_RETRY = 50,          /* Maximum number of retries if a read is incomplete. */

    MEM_SPACE_MASK = 0x00ff0000,  /* mask to get bits which determine memory space */
    FLASH_OFFSET   = 0x00000000,  /* Data in flash has this offset from gdb */
    SRAM_OFFSET    = 0x00800000,  /* Data in sram has this offset from gdb */
    EEPROM_OFFSET  = 0x00810000,  /* Data in eeprom has this offset from gdb */
    SIGNATURE_OFFSET = 0x00840000,/* Present if application used "#include <avr/signature.h>" */

    GDB_BLOCKING_OFF = 0,         /* Signify that a read is non-blocking. */
    GDB_BLOCKING_ON  = 1,         /* Signify that a read will block. */

    GDB_RET_NOTHING_RECEIVED = -5, /* if the read in non blocking receives nothing, we have nothing todo */ 
    GDB_RET_SINGLE_STEP = -4,     /* do one single step in gdb loop */
    GDB_RET_CONTINUE    = -3,     /* step until another command from gdb is received */
    GDB_RET_CTRL_C       = -2,    /* gdb has sent Ctrl-C to interrupt what is doing */
    GDB_RET_KILL_REQUEST = -1,    /* gdb has requested that sim be killed */
    GDB_RET_OK           =  0     /* continue normal processing of gdb requests */ 
        /* means that we should NOT execute any step!!! */
};
#endif /* not DOXYGEN */

#if defined(HAVE_SYS_MINGW) || defined(_MSC_VER)

int GdbServerSocketMingW::socketCount = 0;

void GdbServerSocketMingW::Start() {
    if(socketCount == 0) {
        WSADATA info;
        if(WSAStartup(MAKEWORD(2, 2), &info))
            avr_error("Could not start WSA");
    }
    socketCount++;
}

void GdbServerSocketMingW::End() {
    WSACleanup();
}

GdbServerSocketMingW::GdbServerSocketMingW(int port): _socket(0), _conn(0) {
    sockaddr_in sa;
    
    Start();
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == INVALID_SOCKET)
        avr_error("Couldn't create socket: INVALID_SOCKET");
    
    u_long arg = 1;
    ioctlsocket(_socket, FIONBIO, &arg);
    
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = PF_INET;
    sa.sin_port = htons(port);
    if(bind(_socket, (sockaddr *)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR) {
        closesocket(_socket);
        avr_error("Couldn't bind socket: INVALID_SOCKET");
    }
    
    listen(_socket, 1); // only 1 connection at time
}

GdbServerSocketMingW::~GdbServerSocketMingW() {
    Close();
    socketCount--;
    if(socketCount == 0)
        End();
}

void GdbServerSocketMingW::Close(void) {
    CloseConnection();
    closesocket(_socket);
}

int GdbServerSocketMingW::ReadByte(void) {
    char buf[1];
    int rv = recv(_conn, buf, 1, 0);
    if(rv <= 0)
        return -1;
    return buf[0];
}

void GdbServerSocketMingW::Write(const void* buf, size_t count) {
    send(_conn, (const char *)buf, count, 0);
}

void GdbServerSocketMingW::SetBlockingMode(int mode) {
    u_long arg = 1;
    if(mode)
        arg = 0;
    int res = ioctlsocket(_conn, FIONBIO, &arg);
    if(res)
        avr_warning( "fcntl failed: %d\n", WSAGetLastError() );
}

bool GdbServerSocketMingW::Connect(void) {
    _conn = accept(_socket, 0, 0);
    if(_conn == INVALID_SOCKET) {
        int rc = WSAGetLastError();
        if(rc == WSAEWOULDBLOCK)
            return false;
        else
            avr_error("Couldn't connect: INVALID_SOCKET");
    }
    return true;
}

void GdbServerSocketMingW::CloseConnection(void) {
    closesocket(_conn);
}

#else

GdbServerSocketUnix::GdbServerSocketUnix(int port) {
    conn = -1;        //no connection opened
    
    if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        avr_error("Can't create socket: %s", strerror(errno));

    /* Let the kernel reuse the socket address. This lets us run
    twice in a row, without waiting for the (ip, port) tuple
    to time out. */
    int i = 1;  
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
    fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK); //dont know 

    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    memset(&address->sin_addr, 0, sizeof(address->sin_addr));

    if(bind(sock, (struct sockaddr *)address, sizeof(address)))
        avr_error("Can not bind socket: %s", strerror(errno));

    if(listen(sock, 1) < 0)
        avr_error("Can not listen on socket: %s", strerror(errno));
}

GdbServerSocketUnix::~GdbServerSocketUnix() {
    // do nothing in the moment
}

void GdbServerSocketUnix::Close(void) {
    CloseConnection();
    close(sock);
}

int GdbServerSocketUnix::ReadByte(void) {
    char c;
    int res;
    int cnt = MAX_READ_RETRY;

    while(cnt--) {
        res = read(conn, &c, 1);
        if(res < 0) {
            if (errno == EAGAIN)
                /* fd was set to non-blocking and no data was available */
                return -1;

            avr_error("read failed: %s", strerror(errno));
        }

        if (res == 0) {
            usleep(1000);
            avr_warning("incomplete read\n");
            continue;
        }
        return c;
    }
    avr_error("Maximum read reties reached");

    return 0; /* make compiler happy */
}

void GdbServerSocketUnix::Write(const void* buf, size_t count) {
    int res;

    res = write(conn, buf, count);

    /* FIXME: should we try and catch interrupted system calls here? */
    if(res < 0)
        avr_error("write failed: %s", strerror(errno));

    /* FIXME: if this happens a lot, we could try to resend the
    unsent bytes. */
    if((unsigned int)res != count)
        avr_error("write only wrote %d of %d bytes", res, count);
}

void GdbServerSocketUnix::SetBlockingMode(int mode) {
    if(mode) {
        /* turn non-blocking mode off */
        if(fcntl(conn, F_SETFL, fcntl(conn, F_GETFL, 0) & ~O_NONBLOCK) < 0)
            avr_warning("fcntl failed: %s\n", strerror(errno));
    } else {
        /* turn non-blocking mode on */
        if(fcntl(conn, F_SETFL, fcntl(conn, F_GETFL, 0) | O_NONBLOCK) < 0)
            avr_warning("fcntl failed: %s\n", strerror(errno));
    }
}

bool GdbServerSocketUnix::Connect(void) {
    /* accept() needs this set, or it fails (sometimes) */
    socklen_t addrLength = sizeof(struct sockaddr_in);

    /* We only want to accept a single connection, thus don't need a loop. */
    /* Wait until we have a connection */
    conn = accept(sock, (struct sockaddr *)address, &addrLength);
    if(conn > 0) {
        /* Tell TCP not to delay small packets.  This greatly speeds up
        interactive response. WARNING: If TCP_NODELAY is set on, then gdb
        may timeout in mid-packet if the (gdb)packet is not sent within a
        single (tcp)packet, thus all outgoing (gdb)packets _must_ be sent
        with a single call to write. (see Stevens "Unix Network
        Programming", Vol 1, 2nd Ed, page 202 for more info) */
        int i = 1;
        setsockopt (conn, IPPROTO_TCP, TCP_NODELAY, &i, sizeof (i));

        /* If we got this far, we now have a client connected and can start 
        processing. */
        fprintf(stderr, "Connection opened by host %s, port %hd.\n",
                inet_ntoa(address->sin_addr), ntohs(address->sin_port));

        return true;
    } else
        return false;
}

void GdbServerSocketUnix::CloseConnection(void) {
    close(conn);
    conn = -1;
}

#endif

GdbServer::GdbServer(AvrDevice *c, int _port, int debug, int _waitForGdbConnection):
    core(c),
    global_debug_on(debug),
    waitForGdbConnection(_waitForGdbConnection),
    exitOnKillRequest(false)
{
    last_reply = NULL; //init static var for last_reply()
    runMode = GDB_RET_NOTHING_RECEIVED;
    lastCoreStepFinished = true;
    connState = false;
    m_gdb_thread_id = 1;  // we start with the first thread already created

#if defined(HAVE_SYS_MINGW) || defined(_MSC_VER)
    server = new GdbServerSocketMingW(_port);
#else
    server = new GdbServerSocketUnix(_port);
#endif

    fprintf(stderr, "Waiting on port %d for gdb client to connect...\n", _port);

}

//make the instance of static list of all gdb servers here
vector<GdbServer*> GdbServer::allGdbServers;

GdbServer::~GdbServer() {
    server->Close();
    avr_free(last_reply);
    delete server;
}

word GdbServer::avr_core_flash_read(int addr) {
    assert(0 <= addr && (unsigned) addr+1 < core->Flash->GetSize());
    return core->Flash->ReadMemRawWord(addr);
}

void GdbServer::avr_core_flash_write(int addr, word val) {
    if((addr + 1) >= (int)core->Flash->GetSize())
        avr_error("try to write in flash after last valid address!");
    core->Flash->WriteMemByte(val & 0xff, addr + 1);
    core->Flash->WriteMemByte((val >> 8) & 0xff, addr);
    core->Flash->Decode(addr);
}

void GdbServer::avr_core_flash_write_hi8(int addr, byte val) {
    if(addr >= (int)core->Flash->GetSize())
        avr_error("try to write in flash after last valid address! (hi8)");
    core->Flash->WriteMemByte(val, addr);
    core->Flash->Decode();
}

void GdbServer::avr_core_flash_write_lo8(int addr, byte val) {
    if(addr + 1 >= (int)core->Flash->GetSize())
        avr_error("try to write in flash after last valid address! (lo8)");
    core->Flash->WriteMemByte(val, addr + 1);
    core->Flash->Decode();
}

void GdbServer::avr_core_remove_breakpoint(dword pc) {
    Breakpoints::iterator ii;
    if ((ii= find(core->BP.begin(), core->BP.end(), pc)) != core->BP.end()) core->BP.erase(ii);
}

void GdbServer::avr_core_insert_breakpoint(dword pc) {
    core->BP.push_back(pc);
}

int GdbServer::signal_has_occurred(int signo) {return 0;}
void GdbServer::signal_watch_start(int signo){}
void GdbServer::signal_watch_stop(int signo){}

static char HEX_DIGIT[] = "0123456789abcdef";

//! Convert a hexidecimal digit to a 4 bit nibble.
int GdbServer::hex2nib( char hex )
{
    if ( (hex >= 'A') && (hex <= 'F') )
        return (10 + (hex - 'A'));

    else if ( (hex >= 'a') && (hex <= 'f') )
        return (10 + (hex - 'a'));

    else if ( (hex >= '0') && (hex <= '9') )
        return (hex - '0');

    /* Shouldn't get here unless the developer screwed up ;) */     
    avr_error( "Invalid hexidecimal digit: 0x%02x", hex );

    return 0; /* make compiler happy */
}

/*! Use a single function for storing/getting the last reply message.
If reply is NULL, return pointer to the last reply saved.
Otherwise, make a copy of the buffer pointed to by reply. */
const char* GdbServer::gdb_last_reply( const char *reply )
{

    if (reply == NULL)
    {
        if (last_reply == NULL)
            return "";
        else
            return last_reply;
    }

    if(last_reply == reply)
        return reply;  // we must not delete the reply
    avr_free( last_reply );
    last_reply = avr_strdup( reply );

    return last_reply;
}

//! Acknowledge a packet from GDB
void GdbServer::gdb_send_ack( )
{
    if (global_debug_on)
        fprintf( stderr, " Ack -> gdb\n");

    server->Write( "+", 1 );
}

//! Send a reply to GDB.
void GdbServer::gdb_send_reply( const char *reply )
{
    int cksum = 0;
    int bytes;

    /* Save the reply to last reply so we can resend if need be. */
    gdb_last_reply( reply );

    if (global_debug_on)
        fprintf( stderr, "Sent: $%s#", reply );

    if (*reply == '\0')
    {
        server->Write( "$#00", 4 );

        if (global_debug_on)
            fprintf( stderr, "%02x\n", cksum & 0xff );
    }
    else
    {
        memset( buf, '\0', sizeof(buf) );

        buf[0] = '$';
        bytes = 1;

        while (*reply)
        {
            cksum += (unsigned char)*reply;
            buf[bytes] = *reply;
            bytes++;
            reply++;

            /* must account for "#cc" to be added */
            if (bytes == (sizeof(buf) - 3))
            {
                /* FIXME: TRoth 2002/02/18 - splitting reply would be better */
                avr_error( "buffer overflow" );
            }
        }

        if (global_debug_on)
            fprintf( stderr, "%02x\n", cksum & 0xff );

        buf[bytes++] = '#';
        buf[bytes++] = HEX_DIGIT[(cksum >> 4) & 0xf];
        buf[bytes++] = HEX_DIGIT[cksum & 0xf];

        server->Write( buf, bytes );
    }
}

void GdbServer::gdb_send_hex_reply(const char *reply, const char *reply_to_encode)
{
    std::string result = reply;
    for(int i = 0; reply_to_encode[i] != '\0'; i++) {
        byte val = reply_to_encode[i];
        result += HEX_DIGIT[(val >> 4) & 0xf];
        result += HEX_DIGIT[val & 0xf];
    }

    gdb_send_reply(result.c_str());
}

/*! GDB needs the 32 8-bit, gpw registers (r00 - r31), the 
8-bit SREG, the 16-bit SP (stack pointer) and the 32-bit PC
(program counter). Thus need to send a reply with
r00, r01, ..., r31, SREG, SPL, SPH, PCL, PCH
Low bytes before High since AVR is little endian. */
void GdbServer::gdb_read_registers( )
{
    bool current = core->stack->m_ThreadList.GetCurrentThreadForGDB() == m_gdb_thread_id;
    const Thread* nonrunning = core->stack->m_ThreadList.GetThreadFromGDB(m_gdb_thread_id);
    assert(current || nonrunning->m_sp != 0x0000);

    int   i;
    dword val;                  /* ensure it's 32 bit value */

    /* (32 gpwr, SREG, SP, PC) * 2 hex bytes + terminator */
    size_t  buf_sz = (32 + 1 + 2 + 4)*2 + 1;
    char   *buf;

    buf = avr_new0( char, buf_sz );

    /* 32 gen purpose working registers */
    for ( i=0; i<32; i++ )
    {
        val = current ? core->GetCoreReg(i) : nonrunning->registers[i];
        buf[i*2]   = HEX_DIGIT[(val >> 4) & 0xf];
        buf[i*2+1] = HEX_DIGIT[val & 0xf];
    }

    /* GDB thinks SREG is register number 32 */
    val = *(core->status);
    buf[i*2]   = HEX_DIGIT[(val >> 4) & 0xf];
    buf[i*2+1] = HEX_DIGIT[val & 0xf];
    i++;

    /* GDB thinks SP is register number 33 */
    val = current ? core->stack->GetStackPointer() : nonrunning->m_sp;
    buf[i*2]   = HEX_DIGIT[(val >> 4) & 0xf];
    buf[i*2+1] = HEX_DIGIT[val & 0xf];
    i++;
    val>>=8;
    buf[i*2]   = HEX_DIGIT[(val >> 4) & 0xf];
    buf[i*2+1] = HEX_DIGIT[val & 0xf];
    i++;

    /* GDB thinks PC is register number 34.
    GDB stores PC in a 32 bit value (only uses 23 bits though).
    GDB thinks PC is bytes into flash, not words like in simulavr. */

    val = current ? core->PC * 2 : nonrunning->m_ip;
    buf[i*2]   = HEX_DIGIT[(val >> 4)  & 0xf];
    buf[i*2+1] = HEX_DIGIT[val & 0xf];

    val >>= 8;
    buf[i*2+2] = HEX_DIGIT[(val >> 4) & 0xf];
    buf[i*2+3] = HEX_DIGIT[val & 0xf];

    val >>= 8;
    buf[i*2+4] = HEX_DIGIT[(val >> 4) & 0xf];
    buf[i*2+5] = HEX_DIGIT[val & 0xf];

    val >>= 8;
    buf[i*2+6] = HEX_DIGIT[(val >> 4) & 0xf];
    buf[i*2+7] = HEX_DIGIT[val & 0xf];

    gdb_send_reply(  buf );
    avr_free( buf );
}

/*! GDB is sending values to be written to the registers. Registers are the
same and in the same order as described in gdb_read_registers() above. */
void GdbServer::gdb_write_registers(const char *pkt) {
    int   i;
    byte  bval;
    dword val;                  /* ensure it's a 32 bit value */

    /* 32 gen purpose working registers */
    for ( i=0; i<32; i++ )
    {
        bval  = hex2nib(*pkt++) << 4;
        bval += hex2nib(*pkt++);
        core->SetCoreReg(i, bval);

    }

    /* GDB thinks SREG is register number 32 */
    bval  = hex2nib(*pkt++) << 4;
    bval += hex2nib(*pkt++);
    *(core->status)=bval;

    /* GDB thinks SP is register number 33 */
    bval  = hex2nib(*pkt++) << 4;
    bval += hex2nib(*pkt++);
    val = hex2nib(*pkt++) << 4;
    val += hex2nib(*pkt++);
    val <<= 8;
    val += bval;
    core->stack->SetStackPointer(val);

    /* GDB thinks PC is register number 34.
    GDB stores PC in a 32 bit value (only uses 23 bits though).
    GDB thinks PC is bytes into flash, not words like in simulavr.

    Must cast to dword so as not to get mysterious truncation. */

    val  = ((dword)hex2nib(*pkt++)) << 4;
    val += ((dword)hex2nib(*pkt++));

    val += ((dword)hex2nib(*pkt++)) << 12;
    val += ((dword)hex2nib(*pkt++)) << 8;

    val += ((dword)hex2nib(*pkt++)) << 20;
    val += ((dword)hex2nib(*pkt++)) << 16;

    val += ((dword)hex2nib(*pkt++)) << 28;
    val += ((dword)hex2nib(*pkt++)) << 24;
    core->PC=val/2;

    gdb_send_reply( "OK" );
}

/*! Extract a hexadecimal number from the pkt. Keep scanning pkt until stop char
is reached or size of int is exceeded or a '\0' is reached. pkt is modified
to point to stop char when done.

Use this function to extract a num with an arbitrary num of hex
digits. This should _not_ be used to extract n digits from a m len string
of digits (n <= m). */
int GdbServer::gdb_extract_hex_num(const char **pkt, char stop) {
    int i = 0;
    int num = 0;
    const char *p = *pkt;
    int max_shifts = sizeof(int)*2-1; /* max number of nibbles to shift through */

    while ( (*p != stop) && (*p != '\0') )
    {
        if (i > max_shifts)
            avr_error( "number too large" );

        num = (num << 4) | hex2nib(*p);
        i++;
        p++;
    }

    *pkt = p;
    return num;
}

/*! Read a single register. Packet form: 'pn' where n is a hex number with no
zero padding. */
void GdbServer::gdb_read_register(const char *pkt) {
    int reg;

    char reply[MAX_BUF + 1];

    memset(reply, '\0', sizeof(reply));

    reg = gdb_extract_hex_num(&pkt, '\0');

    if ( (reg >= 0) && (reg < 32) )
    {                           /* general regs */
        byte val = core->GetCoreReg(reg);
        snprintf(reply, sizeof(reply), "%02x", val);
    }
    else if (reg == 32)         /* sreg */
    {
        byte val = *(core->status);
        snprintf(reply, sizeof(reply), "%02x", val);
    }
    else if (reg == 33)         /* SP */
    {
        byte spl, sph;
        unsigned long sp = core->stack->GetStackPointer();
        spl = sp & 0xff;
        sph = (sp >> 8) & 0xff;
        snprintf(reply, sizeof(reply), "%02x%02x", spl, sph);
    }
    else if (reg == 34)         /* PC */
    {
        dword val = core->PC * 2;
        snprintf(reply, sizeof(reply),
                "%02x%02x" "%02x%02x", 
                val & 0xff, (val >> 8) & 0xff,
                (val >> 16) & 0xff, (val >> 24) & 0xff );
    }
    else
    {
        avr_warning( "Bad register value: %d\n", reg );
        gdb_send_reply( "E00" );
        return;
    }
    gdb_send_reply( reply );
}

/*! Write a single register. Packet form: 'Pn=r' where n is a hex number with
no zero padding and r is two hex digits for each byte in register (target
byte order). */
void GdbServer::gdb_write_register(const char *pkt) {
    int reg;
    int val, hval;
    dword dval;

    reg = gdb_extract_hex_num(&pkt, '=');
    pkt++;                      /* skip over '=' character */

    /* extract the low byte of value from pkt */
    val  = hex2nib(*pkt++) << 4;
    val += hex2nib(*pkt++);

    if ( (reg >= 0) && (reg < 33) )
    {
        /* r0 to r31 and SREG */
        if (reg == 32)          /* gdb thinks SREG is register 32 */
        {
            *(core->status)=val&0xff;
        }
        else
            core->SetCoreReg(reg, val & 0xff);
    }
    else if (reg == 33)
    {
        /* SP is 2 bytes long so extract upper byte */
        hval  = hex2nib(*pkt++) << 4;
        hval += hex2nib(*pkt++);

        core->stack->SetStackPointer((val & 0xff) + ((hval & 0xff) << 8));
    }
    else if (reg == 34)
    {
        /* GDB thinks PC is register number 34.
        GDB stores PC in a 32 bit value (only uses 23 bits though).
        GDB thinks PC is bytes into flash, not words like in simulavr.

        Must cast to dword so as not to get mysterious truncation. */

        dval  = (dword)val; /* we already read the first two nibbles */

        dval += ((dword)hex2nib(*pkt++)) << 12;
        dval += ((dword)hex2nib(*pkt++)) << 8;

        dval += ((dword)hex2nib(*pkt++)) << 20;
        dval += ((dword)hex2nib(*pkt++)) << 16;

        dval += ((dword)hex2nib(*pkt++)) << 28;
        dval += ((dword)hex2nib(*pkt++)) << 24;
        core->PC=dval/2;
    }
    else
    {
        avr_warning( "Bad register value: %d\n", reg );
        gdb_send_reply(  "E00" );
        return;
    }

    gdb_send_reply( "OK" );
}

/*! Parse the pkt string for the addr and length.
a_end is first char after addr.
l_end is first char after len.
Returns number of characters to advance pkt. */
int GdbServer::gdb_get_addr_len(const char *pkt, char a_end, char l_end, unsigned int *addr, int *len) {
    const char *orig_pkt = pkt;

    *addr = 0;
    *len  = 0;

    /* Get the addr from the packet */
    while (*pkt != a_end)
        *addr = (*addr << 4) + hex2nib(*pkt++);
    pkt++;                      /* skip over a_end */

    /* Get the length from the packet */
    while (*pkt != l_end)
        *len = (*len << 4) + hex2nib(*pkt++);
    pkt++;                      /* skip over l_end */

    /*      fprintf( stderr, "+++++++++++++ addr = 0x%08x\n", *addr ); */
    /*      fprintf( stderr, "+++++++++++++ len  = %d\n", *len ); */

    return (pkt - orig_pkt);
}

void GdbServer::gdb_read_memory(const char *pkt) {
    unsigned int addr = 0;
    int   len  = 0;
    byte *buf;
    byte  bval;
    word  wval;
    int   i;
    int   is_odd_addr;

    pkt += gdb_get_addr_len( pkt, ',', '\0', &addr, &len );

    buf = avr_new0( byte, (len*2)+1 );

    if ( (addr & MEM_SPACE_MASK) == EEPROM_OFFSET )
    {
        /* addressing eeprom */

        addr = addr & ~MEM_SPACE_MASK; /* remove the offset bits */

        for ( i=0; i<len; i++ )
        {
            bval = core->eeprom->ReadFromAddress( addr+i );
            buf[i*2]   = HEX_DIGIT[bval >> 4];
            buf[i*2+1] = HEX_DIGIT[bval & 0xf];
        }
    }
    else if ( (addr & MEM_SPACE_MASK) == SRAM_OFFSET )
    {
        /* addressing sram */

        addr = addr & ~MEM_SPACE_MASK; /* remove the offset bits */

        /* Return an error to gdb if it tries to read or write any of the 32
        general purpose registers. This allows gdb to know when a zero
        pointer has been dereferenced. */

        /* FIXME: [TRoth 2002/03/31] This isn't working quite the way I
        thought it would so I've removed it for now.*/

        /* if ( (addr >= 0) && (addr < 32) ) */
        if (0)
        {
            snprintf( (char*)buf, len*2, "E%02x", EIO );
        }
        else
        {
            for ( i=0; i<len; i++ )
            {
                bval = core->GetRWMem(addr + i);
                buf[i*2]   = HEX_DIGIT[bval >> 4];
                buf[i*2+1] = HEX_DIGIT[bval & 0xf];
            }
        }
    }
    else if ( (addr & MEM_SPACE_MASK) == FLASH_OFFSET )
    {
        /* addressing flash */

        addr = addr & ~MEM_SPACE_MASK; /* remove the offset bits */

        is_odd_addr = addr % 2;
        i = 0;

        if (is_odd_addr)
        {
            bval = avr_core_flash_read( addr ) >> 8;
            buf[i++] = HEX_DIGIT[bval >> 4];
            buf[i++] = HEX_DIGIT[bval & 0xf];
            addr++;
            len--;
        }

        while (len > 1)
        {
            wval = avr_core_flash_read( addr );

            bval = wval & 0xff;
            buf[i++] = HEX_DIGIT[bval >> 4];
            buf[i++] = HEX_DIGIT[bval & 0xf];

            bval = wval >> 8;
            buf[i++] = HEX_DIGIT[bval >> 4];
            buf[i++] = HEX_DIGIT[bval & 0xf];

            len -= 2;
            addr += 2;
        }

        if (len == 1)
        {
            bval = avr_core_flash_read( addr ) & 0xff;
            buf[i++] = HEX_DIGIT[bval >> 4];
            buf[i++] = HEX_DIGIT[bval & 0xf];
        }
    }
    else
    {
        /* gdb asked for memory space which doesn't exist */
        avr_warning( "Invalid memory address: 0x%x.\n", addr );
        snprintf( (char*)buf, len*2, "E%02x", EIO );
    }


    gdb_send_reply( (char*)buf );

    avr_free( buf );
}

void GdbServer::gdb_write_memory(const char *pkt) {
    unsigned int addr = 0;
    int  len  = 0;
    byte bval;
    unsigned int  i;
    char reply[10];

    /* Set the default reply. */
    strncpy( reply, "OK", sizeof(reply) );

    pkt += gdb_get_addr_len( pkt, ',', ':', &addr, &len );


    if ( (addr & MEM_SPACE_MASK) == EEPROM_OFFSET )
    {
        /* addressing eeprom */

        addr = addr & ~MEM_SPACE_MASK; /* remove the offset bits */

        while (len>0) {
            bval  = hex2nib(*pkt++) << 4;
            bval += hex2nib(*pkt++);
            len--;
            core->eeprom->WriteAtAddress(addr, bval);
            addr++;
        }
    }
    else if ( (addr & MEM_SPACE_MASK) == SRAM_OFFSET )
    {
        /* addressing sram */

        addr = addr & ~MEM_SPACE_MASK; /* remove the offset bits */

        /* Return error. See gdb_read_memory for reasoning. */
        /* FIXME: [TRoth 2002/03/31] This isn't working quite the way I
        thought it would so I've removed it for now.*/
        /* if ( (addr >= 0) && (addr < 32) ) */
        if (0)
        {
            snprintf( reply, sizeof(reply), "E%02x", EIO );
        }
        else
        {
            for ( i=addr; i < addr+len; i++ )
            {
                bval  = hex2nib(*pkt++) << 4;
                bval += hex2nib(*pkt++);
                core->SetRWMem(i, bval);
            }
        }
    }
    else if ( (addr & MEM_SPACE_MASK) == FLASH_OFFSET )
    {
        /* addressing flash */

        addr = addr & ~MEM_SPACE_MASK; /* remove the offset bits */

        if (addr % 2)
        {
            bval  = hex2nib(*pkt++) << 4;
            bval += hex2nib(*pkt++);
            avr_core_flash_write_hi8(addr, bval);
            len--;
            addr++;
        }

        while (len > 1)
        {
            word wval;
            wval  = hex2nib(*pkt++) << 4; /* low byte first */
            wval += hex2nib(*pkt++);
            wval += hex2nib(*pkt++) << 12; /* high byte last */
            wval += hex2nib(*pkt++) << 8;
            avr_core_flash_write( addr, wval);
            len  -= 2;
            addr += 2;
        }

        if ( len == 1 )
        {
            /* one more byte to write */
            bval  = hex2nib(*pkt++) << 4;
            bval += hex2nib(*pkt++);
            avr_core_flash_write_lo8( addr, bval );
        }
    }
    else if ( (addr & MEM_SPACE_MASK) == SIGNATURE_OFFSET && len >= 3)
    {
        int sig3 = (hex2nib(*pkt++) << 4); sig3 += hex2nib(*pkt++);
        int sig2 = (hex2nib(*pkt++) << 4); sig2 += hex2nib(*pkt++);
        int sig1 = (hex2nib(*pkt++) << 4); sig1 += hex2nib(*pkt++);
        if (global_debug_on)
            fprintf(stderr, "Device signature %02x %02x %02x\n", sig1, sig2, sig3);
    }
    else
    {
        /* gdb asked for memory space which doesn't exist */
        avr_warning( "Invalid memory address: 0x%x.\n", addr );
        snprintf( reply, sizeof(reply), "E%02x", EIO );
    }

    gdb_send_reply( reply );
}

/*! Format of breakpoint commands (both insert and remove):

"z<t>,<addr>,<length>"  -  remove break/watch point
"Z<t>,<add>r,<length>"  -  insert break/watch point

In both cases t can be the following:
t = '0'  -  software breakpoint
t = '1'  -  hardware breakpoint
t = '2'  -  write watch point
t = '3'  -  read watch point
t = '4'  -  access watch point

addr is address.
length is in bytes

For a software breakpoint, length specifies the size of the instruction to
be patched. For hardware breakpoints and watchpoints, length specifies the
memory region to be monitored. To avoid potential problems, the operations
should be implemented in an idempotent way. -- GDB 5.0 manual. */
void GdbServer::gdb_break_point(const char *pkt) {
    unsigned int addr = 0;
    int len  = 0;

    char z = *(pkt-1);          /* get char parser already looked at */
    char t = *pkt++;
    pkt++;                      /* skip over first ',' */

    gdb_get_addr_len( pkt, ',', '\0', &addr, &len );

    switch (t) {
        case '0':               /* software breakpoint */
            /* Both `addr' and GetSize() are in bytes. */
            if ( addr >= core->Flash->GetSize() )
            {
                avr_warning( "Attempt to set break at invalid addr\n" );
                gdb_send_reply( "E01" );
                return;
            }

            if (z == 'z') 
            {
                //cout << "Try to UNSET a software breakpoint" << endl;
                //cout << "at address :" << addr << " with len " << len << endl;
                avr_core_remove_breakpoint( addr/2 );
            }
            else
            {
                //cout << "Try to SET a software breakpoint" << endl;
                //cout << "at address :" << addr << " with len " << len << endl;
                avr_core_insert_breakpoint( addr/2 );
            }
            break;

        case '1':               /* hardware breakpoint */
            //cout << "Try to set a hardware breakpoint" << endl;
            //cout << "at address :" << addr << " with len " << len << endl;

            gdb_send_reply( "" );
            return;
            break;

        case '2':               /* write watchpoint */
            //cout << "Try to set a watchpoint" << endl;
            //cout << "at address :" << addr << " with len " << len << endl;
            gdb_send_reply( "" );
            return;
            break;

        case '3':               /* read watchpoint */
            //cout << "Try to set a read watchpoint" << endl;
            //cout << "at address :" << addr << " with len " << len << endl;
            gdb_send_reply( "" );
            return;
            break;

        case '4':               /* access watchpoint */
            //cout << "try to set a access watchpoint" << endl;
            //cout << "at address :" << addr << " with len " << len << endl;
            gdb_send_reply( "" );
            return;             /* unsupported yet */
    }

    gdb_send_reply( "OK" );
}

void GdbServer::gdb_select_thread(const char *pkt)
{
    if(pkt[0] == 'c') {
        gdb_send_reply( "" );  // cannot force thread-switch on target
        return;
    }
    if(pkt[0] != 'g') {
        gdb_send_reply( "" );
        pkt--;
        if (global_debug_on)
            fprintf(stderr, "gdb  '%s' not supported\n", pkt);
        return;
    }

    int thread_id = 0;
    if(strcmp(pkt+1, "-1") == 0)
        thread_id = -1;
    else{
        for(const char* p = pkt+1; *p != '\0'; p++) {
            thread_id = thread_id << 4 | hex2nib(*p);
        }
    }

    if (global_debug_on)
        fprintf(stderr, "gdb* set thread %d\n", thread_id);
    m_gdb_thread_id = (thread_id >= 1) ? thread_id : 1;  // Values "0" (any) and "1" (all) are not supported
    gdb_send_reply( "OK" );
}

void GdbServer::gdb_is_thread_alive(const char *pkt)
{
    int thread_id = 0;
    if(strcmp(pkt, "-1") == 0)
        thread_id = -1;
    else{
        for(const char* p = pkt; *p != '\0'; p++) {
            thread_id = thread_id << 4 | hex2nib(*p);
        }
    }
    if (global_debug_on)
        fprintf(stderr, "gdb  is thread %d alive\n", thread_id);
    bool alive = core->stack->m_ThreadList.IsGDBThreadAlive(thread_id);
    assert(alive);
    gdb_send_reply( alive ? "OK" : "E00" );
}

void GdbServer::gdb_get_thread_list(const char *pkt)
{
    if (global_debug_on)
        fprintf(stderr, "gdb  get thread info\n");
    unsigned char allocated = core->stack->m_ThreadList.GetCount() * 3 + 5;
    char * response = new char[allocated];
    response[0] = 'm';
    unsigned char pos = 1;

    for(unsigned int i = 0; i < core->stack->m_ThreadList.GetCount(); i++) {
        int used = snprintf(response + pos, allocated-pos, "%d,", i+1);
        pos += used;
    }
    assert(response[pos-1] == ',');
    response[pos-1] = '\0';

    gdb_send_reply(response);
    delete [] response;
}

/*! Continue command format: "c<addr>" or "s<addr>"

If addr is given, resume at that address, otherwise, resume at current
address.

Continue with signal command format: "C<sig>;<addr>" or "S<sig>;<addr>"
"<sig>" should always be 2 hex digits, possibly zero padded.
";<addr>" part is optional.

If addr is given, resume at that address, otherwise, resume at current
address. */
int GdbServer::gdb_get_signal(const char *pkt) {
    int signo;
    //char step = *(pkt-1);

    /* strip out the signal part of the packet */

    signo  = (hex2nib( *pkt++ ) << 4);
    signo += (hex2nib( *pkt++ ) & 0xf);

    if (global_debug_on)
        fprintf( stderr, "GDB sent signal: %d\n", signo );

    /* Process signals send via remote protocol from gdb. Signals really don't
    make sense to the program running in the simulator, so we are using
    them sort of as an 'out of band' data. */

    switch (signo)
    {
        case GDB_SIGHUP:
            /* Gdb user issuing the 'signal SIGHUP' command tells sim to reset
            itself. We reply with a SIGTRAP the same as we do when gdb
            makes first connection with simulator. */
            core->Reset( );
            gdb_send_reply( "S05" );
            break;
    }

    return signo;
}

/*! Parse the packet. Assumes that packet is null terminated.
Return GDB_RET_KILL_REQUEST if packet is 'kill' command,
GDB_RET_OK otherwise. */
int GdbServer::gdb_parse_packet(const char *pkt) {
    switch (*pkt++) {
        case '?':               /* last signal */
            gdb_send_reply("S05"); /* signal # 5 is SIGTRAP */
            break;

        case 'H':               /* Set thread */
            gdb_select_thread(pkt);
            break;
        case 'T':               /* Is thread alive */
            gdb_is_thread_alive(pkt);
            break;
        case 'g':               /* read registers */
            gdb_read_registers();
            break;

        case 'G':               /* write registers */
            gdb_write_registers(pkt);
            break;

        case 'p':               /* read a single register */
            gdb_read_register(pkt);
            break;

        case 'P':               /* write single register */
            gdb_write_register(pkt);
            break;

        case 'm':               /* read memory */
            gdb_read_memory(pkt);
            break;

        case 'M':               /* write memory */
            gdb_write_memory(pkt);
            break;

        case 'D':               /* detach the debugger */
        case 'k':               /* kill request */
            /* Reset the simulator since there may be another connection
            before the simulator stops running. */
            gdb_send_reply("OK");
            if(exitOnKillRequest)
                raise(SIGINT);
            return GDB_RET_KILL_REQUEST;

        case 'c':               /* continue */
            if(!core->Flash->IsProgramLoaded()) {
                gdb_send_hex_reply("O", "No program to simulate. Use 'load' to upload it.\n");
                SendPosition(GDB_SIGHUP);
                return GDB_RET_OK;
            }
            return GDB_RET_CONTINUE;
            break;

        case 'C':               /* continue with signal */
            if(GDB_SIGHUP==gdb_get_signal(pkt)) { //very special solution only for regression testing woth
                                              //old scripts from old simulavr! Continue means not continue
                                              //if signal is SIGHUP :-(, so we do nothing then!
                exitOnKillRequest = true;
                return GDB_RET_OK;
            }
            return GDB_RET_CONTINUE;
            break;

        case 'S':               /* step with signal */
            gdb_get_signal(pkt);
            // no break!
        case 's':               /* step */
            if(!core->Flash->IsProgramLoaded()) {
                gdb_send_hex_reply("O", "No program to simulate. Use 'load' to upload it.\n");
                // No SIGTRAP when GDB does "Single stepping until exit from function __vectors"
                SendPosition(GDB_SIGHUP);
                return GDB_RET_OK;
            }
            return GDB_RET_SINGLE_STEP;

        case 'z':               /* remove break/watch point */
        case 'Z':               /* insert break/watch point */
            gdb_break_point(pkt);
            break;

        case 'q':               /* query requests */
            pkt--;
            if(memcmp(pkt, "qSupported", 10) == 0) {
                gdb_send_reply("PacketSize=800;qXfer:features:read+");
                return GDB_RET_OK;
            } else if(memcmp(pkt, "qXfer:features:read:target.xml:", 31) == 0) {
                // GDB XML target descriptions, since GDB 6.7 (2007-10-10)
                // see http://sources.redhat.com/gdb/current/onlinedocs/gdb/Target-Descriptions.html
                gdb_send_reply("l"
                               "<?xml version=\"1.0\"?>\n"
                               "<!DOCTYPE target SYSTEM \"gdb-target.dtd\">\n"
                               "<target version=\"1.0\">\n"
                               "    <architecture>avr</architecture>\n"
                               "</target>\n");
                return GDB_RET_OK;
            } else if(strcmp(pkt, "qC") == 0) {
                int thread_id = core->stack->m_ThreadList.GetCurrentThreadForGDB();
                if (global_debug_on)
                    fprintf(stderr, "gdb  get current thread: %d\n", thread_id);
                char reply[100];
                snprintf( reply, sizeof(reply), "QC%02x", thread_id);
                gdb_send_reply( reply );
                return GDB_RET_OK;
            } else if(strcmp(pkt, "qfThreadInfo") == 0) {
                gdb_get_thread_list(pkt);
                return GDB_RET_OK;
            } else if(strcmp(pkt, "qsThreadInfo") == 0) {
                gdb_send_reply(  "l" );  // note lowercase "L"
                return GDB_RET_OK;
            }
            
            if(global_debug_on)
                fprintf(stderr, "gdb query '%s' not supported\n", pkt);
            gdb_send_reply("");
            break;

        default:
            pkt--;
            if(global_debug_on)
                fprintf(stderr, "gdb command '%s' not supported\n", pkt);
            gdb_send_reply("");
    }

    return GDB_RET_OK;
}

/*! Perform pre-packet parsing. This will handle messages from gdb which are
outside the realm of packets or prepare a packet for parsing.

Use the static block_on flag to reduce the over head of turning blocking on
and off every time this function is called. */
int GdbServer::gdb_receive_and_process_packet(int blocking) {
    int  res;
    int  c;
    std::string pkt_buf;
    int  cksum, pkt_cksum;

    server->SetBlockingMode(blocking);

    c = server->ReadByte();

    switch(c) {
        case '$':           /* read a packet */
            /* make sure we block on fd */
            server->SetBlockingMode(GDB_BLOCKING_ON);

            pkt_cksum = 0;
            c = server->ReadByte();
            while(c != '#') {
                pkt_buf.push_back(c);
                pkt_cksum += (unsigned char)c;
                c = server->ReadByte();
            }

            cksum  = hex2nib(server->ReadByte()) << 4;
            cksum |= hex2nib(server->ReadByte());

            /* FIXME: Should send "-" (Nak) instead of aborting when we get
            checksum errors. Leave this as an error until it is actually
            seen (I've yet to see a bad checksum - TRoth). It's not a simple
            matter of sending (Nak) since you don't want to get into an
            infinite loop of (bad cksum, nak, resend, repeat).*/

            if((pkt_cksum & 0xff) != cksum)
                avr_error("Bad checksum: sent 0x%x <--> computed 0x%x",
                          cksum, pkt_cksum);

            if(global_debug_on)
                fprintf(stderr, "Recv: \"$%s#%02x\"\n", pkt_buf.c_str(), cksum);

            /* always acknowledge a well formed packet immediately */
            gdb_send_ack();

            res = gdb_parse_packet(pkt_buf.c_str());
            if(res < 0)
                return res;

            break;

        case '-':
            // When debugging do type "set remotetimeout 1000000" in GDB.
            if(global_debug_on)
                fprintf(stderr, " gdb -> Nak\n");
            gdb_send_reply(gdb_last_reply(NULL));
            break;

        case '+':
            if(global_debug_on)
                fprintf(stderr, " gdb -> Ack\n");
            break;

        case 0x03:
            /* Gdb sends this when the user hits C-c. This is gdb's way of
             * telling the simulator to interrupt what it is doing and return
             * control back to gdb.
             */
            if (global_debug_on)
                fprintf( stderr, "gdb* Ctrl-C\n" );
            return GDB_RET_CTRL_C;

        case -1:
            /* fd is non-blocking and no data to read */
            return GDB_RET_NOTHING_RECEIVED;
            break;

        default:
            avr_warning("Unknown request from gdb: %c (0x%02x)\n", c, c);
    }

    return GDB_RET_OK;
}

/**
 * \brief Start interacting with gdb.
 * \param core     A previously initialized simulator core
 * \param port     Port which server will listen for connections on.
 * \param debug_on Turn on gdb debug diagnostic messages.
 * 
 * Start a tcp server socket on localhost listening for connections on the
 * given port. Once a connection is established, enter an infinite loop and
 * process command requests from gdb using the remote serial protocol. Only a
 * single connection is allowed at a time.
 */
void GdbServer::Run( )
{
    int res;
    char reply[MAX_BUF + 1];

    while (1)
    {
        res = gdb_receive_and_process_packet( GDB_BLOCKING_ON);
        switch (res)
        {
            case GDB_RET_KILL_REQUEST:
                return;

            case GDB_RET_CTRL_C:
                gdb_send_ack( );
                snprintf(reply, sizeof(reply), "S%02x", GDB_SIGINT);
                gdb_send_reply( reply );
                break;

            default:
                break;
        }
    }
}

//! try to accept a new connection from gdb
void GdbServer::TryConnectGdb() {
    time_t newTime = time(NULL);

    if(oldTime != newTime) {
        oldTime = newTime;

        connState = server->Connect();
        if(connState)
            allGdbServers.push_back(this);  //remark that we now must called everytime
    }
}

int GdbServer::Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns) {
    if(!connState) { // no connection established -> look for it
        TryConnectGdb();
        if (!waitForGdbConnection) {
            core->Step(trueHwStep, timeToNextStepIn_ns);    //if not connected to gdb simple run it  
        } else {
            if (timeToNextStepIn_ns!=0) *timeToNextStepIn_ns=core->GetClockFreq();
        }
        return 0;
    } else {
        return InternalStep(trueHwStep, timeToNextStepIn_ns);
    }
}

void GdbServer::IdleStep() {
    int gdbRet=gdb_receive_and_process_packet(GDB_BLOCKING_OFF);
    cout << "IdleStep Instance" << this << " RunMode:" << dec << runMode << endl;

    if (lastCoreStepFinished) {
        switch(gdbRet) {
            case GDB_RET_NOTHING_RECEIVED:
                break;

            case GDB_RET_OK:
                break;

            case GDB_RET_CONTINUE:
                runMode=GDB_RET_CONTINUE;
                break;

            case GDB_RET_CTRL_C:
                runMode=GDB_RET_CTRL_C;
                SendPosition(GDB_SIGINT); //Give gdb an idea where the core is now 
                break;

            default:
                cout << "wondering" << endl;
        }
    }
}

int GdbServer::InternalStep(bool &untilCoreStepFinished, SystemClockOffset *timeToNextStepIn_ns) {
    //cout << "Internal Step entered" << endl;
    //cout << "RunMode: " << dec << runMode << endl;

    if (lastCoreStepFinished) {
        bool leave;

        do {
            //cout << "Loop" << endl;
            int gdbRet=gdb_receive_and_process_packet((runMode==GDB_RET_CONTINUE) ? GDB_BLOCKING_OFF : GDB_BLOCKING_ON);

            switch (gdbRet) { //GDB_RESULT TYPES
                case GDB_RET_NOTHING_RECEIVED:  //nothing changes here
                    break;

                case GDB_RET_OK:    //dont change any modes
                    runMode=GDB_RET_OK;
                    break;

                case GDB_RET_CONTINUE:
                    //cout << "############################################################ gdb continue" << endl;
                    runMode=GDB_RET_CONTINUE;       //lets continue until we receive something from gdb (normal CTRL-C)
                    break;                          //or we run into a break point or illegal instruction

                case GDB_RET_SINGLE_STEP:
                    //cout << "############################################################# Single Step" << endl;
                    runMode=GDB_RET_SINGLE_STEP;
                    break;

                case GDB_RET_CTRL_C:
                    //cout << "############################################################# CTRL C" << endl;
                    runMode=GDB_RET_CTRL_C;
                    SendPosition(GDB_SIGINT); //Give gdb an idea where the core is now 
                    break;

                case GDB_RET_KILL_REQUEST:
                    core->Reset();
                    server->CloseConnection();   //we are not longer connected
                    connState = false;
                    core->DeleteAllBreakpoints();
                    return 0; 
            } //end switch GDB_RETURN_VALUE

            if (runMode == GDB_RET_SINGLE_STEP || runMode == GDB_RET_CONTINUE) {
                leave = true;
            } else {
                leave = false;
            }

            if(!leave) { //we can�t leave the loop so we have to request the other gdb instances now!
                // step through all gdblist members WITHOUT my self!
                //cout << "we do not leave and check for gdb events" << endl;
                vector<GdbServer*>::iterator ii;
                for (ii=allGdbServers.begin(); ii!=allGdbServers.end(); ii++) {
                    if (*ii!=this) { //run other instances but not me 
                        (*ii)->IdleStep();
                    }
                }
            }
        } while (leave==false);

    } //last core step finished

    int res=core->Step(untilCoreStepFinished, timeToNextStepIn_ns);
    lastCoreStepFinished=untilCoreStepFinished;

    if (res == BREAK_POINT) {
        runMode=GDB_RET_OK; //we will stop next call from GdbServer::Step
        SendPosition(GDB_SIGTRAP);
    }

    if (res == INVALID_OPCODE)
    {
        //why we send here another reply??? is it not better to send it later
        //the signo is set correct.... TODO
        char reply[MAX_BUF+1];
        snprintf(reply, sizeof(reply), "S%02x", GDB_SIGILL);
        gdb_send_reply( reply );
        runMode=GDB_RET_OK;
        SendPosition(GDB_SIGILL);
    }

    if (runMode==GDB_RET_SINGLE_STEP) {
        runMode=GDB_RET_OK;
        SendPosition(GDB_SIGTRAP);
    }

    return 0;
}

void GdbServer::SendPosition(int signo) {
    /* Send gdb PC, FP, SP */
    int bytes = 0;
    char reply[MAX_BUF + 1];
    unsigned long sp = core->stack->GetStackPointer();
    unsigned int spl = sp & 0xff;
    unsigned int sph = (sp >> 8) & 0xff;
    int pc = core->PC * 2;
    int thread_id = core->stack->m_ThreadList.GetCurrentThreadForGDB();

    bytes = snprintf(reply, sizeof(reply), "T%02x", signo);

    /* SREG, SP & PC */
    snprintf(reply + bytes, sizeof(reply) - bytes,
            "20:%02x;" "21:%02x%02x;" "22:%02x%02x%02x%02x;" "thread:%d;",
            ((int)(*(core->status))),
            spl, sph,
            pc & 0xff, (pc >> 8) & 0xff, (pc >> 16) & 0xff, (pc >> 24) & 0xff,
            thread_id);

    gdb_send_reply(reply);
    /* Next "read registers" command will be related to the new thread. */
    m_gdb_thread_id = thread_id;
}

int GdbServer::SleepStep() {
    return 0;
}
