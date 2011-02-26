#include <assert.h>
#include <iostream>
#include "hwpinchange.h"
#include "irqsystem.h"

using namespace std;

HWPcir::HWPcir(	AvrDevice*		avr,
				HWIrqSystem&	irqSystem,
				unsigned		vector0,
				unsigned		vector1,
				unsigned		vector2,
				unsigned		vector3,
				unsigned		vector4,
				unsigned		vector5,
				unsigned		vector6,
				unsigned		vector7
				) throw():
		Hardware(avr),
		_pcifr(0),
		_pcicr(0),
		_irqSystem(irqSystem),
		_vector0(vector0),
		_vector1(vector1),
		_vector2(vector2),
		_vector3(vector3),
		_vector4(vector4),
		_vector5(vector5),
		_vector6(vector6),
		_vector7(vector7),
        pcicr_reg(avr, "PINCHANGE.PCICR", this, &HWPcir::getPcicrMask,
                  &HWPcir::setPcicrMask),
        pcifr_reg(avr, "PINCHANGE.PCIFR", this, &HWPcir::getPcifrMask,
                  &HWPcir::setPcifrMask)
{
	assert(false);  // Unreachable. No code ever constructs this class.
	irqSystem.DebugVerifyInterruptVector(_vector0, this);
}

bool	HWPcir::getPcifr(unsigned pcifrBit) throw(){
	return _pcifr & (1<<pcifrBit);
	}

unsigned	HWPcir::convertBitToVector(unsigned bit) const throw(){
	unsigned vector = ~0;
	switch(bit){
		case 0:
			vector	= _vector0;
			break;
		case 1:
			vector	= _vector1;
			break;
		case 2:
			vector	= _vector2;
			break;
		case 3:
			vector	= _vector3;
			break;
		case 4:
			vector	= _vector4;
			break;
		case 5:
			vector	= _vector5;
			break;
		case 6:
			vector	= _vector6;
			break;
		case 7:
			vector	= _vector7;
			break;
		default:
			cerr << "HWPcir: invalid PCIFR bit specified.." << endl;
			break;
		};
	return vector;
	}

void	HWPcir::setPcifr(unsigned pcifrBit) throw(){
	if(_pcifr & (1<<pcifrBit)){
		// Already set/pending
		return;
		}
	// At this point we have a *new* pin-change interrupt.
	_pcifr	|= (1<<pcifrBit);

	unsigned vector	= convertBitToVector(pcifrBit);

	if(vector == (unsigned)~0){
		cerr << "HWPcir: Attempt to set invalid pin-change interrupt." << endl;
		return;
		}

	if(_pcicr & (1<<pcifrBit)){
		_irqSystem.SetIrqFlag(this,vector);
		}
	}

void	HWPcir::setPcifrMask(unsigned char val) throw(){
	// Only XOR bits that are set/pending
	val		&= _pcifr;

	// Clear the appropriate pending interrupts
	_pcifr ^= val;

	for(unsigned i=0;i<8;++i){
		if(val & (1<<i)){
			// We're trying to clear an interrupt
			// An interrupt is pending
			if(_pcicr & (1<<i)){
				// The interrupt is enabled and pending,
				// so tell the interrupt system
				unsigned vector	= convertBitToVector(i);
				_irqSystem.ClearIrqFlag(vector);
				}
			}
		}

	// Clear the bits in the register
	_pcifr ^= val;
	}

unsigned char	HWPcir::getPcifrMask() throw(){
	return _pcifr;
	}

void	HWPcir::setPcicrMask(unsigned char val) throw(){
	unsigned char	pcicrChanges	= _pcicr ^ val;
	for(unsigned i=0;i<8;++i){
		if(pcicrChanges & (1<<i)){
			// Bit changed
			if(val & (1<<i)){
				//  Bit is being enabled
				if(_pcifr & (1<<i)){
					// Change interrupt pending
					unsigned vector	= convertBitToVector(i);
					_irqSystem.SetIrqFlag(this,vector);
					}
				}
			else {
				// Bit is being disabled
				if(val & (1<<i)){
					// Change interrupt pending
					unsigned vector	= convertBitToVector(i);
					_irqSystem.ClearIrqFlag(vector);
					}
				}
			}
		}
	_pcicr	= val;
	}

unsigned char	HWPcir::getPcicrMask() throw(){
	return _pcicr;
	}

		
void HWPcir::Reset(){
	_pcicr	= 0;
	_pcifr	= 0;
	}

void HWPcir::ClearIrqFlag(unsigned int vector){
	if(vector == _vector0){
		_pcifr	&= ~(1<<0);
        _irqSystem.ClearIrqFlag(vector);
		return;
		}
	if(vector == _vector1){
		_pcifr	&= ~(1<<1);
        _irqSystem.ClearIrqFlag(vector);
		return;
		}
	if(vector == _vector2){
		_pcifr	&= ~(1<<2);
        _irqSystem.ClearIrqFlag(vector);
		return;
		}
	if(vector == _vector3){
		_pcifr	&= ~(1<<3);
        _irqSystem.ClearIrqFlag(vector);
		return;
		}
	if(vector == _vector4){
		_pcifr	&= ~(1<<4);
        _irqSystem.ClearIrqFlag(vector);
		return;
		}
	if(vector == _vector5){
		_pcifr	&= ~(1<<5);
        _irqSystem.ClearIrqFlag(vector);
		return;
		}
	if(vector == _vector6){
		_pcifr	&= ~(1<<6);
        _irqSystem.ClearIrqFlag(vector);
		return;
		}
	if(vector == _vector7){
		_pcifr	&= ~(1<<7);
        _irqSystem.ClearIrqFlag(vector);
		return;
		}
        cerr << "HWPcir: Attempt to clear non-existent irq vector";
	}

HWPcmsk::HWPcmsk(
    AvrDevice *core,
    HWPcifrApi&	pcifrApi,
    unsigned	pcifrBit) throw():
		_pcifrApi(pcifrApi),
		_pcmsk(0),
		_pcifrBit(pcifrBit),
        pcmsk_reg(core, "PINCHANGE.PCMSK",
                  this, &HWPcmsk::getPcmskMask,
                  &HWPcmsk::setPcmskMask)
		{
			assert(false);  // Unreachable. No code ever constructs this class.
	}

void	HWPcmsk::setPcmskMask(unsigned char val) throw(){
	_pcmsk	= val;
	}

unsigned char	HWPcmsk::getPcmskMask() throw(){
	return _pcmsk;
	}

void	HWPcmsk::pinChanged(unsigned bit) throw(){
	if(_pcmsk & (1<<bit)){
		_pcifrApi.setPcifr(_pcifrBit);
		}
	}

PinChange::PinChange(	Pin&				pin,
						HWPcmskPinApi&		pcmskPinApi,
						unsigned			pcmskBit
						) throw():
		_pin(pin),
		_pcmskPinApi(pcmskPinApi),
		_pcmskBit(pcmskBit),
		_prevState(true)
		{
	assert(false);  // Unreachable. No code ever constructs this class.
	pin.RegisterCallback(this);
	}

void PinChange::PinStateHasChanged(Pin* pin){
	bool	currentState	= (bool)*pin;
	if(currentState == _prevState){
		return;
		}

	_prevState	= currentState;

	_pcmskPinApi.pinChanged(_pcmskBit);
	}

