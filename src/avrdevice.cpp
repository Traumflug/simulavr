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
 */
#include "config.h"
#include "bfd.h"


#include "avrdevice.h"
#include "trace.h"
#include "helper.h"
#include "global.h"     //only 2 defines here... please move that sometimes womewhere TODO XXX
#include "irqsystem.h"  //GetNewPc

#include "avrdevice_impl.h"

void AvrDevice::RemoveFromCycleList(Hardware *hw) {
    vector<Hardware*>::iterator element;

    element=find(hwCycleList.begin(), hwCycleList.end(), hw);
    if (element != hwCycleList.end()) hwCycleList.erase(element);
}

void AvrDevice::Load(const char* fname) {
    actualFilename=fname;

    bfd *abfd;
    asection *sec;

    bfd_init ();
    abfd=bfd_openr (fname, NULL);

    if (abfd==0) {
        cerr << "Could not open file: " << fname << endl;
        exit(0);
    }

    bfd_check_format(abfd, bfd_object);

    //reading out the symbols
    {
        long storage_needed;
        asymbol **symbol_table;
        long number_of_symbols;
        long i;


        storage_needed = bfd_get_symtab_upper_bound(abfd);

        if (storage_needed <0 ) {
            exit(0);
        }

        if (storage_needed ==0) {
            return;
        }

        symbol_table= (asymbol **)malloc(storage_needed);

        number_of_symbols = bfd_canonicalize_symtab(abfd, symbol_table);

        if (number_of_symbols < 0 ) {
            exit (0);
        }

        for (i=0; i<number_of_symbols; i++) {
            unsigned int lma=symbol_table[i]->section->lma;
            unsigned int vma=symbol_table[i]->section->vma;
            if (vma<0x7fffff) { //range of flash space
                pair<unsigned int, string> p((symbol_table[i]->value+lma)>>1, symbol_table[i]->name);
                //symbols.insert(p);
                Flash->AddSymbol(p);
            }
            else if (vma<0x80ffff) { //range of ram 
                unsigned int offset=vma-0x800000;
                //if( symbol_table[i]->flags & BSF_OBJECT) {
                {
                    pair<unsigned int, string> p(symbol_table[i]->value+offset, symbol_table[i]->name);
                    //symbolsData.insert(p);
                    data->AddSymbol(p);  //not a real data container, only holding symbols!

                    pair<unsigned int, string> pp(symbol_table[i]->value+lma, symbol_table[i]->name);
                    //symbols.insert(pp);
                    Flash->AddSymbol(pp);
                }
            }
            else if (vma<0x81ffff) { //range of eeprom
                unsigned int offset=vma-0x810000;
                pair<unsigned int, string> p(symbol_table[i]->value+offset, symbol_table[i]->name);
                //symbolsEeprom.insert(p);
                eeprom->AddSymbol(p);
            }
            else {
                cerr << "Unknown symbol address range found!" << endl;
            }
            }
        }

        sec= abfd->sections;

        while (sec!=0)  { 
            if (sec->flags&SEC_LOAD && sec->vma<0x80ffff) { //only read flash bytes and data
                int size;
                size=sec->_cooked_size;
                unsigned char *tmp=(unsigned char *)malloc(size);
                bfd_get_section_contents(abfd, sec, tmp, 0, size);
                Flash->WriteMem( tmp, sec->lma, size);
                free(tmp);
            }

            if (sec->flags&SEC_LOAD && sec->vma>=0x810000) {
                int size;
                size=sec->_cooked_size;
                unsigned char *tmp=(unsigned char *)malloc(size);
                bfd_get_section_contents(abfd, sec, tmp, 0, size);
                unsigned int offset=sec->vma-0x810000;
                eeprom->WriteMem(tmp, offset, size);
                free(tmp);
            }
            sec=sec->next;
        }

        bfd_close(abfd);
    }

#ifdef VI_BUG
}
#endif

void AvrDevice::SetClockFreq(unsigned long f) {
    clockFreq=f;
}

Pin *AvrDevice::GetPin(const char *name) {
    return allPins[name];
}

AvrDevice::~AvrDevice() {}
AvrDevice::AvrDevice(unsigned int ioSpaceSize, unsigned int IRamSize, unsigned int ERamSize, unsigned int flashSize){
    unsigned int currentOffset=0;
    const unsigned int RegisterSpaceSize=32;
    const unsigned int totalIoSpace= 0x10000;

    data= new Data; //only the symbol container

    //memory space for all RW-Memory addresses	
    rw=(RWMemoryMembers**)malloc(sizeof(RWMemoryMembers*) * totalIoSpace);
    if (rw==0) { cerr << "Not enough memory for RWMemoryMembers in AvrDevice::AvrDevice" << endl; exit(0); }

    //the status register is generic to all devices
    status= new HWSreg();
    if (status==0) { cerr << "Not enough memory for HWSreg in AvrDevice::AvrDevice" << endl; exit(0); }

    //the Registers also generic to all avr devices
    R=new MemoryOffsets(0,rw);
    if (R==0) { cerr << "Not enough memory for Register [R] in AvrDevice::AvrDevice" << endl; exit(0); }

    //offset for the io-space is 0x20 for all avr devices (behind the register file)
    ioreg=new MemoryOffsets(0x20,rw);
    if (ioreg==0) { cerr << "Not enough memory for IoReg in AvrDevice::AvrDevice" << endl; exit(0); }

    //the offset for accessing the sram is allways at 0x00 so we can read with lds also the register file!
    Sram=new MemoryOffsets(0x00,rw);
    if (Sram==0) { cerr << "Not enough memory for Sram in AvrDevice::AvrDevice" << endl; exit(0); }

    //create the flash area with specified size
    Flash=new AvrFlash(this, flashSize);
    if (Flash==0) { cerr << "Not enough memory for Flash in AvrDevice::AvrDevice" << endl; exit(0); }

    //create all registers
    for (unsigned int ii=0; ii<RegisterSpaceSize; ii++ ) {
        rw[currentOffset]=new CPURegister(this, currentOffset);
        currentOffset++;
    }      

    // no way to generate a generic IO-Space here, so we need a gap
    currentOffset+=ioSpaceSize;

    //create the internal ram handlers	
    for (unsigned int ii=0; ii<IRamSize; ii++ ) {
        rw[currentOffset]=new IRam(this, currentOffset);
        currentOffset++;
    }

    //create the external ram handlers, TODO: make the configuration from mcucr available here
    for (unsigned int ii=0; ii<ERamSize; ii++ ) {
        rw[currentOffset]=new ERam(currentOffset);
        currentOffset++;
    }

    //fill the rest of the adress space with error handlers
    for ( ; currentOffset<totalIoSpace;  ) {
        rw[currentOffset]=new NotAvailableIo(currentOffset);
        currentOffset++;
    }

}

