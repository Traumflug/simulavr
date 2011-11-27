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

#ifndef _MSC_VER
#   include "config.h"
#   include "bfd.h"
#else
//#   include "C:\cygwin\usr\include\bfd.h"  // binutils, /usr/include/bfd.h
#endif

#include "avrdevice.h"
#include "traceval.h"
#include "helper.h"
#include "global.h"     //only 2 defines here... please move that sometimes womewhere TODO XXX
#include "irqsystem.h"  //GetNewPc
#include "systemclock.h"
#include "avrerror.h"
#include "avrmalloc.h"
#include <assert.h>

#include "avrdevice_impl.h"

using namespace std;

const unsigned int AvrDevice::registerSpaceSize = 32;
const unsigned int AvrDevice::totalIoSpace = 0x10000;

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

#ifdef _MSC_VER
// Stolen from http://developers.sun.com/solaris/articles/elf.html

#define EI_NIDENT     16
// Segment types
#define PT_NULL             0
#define PT_LOAD             1
#define PT_NOTE             4
#define PT_SHLIB            5
#define PT_PHDR             6
// Segment flags
#define PF_X                 1
#define PF_W                 2
#define PF_R                 4

typedef uint32_t  Elf32_Addr;
typedef uint16_t  Elf32_Half;
typedef uint32_t  Elf32_Off;
typedef int32_t   Elf32_Sword;
typedef uint32_t  Elf32_Word;

typedef struct {
	unsigned char e_ident[EI_NIDENT];    /* ident bytes */
	Elf32_Half e_type;                   /* file type */ 
	Elf32_Half e_machine;                /* target machine */
	Elf32_Word e_version;                /* file version */
	Elf32_Addr e_entry;                  /* start address */
	Elf32_Off e_phoff;                   /* phdr file offset */
	Elf32_Off e_shoff;                   /* shdr file offset */
	Elf32_Word e_flags;                  /* file flags */
	Elf32_Half e_ehsize;                 /* sizeof ehdr */
	Elf32_Half e_phentsize;              /* sizeof phdr */
	Elf32_Half e_phnum;                  /* number phdrs */
	Elf32_Half e_shentsize;              /* sizeof shdr */
	Elf32_Half e_shnum;                  /* number shdrs */
	Elf32_Half e_shstrndx;               /* shdr string index */
} Elf32_Ehdr;
// Segment header
typedef struct {
	Elf32_Word p_type; 	/* entry type */
	Elf32_Off p_offset; 	/* file offset */
	Elf32_Addr p_vaddr; 	/* virtual address */
	Elf32_Addr p_paddr; 	/* physical address */
	Elf32_Word p_filesz;	/* file size */
	Elf32_Word p_memsz; 	/* memory size */
	Elf32_Word p_flags; 	/* entry flags */
	Elf32_Word p_align; 	/* memory/file alignment */
} Elf32_Phdr;
#endif

void AvrDevice::Load(const char* fname) {
    actualFilename = fname;

#ifdef _MSC_VER
    FILE * f = fopen(fname, "rb");
    if(f == NULL)
        avr_error("Could not open file: %s", fname);

    Elf32_Ehdr header;
    fread(&header, sizeof(header), 1, f);
    if(header.e_ident[0] != 0x7F || header.e_ident[1] != 'E'
        || header.e_ident[2] != 'L' || header.e_ident[3] != 'F')
        avr_error("File '%s' is not an ELF file", fname);
    // TODO: fix endianity in header
    if(header.e_machine != 83)
        avr_error("ELF file '%s' is not for Atmel AVR architecture (%d)", fname, header.e_machine);

    for(int i = 0; i < header.e_phnum; i++) {
        fseek(f, header.e_phoff + i * header.e_phentsize, SEEK_SET);
        Elf32_Phdr progHeader;
        fread(&progHeader, sizeof(progHeader), 1, f);
        // TODO: fix endianity
        if(progHeader.p_type != PT_LOAD)
            continue;
        if((progHeader.p_flags & PF_X ) == 0 || (progHeader.p_flags & PF_R) == 0)
            continue;  // must be readable and writable
        if(progHeader.p_vaddr >= 0x80ffff)
            continue;  // not into a Flash
        if(progHeader.p_filesz != progHeader.p_memsz) {
            avr_error("Segment sizes 0x%x and 0x%x in ELF file '%s' must be the same",
                progHeader.p_filesz, progHeader.p_memsz);
        }
        unsigned char * tmp = new unsigned char[progHeader.p_filesz];
        fseek(f, progHeader.p_offset, SEEK_SET);
        fread(tmp, progHeader.p_filesz, 1, f);

        Flash->WriteMem(tmp, progHeader.p_vaddr, progHeader.p_filesz);
        delete [] tmp;
    }

    fclose(f);
#else
    /* If you do not want to use libbfd, then edit m4/AX_AVR_ENVIRON.m4
     * and comment out the AVR_BFD_SEARCH_STEP calls and related lines
     * and adjust the #ifdef condition above. Compile and then link
     * without -lbfd. */
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
#endif
}

