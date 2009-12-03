#include <iostream>
#include "spisrc.h"
#include "avrerror.h"

using namespace std;

SpiSource::SpiSource(   const char* fileName,
                        Net&        ssNet,
                        Net&        sclkNet,
                        Net&        mosiNet
                        ) throw():
        _ss(),
        _sclk(),
        _mosi(),
        _spiFile(fileName)
        {
        _ss.outState = Pin::HIGH;
        ssNet.Add(&_ss);

        _sclk.outState = Pin::HIGH;
        sclkNet.Add(&_sclk);

        _mosi.outState = Pin::HIGH;
        mosiNet.Add(&_mosi);

        if(!_spiFile)
            avr_error("Cannot open SPI Source input file '%s'", fileName);
    }

static char*    readNextLine(std::ifstream& f,char* buffer,unsigned len,SystemClockOffset *timeToNextStepIn_ns){
    *timeToNextStepIn_ns    = 100000;   // Once every 100 microseconds
    for(unsigned i=0;i<2;++i){
        while(f.getline(buffer, len)){
            if(buffer[0] == '#') continue;
            return buffer;
            }
        *timeToNextStepIn_ns    = 1000000;  // Once every 100 microseconds
        f.clear();
        f.seekg (0, ios::beg);
        }
    return 0;
    }

int SpiSource::Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns){
    if(!_spiFile) return 0;

    char    lineBuffer[1024];

    if(!readNextLine(_spiFile,lineBuffer,sizeof(lineBuffer),timeToNextStepIn_ns)){
        _spiFile.close();
        return 0;
        }

    char*   p   = lineBuffer;
    unsigned long   ss      = strtoul(p, &p, 0);
    unsigned long   sclk    = strtoul(p, &p, 0);
    unsigned long   output  = strtoul(p, &p, 0);

    _ss = (ss)?'H':'L';
    _sclk   = (sclk)?'H':'L';
    _mosi   = (output)?'H':'L';
    return 0;
    }

