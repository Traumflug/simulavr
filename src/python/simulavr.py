# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _simulavr

def _swig_setattr(self,class_type,name,value):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    self.__dict__[name] = value

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


class SimulationMember(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, SimulationMember, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, SimulationMember, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C SimulationMember instance at %s>" % (self.this,)
    def Step(*args): return _simulavr.SimulationMember_Step(*args)
    def __del__(self, destroy=_simulavr.delete_SimulationMember):
        try:
            if self.thisown: destroy(self)
        except: pass

class SimulationMemberPtr(SimulationMember):
    def __init__(self, this):
        _swig_setattr(self, SimulationMember, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, SimulationMember, 'thisown', 0)
        _swig_setattr(self, SimulationMember,self.__class__,SimulationMember)
_simulavr.SimulationMember_swigregister(SimulationMemberPtr)

class ExternalType(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, ExternalType, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, ExternalType, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C ExternalType instance at %s>" % (self.this,)
    def SetNewValueFromUi(*args): return _simulavr.ExternalType_SetNewValueFromUi(*args)
    def __del__(self, destroy=_simulavr.delete_ExternalType):
        try:
            if self.thisown: destroy(self)
        except: pass

class ExternalTypePtr(ExternalType):
    def __init__(self, this):
        _swig_setattr(self, ExternalType, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, ExternalType, 'thisown', 0)
        _swig_setattr(self, ExternalType,self.__class__,ExternalType)
_simulavr.ExternalType_swigregister(ExternalTypePtr)

class sockbuf(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, sockbuf, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, sockbuf, name)
    def __repr__(self):
        return "<C sockbuf instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, sockbuf, 'this', _simulavr.new_sockbuf(*args))
        _swig_setattr(self, sockbuf, 'thisown', 1)
    def __del__(self, destroy=_simulavr.delete_sockbuf):
        try:
            if self.thisown: destroy(self)
        except: pass

class sockbufPtr(sockbuf):
    def __init__(self, this):
        _swig_setattr(self, sockbuf, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, sockbuf, 'thisown', 0)
        _swig_setattr(self, sockbuf,self.__class__,sockbuf)
_simulavr.sockbuf_swigregister(sockbufPtr)

class sockstream(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, sockstream, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, sockstream, name)
    def __repr__(self):
        return "<C sockstream instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, sockstream, 'this', _simulavr.new_sockstream(*args))
        _swig_setattr(self, sockstream, 'thisown', 1)
    def __del__(self, destroy=_simulavr.delete_sockstream):
        try:
            if self.thisown: destroy(self)
        except: pass

class sockstreamPtr(sockstream):
    def __init__(self, this):
        _swig_setattr(self, sockstream, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, sockstream, 'thisown', 0)
        _swig_setattr(self, sockstream,self.__class__,sockstream)
_simulavr.sockstream_swigregister(sockstreamPtr)

class Socket(sockstream):
    __swig_setmethods__ = {}
    for _s in [sockstream]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, Socket, name, value)
    __swig_getmethods__ = {}
    for _s in [sockstream]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, Socket, name)
    def __repr__(self):
        return "<C Socket instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, Socket, 'this', _simulavr.new_Socket(*args))
        _swig_setattr(self, Socket, 'thisown', 1)
    def __del__(self, destroy=_simulavr.delete_Socket):
        try:
            if self.thisown: destroy(self)
        except: pass
    def Read(*args): return _simulavr.Socket_Read(*args)
    def Poll(*args): return _simulavr.Socket_Poll(*args)
    def Write(*args): return _simulavr.Socket_Write(*args)

class SocketPtr(Socket):
    def __init__(self, this):
        _swig_setattr(self, Socket, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Socket, 'thisown', 0)
        _swig_setattr(self, Socket,self.__class__,Socket)
_simulavr.Socket_swigregister(SocketPtr)

