/*
 *  $Id$
 */

#include <iostream>

#include "mysocket.h"
#include "avrerror.h"

using namespace std;

#if !(defined(_MSC_VER) || defined(HAVE_SYS_MINGW))

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#else

int Socket::socketCount = 0;

void Socket::Start() {
    if(socketCount == 0) {
        WSADATA info;
        if(WSAStartup(MAKEWORD(2, 2), &info))
            avr_error("Could not start WSA");
    }
    socketCount++;
}

void Socket::End() {
    WSACleanup();
}

Socket::Socket(int port) {
    Start();
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == INVALID_SOCKET)
        avr_error("Couldn't create socket: INVALID_SOCKET");
    
    hostent *he;
    if((he = gethostbyname("127.0.0.1")) == 0)
        avr_error("Couldn't create connection: %s", strerror(errno));
    
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *((in_addr *)he->h_addr);
    memset(&(addr.sin_zero), 0, 8);
    
    u_long arg = 1;
    ioctlsocket(_socket, FIONBIO, &arg);
        
    if(::connect(_socket, (sockaddr *) &addr, sizeof(sockaddr)))
        avr_error("Couldn't create connection: %s", strerror(WSAGetLastError()));

    cerr << "User Interface Connection opened by host " << inet_ntoa(addr.sin_addr) << " port " << ntohs(addr.sin_port) << endl;
}

Socket::~Socket() { 
    closesocket(_socket);
    socketCount--;
    if(socketCount == 0)
        End();
}

ssize_t Socket::Poll() {
    u_long arg = 0;
    if(ioctlsocket(_socket, FIONREAD, &arg) != 0)
        return 0;
    return arg;
}

#endif

ssize_t Socket::Read(string &a) {
    char buf[256];
#if defined(_MSC_VER) || defined(HAVE_SYS_MINGW)
    ssize_t len = recv(_socket, buf, 255, 0);
#else
    ssize_t len = read( conn, &buf, 255 );
#endif

    if(len < 0)
        len=0;
    buf[len]= '\0';
    a += buf;
    return len;
}

void Socket::Write(const string &s) {
#if defined(_MSC_VER) || defined(HAVE_SYS_MINGW)
    int err = ::send(_socket, s.c_str(), s.length(), 0);
#else
    int err = ::write( conn, s.c_str(), s.size());
#endif
    if (err<0)
        cerr << "Write in UI fails!" << endl;
}

#if !(defined(_MSC_VER) || defined(HAVE_SYS_MINGW))

Socket::Socket(int port) {
    OpenSocket(port);
}

Socket::~Socket() { 
}

void Socket::OpenSocket(int port) {
    struct sockaddr_in address[1];
    int                i;

    if ( (sock = socket( PF_INET, SOCK_STREAM, 0 )) < 0 )
        cerr << "Can't create socket:" << strerror(errno) << endl;


    /* Tell TCP not to delay small packets.  This greatly speeds up
    interactive response. WARNING: If TCP_NODELAY is set on, then gdb
    may timeout in mid-packet if the (gdb)packet is not sent within a
    single (tcp)packet, thus all outgoing (gdb)packets _must_ be sent
    with a single call to write. (see Stevens "Unix Network
    Programming", Vol 1, 2nd Ed, page 202 for more info) */

#ifdef WE_ACT_AS_SERVER
    socklen_t          addrLength[1];
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    memset( &address->sin_addr, 0, sizeof(address->sin_addr) );


    if ( bind( sock, (struct sockaddr *)address, sizeof(address) ) )
        cerr << "Can not bind socket: "<< strerror(errno) << endl;


    if ( listen(sock, 1) )
    {
        int saved_errno = errno;
        cerr << "Can not listen on socket: " <<  strerror(saved_errno) << endl;
    }

    fprintf( stderr, "Waiting on port %d for user interface client to connect...\n", port );

    /* accept() needs this set, or it fails (sometimes) */
    addrLength[0] = sizeof(struct sockaddr *);

    /* We only want to accept a single connection, thus don't need a loop. */
    conn = accept( sock, (struct sockaddr *)address, addrLength );
    if (conn < 0)
    {
        int saved_errno = errno;
        /*
        if ( signal_has_occurred(SIGINT) )
        {
        break;          // SIGINT will cause accept to be interrupted 
        }
        */
        cerr<<  "Accept connection failed: " <<  strerror(saved_errno) << endl;
    }

    i = 1;
    setsockopt (conn, IPPROTO_TCP, TCP_NODELAY, &i, sizeof (i));
#else //so we are client
    address->sin_family= AF_INET;
    address->sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &address->sin_addr);
    conn=sock;

    int retry=0;
    int ret;
    do {
        ret = connect ( sock, (struct sockaddr *)address, sizeof(address));
        if (ret<0) {
            cerr << "No connect to socket possible now... retry " << strerror(errno) << endl;
            sleep(1);
        } else {
            break; //leave retry loop
        }
        
    } while (retry++ <10);

    if(ret < 0)
        avr_error("Could not contact the ui-server, sorry");

#endif

    /* Let the kernel reuse the socket address. This lets us run
    twice in a row, without waiting for the (ip, port) tuple
    to time out. */
    i = 1;
    setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i) );

    /* If we got this far, we now have a client connected and can start 
    processing. */

    cerr << "User Interface Connection opened by host "<< inet_ntoa(address->sin_addr) << " port " <<   ntohs(address->sin_port) << endl;

    fcntl(conn, F_SETFL, O_NONBLOCK);
}


ssize_t Socket::Poll() {
    pollfd pfd= {
        conn,
        POLLIN 
#ifdef POLLRDNORM
            | POLLRDNORM 
#endif
#ifdef POLLRDBAND
            | POLLRDBAND 
#endif
            | POLLPRI,
        0
    };

    int erg=poll( &pfd, 1, 0); 
    if (erg<0) {
        //perror ("Error in polling");
        return 0; //nix gelesen ggf unterbechung
    }

    return erg;
}

#endif

