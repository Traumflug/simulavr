#include <iostream>
#include "spisink.h"

SpiSink::SpiSink(	Net&		ssNet,
					Net&		sclkNet,
					Net&		misoNet,
					bool		clockIsIdleHigh,
					bool		clockSampleOnLeadingEdge
					) throw():
		_ss(),
		_sclk(),
		_miso(),
		_state(0),
		_sr(0),
		_clockIsIdleHigh(clockIsIdleHigh),
		_clockSampleOnLeadingEdge(clockSampleOnLeadingEdge),
		_prevClkState(clockIsIdleHigh),
		_prevSS(true)
		{
		_ss.SetOutState(Pin::PULLUP);
		ssNet.Add(&_ss);

		_sclk.SetOutState(Pin::PULLUP);
		sclkNet.Add(&_sclk);

		_miso.SetOutState(Pin::PULLUP);
		misoNet.Add(&_miso);
	}

int	SpiSink::Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns){
	*timeToNextStepIn_ns	= 1000;	// Once every microsecond
	bool	sample = false;

	if(!_ss){
		if(_prevClkState != (bool)_sclk){
			_prevClkState	= (bool)_sclk;
			if(_clockIsIdleHigh){
				// Clock is HIGH when idle
				if(_clockSampleOnLeadingEdge){
					// Sample on leading edge
					sample	= ((bool)_sclk)?false:true;
					}
				else {
					// Sample on trailing edge
					sample	= ((bool)_sclk)?true:false;
					}
				}
			else {
				// Clock is LOW when idle
				if(_clockSampleOnLeadingEdge){
					// Sample on leading edge
					sample	= ((bool)_sclk)?true:false;
					}
				else {
					// Sample on trailing edge
					sample	= ((bool)_sclk)?false:true;
					}
				}
			}
		}
	else {
		_sr		= 0;
		_state	= 0;
		}

	for(;;){
		switch(_state){
			case 0:	// Waiting for /SS
				if(!_ss){
					_state	= 1;
					continue;
					}
				break;
			case 1:	// First sample
			case 2:	// Second sample
			case 3:	// Third sample
			case 4:	// Fourth sample
			case 5:	// Fifth sample
			case 6:	// Sixth sample
			case 7:	// Seventh sample
				if(sample){
					_sr	<<= 1;
					if(_miso){
						_sr	|= 0x01;
						}
					++_state;
					}
				break;
			case 8:	// First sample
				if(sample){
					_sr	<<= 1;
					if(_miso){
						_sr	|= 0x01;
						}
					_state	= 1;

					streamsize	streamWidth = cout.width();
					ios_base::fmtflags	saved	= cout.flags();
					cout.setf(ios_base::hex,ios_base::basefield);
					cout.setf(ios_base::uppercase);
					cout.setf(ios_base::right);
					cout << "spisink: 0x";
					cout.width(2);
					cout.fill('0');
					cout << (unsigned long)_sr;
					cout << endl;
					cout.width(streamWidth);
					cout.flags(saved);
					}
				break;
			}
		break;
		}

	if((bool)_ss != _prevSS){
		if(_ss){
			cout << "spisink: /SS negated" << endl;
			}
		else {
			cout << "spisink: /SS asserted" << endl;
			}
		_prevSS	= _ss;
		}

	return 0;
	}