class HasPinNotifyFunction(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, HasPinNotifyFunction, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, HasPinNotifyFunction, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C HasPinNotifyFunction instance at %s>" % (self.this,)
    def PinStateHasChanged(*args): return _simulavr.HasPinNotifyFunction_PinStateHasChanged(*args)
    def __del__(self, destroy=_simulavr.delete_HasPinNotifyFunction):
        try:
            if self.thisown: destroy(self)
        except: pass

class HasPinNotifyFunctionPtr(HasPinNotifyFunction):
    def __init__(self, this):
        _swig_setattr(self, HasPinNotifyFunction, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, HasPinNotifyFunction, 'thisown', 0)
        _swig_setattr(self, HasPinNotifyFunction,self.__class__,HasPinNotifyFunction)
_simulavr.HasPinNotifyFunction_swigregister(HasPinNotifyFunctionPtr)

class AvrDevice(SimulationMember):
    __swig_setmethods__ = {}
    for _s in [SimulationMember]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, AvrDevice, name, value)
    __swig_getmethods__ = {}
    for _s in [SimulationMember]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, AvrDevice, name)
    def __repr__(self):
        return "<C AvrDevice instance at %s>" % (self.this,)
    __swig_setmethods__["actualFilename"] = _simulavr.AvrDevice_actualFilename_set
    __swig_getmethods__["actualFilename"] = _simulavr.AvrDevice_actualFilename_get
    if _newclass:actualFilename = property(_simulavr.AvrDevice_actualFilename_get, _simulavr.AvrDevice_actualFilename_set)
    __swig_setmethods__["BP"] = _simulavr.AvrDevice_BP_set
    __swig_getmethods__["BP"] = _simulavr.AvrDevice_BP_get
    if _newclass:BP = property(_simulavr.AvrDevice_BP_get, _simulavr.AvrDevice_BP_set)
    __swig_setmethods__["PC"] = _simulavr.AvrDevice_PC_set
    __swig_getmethods__["PC"] = _simulavr.AvrDevice_PC_get
    if _newclass:PC = property(_simulavr.AvrDevice_PC_get, _simulavr.AvrDevice_PC_set)
    __swig_setmethods__["PC_size"] = _simulavr.AvrDevice_PC_size_set
    __swig_getmethods__["PC_size"] = _simulavr.AvrDevice_PC_size_get
    if _newclass:PC_size = property(_simulavr.AvrDevice_PC_size_get, _simulavr.AvrDevice_PC_size_set)
    __swig_setmethods__["Flash"] = _simulavr.AvrDevice_Flash_set
    __swig_getmethods__["Flash"] = _simulavr.AvrDevice_Flash_get
    if _newclass:Flash = property(_simulavr.AvrDevice_Flash_get, _simulavr.AvrDevice_Flash_set)
    __swig_setmethods__["eeprom"] = _simulavr.AvrDevice_eeprom_set
    __swig_getmethods__["eeprom"] = _simulavr.AvrDevice_eeprom_get
    if _newclass:eeprom = property(_simulavr.AvrDevice_eeprom_get, _simulavr.AvrDevice_eeprom_set)
    __swig_setmethods__["data"] = _simulavr.AvrDevice_data_set
    __swig_getmethods__["data"] = _simulavr.AvrDevice_data_get
    if _newclass:data = property(_simulavr.AvrDevice_data_get, _simulavr.AvrDevice_data_set)
    __swig_setmethods__["irqSystem"] = _simulavr.AvrDevice_irqSystem_set
    __swig_getmethods__["irqSystem"] = _simulavr.AvrDevice_irqSystem_get
    if _newclass:irqSystem = property(_simulavr.AvrDevice_irqSystem_get, _simulavr.AvrDevice_irqSystem_set)
    __swig_setmethods__["Sram"] = _simulavr.AvrDevice_Sram_set
    __swig_getmethods__["Sram"] = _simulavr.AvrDevice_Sram_get
    if _newclass:Sram = property(_simulavr.AvrDevice_Sram_get, _simulavr.AvrDevice_Sram_set)
    __swig_setmethods__["R"] = _simulavr.AvrDevice_R_set
    __swig_getmethods__["R"] = _simulavr.AvrDevice_R_get
    if _newclass:R = property(_simulavr.AvrDevice_R_get, _simulavr.AvrDevice_R_set)
    __swig_setmethods__["ioreg"] = _simulavr.AvrDevice_ioreg_set
    __swig_getmethods__["ioreg"] = _simulavr.AvrDevice_ioreg_get
    if _newclass:ioreg = property(_simulavr.AvrDevice_ioreg_get, _simulavr.AvrDevice_ioreg_set)
    __swig_setmethods__["rw"] = _simulavr.AvrDevice_rw_set
    __swig_getmethods__["rw"] = _simulavr.AvrDevice_rw_get
    if _newclass:rw = property(_simulavr.AvrDevice_rw_get, _simulavr.AvrDevice_rw_set)
    __swig_setmethods__["stack"] = _simulavr.AvrDevice_stack_set
    __swig_getmethods__["stack"] = _simulavr.AvrDevice_stack_get
    if _newclass:stack = property(_simulavr.AvrDevice_stack_get, _simulavr.AvrDevice_stack_set)
    __swig_setmethods__["status"] = _simulavr.AvrDevice_status_set
    __swig_getmethods__["status"] = _simulavr.AvrDevice_status_get
    if _newclass:status = property(_simulavr.AvrDevice_status_get, _simulavr.AvrDevice_status_set)
    __swig_setmethods__["wado"] = _simulavr.AvrDevice_wado_set
    __swig_getmethods__["wado"] = _simulavr.AvrDevice_wado_get
    if _newclass:wado = property(_simulavr.AvrDevice_wado_get, _simulavr.AvrDevice_wado_set)
    __swig_setmethods__["hwResetList"] = _simulavr.AvrDevice_hwResetList_set
    __swig_getmethods__["hwResetList"] = _simulavr.AvrDevice_hwResetList_get
    if _newclass:hwResetList = property(_simulavr.AvrDevice_hwResetList_get, _simulavr.AvrDevice_hwResetList_set)
    __swig_setmethods__["hwCycleList"] = _simulavr.AvrDevice_hwCycleList_set
    __swig_getmethods__["hwCycleList"] = _simulavr.AvrDevice_hwCycleList_get
    if _newclass:hwCycleList = property(_simulavr.AvrDevice_hwCycleList_get, _simulavr.AvrDevice_hwCycleList_set)
    def AddToResetList(*args): return _simulavr.AvrDevice_AddToResetList(*args)
    def AddToCycleList(*args): return _simulavr.AvrDevice_AddToCycleList(*args)
    def RemoveFromCycleList(*args): return _simulavr.AvrDevice_RemoveFromCycleList(*args)
    def Load(*args): return _simulavr.AvrDevice_Load(*args)
    def GetPin(*args): return _simulavr.AvrDevice_GetPin(*args)
    def __init__(self, *args):
        _swig_setattr(self, AvrDevice, 'this', _simulavr.new_AvrDevice(*args))
        _swig_setattr(self, AvrDevice, 'thisown', 1)
    def Step(*args): return _simulavr.AvrDevice_Step(*args)
    def Reset(*args): return _simulavr.AvrDevice_Reset(*args)
    def SetClockFreq(*args): return _simulavr.AvrDevice_SetClockFreq(*args)
    def GetClockFreq(*args): return _simulavr.AvrDevice_GetClockFreq(*args)
    def __del__(self, destroy=_simulavr.delete_AvrDevice):
        try:
            if self.thisown: destroy(self)
        except: pass
    def GetRampz(*args): return _simulavr.AvrDevice_GetRampz(*args)
    def SetRampz(*args): return _simulavr.AvrDevice_SetRampz(*args)
    def RegisterPin(*args): return _simulavr.AvrDevice_RegisterPin(*args)
    def DeleteAllBreakpoints(*args): return _simulavr.AvrDevice_DeleteAllBreakpoints(*args)
    def GetFname(*args): return _simulavr.AvrDevice_GetFname(*args)

