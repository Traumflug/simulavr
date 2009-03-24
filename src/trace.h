/*
 *  $Id$
 */

#ifndef TRACE_h
#define TRACE_h

#include <iostream>
#include <fstream>

using namespace std;

extern ofstream traceOut;

// Trace enable flag
extern int global_trace_on;

// Verbose enable flag
extern int global_verbose_on;

// Print a message on bad IO and memory references
extern int global_message_on_bad_access;

void setVerbose(int value);
void setMessageOnBadAccess(int value);

void trioaccess(const char *t, unsigned char val);

void StartTrace(const char *fname);
void TraceNextLine();
#endif
