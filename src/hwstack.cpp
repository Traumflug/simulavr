 /*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph       
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 */

#include "hwstack.h"
#include "avrerror.h"
#include "avrmalloc.h"
#include "flash.h"
#include <assert.h>
#include <cstdio>  // NULL

using namespace std;

HWStack::HWStack(AvrDevice *c):
    core(c),
    m_ThreadList(*c)
{
    Reset();
}

void HWStack::Reset(void) {
    returnPointList.clear();
    stackPointer = 0;
    lowestStackPointer = 0;
}

void HWStack::CheckReturnPoints() {
    typedef multimap<unsigned long, Funktor *>::iterator I;
    pair<I,I> l = returnPointList.equal_range(stackPointer);
    
    for(I i = l.first; i != l.second; i++) {
        (*(i->second))(); //execute Funktor
        delete i->second; //and delete it
    }
    returnPointList.erase(l.first, l.second);
}

void HWStack::SetReturnPoint(unsigned long stackPointer, Funktor *f) {
    returnPointList.insert(make_pair(stackPointer, f));
}

HWStackSram::HWStackSram(AvrDevice *c, int bs, bool initRE):
    HWStack(c),
    TraceValueRegister(c, "STACK"),
    initRAMEND(initRE),
    sph_reg(this, "SPH",
            this, &HWStackSram::GetSph, &HWStackSram::SetSph),
    spl_reg(this, "SPL",
            this, &HWStackSram::GetSpl, &HWStackSram::SetSpl)
{
    stackCeil = 1 << bs;  // TODO: The number of bits is unable to acurately represent 0x460 ceiling of ATmega8: has 1024 B RAM (0x400) and 32+64 (0x60) registers.
    Reset();
}

void HWStackSram::Reset() {
    returnPointList.clear();
    if(initRAMEND)
        stackPointer = core->GetMemIRamSize() +
                       core->GetMemIOSize() +
                       core->GetMemRegisterSize() - 1;
    else
        stackPointer = 0;
    lowestStackPointer = stackPointer;
}

void HWStackSram::Push(unsigned char val) {
    core->SetRWMem(stackPointer, val);
    stackPointer--;
    stackPointer %= stackCeil;

    spl_reg.hardwareChange(stackPointer & 0x0000ff);
    sph_reg.hardwareChange((stackPointer & 0x00ff00)>>8);
    
    if(core->trace_on == 1)
        traceOut << "SP=0x" << hex << stackPointer << " 0x" << int(val) << dec << " ";
    m_ThreadList.OnPush();
    CheckReturnPoints();
    
    // measure stack usage, calculate lowest stack pointer
    if(lowestStackPointer > stackPointer)
        lowestStackPointer = stackPointer;
}

unsigned char HWStackSram::Pop() {
    stackPointer++;
    stackPointer %= stackCeil;

    spl_reg.hardwareChange(stackPointer & 0x0000ff);
    sph_reg.hardwareChange((stackPointer & 0x00ff00)>>8);
    
    if(core->trace_on == 1)
        traceOut << "SP=0x" << hex << stackPointer << " 0x" << int(core->GetRWMem(stackPointer)) << dec << " ";
    m_ThreadList.OnPop();
    CheckReturnPoints();
    return core->GetRWMem(stackPointer);
}

void HWStackSram::PushAddr(unsigned long addr) {
    // low byte first, then high byte
    Push(addr & 0xff);
    addr >>= 8;
    Push(addr & 0xff);
    if(core->PC_size == 3) {
        addr >>= 8;
        Push(addr & 0xff);
    }
}

unsigned long HWStackSram::PopAddr() {
    // high byte first, then low byte
    unsigned long val = Pop();
    val <<= 8;
    val += Pop();
    if(core->PC_size == 3) {
        val <<= 8;
        val += Pop();
    }
    return val;
}

void HWStackSram::SetSpl(unsigned char val) {
    uint32_t oldSP = stackPointer;
    stackPointer &= ~0xff;
    stackPointer += val;
    stackPointer %= stackCeil; // zero the not used bits

    spl_reg.hardwareChange(stackPointer & 0x0000ff);
    
    if(core->trace_on == 1)
        traceOut << "SP=0x" << hex << stackPointer << dec << " " ; 
    if(oldSP != stackPointer)
        m_ThreadList.OnSPWrite(stackPointer);
    CheckReturnPoints();
}

void HWStackSram::SetSph(unsigned char val) {
    uint32_t oldSP = stackPointer;
    if(stackCeil <= 0x100)
        avr_warning("assignment to non existent SPH (value=0x%x)", (unsigned int)val);
    stackPointer &= ~0xff00;
    stackPointer += val << 8;
    stackPointer %= stackCeil; // zero the not used bits

    sph_reg.hardwareChange((stackPointer & 0x00ff00)>>8);

    if(core->trace_on == 1)
        traceOut << "SP=0x" << hex << stackPointer << dec << " " ; 
    if(oldSP != stackPointer)
        m_ThreadList.OnSPWrite(stackPointer);
    CheckReturnPoints();
}

unsigned char HWStackSram::GetSph() {
    OnSPReadByTarget();
    return (stackPointer & 0xff00) >> 8;
}

unsigned char HWStackSram::GetSpl() {
    OnSPReadByTarget();
    return stackPointer & 0xff;
}
void HWStackSram::OnSPReadByTarget() {
    m_ThreadList.OnSPRead(stackPointer);
}