class AvrDevicePtr(AvrDevice):
    def __init__(self, this):
        _swig_setattr(self, AvrDevice, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, AvrDevice, 'thisown', 0)
        _swig_setattr(self, AvrDevice,self.__class__,AvrDevice)
_simulavr.AvrDevice_swigregister(AvrDevicePtr)

class AvrDevice_at90s8515(AvrDevice):
    __swig_setmethods__ = {}
    for _s in [AvrDevice]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, AvrDevice_at90s8515, name, value)
    __swig_getmethods__ = {}
    for _s in [AvrDevice]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, AvrDevice_at90s8515, name)
    def __repr__(self):
        return "<C AvrDevice_at90s8515 instance at %s>" % (self.this,)
    def __del__(self, destroy=_simulavr.delete_AvrDevice_at90s8515):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_setmethods__["porta"] = _simulavr.AvrDevice_at90s8515_porta_set
    __swig_getmethods__["porta"] = _simulavr.AvrDevice_at90s8515_porta_get
    if _newclass:porta = property(_simulavr.AvrDevice_at90s8515_porta_get, _simulavr.AvrDevice_at90s8515_porta_set)
    __swig_setmethods__["portb"] = _simulavr.AvrDevice_at90s8515_portb_set
    __swig_getmethods__["portb"] = _simulavr.AvrDevice_at90s8515_portb_get
    if _newclass:portb = property(_simulavr.AvrDevice_at90s8515_portb_get, _simulavr.AvrDevice_at90s8515_portb_set)
    __swig_setmethods__["portc"] = _simulavr.AvrDevice_at90s8515_portc_set
    __swig_getmethods__["portc"] = _simulavr.AvrDevice_at90s8515_portc_get
    if _newclass:portc = property(_simulavr.AvrDevice_at90s8515_portc_get, _simulavr.AvrDevice_at90s8515_portc_set)
    __swig_setmethods__["portd"] = _simulavr.AvrDevice_at90s8515_portd_set
    __swig_getmethods__["portd"] = _simulavr.AvrDevice_at90s8515_portd_get
    if _newclass:portd = property(_simulavr.AvrDevice_at90s8515_portd_get, _simulavr.AvrDevice_at90s8515_portd_set)
    __swig_setmethods__["portx"] = _simulavr.AvrDevice_at90s8515_portx_set
    __swig_getmethods__["portx"] = _simulavr.AvrDevice_at90s8515_portx_get
    if _newclass:portx = property(_simulavr.AvrDevice_at90s8515_portx_get, _simulavr.AvrDevice_at90s8515_portx_set)
    __swig_setmethods__["spi"] = _simulavr.AvrDevice_at90s8515_spi_set
    __swig_getmethods__["spi"] = _simulavr.AvrDevice_at90s8515_spi_get
    if _newclass:spi = property(_simulavr.AvrDevice_at90s8515_spi_get, _simulavr.AvrDevice_at90s8515_spi_set)
    __swig_setmethods__["uart"] = _simulavr.AvrDevice_at90s8515_uart_set
    __swig_getmethods__["uart"] = _simulavr.AvrDevice_at90s8515_uart_get
    if _newclass:uart = property(_simulavr.AvrDevice_at90s8515_uart_get, _simulavr.AvrDevice_at90s8515_uart_set)
    __swig_setmethods__["acomp"] = _simulavr.AvrDevice_at90s8515_acomp_set
    __swig_getmethods__["acomp"] = _simulavr.AvrDevice_at90s8515_acomp_get
    if _newclass:acomp = property(_simulavr.AvrDevice_at90s8515_acomp_get, _simulavr.AvrDevice_at90s8515_acomp_set)
    __swig_setmethods__["prescaler"] = _simulavr.AvrDevice_at90s8515_prescaler_set
    __swig_getmethods__["prescaler"] = _simulavr.AvrDevice_at90s8515_prescaler_get
    if _newclass:prescaler = property(_simulavr.AvrDevice_at90s8515_prescaler_get, _simulavr.AvrDevice_at90s8515_prescaler_set)
    __swig_setmethods__["timer0"] = _simulavr.AvrDevice_at90s8515_timer0_set
    __swig_getmethods__["timer0"] = _simulavr.AvrDevice_at90s8515_timer0_get
    if _newclass:timer0 = property(_simulavr.AvrDevice_at90s8515_timer0_get, _simulavr.AvrDevice_at90s8515_timer0_set)
    __swig_setmethods__["timer1"] = _simulavr.AvrDevice_at90s8515_timer1_set
    __swig_getmethods__["timer1"] = _simulavr.AvrDevice_at90s8515_timer1_get
    if _newclass:timer1 = property(_simulavr.AvrDevice_at90s8515_timer1_get, _simulavr.AvrDevice_at90s8515_timer1_set)
    __swig_setmethods__["mcucr"] = _simulavr.AvrDevice_at90s8515_mcucr_set
    __swig_getmethods__["mcucr"] = _simulavr.AvrDevice_at90s8515_mcucr_get
    if _newclass:mcucr = property(_simulavr.AvrDevice_at90s8515_mcucr_get, _simulavr.AvrDevice_at90s8515_mcucr_set)
    __swig_setmethods__["extirq"] = _simulavr.AvrDevice_at90s8515_extirq_set
    __swig_getmethods__["extirq"] = _simulavr.AvrDevice_at90s8515_extirq_get
    if _newclass:extirq = property(_simulavr.AvrDevice_at90s8515_extirq_get, _simulavr.AvrDevice_at90s8515_extirq_set)
    __swig_setmethods__["timer01irq"] = _simulavr.AvrDevice_at90s8515_timer01irq_set
    __swig_getmethods__["timer01irq"] = _simulavr.AvrDevice_at90s8515_timer01irq_get
    if _newclass:timer01irq = property(_simulavr.AvrDevice_at90s8515_timer01irq_get, _simulavr.AvrDevice_at90s8515_timer01irq_set)
    def __init__(self, *args):
        _swig_setattr(self, AvrDevice_at90s8515, 'this', _simulavr.new_AvrDevice_at90s8515(*args))
        _swig_setattr(self, AvrDevice_at90s8515, 'thisown', 1)
    def GetRampz(*args): return _simulavr.AvrDevice_at90s8515_GetRampz(*args)
    def SetRampz(*args): return _simulavr.AvrDevice_at90s8515_SetRampz(*args)

