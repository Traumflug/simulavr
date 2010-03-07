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

#include "types.h"
#include "decoder.h"
#include "avrdevice.h"

#include "hwsreg.h"
#include "helper.h"
#include "flash.h"
#include "rwmem.h"
#include "ioregs.h"
#include "avrerror.h"

#define MONSREG traceOut << (string)(*(core->status))  

using namespace std;

int avr_op_ADC::Trace()  {
    traceOut << "ADC R" << R1 << ", R" << R2 << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ADD::Trace() {
    traceOut << "ADD R" << R1 << ", R" << R2 << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ADIW::Trace() {
    traceOut << "ADIW R" << Rl << ", " << K << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_AND::Trace() {
    traceOut << "AND R" << R1 << ", R" << R2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ANDI::Trace() {
    traceOut << "ANDI R" << R1 << ", " << HexChar(K) << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ASR::Trace() {
    traceOut << "ASR R" << R1 ;
    int ret = this->operator()();
    MONSREG;
    return ret;
}

const char *opcodes_bclr[8]= {
    "CLC",
    "CLZ",
    "CLN",
    "CLV",
    "CLS",
    "CLH",
    "CLT",
    "CLI"
};

int avr_op_BCLR::Trace() {
    traceOut << opcodes_bclr[Kbit] << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_BLD::Trace() {
    traceOut << "BLD R" << R1 << ", " << Kbit << " ";
    int ret = this->operator()();
    return ret;
}

const char *branch_opcodes_clear[8] = {
    "BRCC",
    "BRNE",
    "BRPL",
    "BRVC",
    "BRGE",
    "BRHC",
    "BRTC",
    "BRID"
};

int avr_op_BRBC::Trace() {
    traceOut << branch_opcodes_clear[bitmask] << " ->" << HexShort(offset * 2) << " ";
    unsigned int oldPC = core->PC;
    int ret = this->operator()();
    
    string sym(core->Flash->GetSymbolAtAddress(core->PC * 2));
    if(oldPC + 1 != core->PC) {
        traceOut << sym << " ";
        for(int len = sym.length(); len < 30; len++)
            traceOut << " ";
    }

    return ret;
}

const char *branch_opcodes_set[8] = {
    "BRCS",
    "BREQ",
    "BRMO",
    "BRVS",
    "BRLT",
    "BRHS",
    "BRTS",
    "BRIE"
};

int avr_op_BRBS::Trace() {
    traceOut << branch_opcodes_set[bitmask] << " ->" << HexShort(offset * 2) << " ";
    unsigned int oldPC = core->PC;
    int ret=this->operator()();

    string sym(core->Flash->GetSymbolAtAddress(core->PC * 2));
    if(oldPC + 1 != core->PC) {
        traceOut << sym << " ";
        for(int len = sym.length(); len < 30; len++)
            traceOut << " ";
    }

    return ret;
}

const char *opcodes_bset[8]= {
    "SEC",
    "SEZ",
    "SEN",
    "SEV",
    "SES",
    "SEH",
    "SET",
    "SEI"
};

int avr_op_BSET::Trace() {
    traceOut << opcodes_bset[Kbit] << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_BST::Trace() {
    traceOut << "BST R" << R1 << ", " << Kbit << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_CALL::Trace() {
    word K_lsb = core->Flash->ReadMemWord((core->PC + 1) * 2);
    int k = (KH << 16) | K_lsb;
    traceOut << "CALL 0x" << hex << k * 2 << dec << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_CBI::Trace() {
    traceOut << "CBI " << HexChar(ioreg) << ", " << Kbit << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_COM::Trace() {
    traceOut << "COM R" << R1 << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_CP::Trace() {
    traceOut << "CP R" << R1 << ", R" << R2 << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_CPC::Trace() {
    traceOut << "CPC R" << R1 << ", R" << R2 << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_CPI::Trace() {
    traceOut << "CPI R" << R1 << ", " << HexChar(K) << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_CPSE::Trace() {
    traceOut << "CPSE R" << R1 << ", R" << R2 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_DEC::Trace() {
    traceOut << "DEC R" << R1 << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_EICALL::Trace() {
    traceOut << "EICALL ";
    int ret = this->operator()();
    return ret;
}

int avr_op_EIJMP::Trace() {
    traceOut << "EIJMP ";
    int ret = this->operator()();
    return ret;
}

int avr_op_ELPM_Z::Trace() {
    traceOut << "ELPM R" << R1 << ", Z " ;
    int ret = this->operator()();

    unsigned char rampz = 0;
    if(core->rampz != NULL)
        rampz = core->rampz->GetRegVal();
    unsigned int Z = (rampz << 16) + core->GetRegZ();

    traceOut << " Flash[0x" << hex << Z << dec << "] ";

    return ret;
}

int avr_op_ELPM_Z_incr::Trace() {
    traceOut << "ELPM R" << R1 << ", Z+ ";
    unsigned char rampz = 0;
    if(core->rampz != NULL)
        rampz = core->rampz->GetRegVal();
    unsigned int Z = (rampz << 16) + core->GetRegZ();
    int ret = this->operator()();

    traceOut << " Flash[0x" << hex << Z << dec << "] ";

    return ret;
}

int avr_op_ELPM::Trace() {
    traceOut << "ELPM ";
    int ret = this->operator()();

    unsigned char rampz = 0;
    if(core->rampz != NULL)
        rampz = core->rampz->GetRegVal();
    unsigned int Z = (rampz << 16) + core->GetRegZ();

    traceOut << " Flash[0x" << hex << Z << dec << "] ";

    return ret;
}

int avr_op_EOR::Trace() {
    traceOut << "EOR R" << R1 << ", R" << R2 << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ESPM::Trace() {
    traceOut << "SPM Z+ ";
    int ret = this->operator()();
    return ret;
}

int avr_op_FMUL::Trace() {
    traceOut << "FMUL R" << Rd << ", R" << Rr << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_FMULS::Trace() {
    traceOut << "FMULS R" << Rd << ", R" << Rr << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_FMULSU::Trace() {
    traceOut << "FMULSU R" << Rd << ", R" << Rr << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ICALL::Trace() {
    traceOut << "ICALL Z " ;
    int ret = this->operator()();
    return ret;
}

int avr_op_IJMP::Trace() {
    traceOut << "IJMP Z " ;
    int ret = this->operator()();
    return ret;
}

int avr_op_IN::Trace() {
    traceOut << "IN R" << R1 << ", " << HexChar(ioreg) << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_INC::Trace() {
    traceOut << "INC R" << R1 << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_JMP::Trace() {
    traceOut << "JMP ";
    word offset = core->Flash->ReadMemWord((core->PC + 1) * 2);  //this is k!
    int ret = this->operator()();
    traceOut << hex << 2 * offset << dec << " ";

    string sym(core->Flash->GetSymbolAtAddress(offset));
    traceOut << sym << " ";
    for(int len = sym.length(); len < 30; len++)
        traceOut << " " ;

    return ret;
}

int avr_op_LDD_Y::Trace() {
    traceOut << "LDD R" << Rd << ", Y+" << K;
    int ret = this->operator()();
    return ret;
}

int avr_op_LDD_Z::Trace() {
    traceOut << "LDD R" << Rd << ", Z+" << K;
    int ret = this->operator()();
    return ret;
}

int avr_op_LDI::Trace() {
    traceOut << "LDI R" << R1 << ", " << HexChar(K) << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_LDS::Trace() {
    word offset = core->Flash->ReadMemWord((core->PC + 1) * 2);  //this is k!
    traceOut << "LDS R" << R1 << ", " << hex << "0x" << offset << dec  << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_X::Trace() {
    traceOut << "LD R" << Rd << ", X ";
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_X_decr::Trace() {
    traceOut << "LD R" << Rd << ", -X ";
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_X_incr::Trace() {
    traceOut << "LD R" << Rd << ", X+ ";
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_Y_decr::Trace() {
    traceOut << "LD R" << Rd << ", -Y ";
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_Y_incr::Trace() {
    traceOut << "LD R" << Rd << ", Y+ " ;
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_Z_incr::Trace() {
    traceOut << "LD R" << Rd << ", Z+ ";
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_Z_decr::Trace() {
    traceOut << "LD R" << Rd << ", -Z";
    int ret = this->operator()();
    return ret;
}

int avr_op_LPM_Z::Trace() {
    traceOut << "LPM R" << Rd << ", Z ";
    int ret = this->operator()();

    /* Z is R31:R30 */
    unsigned int Z = core->GetRegZ();
    string sym(core->Flash->GetSymbolAtAddress(Z));
    traceOut << "FLASH[" << hex << Z << dec << "," << sym << "] ";

    return ret;
}

int avr_op_LPM::Trace() {
    traceOut << "LPM R0, Z "; 
    int ret = this->operator()();

    /* Z is R31:R30 */
    unsigned int Z = core->GetRegZ();
    string sym(core->Flash->GetSymbolAtAddress(Z));
    traceOut << "FLASH[" << hex << Z << dec << "," << sym << "] ";

    return ret;
}

int avr_op_LPM_Z_incr::Trace() {
    traceOut << "LPM R" << Rd << ", Z+ " ;
    /* Z is R31:R30 */
    unsigned int Z = core->GetRegZ();
    int ret = this->operator()();
    
    string sym(core->Flash->GetSymbolAtAddress(Z));
    traceOut << "FLASH[" << hex << Z << dec << "," << sym << "] ";
    return ret;
}

int avr_op_LSR::Trace() {
    traceOut << "LSR R" << Rd << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_MOV::Trace() {
    traceOut << "MOV R" << R1 << ", R" << R2 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_MOVW::Trace() {
    traceOut << "MOVW R" << Rd << ", R" << Rs << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_MUL::Trace() {
    traceOut << "MUL R" << Rd << ", R" << Rr << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_MULS::Trace() {
    traceOut << "MULS R" << Rd << ", R" << Rr << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_MULSU::Trace() {
    traceOut << "MULSU R" << Rd << ", R" << Rr << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_NEG::Trace() {
    traceOut << "NEG R" << Rd <<" ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_NOP::Trace() {
    traceOut << "NOP ";
    int ret = this->operator()();
    return ret;
}

int avr_op_OR::Trace() {
    traceOut << "OR R" << Rd << ", R" << Rr << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ORI::Trace() {
    traceOut << "ORI R" << R1 << ", " << HexChar(K) << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_OUT::Trace() {
    traceOut << "OUT " << HexChar(ioreg) << ", R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_POP::Trace() {
    traceOut << "POP R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_PUSH::Trace() {
    traceOut << "PUSH R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_RCALL::Trace() {
    traceOut << "RCALL " << hex << ((core->PC + K + 1) << 1) << dec << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_RET::Trace() {
    traceOut << "RET " ;
    int ret = this->operator()();
    return ret;
}

int avr_op_RETI::Trace() {
    traceOut << "RETI ";
    int ret = this->operator()();
    return ret;
}

int avr_op_RJMP::Trace() {
    traceOut << "RJMP " << hex << ((core->PC + K + 1) << 1) << dec << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_ROR::Trace() {
    traceOut << "ROR R" << R1 << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_SBC::Trace() {
    traceOut << "SBC R" << R1 << ", R" << R2 << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_SBCI::Trace() {
    traceOut << "SBCI R" << R1 << ", " << HexChar(K) << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_SBI::Trace() {
    traceOut << "SBI " << HexChar(ioreg) << ", " << Kbit << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_SBIC::Trace() {
    traceOut << "SBIC " << HexChar(ioreg) << ", " << Kbit << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_SBIS::Trace() {
    traceOut << "SBIS " << HexChar(ioreg) << ", " << Kbit << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_SBIW::Trace( ) {
    traceOut << "SBIW R" << R1 << ", " << HexChar(K) << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_SBRC::Trace() {
    traceOut << "SBRC R" << R1 << ", " << Kbit << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_SBRS::Trace() {
    traceOut << "SBRS R" << R1 << ", " << Kbit << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_SLEEP::Trace() {
    traceOut << "SLEEP " ;
    int ret = this->operator()();
    return ret;
}

int avr_op_SPM::Trace() {
    traceOut << "SPM " ;
    int ret = this->operator()();
    return ret;
}

int avr_op_STD_Y::Trace() {
    traceOut << "STD Y+" << K << ", R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_STD_Z::Trace() {
    traceOut << "STD Z+" << K << ", R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_STS::Trace() {
    word offset = core->Flash->ReadMemWord((core->PC + 1) * 2);  //this is k!
    traceOut << "STS " << "0x" << hex << offset << dec << ", R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_X::Trace() {
    traceOut << "ST X, R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_X_decr::Trace() {
    traceOut << "ST -X, R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_X_incr::Trace() {
    traceOut << "ST X+, R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_Y_decr::Trace() {
    traceOut << "ST -Y, R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_Y_incr::Trace() {
    traceOut << "ST Y+, R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_Z_decr::Trace() {
    traceOut << "ST -Z, R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_Z_incr::Trace() {
    traceOut << "ST Z+, R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_SUB::Trace() {
    traceOut << "SUB R" << R1 << ", R" << R2 << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_SUBI::Trace() {
    traceOut << "SUBI R" << R1 << ", " << HexChar(K) << " ";
    int ret = this->operator()();
    MONSREG;
    return ret;
}

int avr_op_SWAP::Trace() {
    traceOut << "SWAP R" << R1 << " ";
    int ret = this->operator()();
    return ret;
}

int avr_op_WDR::Trace() {
    traceOut << "WDR ";
    int ret = this->operator()();
    return ret;
}

int avr_op_ILLEGAL::Trace() {
    traceOut << "Invalid Instruction! ";
    int ret = this->operator()();
    return ret;
}

/* EOF */
