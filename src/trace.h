/*
 *  $Id$
 */

#ifndef TRACE_h
#define TRACE_h

#include <iostream>
#include <fstream>

extern std::ofstream traceOut;

// Trace enable flag
extern int global_trace_on;

// Verbose enable flag
extern int global_verbose_on;

void trioaccess(const char *t, unsigned char val);

void StartTrace(const char *fname);
void TraceNextLine();

#endif
