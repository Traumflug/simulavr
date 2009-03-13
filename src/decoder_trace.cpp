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
#include "trace.h"

#include "hwsreg.h"
#include "helper.h"
#include "flash.h"
#include "rwmem.h"

#define MONSREG traceOut << (string)(*(core->status))  

int avr_op_ADC::Trace()  {
    traceOut << "ADC R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ADD::Trace( ) {
    traceOut << "ADD R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ADIW::Trace( ) {
    traceOut << "ADIW R" << p1 << ", " << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_AND::Trace( ) {
    traceOut << "AND R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ANDI::Trace( ) {
    traceOut << "ANDI R" << p1 << ", " << HexChar(p2) << dec << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ASR::Trace( ) {
    traceOut << "ASR R" << p1 ;
    int ret=this->operator()();
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

int avr_op_BCLR::Trace( ) {
    traceOut << opcodes_bclr[p1] << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_BLD::Trace( ) {
    traceOut << "BLD R" << p1 << ", " << p2 << " ";
    int ret=this->operator()();
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

int avr_op_BRBC::Trace( ) {
    traceOut << branch_opcodes_clear[p1] << " ->" << HexShort((core->PC+1+p2)<<1) << dec << " ";
    int ret=this->operator()();

    string sym(core->Flash->GetSymbolAtAddress(core->PC+1+p2));
    traceOut << sym << " ";
    for (int len = sym.length(); len<30;len++) { traceOut << " " ; }

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

int avr_op_BRBS::Trace( ) {
    traceOut << branch_opcodes_set[p1] << " ->" << HexShort((core->PC+1+p2)<<1) << dec << " ";
    int ret=this->operator()();

    string sym(core->Flash->GetSymbolAtAddress(core->PC+1+p2));
    traceOut << sym << " ";
    for (int len = sym.length(); len<30;len++) { traceOut << " " ; }

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

int avr_op_BSET::Trace( ) {
    traceOut << opcodes_bset[p1] << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_BST::Trace( ) {
    traceOut << "BST R" << p1 << ", " << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_CALL::Trace( ) {
    traceOut << "CALL " << p1 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_CBI::Trace( ) {
    traceOut << "CBI " << HexChar(p1) << dec  << ", " << dec << p2 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_COM::Trace( ) {
    traceOut << "COM R" << p1 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_CP::Trace( ) {
    traceOut << "CP R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_CPC::Trace( ) {
    traceOut << "CPC R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_CPI::Trace( ) {
    traceOut << "CPI R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_CPSE::Trace( ) {
    traceOut << "CPSE R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_DEC::Trace( ) {
    traceOut << "DEC R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_EICALL::Trace( ) {
    traceOut << "EICALL " ;
    int ret=this->operator()();
    return ret;
}

int avr_op_EIJMP::Trace( ) {
    traceOut << "EIJMP " ;
    int ret=this->operator()();
    return ret;
}

int avr_op_ELPM_Z::Trace( ) {
    traceOut << "ELPM R" << p1 << ", Z " ;
    int ret=this->operator()();

    unsigned int Z = ((core->GetRampz() & 0x3f) << 16) +
        (((*(core->R))[31]) << 8) + 
        (*(core->R))[30];

    unsigned int flash_addr = Z;// / 2;

    traceOut << " Flash[0x"<<hex<< (unsigned int) flash_addr<<"] ";

    return ret;
}

int avr_op_ELPM_Z_incr::Trace( ) {
    traceOut << "ELPM R" << p1 << ", Z+ ";
    int ret=this->operator()();

    unsigned int Z = ((core->GetRampz() & 0x3f) << 16) +
        (((*(core->R))[31]) << 8) + 
        (*(core->R))[30];

    unsigned int flash_addr = Z; // / 2;

    traceOut << " Flash[0x"<<hex<< (unsigned int) flash_addr<<"] ";



    return ret;
}

int avr_op_ELPM::Trace( ) {
    traceOut << "ELPM ";
    int ret=this->operator()();

    unsigned int Z = ((core->GetRampz() & 0x3f) << 16) +
        (((*(core->R))[31]) << 8) + 
        (*(core->R))[30];

    unsigned int flash_addr = Z; // / 2;

    traceOut << " Flash[0x"<<hex<< (unsigned int) flash_addr<<"] ";


    return ret;
}

int avr_op_EOR::Trace( ) {
    traceOut << "EOR R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ESPM::Trace( ) {
    traceOut << "ESPM??? " << p1 << ", " << p2 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_FMUL::Trace( ) {
    traceOut << "FMUL R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_FMULS::Trace( ) {
    traceOut << "FMULS R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_FMULSU::Trace( ) {
    traceOut << "FMULSU R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ICALL::Trace( ) {
    traceOut << "ICALL " ;
    int ret=this->operator()();
    return ret;
}

int avr_op_IJMP::Trace( ) {
    traceOut << "IJMP " ;
    int ret=this->operator()();
    return ret;
}

int avr_op_IN::Trace( ) {
    traceOut << "IN R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_INC::Trace( ) {
    traceOut << "INC R" << p1 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_JMP::Trace( ) {
    traceOut << "JMP ";
    int ret=this->operator()();
    word *MemPtr=(word*)core->Flash->myMemory;
    word offset=MemPtr[(core->PC)+1];         //this is k!
    offset=(offset>>8)+((offset&0xff)<<8);
    traceOut << hex << 2*offset << " ";

    string sym(core->Flash->GetSymbolAtAddress(offset));
    traceOut << sym << " ";
    for (int len = sym.length(); len<30;len++) { traceOut << " " ; }

    return ret;
}

int avr_op_LDD_Y::Trace( ) {
    traceOut << "LD R" << p1 << ", Y+"<<p2;
    int ret=this->operator()();
    return ret;
}

int avr_op_LDD_Z::Trace( ) {
    traceOut << "LDD R" << p1 << ", Z ";
    int ret=this->operator()();
    return ret;
}

int avr_op_LDI::Trace( ) {
    traceOut << "LDI R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_LDS::Trace( ) {
    word *MemPtr=(word*)core->Flash->myMemory;
    word offset=MemPtr[(core->PC)+1];         //this is k!
    offset=(offset>>8)+((offset&0xff)<<8);
    traceOut << "LDS R" << p1 << ", " << hex << "0x" << offset << dec  << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_LD_X::Trace( ) {
    traceOut << "LD R" << p1 << ", X ";
    int ret=this->operator()();
    return ret;
}

int avr_op_LD_X_decr::Trace( ) {
    traceOut << "LD R" << p1 << ", -X ";
    int ret=this->operator()();
    return ret;
}

int avr_op_LD_X_incr::Trace( ) {
    traceOut << "LD R" << p1 << ", X+ ";
    int ret=this->operator()();
    return ret;
}

int avr_op_LD_Y_decr::Trace( ) {
    traceOut << "LD R" << p1 << ", -Y ";
    int ret=this->operator()();
    return ret;
}

int avr_op_LD_Y_incr::Trace( ) {
    traceOut << "LD R" << p1 << ", Y+ " ;
    int ret=this->operator()();
    return ret;
}

int avr_op_LD_Z_incr::Trace( ) {
    traceOut << "LD R" << p1 << ", Z+ ";
    int ret=this->operator()();
    return ret;
}

int avr_op_LD_Z_decr::Trace( ) {
    traceOut << "LD R" << p1 << ", -Z";
    int ret=this->operator()();
    return ret;
}

int avr_op_LPM_Z::Trace( ) {
    traceOut << "LPM_Z ";
    int ret=this->operator()();

    /* Z is R31:R30 */
    int Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];
    //string sym(core->Flash->GetSymbolAtAddressLpm(Z));
    string sym(core->Flash->GetSymbolAtAddress(Z));
    traceOut << "FLASH[" << hex << Z << "," << sym << "] ";


    return ret;
}

int avr_op_LPM::Trace( ) {
    traceOut << "LPM R0, Z "; 
    int ret=this->operator()();

    /* Z is R31:R30 */
    int Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];
    //string sym(core->Flash->GetSymbolAtAddressLpm(Z));
    string sym(core->Flash->GetSymbolAtAddress(Z));
    traceOut << "FLASH[" << hex << Z << "," << sym << "] ";

    return ret;
}

int avr_op_LPM_Z_incr::Trace( ) {
    traceOut << "LPM R" << p1 << ", Z+ " ;
    int ret=this->operator()();
    /* Z is R31:R30 */
    int Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];
    //string sym(core->Flash->GetSymbolAtAddressLpm(Z));
    string sym(core->Flash->GetSymbolAtAddress(Z));
    traceOut << "FLASH[" << hex << Z << "," << sym << "] ";
    return ret;
}

int avr_op_LSR::Trace( ) {
    traceOut << "LSR R" << p1 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_MOV::Trace( ) {
    traceOut << "MOV R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_MOVW::Trace( ) {
    traceOut << "MOVW R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_MUL::Trace( ) {
    traceOut << "MUL R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_MULS::Trace( ) {
    traceOut << "MULS R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_MULSU::Trace( ) {
    traceOut << "MULSU R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_NEG::Trace( ) {
    traceOut << "NEG R" << p1 <<" ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_NOP::Trace( ) {
    traceOut << "NOP ";
    int ret=this->operator()();
    return ret;
}

int avr_op_OR::Trace( ) {
    traceOut << "OR R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_ORI::Trace( ) {
    traceOut << "ORI R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_OUT::Trace( ) {
    traceOut << "OUT " << HexChar(p1) << dec  << ", R" << p2 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_POP::Trace( ) {
    traceOut << "POP R" << p1 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_PUSH::Trace( ) {
    traceOut << "PUSH R" << p1 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_RCALL::Trace( ) {
    traceOut << "RCALL " << hex << ((core->PC+p1+1)<<1) << dec  << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_RET::Trace( ) {
    traceOut << "RET " ;
    int ret=this->operator()();
    return ret;
}

int avr_op_RETI::Trace( ) {
    traceOut << "RETI ";
    int ret=this->operator()();
    return ret;
}

int avr_op_RJMP::Trace( ) {
    traceOut << "RJMP " << hex << ((core->PC+p1+1)<<1) << dec  << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_ROR::Trace( ) {
    traceOut << "ROR R" << p1 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_SBC::Trace( ) {
    traceOut << "SBC R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_SBCI::Trace( ) {
    traceOut << "SBCI R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_SBI::Trace( ) {
    traceOut << "SBI " << HexChar(p1) << dec  << ", " << dec << p2 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_SBIC::Trace( ) {
    traceOut << "SBIC " << HexChar(p1) << dec  << ", " << dec << p2 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_SBIS::Trace( ) {
    traceOut << "SBIS " << HexChar(p1) << dec  << ", " << dec << p2 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_SBIW::Trace( ) {
    traceOut << "SBIW R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_SBRC::Trace( ) {
    traceOut << "SBRC R" << p1 << ", " << p2 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_SBRS::Trace( ) {
    traceOut << "SBRS R" << p1 << ", " << p2 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_SLEEP::Trace( ) {
    traceOut << "SLEEP " ;
    int ret=this->operator()();
    return ret;
}

int avr_op_SPM::Trace( ) {
    traceOut << "SPM " ;
    int ret=this->operator()();
    return ret;
}

int avr_op_STD_Y::Trace( ) {
    traceOut << "STD Y+"<<p1 <<", R" << p2 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_STD_Z::Trace( ) {
    traceOut << "STD Z, R" << p1 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_STS::Trace( ) {
    word *MemPtr=(word*)core->Flash->myMemory;
    word offset=MemPtr[(core->PC)+1];         //this is k!
    offset=(offset>>8)+((offset&0xff)<<8);
    traceOut << "STS " << "0x" << hex << offset << dec  << ", R" << p2 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_ST_X::Trace( ) {
    traceOut << "ST X, R" << p1 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_ST_X_decr::Trace( ) {
    traceOut << "ST -X, R" << p1 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_ST_X_incr::Trace( ) {
    traceOut << "ST X+, R" << p1 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_ST_Y_decr::Trace( ) {
    traceOut << "ST -Y, R" << p1 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_ST_Y_incr::Trace( ) {
    traceOut << "ST Y+, R" << p1 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_ST_Z_decr::Trace( ) {
    traceOut << "ST -Z, R" << p1 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_ST_Z_incr::Trace( ) {
    traceOut << "ST Z+, R" << p1 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_SUB::Trace( ) {
    traceOut << "SUB R" << p1 << ", R" << p2 << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_SUBI::Trace( ) {
    traceOut << "SUBI R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret=this->operator()();
    MONSREG;
    return ret;
}

int avr_op_SWAP::Trace( ) {
    traceOut << "SWAP R" << p1 << " ";
    int ret=this->operator()();
    return ret;
}

int avr_op_WDR::Trace( ) {
    traceOut << "WDR ";
    int ret=this->operator()();
    return ret;
}

int avr_op_ILLEGAL::Trace( ) {
    traceOut << "Invalid Instruction! ";
    int ret=this->operator()();
    return ret;
}


