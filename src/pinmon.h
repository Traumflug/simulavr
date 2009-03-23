#ifndef _pinmonh_
#define _pinmonh_
#include "avrdevice.h"
#include "pin.h"
#include "pinnotify.h"
#include "net.h"

// This class monitors a single pin and prints
// changes in the pin state to stdout.
class PinMonitor : public HasPinNotifyFunction {
	private:
		// This string printed as a prefix on stdout with each pin change.
		const char*	_pinDescStr;
		// String printed when the pin is HIGH.
		const char*	_pinHighStr;
		// String printed when the pin is LOW.
		const char*	_pinLowStr;
		// Previous state of pin since change callback doesn't *really*
		// mean "change"!
		bool		_prevState;
	public:
		PinMonitor(	AvrDevice&	avr,
					const char*	pinNameStr, // AVR pin name.  (e.g. "B1","C2", etc.)
					const char*	pinDescStr	= 0,
					const char*	pinHighStr	= 0,
					const char*	pinLowStr	= 0
					) throw();
	private:	// HasPinNotifyFunction
        void PinStateHasChanged(Pin*);
	};

#endif