class AvrDevice_at90s8515Ptr(AvrDevice_at90s8515):
    def __init__(self, this):
        _swig_setattr(self, AvrDevice_at90s8515, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, AvrDevice_at90s8515, 'thisown', 0)
        _swig_setattr(self, AvrDevice_at90s8515,self.__class__,AvrDevice_at90s8515)
_simulavr.AvrDevice_at90s8515_swigregister(AvrDevice_at90s8515Ptr)

class AvrDevice_atmega128(AvrDevice):
    __swig_setmethods__ = {}
    for _s in [AvrDevice]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, AvrDevice_atmega128, name, value)
    __swig_getmethods__ = {}
    for _s in [AvrDevice]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, AvrDevice_atmega128, name)
    def __repr__(self):
        return "<C AvrDevice_atmega128 instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, AvrDevice_atmega128, 'this', _simulavr.new_AvrDevice_atmega128(*args))
        _swig_setattr(self, AvrDevice_atmega128, 'thisown', 1)
    def __del__(self, destroy=_simulavr.delete_AvrDevice_atmega128):
        try:
            if self.thisown: destroy(self)
        except: pass
    def GetRampz(*args): return _simulavr.AvrDevice_atmega128_GetRampz(*args)
    def SetRampz(*args): return _simulavr.AvrDevice_atmega128_SetRampz(*args)

class AvrDevice_atmega128Ptr(AvrDevice_atmega128):
    def __init__(self, this):
        _swig_setattr(self, AvrDevice_atmega128, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, AvrDevice_atmega128, 'thisown', 0)
        _swig_setattr(self, AvrDevice_atmega128,self.__class__,AvrDevice_atmega128)
_simulavr.AvrDevice_atmega128_swigregister(AvrDevice_atmega128Ptr)

class AvrDevice_at90s8515special(AvrDevice_at90s8515):
    __swig_setmethods__ = {}
    for _s in [AvrDevice_at90s8515]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, AvrDevice_at90s8515special, name, value)
    __swig_getmethods__ = {}
    for _s in [AvrDevice_at90s8515]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, AvrDevice_at90s8515special, name)
    def __repr__(self):
        return "<C AvrDevice_at90s8515special instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, AvrDevice_at90s8515special, 'this', _simulavr.new_AvrDevice_at90s8515special(*args))
        _swig_setattr(self, AvrDevice_at90s8515special, 'thisown', 1)
    def __del__(self, destroy=_simulavr.delete_AvrDevice_at90s8515special):
        try:
            if self.thisown: destroy(self)
        except: pass
    def GetRampz(*args): return _simulavr.AvrDevice_at90s8515special_GetRampz(*args)
    def SetRampz(*args): return _simulavr.AvrDevice_at90s8515special_SetRampz(*args)

class AvrDevice_at90s8515specialPtr(AvrDevice_at90s8515special):
    def __init__(self, this):
        _swig_setattr(self, AvrDevice_at90s8515special, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, AvrDevice_at90s8515special, 'thisown', 0)
        _swig_setattr(self, AvrDevice_at90s8515special,self.__class__,AvrDevice_at90s8515special)
_simulavr.AvrDevice_at90s8515special_swigregister(AvrDevice_at90s8515specialPtr)

