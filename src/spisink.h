#ifndef _spisinkh_
#define _spisinkh_
#include "avrdevice.h"

// This class monitors the /SS, SCLK, and MISO pin of the AVR and
// prints the results one byte at a time to stdout.
// PetrH: Implemented using simulated polling (each 1 us). TODO: Rewrite.
class SpiSink : public SimulationMember {
	private:
		unsigned char	_port;
		Pin				_ss;	// Output to AVR
		Pin				_sclk;	// Output to AVR
		Pin				_miso;	// Output to AVR
		bool			_ssState;
		bool			_sclkState;
		bool			_misoState;
		unsigned		_state;
		unsigned char	_sr;
		bool			_clockIsIdleHigh;
		bool			_clockSampleOnLeadingEdge;
		bool			_prevClkState;
		bool			_prevSS;
	public:
		SpiSink(	Net&		ssNet,
					Net&		sclkNet,
					Net&		misoNet,
					bool		clockIsIdleHigh	= true,
					bool		clockSampleOnLeadingEdge = true
					) throw();
	private:	// SimulationMember
        int	Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns=0);

	};

#endif
