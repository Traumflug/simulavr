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
#include "types.h"
#include "decoder.h"
#include "avrdevice.h"
#include "ass_do.h"
#include "trace.h"

#include "hwsreg.h"
#include "helper.h"
#include "flash.h"
#include "rwmem.h"

#define MONSREG traceOut << (string)(*(core->status))  

int avr_op_ADC_do_trace( AvrDevice *core, int p1, int p2)  {
    traceOut << "ADC R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_ADC_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_ADD_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ADD R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_ADD_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_ADIW_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ADIW R" << p1 << ", " << p2 << " ";
    int ret= avr_op_ADIW_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_AND_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "AND R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_AND_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_ANDI_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ANDI R" << p1 << ", " << HexChar(p2) << dec << " ";
    int ret= avr_op_ANDI_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_ASR_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ASR R" << p1 ;
    int ret= avr_op_ASR_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

char *opcodes_bclr[8]= {
    "CLC",
    "CLZ",
    "CLN",
    "CLV",
    "CLS",
    "CLH",
    "CLT",
    "CLI"
};

int avr_op_BCLR_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << opcodes_bclr[p1] << " ";
    int ret= avr_op_BCLR_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_BLD_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "BLD R" << p1 << ", " << p2 << " ";
    int ret= avr_op_BLD_do( core, p1, p2) ; 
    return ret;
}

char *branch_opcodes_clear[8] = {
    "BRCC",
    "BRNE",
    "BRPL",
    "BRVC",
    "BRGE",
    "BRHC",
    "BRTC",
    "BRID"
};

int avr_op_BRBC_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << branch_opcodes_clear[p1] << " ->" << HexShort((core->PC+1+p2)<<1) << dec << " ";

    string sym(core->Flash->GetSymbolAtAddress(core->PC+1+p2));
    traceOut << sym << " ";
    for (int len = sym.length(); len<30;len++) { traceOut << " " ; }

    int ret= avr_op_BRBC_do( core, p1, p2) ; 
    return ret;
}

char *branch_opcodes_set[8] = {
    "BRCS",
    "BREQ",
    "BRMO",
    "BRVS",
    "BRLT",
    "BRHS",
    "BRTS",
    "BRIE"
};

int avr_op_BRBS_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << branch_opcodes_set[p1] << " ->" << HexShort((core->PC+1+p2)<<1) << dec << " ";

    string sym(core->Flash->GetSymbolAtAddress(core->PC+1+p2));
    traceOut << sym << " ";
    for (int len = sym.length(); len<30;len++) { traceOut << " " ; }

    int ret= avr_op_BRBS_do( core, p1, p2) ; 
    return ret;
}

char *opcodes_bset[8]= {
    "SEC",
    "SEZ",
    "SEN",
    "SEV",
    "SES",
    "SEH",
    "SET",
    "SEI"
};

int avr_op_BSET_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << opcodes_bset[p1] << " ";
    int ret= avr_op_BSET_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_BST_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "BST R" << p1 << ", " << p2 << " ";
    int ret= avr_op_BST_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_CALL_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "CALL " << p1 << " ";
    int ret= avr_op_CALL_do( core, p1, p2) ; 
    return ret;
}

int avr_op_CBI_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "CBI " << HexChar(p1) << dec  << ", " << dec << p2 << " ";
    int ret= avr_op_CBI_do( core, p1, p2) ; 
    return ret;
}

int avr_op_COM_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "COM R" << p1 << " ";
    int ret= avr_op_COM_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_CP_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "CP R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_CP_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_CPC_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "CPC R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_CPC_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_CPI_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "CPI R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret= avr_op_CPI_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_CPSE_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "CPSE R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_CPSE_do( core, p1, p2) ; 
    return ret;
}

int avr_op_DEC_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "DEC R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret= avr_op_DEC_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_EICALL_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "EICALL " ;
    int ret= avr_op_EICALL_do( core, p1, p2) ; 
    return ret;
}

int avr_op_EIJMP_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "EIJMP " ;
    int ret= avr_op_EIJMP_do( core, p1, p2) ; 
    return ret;
}

int avr_op_ELPM_Z_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ELPM R" << p1 << ", Z " ;

    unsigned int Z = ((core->GetRampz() & 0x3f) << 16) +
        (((*(core->R))[31]) << 8) + 
        (*(core->R))[30];

    unsigned int flash_addr = Z;// / 2;

    traceOut << " Flash[0x"<<hex<< (unsigned int) flash_addr<<"] ";

    int ret= avr_op_ELPM_Z_do( core, p1, p2) ; 
    return ret;
}

int avr_op_ELPM_Z_incr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ELPM R" << p1 << ", Z+ ";

    unsigned int Z = ((core->GetRampz() & 0x3f) << 16) +
        (((*(core->R))[31]) << 8) + 
        (*(core->R))[30];

    unsigned int flash_addr = Z; // / 2;

    traceOut << " Flash[0x"<<hex<< (unsigned int) flash_addr<<"] ";



    int ret= avr_op_ELPM_Z_incr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_ELPM_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ELPM ";

    unsigned int Z = ((core->GetRampz() & 0x3f) << 16) +
        (((*(core->R))[31]) << 8) + 
        (*(core->R))[30];

    unsigned int flash_addr = Z; // / 2;

    traceOut << " Flash[0x"<<hex<< (unsigned int) flash_addr<<"] ";


    int ret= avr_op_ELPM_do( core, p1, p2) ; 
    return ret;
}

