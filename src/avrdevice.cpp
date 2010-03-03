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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 *
 *  $Id$
 */

#include "config.h"
#include "bfd.h"

#include "avrdevice.h"
#include "traceval.h"
#include "helper.h"
#include "global.h"     //only 2 defines here... please move that sometimes womewhere TODO XXX
#include "irqsystem.h"  //GetNewPc
#include "systemclock.h"
#include "avrerror.h"
#include "avrmalloc.h"

#include "avrdevice_impl.h"

using namespace std;

void AvrDevice::AddToResetList(Hardware *hw) {
    if(find(hwResetList.begin(), hwResetList.end(), hw) == hwResetList.end())
        hwResetList.push_back(hw);
}

void AvrDevice::AddToCycleList(Hardware *hw) {
    if(find(hwCycleList.begin(), hwCycleList.end(), hw) == hwCycleList.end())
        hwCycleList.push_back(hw);
}
        
void AvrDevice::RemoveFromCycleList(Hardware *hw) {
    vector<Hardware*>::iterator element;
    element=find(hwCycleList.begin(), hwCycleList.end(), hw);
    if(element != hwCycleList.end())
        hwCycleList.erase(element);
}

void AvrDevice::Load(const char* fname) {
    actualFilename = fname;

    bfd *abfd;
    asection *sec;

    bfd_init();
    abfd=bfd_openr(fname, NULL);

    if(abfd == NULL)
        avr_error("Could not open file: %s", fname);

    if(bfd_check_format(abfd, bfd_object) == FALSE)
        avr_error("File '%s' isn't a elf object", fname);

    //reading out the symbols
    {
        long storage_needed;
        static asymbol **symbol_table;
        long number_of_symbols;
        long i;

        storage_needed = bfd_get_symtab_upper_bound(abfd);

        if(storage_needed < 0)
            avr_error("internal error: storage_needed < 0");

        if(storage_needed == 0)
            return;

        symbol_table = (asymbol **)malloc(storage_needed);

        number_of_symbols = bfd_canonicalize_symtab(abfd, symbol_table);

        if(number_of_symbols < 0)
            avr_error("internal error: number_of_symbols < 0");

        for(i = 0; i < number_of_symbols; i++) {
            // WAR: if no section data, skip
            if(!symbol_table[i]->section)
                continue;
            unsigned int lma = symbol_table[i]->section->lma;
            unsigned int vma = symbol_table[i]->section->vma;

            if(vma < 0x7fffff) { //range of flash space
                pair<unsigned int, string> p((symbol_table[i]->value+lma) >> 1, symbol_table[i]->name);
                //symbols.insert(p);
                Flash->AddSymbol(p);
            }
            else if(vma < 0x80ffff) { //range of ram 
                unsigned int offset = vma - 0x800000;
                //if( symbol_table[i]->flags & BSF_OBJECT) {
                {
                    pair<unsigned int, string> p(symbol_table[i]->value + offset, symbol_table[i]->name);
                    //symbolsData.insert(p);
                    data->AddSymbol(p);  //not a real data container, only holding symbols!

                    pair<unsigned int, string> pp(symbol_table[i]->value + lma, symbol_table[i]->name);
                    //symbols.insert(pp);
                    Flash->AddSymbol(pp);
                }
            }
            else if(vma < 0x81ffff) { //range of eeprom
                unsigned int offset = vma - 0x810000;
                pair<unsigned int, string> p(symbol_table[i]->value + offset, symbol_table[i]->name);
                //symbolsEeprom.insert(p);
                eeprom->AddSymbol(p);
            }
            else
                avr_warning("Unknown symbol address range found!");
        }
    }

    sec = abfd->sections;

    while(sec != 0)  { 
        if(sec->flags & SEC_LOAD && sec->vma < 0x80ffff) { //only read flash bytes and data
            int size;
            size = sec->size;
            unsigned char *tmp = (unsigned char *)malloc(size);
            bfd_get_section_contents(abfd, sec, tmp, 0, size);
            Flash->WriteMem(tmp, sec->lma, size);
            free(tmp);
        }

        if(sec->flags & SEC_LOAD && sec->vma >= 0x810000) {
            int size;
            size = sec->size;
            unsigned char *tmp = (unsigned char *)malloc(size);
            bfd_get_section_contents(abfd, sec, tmp, 0, size);
            unsigned int offset = sec->vma - 0x810000;
            eeprom->WriteMem(tmp, offset, size);
            free(tmp);
        }
        sec = sec->next;
    }

    bfd_close(abfd);
}

