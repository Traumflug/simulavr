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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 */

#ifndef SIM_AVRERROR_H
#define SIM_AVRERROR_H

#include <iostream>

#if defined(_MSC_VER) && !defined(SWIG)
#define ATTRIBUTE_NORETURN __declspec(noreturn)
#define ATTRIBUTE_PRINTF(string_arg, first_arg)
#elif defined(__GNUC__)
#ifndef ATTRIBUTE_NORETURN
#define ATTRIBUTE_NORETURN __attribute__((noreturn))
#endif
#ifndef ATTRIBUTE_PRINTF
#define ATTRIBUTE_PRINTF(string_arg, first_arg) __attribute__ ((format (printf, string_arg, first_arg)))
#endif
#else
#define ATTRIBUTE_NORETURN
#define ATTRIBUTE_PRINTF(string_arg, first_arg)
#endif

//! Class, that handle messages to console and also exit/abort calls
class SystemConsoleHandler {
    
    public:
        //! creates a SystemConsoleHandler instance
        /*! This is needed only once for a application, see global variable
          sysConHandler, where such instance is created by default. */
        SystemConsoleHandler();
        ~SystemConsoleHandler();
        
        //! Tells the handler, that exit/abort is to use instead of exceptions
        void SetUseExit(bool useExit = true);
        //! Sets the output stream, where messages are sent to
        void SetMessageStream(std::ostream *s);
        //! Sets the output stream, where warnings and errors are sent to
        void SetWarningStream(std::ostream *s);
        
        //! Sets the trace to file stream and enables tracing global
        void SetTraceFile(const char *name, unsigned int maxlines = 0);
        //! Sets the trace to given stream and enables tracing global
        void SetTraceStream(std::ostream *s);
        //! Stops tracing global, close file, if set, redirect trace to nullStream
        void StopTrace(void);
        //! Returns true, if tracing is global enabled
        bool GetTraceState(void) { return traceEnabled; }
        //! Gives Access to trace stream
        std::ostream &traceOutStream(void) { return *traceStream; }
        //! Ends a trace line, performs reopen new filestream, if necessary
        void TraceNextLine(void);
        
        //! Format and send a message to message stream (default stdout)
        void vfmessage(const char *fmt, ...)
            ATTRIBUTE_PRINTF(2, 3);
        //! Format and send a warning message to warning stream (default stderr)
        void vfwarning(const char *file, int line, const char *fmt, ...)
            ATTRIBUTE_PRINTF(4, 5);
        //! Format and send a error message to warning stream (default stderr)
        void vferror(const char *file, int line, const char *fmt, ...)
            ATTRIBUTE_PRINTF(4, 5);
        //! Format and send a error message to stderr and call exit or raise a exception
        ATTRIBUTE_NORETURN
        void vffatal(const char *file, int line, const char *fmt, ...)
            ATTRIBUTE_PRINTF(4, 5);
        
        //! Aborts application: uses abort or exception depending on useExitAndAbort
        ATTRIBUTE_NORETURN
        void AbortApplication(int code);
        //! Exits application: uses exit or exception depending on useExitAndAbort
        ATTRIBUTE_NORETURN
        void ExitApplication(int code);
        
    protected:
        bool useExitAndAbort; //!< Flag, if exit/abort have to be used instead of exceptions
        char formatStringBuffer[192]; //!< Buffer for format strings to format a message
        char messageStringBuffer[768]; //!< Buffer for built message string itself, 4 times bigger than formatStringBuffer
        std::ostream *msgStream; //!< Stream, where normal messages are sent to
        std::ostream *wrnStream; //!< Stream, where warning and error messages are sent to
        std::ostream *traceStream; //!< Stream for trace output
        std::ostream *nullStream; //!< /dev/null! ;-)
        bool traceEnabled; //!< flag, true if trace is enabled
        bool traceToFile; //!< flag, true if trace writes to filestream
        std::string traceFilename; //!< file name for trace file (will be appended with file count!)
        unsigned int traceLinesOnFile; //!< how much lines will be written on one trace file 0->means endless
        unsigned int traceLines; //!< how much lines are written on current trace file
        int traceFileCount; //!< Counter for trace files
        
        //! Creates the format string for formatting a message
        char *getFormatString(const char *prefix, const char *file, int line, const char *fmtstr);
};

//! The SystemConsoleHandler instance for common usage
extern SystemConsoleHandler sysConHandler;

// redirect old definition ostream traceOut to SystemConsoleHandler.traceStream
#define traceOut sysConHandler.traceOutStream()

// moved from trace.h
//! Verbose enable flag
extern int global_verbose_on;

// moved from trace.h
//! Helper function for writing trace (trace IO access)
void trioaccess(const char *t, unsigned char val);

#define avr_message(...) sysConHandler.vfmessage(__VA_ARGS__)
#define avr_warning(...) sysConHandler.vfwarning(__FILE__, __LINE__, ## __VA_ARGS__)
#define avr_failure(...) sysConHandler.vferror(__FILE__, __LINE__, ## __VA_ARGS__)
#define avr_error(...)   sysConHandler.vffatal(__FILE__, __LINE__, ## __VA_ARGS__)

#endif /* SIM_AVRERROR_H */