int avr_op_EOR_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "EOR R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_EOR_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_ESPM_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ESPM??? " << p1 << ", " << p2 << " ";
    int ret= avr_op_ESPM_do( core, p1, p2) ; 
    return ret;
}

int avr_op_FMUL_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "FMUL R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_FMUL_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_FMULS_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "FMULS R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_FMULS_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_FMULSU_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "FMULSU R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_FMULSU_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_ICALL_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ICALL " ;
    int ret= avr_op_ICALL_do( core, p1, p2) ; 
    return ret;
}

int avr_op_IJMP_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "IJMP " ;
    int ret= avr_op_IJMP_do( core, p1, p2) ; 
    return ret;
}

int avr_op_IN_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "IN R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret= avr_op_IN_do( core, p1, p2) ; 
    return ret;
}

int avr_op_INC_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "INC R" << p1 << " ";
    int ret= avr_op_INC_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_JMP_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "JMP ";
    word *MemPtr=(word*)core->Flash->myMemory;
    word offset=MemPtr[(core->PC)+1];         //this is k!
    offset=(offset>>8)+((offset&0xff)<<8);
    traceOut << hex << 2*offset << " ";

    string sym(core->Flash->GetSymbolAtAddress(offset));
    traceOut << sym << " ";
    for (int len = sym.length(); len<30;len++) { traceOut << " " ; }

    int ret= avr_op_JMP_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LDD_Y_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LD R" << p1 << ", Y+"<<p2;
    int ret= avr_op_LDD_Y_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LDD_Z_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LDD R" << p1 << ", Z ";
    int ret= avr_op_LDD_Z_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LDI_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LDI R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret= avr_op_LDI_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LDS_do_trace( AvrDevice *core, int p1, int p2 ) {
    word *MemPtr=(word*)core->Flash->myMemory;
    word offset=MemPtr[(core->PC)+1];         //this is k!
    offset=(offset>>8)+((offset&0xff)<<8);
    traceOut << "LDS R" << p1 << ", " << hex << "0x" << offset << dec  << " ";
    int ret= avr_op_LDS_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LD_X_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LD R" << p1 << ", X ";
    int ret= avr_op_LD_X_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LD_X_decr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LD R" << p1 << ", -X ";
    int ret= avr_op_LD_X_decr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LD_X_incr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LD R" << p1 << ", X+ ";
    int ret= avr_op_LD_X_incr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LD_Y_decr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LD R" << p1 << ", -Y ";
    int ret= avr_op_LD_Y_decr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LD_Y_incr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LD R" << p1 << ", Y+ " ;
    int ret= avr_op_LD_Y_incr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LD_Z_incr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LD R" << p1 << ", Z+ ";
    int ret= avr_op_LD_Z_incr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LD_Z_decr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LD R" << p1 << ", -Z";
    int ret= avr_op_LD_Z_decr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LPM_Z_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LPM_Z ";

    /* Z is R31:R30 */
    int Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];
    //string sym(core->Flash->GetSymbolAtAddressLpm(Z));
    string sym(core->Flash->GetSymbolAtAddress(Z));
    traceOut << "FLASH[" << hex << Z << "," << sym << "] ";


    int ret= avr_op_LPM_Z_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LPM_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LPM R0, Z "; 

    /* Z is R31:R30 */
    int Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];
    //string sym(core->Flash->GetSymbolAtAddressLpm(Z));
    string sym(core->Flash->GetSymbolAtAddress(Z));
    traceOut << "FLASH[" << hex << Z << "," << sym << "] ";

    int ret= avr_op_LPM_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LPM_Z_incr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LPM R" << p1 << ", Z+ " ;
    /* Z is R31:R30 */
    int Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];
    //string sym(core->Flash->GetSymbolAtAddressLpm(Z));
    string sym(core->Flash->GetSymbolAtAddress(Z));
    traceOut << "FLASH[" << hex << Z << "," << sym << "] ";
    int ret= avr_op_LPM_Z_incr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_LSR_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "LSR R" << p1 << " ";
    int ret= avr_op_LSR_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_MOV_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "MOV R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_MOV_do( core, p1, p2) ; 
    return ret;
}

int avr_op_MOVW_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "MOVW R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_MOVW_do( core, p1, p2) ; 
    return ret;
}

int avr_op_MUL_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "MUL R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_MUL_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_MULS_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "MULS R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_MULS_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_MULSU_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "MULSU R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_MULSU_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_NEG_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "NEG R" << p1 <<" ";
    int ret= avr_op_NEG_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_NOP_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "NOP ";
    int ret= avr_op_NOP_do( core, p1, p2) ; 
    return ret;
}

int avr_op_OR_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "OR R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_OR_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_ORI_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ORI R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret= avr_op_ORI_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_OUT_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "OUT " << HexChar(p1) << dec  << ", R" << p2 << " ";
    int ret= avr_op_OUT_do( core, p1, p2) ; 
    return ret;
}