#ifdef VI_BUG
}
#endif

void AvrDevice::SetClockFreq(SystemClockOffset f) {
    clockFreq=f;
}

SystemClockOffset AvrDevice::GetClockFreq() {
    return clockFreq;
}

Pin *AvrDevice::GetPin(const char *name) {
    Pin *ret = allPins[name];
    if(!ret)
        avr_error("unknown Pin requested! -> %s is not available", name);
    return ret;
}

AvrDevice::~AvrDevice() {
    // unregister device on DumpManager
    dump_manager->unregisterAvrDevice(this);
    
    // delete invalid RW memory cells on shadow store + shadow store self
    unsigned size = totalIoSpace - registerSpaceSize - iRamSize - eRamSize;
    for(unsigned idx = 0; idx < size; idx++)
        delete invalidRW[idx];
    avr_free(invalidRW);
    
    // delete Ram cells and registers
    for(unsigned idx = 0; idx < registerSpaceSize; idx++)
        delete rw[idx];
    size = registerSpaceSize + ioSpaceSize + iRamSize + eRamSize;
    for(unsigned idx = (registerSpaceSize + ioSpaceSize); idx < size; idx++)
        delete rw[idx];
    
    // delete rw and other allocated objects
    delete Flash;
    delete Sram;
    delete ioreg;
    delete R;
    delete statusRegister;
    delete status;
    avr_free(rw);
    delete data;
}

/*! To ease debugging, also supply the option to have the PC*2 in the trace
 * output file. This is also the format the other normal tracing will output
 * addresses and the format avr-objdump produces disassemblies in. */
class TwiceTV : public TraceValue {
public:
    TwiceTV(const std::string &_name, TraceValue *_ref)
        : TraceValue(_ref->bits()+1, _name), ref(_ref) {}
    
