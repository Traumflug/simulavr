/*
 *  $Id$
 */

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
#include "trace.h"
#include "helper.h"

ofstream traceOut;
int global_trace_on=0;
int global_verbose_on=0;
int global_message_on_bad_access=1;

void setVerbose(int value)
{
  global_verbose_on = value;
}

void setMessageOnBadAccess(int value)
{
  global_message_on_bad_access = value;
}


void trioaccess(const char *t, unsigned char val) {
        traceOut << t << "=" << HexChar(val) << " ";
}

string traceFname;

void StartTrace(const char *fname) {
    traceFname=fname;
    traceOut.open(fname);
    global_trace_on=1;
}

void TraceNextLine() {
    static long lineCnt=0;
    static int fileCnt=0;

    lineCnt++;
    if (lineCnt> 10000000) {
        fileCnt++;
        lineCnt=0;
        traceOut.close();

        ostringstream os;
        os << traceFname << fileCnt;
        traceOut.open(os.str().c_str());
    }
}