int avr_op_POP_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "POP R" << p1 << " ";
    int ret= avr_op_POP_do( core, p1, p2) ; 
    return ret;
}

int avr_op_PUSH_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "PUSH R" << p1 << " ";
    int ret= avr_op_PUSH_do( core, p1, p2) ; 
    return ret;
}

int avr_op_RCALL_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "RCALL " << hex << ((core->PC+p1+1)<<1) << dec  << " ";
    int ret= avr_op_RCALL_do( core, p1, p2) ; 
    return ret;
}

int avr_op_RET_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "RET " ;
    int ret= avr_op_RET_do( core, p1, p2) ; 
    return ret;
}

int avr_op_RETI_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "RETI ";
    int ret= avr_op_RETI_do( core, p1, p2) ; 
    return ret;
}

int avr_op_RJMP_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "RJMP " << hex << ((core->PC+p1+1)<<1) << dec  << " ";
    int ret= avr_op_RJMP_do( core, p1, p2) ; 
    return ret;
}

int avr_op_ROR_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ROR R" << p1 << " ";
    int ret= avr_op_ROR_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_SBC_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "SBC R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_SBC_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_SBCI_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "SBCI R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret= avr_op_SBCI_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_SBI_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "SBI " << HexChar(p1) << dec  << ", " << dec << p2 << " ";
    int ret= avr_op_SBI_do( core, p1, p2) ; 
    return ret;
}

int avr_op_SBIC_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "SBIC " << HexChar(p1) << dec  << ", " << dec << p2 << " ";
    int ret= avr_op_SBIC_do( core, p1, p2) ; 
    return ret;
}

int avr_op_SBIS_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "SBIS " << HexChar(p1) << dec  << ", " << dec << p2 << " ";
    int ret= avr_op_SBIS_do( core, p1, p2) ; 
    return ret;
}

int avr_op_SBIW_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "SBIW R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret= avr_op_SBIW_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_SBRC_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "SBRC R" << p1 << ", " << p2 << " ";
    int ret= avr_op_SBRC_do( core, p1, p2) ; 
    return ret;
}

int avr_op_SBRS_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "SBRS R" << p1 << ", " << p2 << " ";
    int ret= avr_op_SBRS_do( core, p1, p2) ; 
    return ret;
}

int avr_op_SLEEP_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "SLEEP " ;
    int ret= avr_op_SLEEP_do( core, p1, p2) ; 
    return ret;
}

int avr_op_SPM_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "SPM " ;
    int ret= avr_op_SPM_do( core, p1, p2) ; 
    return ret;
}

int avr_op_STD_Y_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "STD Y+"<<p1 <<", R" << p2 << " ";
    int ret= avr_op_STD_Y_do( core, p1, p2) ; 
    return ret;
}

int avr_op_STD_Z_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "STD Z, R" << p1 << " ";
    int ret= avr_op_STD_Z_do( core, p1, p2) ; 
    return ret;
}

int avr_op_STS_do_trace( AvrDevice *core, int p1, int p2 ) {
    word *MemPtr=(word*)core->Flash->myMemory;
    word offset=MemPtr[(core->PC)+1];         //this is k!
    offset=(offset>>8)+((offset&0xff)<<8);
    traceOut << "STS " << "0x" << hex << offset << dec  << ", R" << p2 << " ";
    int ret= avr_op_STS_do( core, p1, p2) ; 
    return ret;
}

int avr_op_ST_X_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ST X, R" << p1 << " ";
    int ret= avr_op_ST_X_do( core, p1, p2) ; 
    return ret;
}

int avr_op_ST_X_decr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ST -X, R" << p1 << " ";
    int ret= avr_op_ST_X_decr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_ST_X_incr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ST X+, R" << p1 << " ";
    int ret= avr_op_ST_X_incr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_ST_Y_decr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ST -Y, R" << p1 << " ";
    int ret= avr_op_ST_Y_decr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_ST_Y_incr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ST Y+, R" << p1 << " ";
    int ret= avr_op_ST_Y_incr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_ST_Z_decr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ST -Z, R" << p1 << " ";
    int ret= avr_op_ST_Z_decr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_ST_Z_incr_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "ST Z+, R" << p1 << " ";
    int ret= avr_op_ST_Z_incr_do( core, p1, p2) ; 
    return ret;
}

int avr_op_SUB_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "SUB R" << p1 << ", R" << p2 << " ";
    int ret= avr_op_SUB_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_SUBI_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "SUBI R" << p1 << ", " << HexChar(p2) << dec  << " ";
    int ret= avr_op_SUBI_do( core, p1, p2) ; 
    MONSREG;
    return ret;
}

int avr_op_SWAP_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "SWAP R" << p1 << " ";
    int ret= avr_op_SWAP_do( core, p1, p2) ; 
    return ret;
}

int avr_op_WDR_do_trace( AvrDevice *core, int p1, int p2 ) {
    traceOut << "WDR ";
    int ret= avr_op_WDR_do( core, p1, p2) ; 
    return ret;
}