    virtual void cycle() {
        change(ref->value()*2);
        set_written();
    }
private:
    TraceValue *ref; // Reference value that will be doubled
};

    
AvrDevice::AvrDevice(unsigned int _ioSpaceSize,
                     unsigned int IRamSize,
                     unsigned int ERamSize,
                     unsigned int flashSize):
    TraceValueRegister(),
    coreTraceGroup(this),
    abortOnInvalidAccess(false),
    totalIoSpace(0x10000),
    registerSpaceSize(32),
    iRamSize(IRamSize),
    eRamSize(ERamSize),
    ioSpaceSize(_ioSpaceSize),
    flagIWInstructions(true),
    flagJMPInstructions(true),
    flagTiny10(false),
    flagXMega(false)
{
    dump_manager = DumpManager::Instance();
    dump_manager->registerAvrDevice(this);
    
    TraceValue* pc_tracer=trace_direct(&coreTraceGroup, "PC", &cPC);
    coreTraceGroup.RegisterTraceValue(new TwiceTV(coreTraceGroup.GetTraceValuePrefix()+"PCb",  pc_tracer));
    
    data = new Data; //only the symbol container

    // placeholder for RAMPZ register
    rampz = NULL;
    
    // memory space for all RW-Memory addresses + shadow store for invalid cells
    unsigned invalidSize = totalIoSpace - registerSpaceSize - IRamSize - ERamSize; 
    rw = (RWMemoryMember**)avr_malloc(sizeof(RWMemoryMember*) * totalIoSpace);
    invalidRW = (RWMemoryMember**)avr_malloc(sizeof(RWMemoryMember*) * invalidSize);
    
    // the status register is generic to all devices
    status = new HWSreg();
    if(status == NULL)
        avr_error("Not enough memory for HWSreg in AvrDevice::AvrDevice");
    statusRegister = new RWSreg(&coreTraceGroup, status);
    if(statusRegister == NULL)
        avr_error("Not enough memory for RWSreg in AvrDevice::AvrDevice");
    
    // the Registers also generic to all avr devices
    R = new MemoryOffsets(0, rw);
    if(R == NULL)
        avr_error("Not enough memory for Register [R] in AvrDevice::AvrDevice");

    // offset for the io-space is 0x20 for all avr devices (behind the register file)
    ioreg = new MemoryOffsets(0x20, rw);
    if(ioreg == NULL)
        avr_error("Not enough memory for IoReg in AvrDevice::AvrDevice");

    // the offset for accessing the sram is allways at 0x00 so we can read with
    // lds also the register file!
    Sram = new MemoryOffsets(0x00, rw);
    if(Sram == NULL)
        avr_error("Not enough memory for Sram in AvrDevice::AvrDevice");

    // placeholder for SPM register
    spmRegister = NULL;
    
    // create the flash area with specified size
    Flash = new AvrFlash(this, flashSize);
    if(Flash == NULL)
        avr_error("Not enough memory for Flash in AvrDevice::AvrDevice");

    // create all registers
    unsigned currentOffset = 0;
    unsigned invalidRWOffset = 0;

    for(unsigned ii = 0; ii < registerSpaceSize; ii++) {
        rw[currentOffset] = new RAM(&coreTraceGroup, "r", ii, registerSpaceSize);
        if(rw[currentOffset] == NULL)
            avr_error("Not enough memory for registers in AvrDevice::AvrDevice");
        currentOffset++;
    }      

    /* Create invalid registers in I/O space which will fail on access (to
       make simulavrxx more robust!)  In all well implemented devices, these
       should be overwritten by the particular device type. But accessing such
       a register will at least notify the user that there is an unimplemented
       feature or reserved register. */
    for(unsigned ii = 0; ii < ioSpaceSize; ii++) {
        invalidRW[invalidRWOffset] = new InvalidMem(this, currentOffset);
        if(invalidRW[invalidRWOffset] == NULL)
            avr_error("Not enough memory for io space in AvrDevice::AvrDevice");
        rw[currentOffset] = invalidRW[invalidRWOffset];
        currentOffset++;
        invalidRWOffset++;
    }

    // create the internal ram handlers 
    for(unsigned ii = 0; ii < IRamSize; ii++ ) {
        rw[currentOffset] = new RAM(&coreTraceGroup, "IRAM", ii, IRamSize);
        if(rw[currentOffset] == NULL)
            avr_error("Not enough memory for IRAM in AvrDevice::AvrDevice");
        currentOffset++;
    }

    // create the external ram handlers, TODO: make the configuration from
    // mcucr available here
    for(unsigned ii = 0; ii < ERamSize; ii++ ) {
        rw[currentOffset] = new RAM(&coreTraceGroup, "ERAM", ii, ERamSize);
        if(rw[currentOffset] == NULL)
            avr_error("Not enough memory for io space in AvrDevice::AvrDevice");
        currentOffset++;
    }

    // fill the rest of the adress space with error handlers
    for(; currentOffset < totalIoSpace; currentOffset++, invalidRWOffset++) {
        invalidRW[invalidRWOffset] = new InvalidMem(this, currentOffset);
        if(invalidRW[invalidRWOffset] == NULL)
            avr_error("Not enough memory for fill address space in AvrDevice::AvrDevice");
        rw[currentOffset] = invalidRW[invalidRWOffset];
    }

}

