#ifndef _adcpinh_
#define _adcpinh_
#include <fstream>
#include "avrdevice.h"

// This class allows the analog simulator access
// to the analogValue field of the pin and causes
// the Net to update (CalcNet). Note there is
// no dependency on the UserInterface class.
class AdcAnalogPin : public Pin {
	public:
		// Set the analog value and propagte through Net.
		inline void	setAnalogValue(int value) throw(){
			analogValue	= value;
    		connectedTo->CalcNet();
			}
	};

// The purpose of this class is to stimulate a pin
// with an analog pattern specified by a file.
// The file will contain an "analog sample value" on
// each line, along with a duration in nano-seconds
// that must elapse before the value is changed.
class AdcPin : public SimulationMember {
	private:
		AdcAnalogPin		_analogPin;	// Output to AVR

		// The analog input file.
		std::ifstream		_anaFile;

	public:
		AdcPin(	const char*		fileName,
				Net&			pinNet
				) throw();

	private:	// SimulationMember
        int	Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns=0);
	};

#endif