#ifdef VI_BUG
}
#endif

void AvrDevice::SetClockFreq(SystemClockOffset nanosec) {
   clockFreq = nanosec;
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
    delete [] invalidRW;
    
    // delete Ram cells and registers
    for(unsigned idx = 0; idx < registerSpaceSize; idx++)
        delete rw[idx];
    size = registerSpaceSize + ioSpaceSize + iRamSize + eRamSize;
    for(unsigned idx = (registerSpaceSize + ioSpaceSize); idx < size; idx++)
        delete rw[idx];
    
    // delete rw and other allocated objects
    delete Flash;
    delete statusRegister;
    delete status;
    delete [] rw;
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
    iRamSize(IRamSize),
    eRamSize(ERamSize),
    ioSpaceSize(_ioSpaceSize),
    flagIWInstructions(true),
    flagJMPInstructions(true),
    flagIJMPInstructions(true),
    flagEIJMPInstructions(false),
    flagLPMInstructions(true),
    flagELPMInstructions(false),
    flagMULInstructions(true),
    flagMOVWInstruction(true),
    flagTiny10(false),
    flagTiny1x(false),
    flagXMega(false),
    instructionSEIJustEnabledInterrupts(false)
{
    dump_manager = DumpManager::Instance();
    dump_manager->registerAvrDevice(this);
	DebugRecentJumpsIndex = 0;
    
    TraceValue* pc_tracer=trace_direct(&coreTraceGroup, "PC", &cPC);
    coreTraceGroup.RegisterTraceValue(new TwiceTV(coreTraceGroup.GetTraceValuePrefix()+"PCb",  pc_tracer));
	trace_on = 0;
    
    data = new Data; //only the symbol container

    // placeholder for RAMPZ and EIND register
    rampz = NULL;
    eind = NULL;
    
    // memory space for all RW-Memory addresses + shadow store for invalid cells
    unsigned invalidSize = totalIoSpace - registerSpaceSize - IRamSize - ERamSize; 
    rw = new RWMemoryMember* [totalIoSpace];
    invalidRW = new RWMemoryMember* [invalidSize];
    
    // the status register is generic to all devices
    status = new HWSreg();
    if(status == NULL)
        avr_error("Not enough memory for HWSreg in AvrDevice::AvrDevice");
    statusRegister = new RWSreg(&coreTraceGroup, status);
    if(statusRegister == NULL)
        avr_error("Not enough memory for RWSreg in AvrDevice::AvrDevice");

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

    assert(currentOffset<=totalIoSpace);
    // fill the rest of the address space with error handlers
    for(; currentOffset < totalIoSpace; currentOffset++, invalidRWOffset++) {
        invalidRW[invalidRWOffset] = new InvalidMem(this, currentOffset);
        if(invalidRW[invalidRWOffset] == NULL)
            avr_error("Not enough memory for fill address space in AvrDevice::AvrDevice");
        rw[currentOffset] = invalidRW[invalidRWOffset];
    }
}

//do a single core step, (0)->a real hardware step, (1) until the uC finish the opcode! 
int AvrDevice::Step(bool &untilCoreStepFinished, SystemClockOffset *nextStepIn_ns) {
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

    bool hwWait = false;
    for(unsigned i = 0; i < hwCycleList.size(); i++) {
        Hardware * p = hwCycleList[i];
        if (p->CpuCycle() > 0)
            hwWait = true;
    }

    if(hwWait) {
        if(trace_on)
            traceOut << "CPU-Hold by IO-Hardware ";
    } else if(cpuCycles <= 0) {

            //check for enabled breakpoints here
            if(BP.end() != find(BP.begin(), BP.end(), PC)) {
                if(trace_on)
                    traceOut << "Breakpoint found at 0x" << hex << PC << dec << endl;
                if(nextStepIn_ns != 0)
                    *nextStepIn_ns=clockFreq;
                untilCoreStepFinished = !(cpuCycles > 0);
                dump_manager->cycle();
                return BREAK_POINT;
            }

            if(EP.end() != find(EP.begin(), EP.end(), PC)) {
                if(global_verbose_on)
                    cout << "Simulation finished!" << endl;
                SystemClock::Instance().stop();
                dump_manager->cycle();
                return 0;
            }

            if(instructionSEIJustEnabledInterrupts) {
                instructionSEIJustEnabledInterrupts = false;
                // And skip executing the interrupt stuff below.
            } else if(status->I == 1) {
                unsigned int actualIrqVector;
                unsigned int newIrqPc = irqSystem->GetNewPc(actualIrqVector);
                if(newIrqPc != 0xffffffff) {
                    if(trace_on)
                        traceOut << "IRQ DETECTED: VectorAddr: " << newIrqPc ;

                    irqSystem->IrqHandlerStarted(actualIrqVector);    //what vector we raise?
                    //Funktor* fkt=new IrqFunktor(irqSystem, &HWIrqSystem::IrqHandlerFinished, actualIrqVector);
                    stack->SetReturnPoint(stack->GetStackPointer(),
                                          IrqFunktor(irqSystem, &HWIrqSystem::IrqHandlerFinished, actualIrqVector).clone());
                    stack->PushAddr(PC);
                    cpuCycles = 4; //push needs 4 cycles! (on external RAM +2, this is handled from HWExtRam!)
                    status->I = 0; //irq started so remove I-Flag from SREG
                    PC = newIrqPc - 1;   //we add a few lines later 1 so we sub here 1 :-)
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
            }

            if(cpuCycles != BREAK_POINT) {
                PC++;
                cpuCycles--;
            }
    } else { //cpuCycles>0
        if(trace_on == 1)
            traceOut << "CPU-waitstate";
        cpuCycles--;
    }

    if(nextStepIn_ns != NULL)
        *nextStepIn_ns = clockFreq;

    if(trace_on == 1) {
        traceOut << endl;
        sysConHandler.TraceNextLine();
    }

    untilCoreStepFinished = !((cpuCycles > 0) || hwWait);
    dump_manager->cycle();
    return (cpuCycles < 0) ? cpuCycles : 0;
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
}

void AvrDevice::DeleteAllBreakpoints() {
    BP.erase(BP.begin(), BP.end());
}

void AvrDevice::ReplaceIoRegister(unsigned int offset, RWMemoryMember *newMember) {
    if (offset >= ioSpaceSize + registerSpaceSize)
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
#ifdef _MSC_VER
    fprintf(stderr, "Fatal: Cannot specify terminating symbol. Loading symbols from ELF file is not implemented\n");
    assert(false);  // TODO: Implement loading symbols from ELF file
#endif
    unsigned int epa = Flash->GetAddressAtSymbol(symbol);
    EP.push_back(epa);
}

void AvrDevice::DebugOnJump()
{
	const int COUNT = sizeof DebugRecentJumps / sizeof DebugRecentJumps[0];
    DebugRecentJumpsIndex = (DebugRecentJumpsIndex + 1) % COUNT;
    DebugRecentJumps[DebugRecentJumpsIndex] = PC * 2;
    int next = (DebugRecentJumpsIndex + 1) % COUNT;
    DebugRecentJumps[next] = -1;
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
    assert(addr < registerSpaceSize);
    return *(rw[addr]);
}

bool AvrDevice::SetCoreReg(unsigned addr, unsigned char val) {
    assert(addr < registerSpaceSize);
    *(rw[addr]) = val;
    return true;
}

unsigned char AvrDevice::GetIOReg(unsigned addr) {
    assert(addr < ioSpaceSize);  // callers do use 0x00 base, not 0x20
    return *(rw[addr + registerSpaceSize]);
}

bool AvrDevice::SetIOReg(unsigned addr, unsigned char val) {
    assert(addr < ioSpaceSize);  // callers do use 0x00 base, not 0x20
    *(rw[addr + registerSpaceSize]) = val;
    return true;
}

bool AvrDevice::SetIORegBit(unsigned addr, unsigned bitaddr, bool bval) {
    assert(addr < 0x20);  // only first 32 IO registers are bit-settable
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