//do a single core step, (0)->a real hardware step, (1) until the uC finish the opcode! 
int AvrDevice::Step(bool &untilCoreStepFinished, SystemClockOffset *nextStepIn_ns) {
    int bpFlag = 0;
    int hwWait = 0;

    //    do {

    if (cpuCycles<=0)
        cPC=PC;
    if(trace_on == 1) {
        traceOut << actualFilename << " ";
        traceOut << HexShort(cPC << 1) << dec << ": ";

        string sym(Flash->GetSymbolAtAddress(cPC));
        traceOut << sym << " ";
        for (int len = sym.length(); len < 30;len++)
            traceOut << " " ;
    }
    
    hwWait = 0;
    vector<Hardware *>::iterator ii;
    vector<Hardware *>::iterator end;
    end = hwCycleList.end();

    for(ii = hwCycleList.begin(); ii != end; ii++) {
        if (((*ii)->CpuCycle()) > 0)
            hwWait = 1;
    }

    if(cpuCycles <= 0) {

        if(hwWait != 0) {
            if(trace_on)
                traceOut << "CPU-Hold by IO-Hardware ";
        } else {
            //check for enabled breakpoints here

            if(BP.end() != find(BP.begin(), BP.end(), PC)) {
                if(trace_on)
                    traceOut << "Breakpoint found at 0x" << hex << PC << dec << endl;
                bpFlag = BREAK_POINT;
                if(nextStepIn_ns != 0)
                    *nextStepIn_ns=clockFreq;
                untilCoreStepFinished = !((cpuCycles > 0) || (hwWait > 0));
                dump_manager->cycle();
                return bpFlag;
            }

            if(EP.end() != find(EP.begin(), EP.end(), PC)) {
                if(global_verbose_on)
                    cout << "Simulation finished!" << endl;
                SystemClock::Instance().stop();
                dump_manager->cycle();
                return bpFlag;
            }

            if(newIrqPc != 0xffffffff) {
                if(noDirectIrqJump == 0) {
                    if(trace_on)
                        traceOut << "IRQ DETECTED: VectorAddr: " << newIrqPc ;

                    irqSystem->IrqHandlerStarted(actualIrqVector);    //what vector we raise?
                    //Funktor* fkt=new IrqFunktor(irqSystem, &HWIrqSystem::IrqHandlerFinished, actualIrqVector);
                    stack->SetReturnPoint(stack->GetStackPointer(),
                                          IrqFunktor(irqSystem, &HWIrqSystem::IrqHandlerFinished, actualIrqVector).clone());

                    //pushing the stack
                    unsigned long val = PC;
                    stack->PushAddr(val);
                    cpuCycles = 4; //push needs 4 cycles! (on external RAM +2, this is handled from HWExtRam!)
                    status->I = 0; //irq started so remove I-Flag from SREG

                    PC = newIrqPc - 1;   //we add a few lines later 1 so we sub here 1 :-)
                    newIrqPc = 0xffffffff;
                } else {
                    noDirectIrqJump = 0;
                }
            }

            if(cpuCycles <= 0) {
                if((unsigned int)(PC << 1) >= (unsigned int)Flash->GetSize() ) {
                    ostringstream os;
                    os << actualFilename << " Simulation runs out of Flash Space at " << hex << (PC << 1);
                    string s = os.str();
                    if(trace_on)
                        traceOut << s << endl;
                    avr_error(s.c_str());
                }

                DecodedInstruction *de = (Flash->GetInstruction(PC));
                if(trace_on) {
                    cpuCycles = de->Trace();
                } else {
                    cpuCycles = (*de)(); 
                }
                // report changes on status
                statusRegister->trigger_change();
                
                // cpuCycles=(*(Flash->DecodedMem[PC]))();
            }

            if(cpuCycles < 0)
                bpFlag = cpuCycles;
            if(bpFlag != BREAK_POINT)
                PC++;

            if(((status->I) == 1) && (newIrqPc == 0xffffffff)) {
                newIrqPc = irqSystem->GetNewPc(actualIrqVector); //If any interrupt is pending get new PC 
                noDirectIrqJump = 1;
            }

        }
    } else { //cpuCycles>0
        if(trace_on == 1)
            traceOut << "CPU-waitstate";
    }

    if(nextStepIn_ns != 0)
        *nextStepIn_ns = clockFreq;

    cpuCycles--;
    if(trace_on == 1) {
        traceOut << endl;
        sysConHandler.TraceNextLine();
    }

    //if (untilCoreStepFinished == false) { //we wait not until end so reply the finish state
    untilCoreStepFinished = !((cpuCycles > 0) || (hwWait > 0));
    dump_manager->cycle();
    return bpFlag;
    //}
    //} while ( untilCoreStepFinished && ((cpuCycles>0) || (hwWait>0)));

    //return bpFlag;
}

