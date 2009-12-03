#include <iostream>
#include "spisink.h"

using namespace std;

enum {
		SSBIT = 0,
		SCLKBIT = 1,
		MISOBIT = 2
		};

SpiSink::SpiSink(	Net&		ssNet,
					Net&		sclkNet,
					Net&		misoNet,
					bool		clockIsIdleHigh,
					bool		clockSampleOnLeadingEdge
					) throw():
		_port(0),
		_ss( &_port, (unsigned char)(1<<SSBIT) ),
		_sclk( &_port, (unsigned char)(1<<SCLKBIT) ),
		_miso( &_port, (unsigned char)(1<<MISOBIT) ),
		_ssState(false),
		_sclkState(false),
		_misoState(false),
		_state(0),
		_sr(0),
		_clockIsIdleHigh(clockIsIdleHigh),
		_clockSampleOnLeadingEdge(clockSampleOnLeadingEdge),
		_prevClkState(clockIsIdleHigh),
		_prevSS(true)
		{
		_ss.outState = Pin::PULLUP;
		ssNet.Add(&_ss);

		_sclk.outState = Pin::PULLUP;
		sclkNet.Add(&_sclk);

		_miso.outState = Pin::PULLUP;
		misoNet.Add(&_miso);
	}

int	SpiSink::Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns){
	*timeToNextStepIn_ns	= 1000;	// Once every microsecond
	bool	sample = false;

	_ssState	= (_port & (1<<SSBIT))?true:false;
	_sclkState	= (_port & (1<<SCLKBIT))?true:false;
	_misoState	= (_port & (1<<MISOBIT))?true:false;

	if(!_ssState){
		if(_prevClkState != _sclkState){
			_prevClkState	= _sclkState;
			if(_clockIsIdleHigh){
				// Clock is HIGH when idle
				if(_clockSampleOnLeadingEdge){
					// Sample on leading edge
					sample	= (_sclkState)?false:true;
					}
				else {
					// Sample on trailing edge
					sample	= (_sclkState)?true:false;
					}
				}
			else {
				// Clock is LOW when idle
				if(_clockSampleOnLeadingEdge){
					// Sample on leading edge
					sample	= (_sclkState)?true:false;
					}
				else {
					// Sample on trailing edge
					sample	= (_sclkState)?false:true;
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
				if(!_ssState){
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
					if(_misoState){
						_sr	|= 0x01;
						}
					++_state;
					}
				break;
			case 8:	// First sample
				if(sample){
					_sr	<<= 1;
					if(_misoState){
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

	if(_ssState != _prevSS){
		if(_ssState){
			cout << "spisink: /SS negated" << endl;
			}
		else {
			cout << "spisink: /SS asserted" << endl;
			}
		_prevSS	= _ssState;
		}

	return 0;
	}

