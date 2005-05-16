#include <iostream>
#include <fstream>

using namespace std;

extern ofstream traceOut;
extern int global_trace_on;
void trioaccess(const char *t, unsigned char val);

void StartTrace(const char *fname);
void TraceNextLine();



