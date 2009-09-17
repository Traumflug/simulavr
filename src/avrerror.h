/*
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

#ifndef SIM_AVRERROR_H
#define SIM_AVRERROR_H

#include <iostream>

//! Class, that handle messages to console and also exit/abort calls
class SystemConsoleHandler {
    
    public:
        //! creates a SystemConsoleHandler instance
        /*! This is needed only once for a application, see global variable
          sysConHandler, where such instance is created by default. */
        SystemConsoleHandler();
        
        //! Tells the handler, that exit/abort is to use instead of exceptions
        void SetUseExit(bool useExit = true);
        //! Sets the output stream, where messages are sent to
        void SetMessageStream(std::ostream *s);
        //! Sets the output stream, where warnings and errors are sent to
        void SetWarningStream(std::ostream *s);
        
        //! Format and send a message to message stream (default stdout)
        void vfmessage(const char *file, int line, const char *fmt, ...);
        //! Format and send a warning message to warning stream (default stderr)
        void vfwarning(const char *file, int line, const char *fmt, ...);
        //! Format and send a error message to warning stream (default stderr)
        void vferror(const char *file, int line, const char *fmt, ...);
        //! Format and send a error message to stderr and call exit or raise a exception
        void vffatal(const char *file, int line, const char *fmt, ...);
        
        //! Aborts application: uses abort or exeption depending on useExitAndAbort
        void AbortApplication(int code);
        //! Exits application: uses exit or exeption depending on useExitAndAbort
        void ExitApplication(int code);
        
    protected:
        bool useExitAndAbort; //!< Flag, if exit/abort have to be used instead of exceptions
        char formatStringBuffer[128]; //!< Buffer for format strings to format a message
        char messageStringBuffer[512]; //!< Buffer for built message string itself
        std::ostream *msgStream; //!< Stream, where normal messages are sent to
        std::ostream *wrnStream; //!< Stream, where warning and error messages are sent to
        
        //! Creates the format string for formatting a message
        char *getFormatString(const char *prefix, const char *file, int line, const char *fmtstr);
};

//! The SystemConsoleHandler instance for common usage
extern SystemConsoleHandler sysConHandler;

/* FIXME: TRoth 2002-02-23 : '## args' is gcc specific. If porting to another
   compiler, this will have to be handled. Although, I believe the C99
   standard added this to precompiler. */
#define avr_message(fmt, args...) sysConHandler.vfmessage(__FILE__, __LINE__, fmt, ## args)
#define avr_warning(fmt, args...) sysConHandler.vfwarning(__FILE__, __LINE__, fmt, ## args)
#define avr_failure(fmt, args...) sysConHandler.vferror(__FILE__, __LINE__, fmt, ## args)
#define avr_error(fmt, args...)   sysConHandler.vffatal(__FILE__, __LINE__, fmt, ## args)

#endif /* SIM_AVRERROR_H */
