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
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  $Id$
 */

/* This has been adapted from the hello_vpi.c example in the Icarus Verilog Source
   Distribution. */

#include "config.h"
#if HAVE_IVERILOG_VPI_USER_H
#include  <iverilog/vpi_user.h>
#else
#include  <vpi_user.h>
#endif

#include "avrdevice.h"
#include "avrfactory.h"
#include "rwmem.h"
#include "pin.h"
#include "avrerror.h"
#include "cmd/dumpargs.h"
#include "systemclock.h"

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

#define VPI_UNPACKS(name)                   \
    {                               \
    vpiHandle name  = vpi_scan(argv);           \
    if (! name) {                       \
        vpi_printf("%s: " #name " parameter missing.\n", xx);\
        vpi_free_object(argv);              \
        return 0;                       \
    }                           \
    value.format = vpiStringVal;                \
    vpi_get_value(name, &value);                \
    }                               \
    std::string name = value.value.str;

#define VPI_UNPACKI(name)                   \
    {                               \
    vpiHandle name  = vpi_scan(argv);           \
    if (! name) {                       \
        vpi_printf("%s: " #name " parameter missing.\n", xx);\
        vpi_free_object(argv);              \
        return 0;                       \
    }                           \
    value.format = vpiIntVal;               \
    vpi_get_value(name, &value);                \
    }                               \
    int name = value.value.integer;

#define VPI_UNPACKT(name)                   \
    {                               \
    vpiHandle name  = vpi_scan(argv);           \
    if (! name) {                       \
        vpi_printf("%s: " #name " parameter missing.\n", xx);\
        vpi_free_object(argv);              \
        return 0;                       \
    }                           \
    value.format = vpiTimeVal;               \
    vpi_get_value(name, &value);                \
    }                               \
    t_vpi_time *name = value.value.time;

#define VPI_RETURN_INT(val)                     \
    value.format = vpiIntVal;                   \
    value.value.integer = (val);                \
    vpi_put_value(ch, &value, 0, vpiNoDelay);           \
    return 0;

#define AVR_HCHECK()                        \
    if (!checkHandle(handle)) {                 \
    vpi_printf("%s: Invalid handle parameter.\n", xx);  \
    return 0;                       \
    }

#define VPI_BEGIN()                     \
    s_vpi_value value;                      \
    vpiHandle ch    = vpi_handle(vpiSysTfCall, 0);      \
    vpiHandle argv  = vpi_iterate(vpiArgument, ch);

#define VPI_END()                       \
    vpi_free_object(argv);

#define VPI_REGISTER_TASK(name)                 \
    {                               \
    s_vpi_systf_data tf_data;               \
    tf_data.type      = vpiSysTask;             \
    tf_data.tfname    = "$" #name;              \
    tf_data.calltf    = name ## _tf;            \
    tf_data.compiletf = 0;                  \
    tf_data.sizetf    = 0;                  \
    tf_data.user_data = "$" #name;              \
    vpi_register_systf(&tf_data);               \
    }

#define VPI_REGISTER_FUNC(name)                 \
    {                               \
    s_vpi_systf_data tf_data;               \
    tf_data.type      = vpiSysFunc;             \
    tf_data.tfname    = "$" #name;              \
    tf_data.calltf    = name ## _tf;            \
    tf_data.compiletf = 0;                  \
    tf_data.sizetf    = 0;                  \
    tf_data.user_data = "$" #name;              \
    vpi_register_systf(&tf_data);               \
    }

/*!
  This function creates a new AVR core and `returns' a handle to it
  Usage from Verilog:

  $avr_create(device, progname) -> handle

  where
  handle is an integer handle by which the avr can be accessed in all other
  calls here
  device is the name of the AVR device to create
  progname is the path to the flash program elf binary
*/
static PLI_INT32 avr_create_tf(char *xx) {
    VPI_BEGIN();
    VPI_UNPACKS(device);
    VPI_UNPACKS(progname);
    VPI_END();

    AvrDevice* dev=AvrFactory::instance().makeDevice(device.c_str());
    devices.push_back(dev);
    
    dev->Load(progname.c_str());

    VPI_RETURN_INT(devices.size()-1);
}

/*!
  This function resets an existing AVR core. 
  Usage from Verilog:

  $avr_reset(handle)
*/
static PLI_INT32 avr_reset_tf(char *xx) {
    VPI_BEGIN();
    VPI_UNPACKI(handle);
    VPI_END();
    
    AVR_HCHECK();
    devices[handle]->Reset();
    return 0;
}

/*!
  This function destroys an existing AVR core. 
  Usage from Verilog:

  $avr_destroy(handle)
*/
static PLI_INT32 avr_destroy_tf(char *xx) {
    VPI_BEGIN();
    VPI_UNPACKI(handle);
    VPI_END();
    
    AVR_HCHECK();

    /* We may leak a bity of memory for the pointer in the vector,
       but... what the hell! */
    delete devices[handle];
    devices[handle]=0;
    return 0;
}

/*!
  This function ticks the clock of an AVR.
  Usage from Verilog:

  $avr_tick(handle)
*/
static PLI_INT32 avr_tick_tf(char *xx) {
    VPI_BEGIN();
    VPI_UNPACKI(handle);
    VPI_END();
    AVR_HCHECK();
    
    bool no_hw=false;
    devices[handle]->Step(no_hw);
    return 0;
}

/*!
  Set the time in the AVR system, in ns. Used for trace dumps etc.
  $avr_time(handle)
*/
static PLI_INT32 avr_set_time_tf(char *xx) {
    VPI_BEGIN();
    VPI_UNPACKT(time);
    VPI_END();

    // For some weird reason (bug?) the low part is the high longword, really...
    uint64_t tfull=(time->low)<<32+((uint64_t) time->high);
    SystemClock::Instance().SetCurrentTime(tfull);

    return 0;
}

/*!
  This function reads an AVR pin value.
  Usage from verilog:
  $avr_pin_get(handle, name) -> value
*/
static PLI_INT32 avr_get_pin_tf(char *xx) {
    VPI_BEGIN();
    VPI_UNPACKI(handle);
    VPI_UNPACKS(name);
    VPI_END();

    AVR_HCHECK();
    
    Pin *pin=devices[handle]->GetPin(name.c_str());
    int ret(pin->outState);

    VPI_RETURN_INT(ret);
}

/*!
  This function sets an AVR pin value.
  Usage from verilog:
  $avr_pin_set(handle, name, val)
*/
static PLI_INT32 avr_set_pin_tf(char *xx) {
    VPI_BEGIN();
    VPI_UNPACKI(handle);
    VPI_UNPACKS(name);
    VPI_UNPACKI(val);
    VPI_END();
    
    AVR_HCHECK();
    
    Pin *pin=devices[handle]->GetPin(name.c_str());

    /* FIXME: Simply exports AVR pin states to verilog. This
       may be considered a breach of abstractions.
       */
    pin->SetInState(Pin::T_Pinstate(val));
    return 0;
}

/*!
  This function reads the value of the program counter
  in the AVR.
  Usage from verilog:
  $avr_get_pc(handle) -> pc_value
*/
static PLI_INT32 avr_get_pc_tf(char *xx) {
    VPI_BEGIN();
    VPI_UNPACKI(handle);
    VPI_END();

    AVR_HCHECK();

    VPI_RETURN_INT(devices[handle]->cPC);
}

/*!
  This function reads the value of a RAM-readable
  location in the AVR (0..31 are the regs, followed by the IO-space etc).
  Usage from verilog:
  $avr_get_rw(handle, adr) -> val
  where
  adr is the adress to read
  and val is the returned value at that address
*/
static PLI_INT32 avr_get_rw_tf(char *xx) {
    VPI_BEGIN();
    VPI_UNPACKI(handle);
    VPI_UNPACKI(address);
    VPI_END();

    AVR_HCHECK();

    VPI_RETURN_INT(devices[handle]->GetRWMem(address));
}

/*!
  Counterpart to avr_get_rw_tf. It sets the value of a RAM-readable
  location in the AVR (0..31 are the regs, followed by the IO-space etc).
  Usage from verilog:
  $avr_set_rw(handle, adr, val)
  where
  adr is the adress to read
  and val is the value to set at that address
*/
static PLI_INT32 avr_set_rw_tf(char *xx) {
    VPI_BEGIN();
    VPI_UNPACKI(handle);
    VPI_UNPACKI(address);
    VPI_UNPACKI(val);
    VPI_END();

    AVR_HCHECK();

    devices[handle]->SetRWMem(address, val);
    return 0;
}

/*!
  Enable or disable tracing for all AVR core.
  TODO: Implement tracing per core?
  
  Usage from Verilog:

  $avr_trace(tracename)

  where
  tracename is the output file name for tracing. If it is the empty string,
  tracing will be disabled again and the file will be closed.
*/

static PLI_INT32 avr_trace_tf(char *xx) {
    VPI_BEGIN();
    VPI_UNPACKS(tracename);
    VPI_END();
    
    if (tracename.length()) {
    sysConHandler.SetTraceFile(tracename.c_str(), 1000000);
    for (size_t i=0; i < devices.size(); i++)
        devices[i]->trace_on=1;
    } else {
    sysConHandler.StopTrace();
    for (size_t i=0; i < devices.size(); i++)
        devices[i]->trace_on=0;
    }
    return 0;
}

/*! This allows to enable dumping of internal variables in the simulavrxx core
 * as a Value Change Dump file. The arguments given to this function are
 * exactly the same as when calling simulavrxx from the command line. */
static PLI_INT32 avr_dump_arg_tf(char *xx) {
    VPI_BEGIN();
    VPI_UNPACKI(handle);
    VPI_UNPACKS(dumparg);
    VPI_END();
    AVR_HCHECK();
    std::vector<std::string> dargs;
    dargs.push_back(dumparg);
    SetDumpTraceArgs(dargs, devices[handle]);
    return 0;
}

/*! Initialize all dumpers and start them. Has to be done before any clock
 * ticks on any AVR devices, if dumping is used! */
static PLI_INT32 avr_dump_start_tf(char *xx) {
    DumpManager::Instance()->start();
    return 0;
}

/*! Finishes all dumping processes at the end of the application. */
static PLI_INT32 avr_dump_stop_tf(char *xx) {
    DumpManager::Instance()->stopApplication();
    return 0;
}

static void register_tasks() {
    VPI_REGISTER_FUNC(avr_create);
    VPI_REGISTER_TASK(avr_reset);
    VPI_REGISTER_TASK(avr_destroy);
    VPI_REGISTER_TASK(avr_tick);
    VPI_REGISTER_TASK(avr_set_time);
    VPI_REGISTER_FUNC(avr_get_pin);
    VPI_REGISTER_TASK(avr_set_pin);
    VPI_REGISTER_FUNC(avr_get_pc);
    VPI_REGISTER_FUNC(avr_get_rw);
    VPI_REGISTER_TASK(avr_set_rw);
    VPI_REGISTER_TASK(avr_trace);
    VPI_REGISTER_TASK(avr_dump_arg);
    VPI_REGISTER_TASK(avr_dump_start);
    VPI_REGISTER_TASK(avr_dump_stop);
}

/* This is a table of register functions. This table is the external symbol
   that the simulator looks for when loading this .vpi module. */
void (*vlog_startup_routines[])() = {
    &register_tasks,
    0
};
