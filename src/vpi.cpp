/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2007 Onno Kortmann <onno@gmx.net>
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
/* This has been adapted from the hello_vpi.c example in the Icarus Verilog Source
   Distribution. */

#include  <vpi_user.h>
#include "avrdevice.h"
#include "avrfactory.h"

static std::vector<AvrDevice*> devices;

static bool checkHandle(int h) {
    if (h>=devices.size()) {
	vpi_printf("There has never been an AVR device with the handle %d.", h);
	vpi_control(vpiFinish, 1);
	return false;
    } else {
	if (!devices[h]) {
	    vpi_printf("The AVR with handle %d has already been destroyed.", h);
	    vpi_control(vpiFinish, 1);
	    return false;
	}
    }
    return true;
}

/*!
  This function creates a new AVR core and `returns' a handle to it
  Usage from Verilog:

  $avr_create(handle, device, progname)

  where
  handle is an integer handle by which the avr can be accessed in all other
  calls here
  devic is the name of the AVR device to create
  progname is the path to the flash program elf binary
*/
static PLI_INT32 avr_create_tf(char *xx) {
    s_vpi_value value;
    vpiHandle ch = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, ch);
    vpiHandle handle=vpi_scan(argv);
    vpiHandle _device=vpi_scan(argv);
    vpiHandle _progname=vpi_scan(argv);

    value.format = vpiStringVal;
    vpi_get_value(_device, &value);
    std::string device=value.value.str;

    value.format = vpiStringVal;
    vpi_get_value(_progname, &value);
    std::string progname=value.value.str;

    // FIXME: Better error handling than exit(...) here. Exceptions!
    AvrDevice* dev;
    devices.push_back(dev=AvrFactory::instance().makeDevice(device));
    
    /*
    if (device=="AT90S4433") {
	devices.push_back(dev=new AvrDevice_at90s4433());
    } else {
	vpi_printf("Invalid AVR device: %s\n", device.c_str());
	vpi_control(vpiFinish, 1);
	return 0;
    }
    if (!dev) {
	vpi_printf("Can't create backend AVR simulavrxx device.");
	vpi_control(vpiFinish, 1);
	return 0;
	}*/
    
    dev->Load(progname.c_str());
    
    value.format = vpiIntVal;
    value.value.integer = devices.size()-1;
    vpi_put_value(handle, &value, 0, vpiNoDelay);
}

/*!
  This function resets an existing AVR core. 
  Usage from Verilog:

  $avr_reset(handle)
*/
static PLI_INT32 avr_reset_tf(char *xx) {
    s_vpi_value value;
    vpiHandle ch = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, ch);
    vpiHandle handle=vpi_scan(argv);
    
    value.format = vpiIntVal;
    vpi_get_value(handle, &value);
    int h = value.value.integer;
    if (!checkHandle(h)) return 0;
    
    devices[h]->Reset();
    return 0;
}

/*!
  This function destroys an existing AVR core. 
  Usage from Verilog:

  $avr_destroy(handle)
*/
static PLI_INT32 avr_destroy_tf(char *xx) {
    s_vpi_value value;
    vpiHandle ch = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, ch);
    vpiHandle handle=vpi_scan(argv);
    
    value.format = vpiIntVal;
    vpi_get_value(handle, &value);
    int h = value.value.integer;
    if (!checkHandle(h)) return 0;
    
    /* We leak a bit of memory for the pointer in the vector,
       but... what the hell! */
    delete devices[h];
    devices[h]=0;
    return 0;
}

/*!
  This function ticks the clock of an AVR.
  Usage from Verilog:

  $avr_tick(handle)
*/
static PLI_INT32 avr_tick_tf(char *xx) {
    s_vpi_value value;
    vpiHandle ch = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, ch);
    vpiHandle handle=vpi_scan(argv);
    
    value.format = vpiIntVal;
    vpi_get_value(handle, &value);
    int h = value.value.integer;
    if (!checkHandle(h)) return 0;
    
    /* Lets do a HARDWARE step in the AVR core.
       uC stepping in opcode units does not seem to make any sense from this
       HDL view. But it looks like avrdevice does have no interest in this
       value at all? */
    bool no_hw=false;
    devices[h]->Step(no_hw); 
    return 0;
}