void AvrDevice::Reset() {
    PC_size = 2;
    PC = 0;

    vector<Hardware *>::iterator ii;
    for(ii= hwResetList.begin(); ii != hwResetList.end(); ii++)
        (*ii)->Reset();

    PC = 0; cPC=0;
    *status = 0;

    //init the old static vars from Step()
    cpuCycles = 0;
    newIrqPc = 0xffffffff;
    noDirectIrqJump = 0;
}

void AvrDevice::DeleteAllBreakpoints() {
    BP.erase(BP.begin(), BP.end());
}

void AvrDevice::ReplaceIoRegister(unsigned int offset, RWMemoryMember *newMember) {
    if (offset >= ioSpaceSize + ioreg->getOffset())
        avr_error("Could not replace register in non existing IoRegisterSpace");
    rw[offset] = newMember;
}

bool AvrDevice::ReplaceMemRegister(unsigned int offset, RWMemoryMember *newMember) {
    if(offset < totalIoSpace) {
        rw[offset] = newMember;
        return true;
    }
    return false;
}

RWMemoryMember* AvrDevice::GetMemRegisterInstance(unsigned int offset) {
    if(offset < totalIoSpace)
        return rw[offset];
    return NULL;
}

void AvrDevice::RegisterTerminationSymbol(const char *symbol) {
    unsigned int epa = Flash->GetAddressAtSymbol(symbol);
    EP.push_back(epa);
}

unsigned char AvrDevice::GetRWMem(unsigned addr) {
    if(addr >= GetMemTotalSize())
        return 0;
    return *(rw[addr]);
}

bool AvrDevice::SetRWMem(unsigned addr, unsigned char val) {
    if(addr >= GetMemTotalSize())
        return false;
    *(rw[addr]) = val;
    return true;
}

unsigned char AvrDevice::GetCoreReg(unsigned addr) {
    if(addr >= registerSpaceSize)
        return 0;
    return *(rw[addr]);
}

bool AvrDevice::SetCoreReg(unsigned addr, unsigned char val) {
    if(addr >= registerSpaceSize)
        return false;
    *(rw[addr]) = val;
    return true;
}

unsigned char AvrDevice::GetIOReg(unsigned addr) {
    if(addr >= ioSpaceSize)
        return 0;
    return *(rw[addr + registerSpaceSize]);
}

bool AvrDevice::SetIOReg(unsigned addr, unsigned char val) {
    if(addr >= ioSpaceSize)
        return false;
    *(rw[addr + registerSpaceSize]) = val;
    return true;
}

bool AvrDevice::SetIORegBit(unsigned addr, unsigned bitaddr, bool bval) {
    if(addr >= 0x20)
        return false;
    unsigned char val = *(rw[addr + registerSpaceSize]);
    if(bval)
      val |= 1 << bitaddr;
    else
      val &= ~(1 << bitaddr);
    *(rw[addr + registerSpaceSize]) = val;
    return true;
}

unsigned AvrDevice::GetRegX(void) {
    // R27:R26
    return (*(rw[27]) << 8) + *(rw[26]);
}

unsigned AvrDevice::GetRegY(void) {
    // R29:R28
    return (*(rw[29]) << 8) + *(rw[28]);
}

unsigned AvrDevice::GetRegZ(void) {
    // R31:R30
    return (*(rw[31]) << 8) + *(rw[30]);
}

// EOF