class AvrDevice_at90s4433(AvrDevice):
    __swig_setmethods__ = {}
    for _s in [AvrDevice]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, AvrDevice_at90s4433, name, value)
    __swig_getmethods__ = {}
    for _s in [AvrDevice]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, AvrDevice_at90s4433, name)
    def __repr__(self):
        return "<C AvrDevice_at90s4433 instance at %s>" % (self.this,)
    def __del__(self, destroy=_simulavr.delete_AvrDevice_at90s4433):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_setmethods__["portb"] = _simulavr.AvrDevice_at90s4433_portb_set
    __swig_getmethods__["portb"] = _simulavr.AvrDevice_at90s4433_portb_get
    if _newclass:portb = property(_simulavr.AvrDevice_at90s4433_portb_get, _simulavr.AvrDevice_at90s4433_portb_set)
    __swig_setmethods__["portc"] = _simulavr.AvrDevice_at90s4433_portc_set
    __swig_getmethods__["portc"] = _simulavr.AvrDevice_at90s4433_portc_get
    if _newclass:portc = property(_simulavr.AvrDevice_at90s4433_portc_get, _simulavr.AvrDevice_at90s4433_portc_set)
    __swig_setmethods__["portd"] = _simulavr.AvrDevice_at90s4433_portd_set
    __swig_getmethods__["portd"] = _simulavr.AvrDevice_at90s4433_portd_get
    if _newclass:portd = property(_simulavr.AvrDevice_at90s4433_portd_get, _simulavr.AvrDevice_at90s4433_portd_set)
    __swig_setmethods__["portx"] = _simulavr.AvrDevice_at90s4433_portx_set
    __swig_getmethods__["portx"] = _simulavr.AvrDevice_at90s4433_portx_get
    if _newclass:portx = property(_simulavr.AvrDevice_at90s4433_portx_get, _simulavr.AvrDevice_at90s4433_portx_set)
    __swig_setmethods__["porty"] = _simulavr.AvrDevice_at90s4433_porty_set
    __swig_getmethods__["porty"] = _simulavr.AvrDevice_at90s4433_porty_get
    if _newclass:porty = property(_simulavr.AvrDevice_at90s4433_porty_get, _simulavr.AvrDevice_at90s4433_porty_set)
    __swig_setmethods__["admux"] = _simulavr.AvrDevice_at90s4433_admux_set
    __swig_getmethods__["admux"] = _simulavr.AvrDevice_at90s4433_admux_get
    if _newclass:admux = property(_simulavr.AvrDevice_at90s4433_admux_get, _simulavr.AvrDevice_at90s4433_admux_set)
    __swig_setmethods__["ad"] = _simulavr.AvrDevice_at90s4433_ad_set
    __swig_getmethods__["ad"] = _simulavr.AvrDevice_at90s4433_ad_get
    if _newclass:ad = property(_simulavr.AvrDevice_at90s4433_ad_get, _simulavr.AvrDevice_at90s4433_ad_set)
    __swig_setmethods__["spi"] = _simulavr.AvrDevice_at90s4433_spi_set
    __swig_getmethods__["spi"] = _simulavr.AvrDevice_at90s4433_spi_get
    if _newclass:spi = property(_simulavr.AvrDevice_at90s4433_spi_get, _simulavr.AvrDevice_at90s4433_spi_set)
    __swig_setmethods__["uart"] = _simulavr.AvrDevice_at90s4433_uart_set
    __swig_getmethods__["uart"] = _simulavr.AvrDevice_at90s4433_uart_get
    if _newclass:uart = property(_simulavr.AvrDevice_at90s4433_uart_get, _simulavr.AvrDevice_at90s4433_uart_set)
    __swig_setmethods__["acomp"] = _simulavr.AvrDevice_at90s4433_acomp_set
    __swig_getmethods__["acomp"] = _simulavr.AvrDevice_at90s4433_acomp_get
    if _newclass:acomp = property(_simulavr.AvrDevice_at90s4433_acomp_get, _simulavr.AvrDevice_at90s4433_acomp_set)
    __swig_setmethods__["prescaler"] = _simulavr.AvrDevice_at90s4433_prescaler_set
    __swig_getmethods__["prescaler"] = _simulavr.AvrDevice_at90s4433_prescaler_get
    if _newclass:prescaler = property(_simulavr.AvrDevice_at90s4433_prescaler_get, _simulavr.AvrDevice_at90s4433_prescaler_set)
    __swig_setmethods__["timer0"] = _simulavr.AvrDevice_at90s4433_timer0_set
    __swig_getmethods__["timer0"] = _simulavr.AvrDevice_at90s4433_timer0_get
    if _newclass:timer0 = property(_simulavr.AvrDevice_at90s4433_timer0_get, _simulavr.AvrDevice_at90s4433_timer0_set)
    __swig_setmethods__["timer1"] = _simulavr.AvrDevice_at90s4433_timer1_set
    __swig_getmethods__["timer1"] = _simulavr.AvrDevice_at90s4433_timer1_get
    if _newclass:timer1 = property(_simulavr.AvrDevice_at90s4433_timer1_get, _simulavr.AvrDevice_at90s4433_timer1_set)
    __swig_setmethods__["mcucr"] = _simulavr.AvrDevice_at90s4433_mcucr_set
    __swig_getmethods__["mcucr"] = _simulavr.AvrDevice_at90s4433_mcucr_get
    if _newclass:mcucr = property(_simulavr.AvrDevice_at90s4433_mcucr_get, _simulavr.AvrDevice_at90s4433_mcucr_set)
    __swig_setmethods__["extirq"] = _simulavr.AvrDevice_at90s4433_extirq_set
    __swig_getmethods__["extirq"] = _simulavr.AvrDevice_at90s4433_extirq_get
    if _newclass:extirq = property(_simulavr.AvrDevice_at90s4433_extirq_get, _simulavr.AvrDevice_at90s4433_extirq_set)
    __swig_setmethods__["timer01irq"] = _simulavr.AvrDevice_at90s4433_timer01irq_set
    __swig_getmethods__["timer01irq"] = _simulavr.AvrDevice_at90s4433_timer01irq_get
    if _newclass:timer01irq = property(_simulavr.AvrDevice_at90s4433_timer01irq_get, _simulavr.AvrDevice_at90s4433_timer01irq_set)
    def __init__(self, *args):
        _swig_setattr(self, AvrDevice_at90s4433, 'this', _simulavr.new_AvrDevice_at90s4433(*args))
        _swig_setattr(self, AvrDevice_at90s4433, 'thisown', 1)
    def GetRampz(*args): return _simulavr.AvrDevice_at90s4433_GetRampz(*args)
    def SetRampz(*args): return _simulavr.AvrDevice_at90s4433_SetRampz(*args)

class AvrDevice_at90s4433Ptr(AvrDevice_at90s4433):
    def __init__(self, this):
        _swig_setattr(self, AvrDevice_at90s4433, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, AvrDevice_at90s4433, 'thisown', 0)
        _swig_setattr(self, AvrDevice_at90s4433,self.__class__,AvrDevice_at90s4433)
_simulavr.AvrDevice_at90s4433_swigregister(AvrDevice_at90s4433Ptr)