ThreeLevelStack::ThreeLevelStack(AvrDevice *c):
    HWStack(c),
    TraceValueRegister(c, "STACK") {
    stackArea = avr_new(unsigned long, 3);
    trace_direct(this, "PTR", &stackPointer);
    Reset();
}

ThreeLevelStack::~ThreeLevelStack() {
    avr_free(stackArea);
}

void ThreeLevelStack::Reset(void) {
    returnPointList.clear();
    stackPointer = 3;
    lowestStackPointer = stackPointer;
}

void ThreeLevelStack::Push(unsigned char val) {
    avr_error("Push method isn't available on TreeLevelStack");
}

unsigned char ThreeLevelStack::Pop() {
    avr_error("Pop method isn't available on TreeLevelStack");
    return 0;
}

void ThreeLevelStack::PushAddr(unsigned long addr) {
    stackArea[2] = stackArea[1];
    stackArea[1] = stackArea[0];
    stackArea[0] = addr;
    if(stackPointer > 0)
        stackPointer--;
    if(lowestStackPointer > stackPointer)
        lowestStackPointer = stackPointer;
    if(stackPointer == 0)
        avr_warning("stack overflow");
}

unsigned long ThreeLevelStack::PopAddr() {
    unsigned long val = stackArea[0];
    stackArea[0] = stackArea[1];
    stackArea[1] = stackArea[2];
    stackPointer++;
    if(stackPointer > 3) {
        stackPointer = 3;
        avr_warning("stack underflow");
    }
    return val;
}

ThreadList::ThreadList(AvrDevice & core)
	: m_core(core)
{
	m_phase_of_switch = eNormal;
	m_last_SP_read = 0x0000;
	m_last_SP_writen = 0x0000;
	m_cur_thread = 0;  // we are running main() thread

	Thread * main_thread = new Thread;
	main_thread->m_sp = 0;  // invalid address, GDB never sees it, updated on switch
	main_thread->m_ip = 0;
	main_thread->m_alive = true;
	m_threads.push_back(main_thread);
}
ThreadList::~ThreadList()
{
	OnReset();
}
void ThreadList::OnReset()
{
	for(unsigned int i = 0; i < m_threads.size(); i++) {
		Thread * p = m_threads[i];
		delete p;
	}
	m_threads.resize(0);
}

void ThreadList::OnCall()
{
	m_on_call_sp = m_core.stack->GetStackPointer();
	assert(m_on_call_sp != 0x0000);
	m_on_call_ip = m_core.PC * 2;
	Thread * old = m_threads[m_cur_thread];
	for(unsigned int i = 0; i < 32; i++) {
		old->registers[i] = m_core.GetCoreReg(i);
	}
}

void ThreadList::OnSPRead(int SP_value)
{
	assert(0 <= SP_value && SP_value <= 0xFFFF);
	assert(0 != SP_value);  // SP must not point to register area
	m_phase_of_switch = eReaded;
	m_last_SP_read = SP_value;
}

void ThreadList::OnSPWrite( int new_SP )
{
	if( ! m_core.Flash->LooksLikeContextSwitch(m_core.PC*2))
		return;
	m_phase_of_switch = (m_phase_of_switch==eWritten) ? eWritten2 : eWritten;
	m_last_SP_writen = new_SP;
}

void ThreadList::OnPush()
{
	m_phase_of_switch = eNormal;
	m_last_SP_read = 0x0000;
	m_last_SP_writen = 0x0000;
}

void ThreadList::OnPop()
{
	if(m_phase_of_switch != eWritten2)
	{
		m_phase_of_switch = eNormal;
		m_last_SP_read = 0x0000;
		m_last_SP_writen = 0x0000;
		return;
	}
	m_phase_of_switch = eNormal;
	int addr = m_core.PC * 2;
	assert(0 <= m_cur_thread && m_cur_thread < (int) m_threads.size());
	Thread * old = m_threads[m_cur_thread];
	assert(m_on_call_sp != 0x0000);
	old->m_sp = m_on_call_sp;
	old->m_ip = m_on_call_ip;
	old->m_alive = true; // does not on FreeRTOS's vPortYieldFromTick: (m_last_SP_read != 0x0000);

	int n = GetThreadBySP(m_last_SP_writen);
	if(n == -1) {
		m_threads.push_back( new Thread);
		n = m_threads.size() - 1;
	}
	Thread * new_thread = m_threads[n];
	new_thread->m_sp = 0x0000;  // invalid
	new_thread->m_ip = 0x0000;
	new_thread->m_alive = true;

    avr_message("Context switch at PC 0x%05x from thread %d to %d\n", addr, m_cur_thread, n);
    m_cur_thread = n;
}

int ThreadList::GetThreadBySP(int sp) const
{
	for(unsigned int i = 0; i < m_threads.size(); i++) {
		Thread * p = m_threads[i];
		if(p->m_sp == sp)
			return i;
	}
	return -1;  // not found
}

int ThreadList::GetCurrentThreadForGDB() const
{
	return m_cur_thread + 1;
}

const Thread * ThreadList::GetThreadFromGDB(int thread_id) const
{
	assert(thread_id >= 1);
	unsigned int index = thread_id - 1;
	assert(index < m_threads.size());
	return m_threads[index];
}
bool ThreadList::IsGDBThreadAlive(int thread_id) const
{
	assert(thread_id >= 1);
	unsigned int index = thread_id - 1;
	if(index >= m_threads.size())
		return false;

	Thread * p = m_threads[index];
	return p->m_alive;
}
unsigned int ThreadList::GetCount() const
{
	return m_threads.size();
}

/* EOF */