/*!
  This function reads an AVR pin value.
  Usage from verilog:
  $avr_pin_get(handle, name, value)
*/
static PLI_INT32 avr_get_pin_tf(char *xx) {
    s_vpi_value value;
    vpiHandle ch = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, ch);
    vpiHandle handle=vpi_scan(argv);
    vpiHandle _name=vpi_scan(argv);
    vpiHandle _value=vpi_scan(argv);
    value.format = vpiIntVal;
    vpi_get_value(handle, &value);
    int h = value.value.integer;
    if (!checkHandle(h)) return 0;

    value.format = vpiStringVal;
    vpi_get_value(_name, &value);
    std::string name=value.value.str;

    Pin *pin=devices[h]->GetPin(name.c_str());
    int ret(pin->outState);

    value.format = vpiIntVal;
    value.value.integer = ret;
    vpi_put_value(_value, &value, 0, vpiNoDelay);
    return 0;
}

/*!
  This function sets an AVR pin value.
  Usage from verilog:
  $avr_pin_set(handle, name, val)
*/
static PLI_INT32 avr_set_pin_tf(char *xx) {
    s_vpi_value value;
    vpiHandle ch = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, ch);
    vpiHandle handle=vpi_scan(argv);
    vpiHandle _name=vpi_scan(argv);
    vpiHandle _value=vpi_scan(argv);
    value.format = vpiIntVal;
    vpi_get_value(handle, &value);
    int h = value.value.integer;
    if (!checkHandle(h)) return 0;

    value.format = vpiStringVal;
    vpi_get_value(_name, &value);
    std::string name=value.value.str;

    Pin *pin=devices[h]->GetPin(name.c_str());

    value.format = vpiIntVal;
    vpi_get_value(_value, &value);
    int val=value.value.integer;
    /* FIXME: Simply exports AVR pin states to verilog. This
       a breach of abstractions. */
    pin->SetInState(Pin::T_Pinstate(val));
    return 0;
}

static void register_tasks() {
    {
	s_vpi_systf_data tf_data;
      
	tf_data.type      = vpiSysTask;
	tf_data.tfname    = "$avr_create";
	tf_data.calltf    = avr_create_tf;
	tf_data.compiletf = 0;
	tf_data.sizetf    = 0;
	vpi_register_systf(&tf_data);
    }
    {
	s_vpi_systf_data tf_data;
      
	tf_data.type      = vpiSysTask;
	tf_data.tfname    = "$avr_reset";
	tf_data.calltf    = avr_reset_tf;
	tf_data.compiletf = 0;
	tf_data.sizetf    = 0;
	vpi_register_systf(&tf_data);
    }
    {
	s_vpi_systf_data tf_data;
      
	tf_data.type      = vpiSysTask;
	tf_data.tfname    = "$avr_destroy";
	tf_data.calltf    = avr_destroy_tf;
	tf_data.compiletf = 0;
	tf_data.sizetf    = 0;
	vpi_register_systf(&tf_data);
    }

    {
	s_vpi_systf_data tf_data;
      
	tf_data.type      = vpiSysTask;
	tf_data.tfname    = "$avr_tick";
	tf_data.calltf    = avr_tick_tf;
	tf_data.compiletf = 0;
	tf_data.sizetf    = 0;
	vpi_register_systf(&tf_data);
    }

    {
	s_vpi_systf_data tf_data;
      
	tf_data.type      = vpiSysTask;
	tf_data.tfname    = "$avr_get_pin";
	tf_data.calltf    = avr_get_pin_tf;
	tf_data.compiletf = 0;
	tf_data.sizetf    = 0;
	vpi_register_systf(&tf_data);
    }

    {
	s_vpi_systf_data tf_data;
      
	tf_data.type      = vpiSysTask;
	tf_data.tfname    = "$avr_set_pin";
	tf_data.calltf    = avr_set_pin_tf;
	tf_data.compiletf = 0;
	tf_data.sizetf    = 0;
	vpi_register_systf(&tf_data);
    }
}

/* This is a table of register functions. This table is the external symbol
   that the simulator looks for when loading this .vpi module. */
void (*vlog_startup_routines[])() = {
    &register_tasks,
    0
};