class SystemClock(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, SystemClock, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, SystemClock, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C SystemClock instance at %s>" % (self.this,)
    def Add(*args): return _simulavr.SystemClock_Add(*args)
    def AddAsyncMember(*args): return _simulavr.SystemClock_AddAsyncMember(*args)
    def Step(*args): return _simulavr.SystemClock_Step(*args)
    def IncrTime(*args): return _simulavr.SystemClock_IncrTime(*args)
    def GetCurrentTime(*args): return _simulavr.SystemClock_GetCurrentTime(*args)
    def Endless(*args): return _simulavr.SystemClock_Endless(*args)
    __swig_getmethods__["Instance"] = lambda x: _simulavr.SystemClock_Instance
    if _newclass:Instance = staticmethod(_simulavr.SystemClock_Instance)
    def Rescedule(*args): return _simulavr.SystemClock_Rescedule(*args)
    def __del__(self, destroy=_simulavr.delete_SystemClock):
        try:
            if self.thisown: destroy(self)
        except: pass

class SystemClockPtr(SystemClock):
    def __init__(self, this):
        _swig_setattr(self, SystemClock, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, SystemClock, 'thisown', 0)
        _swig_setattr(self, SystemClock,self.__class__,SystemClock)
_simulavr.SystemClock_swigregister(SystemClockPtr)

SystemClock_Instance = _simulavr.SystemClock_Instance

class UserInterface(SimulationMember,Socket,ExternalType):
    __swig_setmethods__ = {}
    for _s in [SimulationMember,Socket,ExternalType]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, UserInterface, name, value)
    __swig_getmethods__ = {}
    for _s in [SimulationMember,Socket,ExternalType]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, UserInterface, name)
    def __repr__(self):
        return "<C UserInterface instance at %s>" % (self.this,)
    def SetNewValueFromUi(*args): return _simulavr.UserInterface_SetNewValueFromUi(*args)
    def AddExternalType(*args): return _simulavr.UserInterface_AddExternalType(*args)
    def __init__(self, *args):
        _swig_setattr(self, UserInterface, 'this', _simulavr.new_UserInterface(*args))
        _swig_setattr(self, UserInterface, 'thisown', 1)
    def __del__(self, destroy=_simulavr.delete_UserInterface):
        try:
            if self.thisown: destroy(self)
        except: pass
    def SendUiNewState(*args): return _simulavr.UserInterface_SendUiNewState(*args)
    def Step(*args): return _simulavr.UserInterface_Step(*args)
    def SwitchUpdateOnOff(*args): return _simulavr.UserInterface_SwitchUpdateOnOff(*args)
    def Write(*args): return _simulavr.UserInterface_Write(*args)

class UserInterfacePtr(UserInterface):
    def __init__(self, this):
        _swig_setattr(self, UserInterface, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, UserInterface, 'thisown', 0)
        _swig_setattr(self, UserInterface,self.__class__,UserInterface)
_simulavr.UserInterface_swigregister(UserInterfacePtr)

class Hardware(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Hardware, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Hardware, name)
    def __repr__(self):
        return "<C Hardware instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, Hardware, 'this', _simulavr.new_Hardware(*args))
        _swig_setattr(self, Hardware, 'thisown', 1)
    def CpuCycle(*args): return _simulavr.Hardware_CpuCycle(*args)
    def Reset(*args): return _simulavr.Hardware_Reset(*args)
    def ClearIrqFlag(*args): return _simulavr.Hardware_ClearIrqFlag(*args)
    def __del__(self, destroy=_simulavr.delete_Hardware):
        try:
            if self.thisown: destroy(self)
        except: pass

class HardwarePtr(Hardware):
    def __init__(self, this):
        _swig_setattr(self, Hardware, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Hardware, 'thisown', 0)
        _swig_setattr(self, Hardware,self.__class__,Hardware)
_simulavr.Hardware_swigregister(HardwarePtr)

class Pin(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Pin, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Pin, name)
    def __repr__(self):
        return "<C Pin instance at %s>" % (self.this,)
    __swig_setmethods__["connectedTo"] = _simulavr.Pin_connectedTo_set
    __swig_getmethods__["connectedTo"] = _simulavr.Pin_connectedTo_get
    if _newclass:connectedTo = property(_simulavr.Pin_connectedTo_get, _simulavr.Pin_connectedTo_set)
    SHORTED = _simulavr.Pin_SHORTED
    HIGH = _simulavr.Pin_HIGH
    PULLUP = _simulavr.Pin_PULLUP
    TRISTATE = _simulavr.Pin_TRISTATE
    PULLDOWN = _simulavr.Pin_PULLDOWN
    LOW = _simulavr.Pin_LOW
    ANALOG = _simulavr.Pin_ANALOG
    ANALOG_SHORTED = _simulavr.Pin_ANALOG_SHORTED
    __swig_setmethods__["outState"] = _simulavr.Pin_outState_set
    __swig_getmethods__["outState"] = _simulavr.Pin_outState_get
    if _newclass:outState = property(_simulavr.Pin_outState_get, _simulavr.Pin_outState_set)
    __swig_setmethods__["notifyList"] = _simulavr.Pin_notifyList_set
    __swig_getmethods__["notifyList"] = _simulavr.Pin_notifyList_get
    if _newclass:notifyList = property(_simulavr.Pin_notifyList_get, _simulavr.Pin_notifyList_set)
    def SetOutState(*args): return _simulavr.Pin_SetOutState(*args)
    def SetInState(*args): return _simulavr.Pin_SetInState(*args)
    def __init__(self, *args):
        _swig_setattr(self, Pin, 'this', _simulavr.new_Pin(*args))
        _swig_setattr(self, Pin, 'thisown', 1)
    def RegisterNet(*args): return _simulavr.Pin_RegisterNet(*args)
    def GetPin(*args): return _simulavr.Pin_GetPin(*args)
    def __del__(self, destroy=_simulavr.delete_Pin):
        try:
            if self.thisown: destroy(self)
        except: pass
    def GetAnalog(*args): return _simulavr.Pin_GetAnalog(*args)
    def RegisterCallback(*args): return _simulavr.Pin_RegisterCallback(*args)

class PinPtr(Pin):
    def __init__(self, this):
        _swig_setattr(self, Pin, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Pin, 'thisown', 0)
        _swig_setattr(self, Pin,self.__class__,Pin)
_simulavr.Pin_swigregister(PinPtr)

