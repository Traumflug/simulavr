#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
#include "trace.h"
#include "helper.h"

ofstream traceOut;
int trace_on=0;


void trioaccess(const char *t, unsigned char val) {
    if (trace_on) {
        traceOut << t << "=" << HexChar(val) << " ";
    }
}

string traceFname;

void StartTrace(const char *fname) {
    traceFname=fname;
    traceOut.open(fname);
    trace_on=1;
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


