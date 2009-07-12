#include <iostream>
#include "pinmon.h"

using namespace std;

PinMonitor::PinMonitor(	AvrDevice&	avr,
						const char*	pinNameStr, // AVR pin name.  (e.g. "B1","C2", etc.)
						const char*	pinDescStr,
						const char*	pinHighStr,
						const char*	pinLowStr
						) throw():
		_prevState(true)
		{
	Pin*	pin	= avr.GetPin(pinNameStr);
	pin->RegisterCallback(this);
	_pinDescStr	= pinDescStr?pinDescStr:pinNameStr;
	_pinHighStr	= pinHighStr?pinHighStr:"HIGH";
	_pinLowStr	= pinLowStr?pinLowStr:"LOW";
	}

void PinMonitor::PinStateHasChanged(Pin* pin){
	const char*	stateStr;
	if((bool)*pin == _prevState){
		return;
		}
	if((bool)*pin){
		stateStr	= _pinHighStr;
		_prevState	= true;
		}
	else {
		stateStr	= _pinLowStr;
		_prevState	= false;
		}

	cout << _pinDescStr << ": " << stateStr << endl;
	}

