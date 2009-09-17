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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "avrerror.h"

SystemConsoleHandler::SystemConsoleHandler() {
    useExitAndAbort = true;
    msgStream = &std::cout;
    wrnStream = &std::cerr;
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

void SystemConsoleHandler::vfmessage(const char *file, int line, const char *fmt, ...) {
    va_list ap;
    char *mfmt = getFormatString("MESSAGE", file, line, fmt);
    va_start(ap, fmt);
    vsnprintf(messageStringBuffer, sizeof(messageStringBuffer), mfmt, ap);
    va_end(ap);
    *msgStream << messageStringBuffer;
}

void SystemConsoleHandler::vfwarning(const char *file, int line, const char *fmt, ...) {
    va_list ap;
    char *mfmt = getFormatString("WARNING", file, line, fmt);
    va_start(ap, fmt);
    vsnprintf(messageStringBuffer, sizeof(messageStringBuffer), mfmt, ap);
    va_end(ap);
    *wrnStream << messageStringBuffer;
    wrnStream->flush();
}

void SystemConsoleHandler::vferror(const char *file, int line, const char *fmt, ...) {
    va_list ap;
    char *mfmt = getFormatString("ERROR", file, line, fmt);
    va_start(ap, fmt);
    vsnprintf(messageStringBuffer, sizeof(messageStringBuffer), mfmt, ap);
    va_end(ap);
    *wrnStream << messageStringBuffer;
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
        abort();
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

// EOF
