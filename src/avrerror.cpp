/*
 *
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002  Theodore A. Roth
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

/*!
   \file avrerror.cpp
   \brief Functions for printing messages, warnings and errors.

   This module provides output printing facilities. Further it provides
   raising exceptions instead of calling exit/abort, if needed and the
   possibility to redirect output to a stream instead of stdout/stderr. */

#include <fstream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "avrerror.h"
#include "helper.h"

/* for preprocessor symbol HAVE_SYS_MINGW */
#include "config.h"

#ifdef _MSC_VER
#  define snprintf _snprintf
#endif

SystemConsoleHandler::SystemConsoleHandler() {
    useExitAndAbort = true;
    nullStream = new std::ostream(0);
    msgStream = &std::cout;
    wrnStream = &std::cerr;
    traceStream = nullStream;
    traceEnabled = false;
}

SystemConsoleHandler::~SystemConsoleHandler() {
    StopTrace();
    delete nullStream;
}

void SystemConsoleHandler::SetUseExit(bool useExit) {
    useExitAndAbort = useExit;
}

void SystemConsoleHandler::SetMessageStream(std::ostream *s) {
    msgStream = s;
}

void SystemConsoleHandler::SetWarningStream(std::ostream *s) {
    wrnStream = s;
}

void SystemConsoleHandler::SetTraceFile(const char *name, unsigned int maxlines) {
    StopTrace();
    std::ofstream* os = new std::ofstream();
    os->open(name);
    traceFilename = name;
    traceStream = os;
    traceFileCount = 1;
    traceLinesOnFile = maxlines;
    traceLines = 0;
    traceEnabled = true;
    traceToFile = true;
}

void SystemConsoleHandler::SetTraceStream(std::ostream *s) {
    StopTrace();
    traceStream = s;
    traceEnabled = true;
    traceToFile = false;
}

void SystemConsoleHandler::StopTrace(void) {
    if(!traceEnabled)
        return;
    if(traceToFile)
        ((std::ofstream *)traceStream)->close();
    traceStream = nullStream;
    traceEnabled = false;
}

void SystemConsoleHandler::TraceNextLine(void) {
    if(!traceEnabled || !traceToFile)
        return;

    traceLines++;
    if( ( traceLinesOnFile ) && ( traceLines >= traceLinesOnFile)) {
        traceFileCount++;
        traceLines = 0;
        
        ((std::ofstream *)traceStream)->close();
        delete traceStream;
        
        std::ostringstream n;
        int idx = traceFilename.rfind('.');
        n << traceFilename.substr(0, idx) << "_" << traceFileCount << traceFilename.substr(idx);
        std::ofstream* os = new std::ofstream();
        os->open(n.str().c_str());
        
        traceStream = os;
    }
}

void SystemConsoleHandler::vfmessage(const char *fmt, ...) {
    if ( ! global_verbose_on)
        return;

    va_list ap;
    snprintf(formatStringBuffer, sizeof(formatStringBuffer),
             "MESSAGE %s", fmt);
    va_start(ap, fmt);
    vsnprintf(messageStringBuffer, sizeof(messageStringBuffer),
              formatStringBuffer, ap);
    va_end(ap);
    *msgStream << messageStringBuffer;
    if(fmt[strlen(fmt) - 1] != '\n')
        *msgStream << std::endl;
    msgStream->flush();
}

void SystemConsoleHandler::vfwarning(const char *file, int line, const char *fmt, ...) {
    va_list ap;
    char *mfmt = getFormatString("WARNING", file, line, fmt);
    va_start(ap, fmt);
    vsnprintf(messageStringBuffer, sizeof(messageStringBuffer), mfmt, ap);
    va_end(ap);
    *wrnStream << messageStringBuffer;
    if(fmt[strlen(fmt) - 1] != '\n')
        *wrnStream << std::endl;
    wrnStream->flush();
}

void SystemConsoleHandler::vferror(const char *file, int line, const char *fmt, ...) {
    va_list ap;
    char *mfmt = getFormatString("ERROR", file, line, fmt);
    va_start(ap, fmt);
    vsnprintf(messageStringBuffer, sizeof(messageStringBuffer), mfmt, ap);
    va_end(ap);
    *wrnStream << messageStringBuffer;
    if(fmt[strlen(fmt) - 1] != '\n')
        *wrnStream << std::endl;
    wrnStream->flush();
}

void SystemConsoleHandler::vffatal(const char *file, int line, const char *fmt, ...) {
    va_list ap;
    char *mfmt = getFormatString("FATAL", file, line, fmt);
    va_start(ap, fmt);
    vsnprintf(messageStringBuffer, sizeof(messageStringBuffer), mfmt, ap);
    va_end(ap);
    if(useExitAndAbort) {
        *wrnStream << "\n" << messageStringBuffer << "\n" << std::endl;
        exit(1);
    } else {
        throw (char const*)messageStringBuffer;
    }
}

void SystemConsoleHandler::AbortApplication(int code) {
    if(useExitAndAbort) {
#if defined(HAVE_SYS_MINGW) || defined(_MSC_VER)
        /* TODO: changed because of problems on windows7 with abort call, with abort it will bring up a
           message box and break regression test. It will also irritate user. There is a call _set_abort_behavior
           reported on MSDN, which could help, but in the moment not available in my MSys. */
        exit(3);
#else
        abort();
#endif
    } else {
        throw -code;
    }
}

void SystemConsoleHandler::ExitApplication(int code) {
    if(useExitAndAbort) {
        exit(code);
    } else {
        throw code;
    }
}

char* SystemConsoleHandler::getFormatString(const char *prefix,
                                            const char *file,
                                            int line,
                                            const char *fmtstr) {
    snprintf(formatStringBuffer,
             sizeof(formatStringBuffer),
             "%s: file %s: line %d: %s",
             prefix,
             file,
             line,
             fmtstr);
    formatStringBuffer[sizeof(formatStringBuffer) - 1] = '\0';
    return formatStringBuffer;
}

// create the handler instance
SystemConsoleHandler sysConHandler;

int global_verbose_on = 0;

void trioaccess(const char *t, unsigned char val) {
    sysConHandler.traceOutStream() << t << "=" << HexChar(val) << " ";
}

// EOF
