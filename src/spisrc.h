#ifndef _spisrch_
#define _spisrch_
#include <fstream>
#include "avrdevice.h"

/** Reads stimuli from file and outputs data via SPI to nets provided to constructor.
Simulates SPI clock rate 10 kHz. */
class SpiSource : public SimulationMember {
	private:
		Pin				_ss;	// Output to AVR
		Pin				_sclk;	// Output to AVR
		Pin				_mosi;	// Output to AVR
		std::ifstream	_spiFile;
	public:
		SpiSource(	const char*	fileName,
					Net&		ssNet,
					Net&		sclkNet,
					Net&		mosiNet
					) throw();
	private:	// SimulationMember
        int	Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns=0);
	};


#endif
