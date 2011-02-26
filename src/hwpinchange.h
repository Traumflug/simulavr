#ifndef _hwpinchangeh_
#define _hwpinchangeh_
#include "avrdevice.h"
#include "pin.h"
#include "pinnotify.h"
#include "net.h"
#include "rwmem.h"
#include "hardware.h"

class HWPcifrApi {
	public:
		virtual ~HWPcifrApi(){}
		virtual bool	getPcifr(unsigned bit) throw()=0;
		virtual void	setPcifr(unsigned bit) throw()=0;

	};

class HWPcicrApi {
	public:
		virtual ~HWPcicrApi(){}
		virtual bool	getPcicr(unsigned bit) throw()=0;
		virtual void	setPcicr(unsigned bit) throw()=0;

	};

class HWPcmskApi {
	public:
		virtual ~HWPcmskApi(){}
		virtual void			setPcmskMask(unsigned char val) throw()=0;
		virtual unsigned char	getPcmskMask() const throw()=0;

//		virtual bool	getPcmsk(unsigned bit) throw()=0;
//		virtual void	setPcmsk(unsigned bit) throw()=0;

	};

class HWPcmskPinApi {
	public:
		virtual ~HWPcmskPinApi(){}
		virtual void	pinChanged(unsigned bit) throw()=0;
	};

class HWPcirMaskApi {
	public: // HWPcirMaskApi
		virtual void			setPcifrMask(unsigned char val) throw()=0;
		virtual unsigned char	getPcifrMask() const throw()=0;

		virtual void			setPcicrMask(unsigned char val) throw()=0;
		virtual unsigned char	getPcicrMask() const throw()=0;
	};
		
/// This class is never used.
class HWPcir : public HWPcifrApi , public HWPcirMaskApi , public Hardware {
	private:
		unsigned char	_pcifr;
		unsigned char	_pcicr;
		HWIrqSystem&	_irqSystem;

		const unsigned	_vector0;
		const unsigned	_vector1;
		const unsigned	_vector2;
		const unsigned	_vector3;
		const unsigned	_vector4;
		const unsigned	_vector5;
		const unsigned	_vector6;
		const unsigned	_vector7;

	public:
		// Unimplemented vectors are set
		// to a value of all ones (i.e. ~0).
		HWPcir(	AvrDevice*		avr,
				HWIrqSystem&	irqSystem,
				unsigned		vector0 = ~0,
				unsigned		vector1	= ~0,
				unsigned		vector2	= ~0,
				unsigned		vector3	= ~0,
				unsigned		vector4	= ~0,
				unsigned		vector5	= ~0,
				unsigned		vector6	= ~0,
				unsigned		vector7	= ~0
				) throw();

	private:
		unsigned	convertBitToVector(unsigned bit) const throw();

	public: // HWPcifrApi
		bool			getPcifr(unsigned pcifrBit) throw();
		void			setPcifr(unsigned pcifrBit) throw();

	public: // HWPcirMaskApi
		void			setPcifrMask(unsigned char val) throw();
		unsigned char	getPcifrMask() throw();

		void			setPcicrMask(unsigned char val) throw();
		unsigned char	getPcicrMask() throw();

        
        IOReg<HWPcir>
            pcicr_reg,
            pcifr_reg;
        
	private:	// Hardware
        void Reset();
        void ClearIrqFlag(unsigned int vector);

	
	};

/// This class is never used.
class HWPcmsk : public HWPcmskApi , public HWPcmskPinApi {
	private:
		HWPcifrApi&			_pcifrApi;
		unsigned char		_pcmsk;
		const unsigned		_pcifrBit;

	public:
		// constructor
		HWPcmsk(
            AvrDevice *core,
            HWPcifrApi&	pcifrApi,
            unsigned	pcifrBit
            ) throw();

	public: // HWPcmskApi
		void			setPcmskMask(unsigned char val) throw();
		unsigned char	getPcmskMask() throw();

	public: // HWPcmskPinApi
		void			pinChanged(unsigned bit) throw();

        IOReg<HWPcmsk> pcmsk_reg;
	};

// This class monitors a single pin for changes
// an reports an interrupt if the pin chages.
/// This class is never used. Delete? (Pin-change interrupt is done by ExternalIRQPort.)
class PinChange : public HasPinNotifyFunction {
	private:
		Pin&				_pin;
		HWPcmskPinApi&		_pcmskPinApi;
		const unsigned		_pcmskBit;

		// Previous state of pin since change callback doesn't *really*
		// mean "change"!
		bool				_prevState;

	public:
		PinChange(	Pin&			pin,
					HWPcmskPinApi&	pcmskPinApi,
					unsigned		pcmskBit
					) throw();
        
        
	private:	// HasPinNotifyFunction
        void PinStateHasChanged(Pin*);
	};

#endif