//do a single core step, (0)->a real hardware step, (1) until the uC finish the opcode! 
int AvrDevice::Step(bool untilCoreStepFinished, unsigned long long *nextStepIn_ns) {
    int bpFlag=0;
    int hwWait=0;

    do {

        if (trace_on==1) {
            traceOut << actualFilename << " ";
            traceOut << HexShort(PC<<1)<<dec<<": ";

            string sym(Flash->GetSymbolAtAddress(PC));
            traceOut << sym << " ";
            for (int len = sym.length(); len<30;len++) { traceOut << " " ; }
        }

        hwWait=0;
        vector<Hardware *>::iterator ii;
        vector<Hardware *>::iterator end;
        end= hwCycleList.end();

        for (ii=hwCycleList.begin(); ii!=end; ii++) {
            if (((*ii)->CpuCycle())>0) hwWait=1;
        }

        if (cpuCycles<=0) {

            if (hwWait!=0) { 
                if(trace_on)traceOut << "CPU-Hold by IO-Hardware ";
            } else {
                //check for enabled breakpoints here
                if (BP.end()!=find(BP.begin(), BP.end(), PC)) {
                    if(trace_on)traceOut << "Breakpoint found at 0x" << hex << PC << dec << endl;
                    bpFlag=BREAK_POINT;
                    return bpFlag;
                }

                if (newIrqPc!= 0xffffffff) {
                    if (noDirectIrqJump==0) {
                        if (trace_on){
                            traceOut << "IRQ DETECTED: VectorAddr: " << newIrqPc ;
                        }

                        irqSystem->IrqHandlerStarted(actualIrqVector);    //what vector we raise?
                        //Funktor* fkt=new IrqFunktor(irqSystem, &HWIrqSystem::IrqHandlerFinished, actualIrqVector);
                        stack->SetBreakPoint(stack->GetStackPointer(),IrqFunktor(irqSystem, &HWIrqSystem::IrqHandlerFinished, actualIrqVector).clone());

                        //pushing the stack
                        unsigned long val=PC;
                        for (int tt=0; tt<PC_size; tt++) {
                            stack->Push(val&0xff);
                            val>>=8;
                        }
                        cpuCycles=4; //push needs 4 cycles! (on external RAM +2, this is handled from HWExtRam!)
                        status->I=0; //irq started so remove I-Flag from SREG

                        PC=newIrqPc-1;	//we add a few lines later 1 so we sub here 1 :-)
                        newIrqPc=0xffffffff;
                    } else {
                        noDirectIrqJump=0;
                    }
                }

                if (cpuCycles<=0) {
                    if ((unsigned int)(PC<<1) >= (unsigned int)Flash->GetSize() ) {
                        if (trace_on) {
                            traceOut << "Simulation runs out of Flash Space" << endl;
                        } else {
                            cerr << "Simulation runs out of Flash Space" << endl;
                        }
                        exit(0);
                    }
                    
                    DecodedInstruction *de= (Flash->DecodedMem[PC]);
                    if (trace_on) {
                    cpuCycles= de->Trace();
                    } else {
                    cpuCycles=(*de)(); 
                    }
                    
                   // cpuCycles=(*(Flash->DecodedMem[PC]))();
                }

                if (cpuCycles<0) bpFlag=cpuCycles;
                if( bpFlag!=BREAK_POINT) PC++;


                if (((status->I)==1) && (newIrqPc==0xffffffff)) {
                    newIrqPc= irqSystem->GetNewPc(actualIrqVector); //If any interrupt is pending get new PC 
                    noDirectIrqJump=1;
                }

            }
        } else { //cpuCycles>0
            if (trace_on==1) traceOut << "CPU-waitstate";
        }

        if(nextStepIn_ns!=0) {
            *nextStepIn_ns=clockFreq;
        }

        cpuCycles--;
        if (trace_on==1) traceOut << endl;
    } while ( untilCoreStepFinished && ((cpuCycles>0) || (hwWait>0)));


    return bpFlag;
}

void AvrDevice::Reset() {
    PC_size=2;
    PC=0;

    vector<Hardware *>::iterator ii;
    for (ii= hwResetList.begin(); ii!= hwResetList.end(); ii++) {
        (*ii)->Reset();
    }

    PC=0;
    *status=0;

    //init the old static vars from Step()
    cpuCycles=0;
    newIrqPc=0xffffffff;
    noDirectIrqJump=0;
}

unsigned char AvrDevice::GetRampz() { 
    cerr << "Illegal use off virtual Base Class!!! AvrDevice";
    return 0;
}

void AvrDevice::SetRampz(unsigned char val) {
    cerr << "Illegal use of virtual BAseClass AvrDevice";
}
