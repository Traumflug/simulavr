#include <iostream>
#include <fstream>

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

void StartTrace(const char *fname) {
    traceOut.open(fname);
    trace_on=1;
}