class ExtPin(Pin,ExternalType):
    __swig_setmethods__ = {}
    for _s in [Pin,ExternalType]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, ExtPin, name, value)
    __swig_getmethods__ = {}
    for _s in [Pin,ExternalType]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, ExtPin, name)
    def __repr__(self):
        return "<C ExtPin instance at %s>" % (self.this,)
    def SetNewValueFromUi(*args): return _simulavr.ExtPin_SetNewValueFromUi(*args)
    def __init__(self, *args):
        _swig_setattr(self, ExtPin, 'this', _simulavr.new_ExtPin(*args))
        _swig_setattr(self, ExtPin, 'thisown', 1)
    def SetInState(*args): return _simulavr.ExtPin_SetInState(*args)
    def __del__(self, destroy=_simulavr.delete_ExtPin):
        try:
            if self.thisown: destroy(self)
        except: pass

class ExtPinPtr(ExtPin):
    def __init__(self, this):
        _swig_setattr(self, ExtPin, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, ExtPin, 'thisown', 0)
        _swig_setattr(self, ExtPin,self.__class__,ExtPin)
_simulavr.ExtPin_swigregister(ExtPinPtr)

class ExtAnalogPin(Pin,ExternalType):
    __swig_setmethods__ = {}
    for _s in [Pin,ExternalType]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, ExtAnalogPin, name, value)
    __swig_getmethods__ = {}
    for _s in [Pin,ExternalType]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, ExtAnalogPin, name)
    def __repr__(self):
        return "<C ExtAnalogPin instance at %s>" % (self.this,)
    def SetNewValueFromUi(*args): return _simulavr.ExtAnalogPin_SetNewValueFromUi(*args)
    def __init__(self, *args):
        _swig_setattr(self, ExtAnalogPin, 'this', _simulavr.new_ExtAnalogPin(*args))
        _swig_setattr(self, ExtAnalogPin, 'thisown', 1)
    def SetInState(*args): return _simulavr.ExtAnalogPin_SetInState(*args)
    def __del__(self, destroy=_simulavr.delete_ExtAnalogPin):
        try:
            if self.thisown: destroy(self)
        except: pass

class ExtAnalogPinPtr(ExtAnalogPin):
    def __init__(self, this):
        _swig_setattr(self, ExtAnalogPin, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, ExtAnalogPin, 'thisown', 0)
        _swig_setattr(self, ExtAnalogPin,self.__class__,ExtAnalogPin)
_simulavr.ExtAnalogPin_swigregister(ExtAnalogPinPtr)

class OpenDrain(Pin):
    __swig_setmethods__ = {}
    for _s in [Pin]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OpenDrain, name, value)
    __swig_getmethods__ = {}
    for _s in [Pin]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OpenDrain, name)
    def __repr__(self):
        return "<C OpenDrain instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, OpenDrain, 'this', _simulavr.new_OpenDrain(*args))
        _swig_setattr(self, OpenDrain, 'thisown', 1)
    def GetPin(*args): return _simulavr.OpenDrain_GetPin(*args)
    def RegisterNet(*args): return _simulavr.OpenDrain_RegisterNet(*args)
    def __del__(self, destroy=_simulavr.delete_OpenDrain):
        try:
            if self.thisown: destroy(self)
        except: pass

class OpenDrainPtr(OpenDrain):
    def __init__(self, this):
        _swig_setattr(self, OpenDrain, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, OpenDrain, 'thisown', 0)
        _swig_setattr(self, OpenDrain,self.__class__,OpenDrain)
_simulavr.OpenDrain_swigregister(OpenDrainPtr)

class NetInterface(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, NetInterface, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, NetInterface, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C NetInterface instance at %s>" % (self.this,)
    def CalcNet(*args): return _simulavr.NetInterface_CalcNet(*args)
    def Delete(*args): return _simulavr.NetInterface_Delete(*args)
    def __del__(self, destroy=_simulavr.delete_NetInterface):
        try:
            if self.thisown: destroy(self)
        except: pass

class NetInterfacePtr(NetInterface):
    def __init__(self, this):
        _swig_setattr(self, NetInterface, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, NetInterface, 'thisown', 0)
        _swig_setattr(self, NetInterface,self.__class__,NetInterface)
_simulavr.NetInterface_swigregister(NetInterfacePtr)

class Net(NetInterface):
    __swig_setmethods__ = {}
    for _s in [NetInterface]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, Net, name, value)
    __swig_getmethods__ = {}
    for _s in [NetInterface]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, Net, name)
    def __repr__(self):
        return "<C Net instance at %s>" % (self.this,)
    def Add(*args): return _simulavr.Net_Add(*args)
    def Delete(*args): return _simulavr.Net_Delete(*args)
    def CalcNet(*args): return _simulavr.Net_CalcNet(*args)
    def __del__(self, destroy=_simulavr.delete_Net):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __init__(self, *args):
        _swig_setattr(self, Net, 'this', _simulavr.new_Net(*args))
        _swig_setattr(self, Net, 'thisown', 1)

class NetPtr(Net):
    def __init__(self, this):
        _swig_setattr(self, Net, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Net, 'thisown', 0)
        _swig_setattr(self, Net,self.__class__,Net)
_simulavr.Net_swigregister(NetPtr)

class MirrorNet(NetInterface):
    __swig_setmethods__ = {}
    for _s in [NetInterface]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, MirrorNet, name, value)
    __swig_getmethods__ = {}
    for _s in [NetInterface]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, MirrorNet, name)
    def __repr__(self):
        return "<C MirrorNet instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, MirrorNet, 'this', _simulavr.new_MirrorNet(*args))
        _swig_setattr(self, MirrorNet, 'thisown', 1)
    def CalcNet(*args): return _simulavr.MirrorNet_CalcNet(*args)
    def Delete(*args): return _simulavr.MirrorNet_Delete(*args)
    def __del__(self, destroy=_simulavr.delete_MirrorNet):
        try:
            if self.thisown: destroy(self)
        except: pass

