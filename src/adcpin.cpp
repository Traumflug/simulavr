#include <iostream>
#include "adcpin.h"
using namespace std;

AdcPin::AdcPin(	const char*		fileName,
				Net&			pinNet
				) throw():
		_analogPin(),
		_anaFile(fileName)
		{
		_analogPin.SetOutState(Pin::ANALOG);
		pinNet.Add(&_analogPin);

		if(!_anaFile){
			cerr << "Cannot open Analog input file \"" << fileName << "\"." << endl;
			exit(1);
			}
	}

char*	readNextLine(std::ifstream& f,char* buffer,unsigned len,SystemClockOffset *timeToNextStepIn_ns){
	for(unsigned i=0;i<2;++i){
		while(f.getline(buffer, len)){
			// Skip comment lines
			if(buffer[0] == '#') continue;
			return buffer;
			}
		f.clear();
		f.seekg (0, ios::beg);
		}
	return 0;
	}

int	AdcPin::Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns){
	if(!_anaFile) return 0;

	char	lineBuffer[1024];

	if(!readNextLine(_anaFile,lineBuffer,sizeof(lineBuffer),timeToNextStepIn_ns)){
		_anaFile.close();
		}

	char*	p	= lineBuffer;
	unsigned long	delayInNs		= strtoul(p, &p, 0);
	int	analogValue		= (int)strtol(p, &p, 0);

	_analogPin.setAnalogValue(analogValue);

	*timeToNextStepIn_ns	= delayInNs;

	return 0;
	}