class MirrorNetPtr(MirrorNet):
    def __init__(self, this):
        _swig_setattr(self, MirrorNet, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, MirrorNet, 'thisown', 0)
        _swig_setattr(self, MirrorNet,self.__class__,MirrorNet)
_simulavr.MirrorNet_swigregister(MirrorNetPtr)

MAX_BUF = _simulavr.MAX_BUF
class GdbServer(SimulationMember):
    __swig_setmethods__ = {}
    for _s in [SimulationMember]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, GdbServer, name, value)
    __swig_getmethods__ = {}
    for _s in [SimulationMember]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, GdbServer, name)
    def __repr__(self):
        return "<C GdbServer instance at %s>" % (self.this,)
    def Step(*args): return _simulavr.GdbServer_Step(*args)
    def InternalStep(*args): return _simulavr.GdbServer_InternalStep(*args)
    def TryConnectGdb(*args): return _simulavr.GdbServer_TryConnectGdb(*args)
    def SendPosition(*args): return _simulavr.GdbServer_SendPosition(*args)
    def SleepStep(*args): return _simulavr.GdbServer_SleepStep(*args)
    def __init__(self, *args):
        _swig_setattr(self, GdbServer, 'this', _simulavr.new_GdbServer(*args))
        _swig_setattr(self, GdbServer, 'thisown', 1)
    def __del__(self, destroy=_simulavr.delete_GdbServer):
        try:
            if self.thisown: destroy(self)
        except: pass
    def Run(*args): return _simulavr.GdbServer_Run(*args)

class GdbServerPtr(GdbServer):
    def __init__(self, this):
        _swig_setattr(self, GdbServer, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, GdbServer, 'thisown', 0)
        _swig_setattr(self, GdbServer,self.__class__,GdbServer)
_simulavr.GdbServer_swigregister(GdbServerPtr)

class Lcd(SimulationMember):
    __swig_setmethods__ = {}
    for _s in [SimulationMember]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, Lcd, name, value)
    __swig_getmethods__ = {}
    for _s in [SimulationMember]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, Lcd, name)
    def __repr__(self):
        return "<C Lcd instance at %s>" % (self.this,)
    def Step(*args): return _simulavr.Lcd_Step(*args)
    def __init__(self, *args):
        _swig_setattr(self, Lcd, 'this', _simulavr.new_Lcd(*args))
        _swig_setattr(self, Lcd, 'thisown', 1)
    def __del__(self, destroy=_simulavr.delete_Lcd):
        try:
            if self.thisown: destroy(self)
        except: pass
    def GetPin(*args): return _simulavr.Lcd_GetPin(*args)

class LcdPtr(Lcd):
    def __init__(self, this):
        _swig_setattr(self, Lcd, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Lcd, 'thisown', 0)
        _swig_setattr(self, Lcd,self.__class__,Lcd)
_simulavr.Lcd_swigregister(LcdPtr)

class SerialRx(SimulationMember,ExternalType,HasPinNotifyFunction):
    __swig_setmethods__ = {}
    for _s in [SimulationMember,ExternalType,HasPinNotifyFunction]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, SerialRx, name, value)
    __swig_getmethods__ = {}
    for _s in [SimulationMember,ExternalType,HasPinNotifyFunction]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, SerialRx, name)
    def __repr__(self):
        return "<C SerialRx instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, SerialRx, 'this', _simulavr.new_SerialRx(*args))
        _swig_setattr(self, SerialRx, 'thisown', 1)
    def CpuCycle(*args): return _simulavr.SerialRx_CpuCycle(*args)
    def Reset(*args): return _simulavr.SerialRx_Reset(*args)
    def GetPin(*args): return _simulavr.SerialRx_GetPin(*args)
    def __del__(self, destroy=_simulavr.delete_SerialRx):
        try:
            if self.thisown: destroy(self)
        except: pass
    def Step(*args): return _simulavr.SerialRx_Step(*args)
    def SetNewValueFromUi(*args): return _simulavr.SerialRx_SetNewValueFromUi(*args)

class SerialRxPtr(SerialRx):
    def __init__(self, this):
        _swig_setattr(self, SerialRx, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, SerialRx, 'thisown', 0)
        _swig_setattr(self, SerialRx,self.__class__,SerialRx)
_simulavr.SerialRx_swigregister(SerialRxPtr)

class SerialTx(SimulationMember,ExternalType):
    __swig_setmethods__ = {}
    for _s in [SimulationMember,ExternalType]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, SerialTx, name, value)
    __swig_getmethods__ = {}
    for _s in [SimulationMember,ExternalType]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, SerialTx, name)
    def __repr__(self):
        return "<C SerialTx instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, SerialTx, 'this', _simulavr.new_SerialTx(*args))
        _swig_setattr(self, SerialTx, 'thisown', 1)
    def CpuCycle(*args): return _simulavr.SerialTx_CpuCycle(*args)
    def Reset(*args): return _simulavr.SerialTx_Reset(*args)
    def GetPin(*args): return _simulavr.SerialTx_GetPin(*args)
    def __del__(self, destroy=_simulavr.delete_SerialTx):
        try:
            if self.thisown: destroy(self)
        except: pass
    def Step(*args): return _simulavr.SerialTx_Step(*args)
    def SetNewValueFromUi(*args): return _simulavr.SerialTx_SetNewValueFromUi(*args)

class SerialTxPtr(SerialTx):
    def __init__(self, this):
        _swig_setattr(self, SerialTx, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, SerialTx, 'thisown', 0)
        _swig_setattr(self, SerialTx,self.__class__,SerialTx)
_simulavr.SerialTx_swigregister(SerialTxPtr)


StartTrace = _simulavr.StartTrace

GetSystemClock = _simulavr.GetSystemClock

