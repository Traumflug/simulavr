/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Theodore A. Roth, Klaus Rudolph     
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

#include "decoder.h"
#include "hwstack.h"
#include "flash.h"
#include "hwwado.h"
#include "hwsreg.h"
#include "avrerror.h"
#include "ioregs.h"

static int n_bit_unsigned_to_signed(unsigned int val, int n );

static int get_add_carry( byte res, byte rd, byte rr, int b );
static int get_add_overflow( byte res, byte rd, byte rr );
static int get_sub_carry( byte res, byte rd, byte rr, int b );
static int get_sub_overflow( byte res, byte rd, byte rr );
static int get_compare_carry( byte res, byte rd, byte rr, int b );
static int get_compare_overflow( byte res, byte rd, byte rr );

enum decoder_operand_masks {
    /** 2 bit register id  ( R24, R26, R28, R30 ) */
    mask_Rd_2     = 0x0030,
    /** 3 bit register id  ( R16 - R23 ) */
    mask_Rd_3     = 0x0070,
    /** 4 bit register id  ( R16 - R31 ) */
    mask_Rd_4     = 0x00f0,
    /** 5 bit register id  ( R00 - R31 ) */
    mask_Rd_5     = 0x01f0,

    /** 3 bit register id  ( R16 - R23 ) */
    mask_Rr_3     = 0x0007,
    /** 4 bit register id  ( R16 - R31 ) */
    mask_Rr_4     = 0x000f,
    /** 5 bit register id  ( R00 - R31 ) */
    mask_Rr_5     = 0x020f,

    /** for 8 bit constant */
    mask_K_8      = 0x0F0F,
    /** for 6 bit constant */
    mask_K_6      = 0x00CF,

    /** for 7 bit relative address */
    mask_k_7      = 0x03F8,
    /** for 12 bit relative address */
    mask_k_12     = 0x0FFF,
    /** for 22 bit absolute address */
    mask_k_22     = 0x01F1,

    /** register bit select */
    mask_reg_bit  = 0x0007,
    /** status register bit select */
    mask_sreg_bit = 0x0070,
    /** address displacement (q) */
    mask_q_displ  = 0x2C07,

    /** 5 bit register id  ( R00 - R31 ) */
    mask_A_5      = 0x00F8,
    /** 6 bit IO port id */
    mask_A_6      = 0x060F
};

static int get_rd_2( word opcode );
static int get_rd_3( word opcode );
static int get_rd_4( word opcode );
static int get_rd_5( word opcode );
static int get_rr_3( word opcode );
static int get_rr_4( word opcode );
static int get_rr_5( word opcode );
static byte get_K_8( word opcode );
static byte get_K_6( word opcode );
static int get_k_7( word opcode );
static int get_k_12( word opcode );
static int get_k_22( word opcode );
static int get_reg_bit( word opcode );
static int get_sreg_bit( word opcode );
static int get_q( word opcode );
static int get_A_5( word opcode );
static int get_A_6( word opcode );

avr_op_ADC::avr_op_ADC(word opcode, AvrDevice *c): 
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    R2(get_rr_5(opcode)),
    status(c->status) {}

unsigned char avr_op_ADC::GetModifiedR() const {
    return R1;
}
int avr_op_ADC::operator()() { 
    unsigned char rd = core->GetCoreReg(R1);
    unsigned char rr = core->GetCoreReg(R2);
    unsigned char res = rd + rr + status->C;

    status->H = get_add_carry(res, rd, rr, 3);
    status->V = get_add_overflow(res, rd, rr);
    status->N = (res >> 7) & 0x1;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0;
    status->C = get_add_carry(res, rd, rr, 7);

    core->SetCoreReg(R1, res);

    return 1;   //used clocks
}

avr_op_ADD::avr_op_ADD(word opcode, AvrDevice *c): 
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    R2(get_rr_5(opcode)), 
    status(c->status) {}

unsigned char avr_op_ADD::GetModifiedR() const {
    return R1;
}
int avr_op_ADD::operator()() { 
    unsigned char rd = core->GetCoreReg(R1);
    unsigned char rr = core->GetCoreReg(R2);
    unsigned char res = rd + rr;

    status->H = get_add_carry(res, rd, rr, 3);
    status->V = get_add_overflow(res, rd, rr);
    status->N = (res >> 7) & 0x1;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0;
    status->C = get_add_carry(res, rd, rr, 7);

    core->SetCoreReg(R1, res);

    return 1;   //used clocks
}

avr_op_ADIW::avr_op_ADIW(word opcode, AvrDevice *c): 
    DecodedInstruction(c),
    Rl(get_rd_2(opcode)),
    Rh(get_rd_2(opcode) + 1),
    K(get_K_6(opcode)),
    status(c->status) {
    }

unsigned char avr_op_ADIW::GetModifiedR() const {
    return Rl;
}
unsigned char avr_op_ADIW::GetModifiedRHi() const {
    return Rh;
}
int avr_op_ADIW::operator()() {
    word rd = (core->GetCoreReg(Rh) << 8) + core->GetCoreReg(Rl);
    word res = rd + K;
    unsigned char rdh = core->GetCoreReg(Rh);


    status->V = ~(rdh >> 7 & 0x1) & (res >> 15 & 0x1); 
    status->N = (res >> 15) & 0x1;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xffff) == 0;
    status->C = ~(res >> 15 & 0x1) & (rdh >> 7 & 0x1); 

    core->SetCoreReg(Rl, res & 0xff);
    core->SetCoreReg(Rh, res >> 8);

    return 2; 
}

avr_op_AND::avr_op_AND(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    R2(get_rr_5(opcode)),
    status(c->status) {}

int avr_op_AND::operator()() {
    unsigned char res = core->GetCoreReg(R1) & core->GetCoreReg(R2);

    status->V = 0;
    status->N = (res >> 7) & 0x1; 
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0; 

    core->SetCoreReg(R1, res);

    return 1; 
}

avr_op_ANDI::avr_op_ANDI(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_4(opcode)),
    K(get_K_8(opcode)),
    status(c->status) {}

int avr_op_ANDI::operator()() {
    unsigned char rd = core->GetCoreReg(R1);
    unsigned char res = rd & K;

    status->V = 0;
    status->N = (res >> 7) & 0x1; 
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0; 

    core->SetCoreReg(R1, res);
    
    return 1;
}

avr_op_ASR::avr_op_ASR(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    status(c->status) {}

int avr_op_ASR::operator()() {
    unsigned char rd = core->GetCoreReg(R1); 
    unsigned char res = (rd >> 1) + (rd & 0x80);

    status->N = (res >> 7) & 0x1;
    status->C = rd & 0x1;
    status->V = status->N ^ status->C;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0;

    core->SetCoreReg(R1, res);

    return 1;
}


avr_op_BCLR::avr_op_BCLR(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    status(c->status),
    Kbit(get_sreg_bit(opcode)) {}

int avr_op_BCLR::operator()() {
    *status = (*status) & ~(1 << Kbit);
    
    return 1;
}

avr_op_BLD::avr_op_BLD(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    Kbit(get_reg_bit(opcode)),
    status(c->status) {}

int avr_op_BLD::operator()() {
    unsigned char rd = core->GetCoreReg(R1);
    int T = status->T;
    unsigned char res;

    if(T == 0)
        res = rd & ~(1 << Kbit);
    else
        res = rd | (1 << Kbit); 

    core->SetCoreReg(R1, res);

    return 1;
}

avr_op_BRBC::avr_op_BRBC(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    status(c->status),
    bitmask(1 << get_reg_bit(opcode)),
    offset(n_bit_unsigned_to_signed(get_k_7(opcode), 7)) {}

int avr_op_BRBC::operator()() {
    int clks;

    if((bitmask & (*(status))) == 0) {
        core->DebugOnJump();
        core->PC += offset;
        clks = 2;
    } else {
        clks = 1;
    }

    return clks;
}

avr_op_BRBS::avr_op_BRBS(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    status(c->status),
    bitmask(1 << get_reg_bit(opcode)),
    offset(n_bit_unsigned_to_signed(get_k_7(opcode), 7)) {}

int avr_op_BRBS::operator()() {
    int clks;

    if((bitmask & (*(status))) != 0) {
        core->DebugOnJump();
        core->PC += offset;
        clks = 2;
    } else {
        clks = 1;
    }

    return clks;
}

avr_op_BSET::avr_op_BSET(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    status(c->status),
    Kbit(get_sreg_bit(opcode)) {}

int avr_op_BSET::operator()() {
    *(status) = *(status) | 1 << Kbit;
    
    return 1;
}

avr_op_BST::avr_op_BST(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    Kbit(get_reg_bit(opcode)),
    status(c->status) {}

int avr_op_BST::operator()() {
    status->T = ((core->GetCoreReg(R1) & (1 << Kbit)) != 0); 

    return 1;
}

avr_op_CALL::avr_op_CALL(word opcode, AvrDevice *c):
    DecodedInstruction(c, true),
    KH(get_k_22(opcode)) {}

int avr_op_CALL::operator()() 
{
    word K_lsb = core->Flash->ReadMemWord((core->PC + 1) * 2);
    int k = (KH << 16) + K_lsb;
    int clkadd = core->flagXMega ? 1 : 2;
    
    core->stack->m_ThreadList.OnCall();
    core->stack->PushAddr(core->PC + 2);
    core->DebugOnJump();
    core->PC = k - 1;

    return core->PC_size + clkadd;
}

avr_op_CBI::avr_op_CBI(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    ioreg(get_A_5(opcode)),
    Kbit(get_reg_bit(opcode)) {}

int avr_op_CBI::operator()() {
    int clks = (core->flagXMega || core->flagTiny10) ? 1 : 2;
    
    core->SetIORegBit(ioreg, Kbit, false);
    
    return clks;
}

avr_op_COM::avr_op_COM(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    status(c->status) {}

int avr_op_COM::operator()() {
    byte rd  = core->GetCoreReg(R1);
    byte res = 0xff - rd;

    status->N = (res >> 7) & 0x1;
    status->C = 1;
    status->V = 0;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0;

    core->SetCoreReg(R1, res);

    return 1;
}

avr_op_CP::avr_op_CP(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    R2(get_rr_5(opcode)),
    status(c->status) {}

int avr_op_CP::operator()() {
    byte rd  = core->GetCoreReg(R1);
    byte rr  = core->GetCoreReg(R2);
    byte res = rd - rr;

    status->H = get_compare_carry(res, rd, rr, 3);
    status->V = get_compare_overflow(res, rd, rr);
    status->N = (res >> 7) & 0x1;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0;
    status->C = get_compare_carry(res, rd, rr, 7);

    return 1;
}

avr_op_CPC::avr_op_CPC(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    R2(get_rr_5(opcode)),
    status(c->status) {}

int avr_op_CPC::operator()() {
    byte rd  = core->GetCoreReg(R1);
    byte rr  = core->GetCoreReg(R2);
    byte res = rd - rr - status->C;

    status->H = get_compare_carry(res, rd, rr, 3);
    status->V = get_compare_overflow(res, rd, rr);
    status->N = (res >> 7) & 0x1;
    status->S = status->N ^ status->V;
    status->C = get_compare_carry(res, rd, rr, 7);

    /* Previous value remains unchanged when result is 0; cleared otherwise */
    bool Z = (res & 0xff) == 0;
    bool prev_Z = status->Z;
    status->Z = Z && prev_Z;

    return 1;
}


avr_op_CPI::avr_op_CPI(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_4(opcode)),
    K(get_K_8(opcode)), 
    status(c->status) {}

int avr_op_CPI::operator()() {
    byte rd  = core->GetCoreReg(R1);
    byte res = rd - K;

    status->H = get_compare_carry(res, rd, K, 3);
    status->V = get_compare_overflow(res, rd, K);
    status->N = (res >> 7) & 0x1;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0;
    status->C = get_compare_carry(res, rd, K, 7);

    return 1;
}

avr_op_CPSE::avr_op_CPSE(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    R2(get_rr_5(opcode)),
    status(c->status) {}

int avr_op_CPSE::operator()() {
    int skip;
    byte rd = core->GetCoreReg(R1);
    byte rr = core->GetCoreReg(R2);
    int clks;

    if(core->Flash->DecodedMem[core->PC + 1]->IsInstruction2Words())
        skip = 3;
    else
        skip = 2;

    if(rd == rr) {
        core->DebugOnJump();
        core->PC += skip - 1;
        clks = skip;
    } else
        clks = 1;

    return clks;
}

avr_op_DEC::avr_op_DEC(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    status(c->status) {}

int avr_op_DEC::operator()() {
    byte res = core->GetCoreReg(R1) - 1;

    status->N = (res >> 7) & 0x1;
    status->V = res == 0x7f;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0;

    core->SetCoreReg(R1, res);

    return 1;
}

avr_op_EICALL::avr_op_EICALL(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_EICALL::operator()() {
    unsigned new_PC = core->GetRegZ() + (core->eind->GetRegVal() << 16);

    core->stack->m_ThreadList.OnCall();
    core->stack->PushAddr(core->PC + 1);

    core->DebugOnJump();
    core->PC = new_PC;

    return core->flagXMega ? 3 : 4;
}

avr_op_EIJMP::avr_op_EIJMP(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_EIJMP::operator()() {
    core->DebugOnJump();
    core->PC = (core->eind->GetRegVal() << 16) + core->GetRegZ();

    return 2;
}

avr_op_ELPM_Z::avr_op_ELPM_Z(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)) {}

int avr_op_ELPM_Z::operator()() {
    unsigned int Z;
    unsigned char rampz = 0;

    if(core->rampz != NULL)
        rampz = core->rampz->GetRegVal();
    Z = (rampz << 16) + core->GetRegZ();

    core->SetCoreReg(R1, core->Flash->ReadMem(Z ^ 0x1));

    return 3;
}

avr_op_ELPM_Z_incr::avr_op_ELPM_Z_incr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)) {}

int avr_op_ELPM_Z_incr::operator()() {
    unsigned int Z;
    unsigned char rampz = 0;

    if(core->rampz != NULL)
        rampz = core->rampz->GetRegVal();
    Z = (rampz << 16) + core->GetRegZ();

    core->SetCoreReg(R1, core->Flash->ReadMem(Z ^ 0x1));

    /* post increment Z */
    Z++;
    if(core->rampz != NULL)
        core->rampz->SetRegVal(Z >> 16);
    core->SetCoreReg(30, Z & 0xff);
    core->SetCoreReg(31, (Z >> 8) & 0xff);

    return 3;
}

avr_op_ELPM::avr_op_ELPM(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_ELPM::operator()() {
    unsigned char rampz = 0;

    if(core->rampz != NULL)
        rampz = core->rampz->GetRegVal();
    unsigned Z = (rampz << 16) + core->GetRegZ();

    core->SetCoreReg(0, core->Flash->ReadMem(Z ^ 0x1));
    
    return 3;
}

avr_op_EOR::avr_op_EOR(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    R2(get_rr_5(opcode)),
    status(c->status) {}

int avr_op_EOR::operator()() {
    byte rd = core->GetCoreReg(R1); 
    byte rr = core->GetCoreReg(R2);
    byte res = rd ^ rr;

    status->V = 0;
    status->N = (res >> 7) & 0x1;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0;

    core->SetCoreReg(R1, res);

    return 1;
}

avr_op_ESPM::avr_op_ESPM(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_ESPM::operator()() {
    unsigned char xaddr = 0;
    int cycles = 1;
    if(core->rampz != NULL)
        xaddr = core->rampz->GetRegVal();
    if(core->spmRegister != NULL) {
        unsigned int Z = core->GetRegZ();
        unsigned int D = core->GetCoreReg(0) + (core->GetCoreReg(1) << 8);
        cycles += core->spmRegister->SPM_action(D, xaddr, Z);
        // increment Z register
        Z++;
        core->SetCoreReg(30, Z & 0xff);
        core->SetCoreReg(31, (Z >> 8) & 0xff);
        if(core->rampz != NULL)
            core->rampz->SetRegVal(Z >> 16);
    }
    return cycles;
}

avr_op_FMUL::avr_op_FMUL(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_3(opcode)),
    Rr(get_rr_3(opcode)),
    status(c->status) {}

int avr_op_FMUL::operator()() {
    byte rd = core->GetCoreReg(Rd);
    byte rr = core->GetCoreReg(Rr);

    word resp = rd * rr;
    word res = resp << 1;

    status->Z = (res & 0xffff) == 0;
    status->C= (resp >> 15) & 0x1;

    /* result goes in R1:R0 */
    core->SetCoreReg(0, res & 0xff);
    core->SetCoreReg(1, (res >> 8) & 0xff);

    return 2;
}


avr_op_FMULS::avr_op_FMULS(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_3(opcode)),
    Rr(get_rr_3(opcode)),
    status(c->status) {}

int avr_op_FMULS::operator()() {
    sbyte rd = core->GetCoreReg(Rd); 
    sbyte rr = core->GetCoreReg(Rr);

    word resp = rd * rr;
    word res = resp << 1;

    status->Z = (res & 0xffff) == 0;
    status->C = (resp >> 15) & 0x1;

    /* result goes in R1:R0 */
    core->SetCoreReg(0, res & 0xff);
    core->SetCoreReg(1, (res >> 8) & 0xff);

    return 2;
}


avr_op_FMULSU::avr_op_FMULSU(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_3(opcode)),
    Rr(get_rr_3(opcode)),
    status(c->status) {}

int avr_op_FMULSU::operator()() {
    sbyte rd = core->GetCoreReg(Rd);
    byte rr = core->GetCoreReg(Rr);

    word resp = rd * rr;
    word res = resp << 1;

    status->Z = (res & 0xffff) == 0;
    status->C = (resp >> 15) & 0x1;

    /* result goes in R1:R0 */
    core->SetCoreReg(0, res & 0xff);
    core->SetCoreReg(1, (res >> 8) & 0xff);
    return 2;
}

avr_op_ICALL::avr_op_ICALL(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_ICALL::operator()() {
    unsigned int pc = core->PC;
    /* Z is R31:R30 */
    unsigned int new_pc = core->GetRegZ();

    core->stack->m_ThreadList.OnCall();
    core->stack->PushAddr(pc + 1);

    core->DebugOnJump();
    core->PC = new_pc - 1;

    return core->PC_size + (core->flagXMega ? 0 : 1);
}

avr_op_IJMP::avr_op_IJMP(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_IJMP::operator()() {
    int new_pc = core->GetRegZ();
    
    core->DebugOnJump();
    core->PC = new_pc - 1;

    return 2;
}

avr_op_IN::avr_op_IN(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    ioreg(get_A_6(opcode)) {}

int avr_op_IN::operator()() {
    core->SetCoreReg(R1, core->GetIOReg(ioreg));

    return 1;
}

avr_op_INC::avr_op_INC(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    status(c->status) {}

int avr_op_INC::operator()() {
    byte rd  = core->GetCoreReg(R1);
    byte res = rd + 1;

    status->N = (res >> 7) & 0x1;
    status->V = rd == 0x7f;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0;

    core->SetCoreReg(R1, res);

    return 1;
}

avr_op_JMP::avr_op_JMP(word opcode, AvrDevice *c):
    DecodedInstruction(c, true),
    K(get_k_22(opcode)) {}

int avr_op_JMP::operator()() {
    word K_lsb = core->Flash->ReadMemWord((core->PC + 1) * 2);
    core->DebugOnJump();
    core->PC = (K << 16) + K_lsb - 1;
    return 3;
}

avr_op_LDD_Y::avr_op_LDD_Y(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)),
    K(get_q(opcode)) {}

int avr_op_LDD_Y::operator()() {
    /* Y is R29:R28 */
    word Y = core->GetRegY();

    core->SetCoreReg(Rd, core->GetRWMem(Y + K));
    
    return ((core->flagXMega || core->flagTiny10) && K == 0) ? 1 : 2;
}

avr_op_LDD_Z::avr_op_LDD_Z(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)),
    K(get_q(opcode)) {}

int avr_op_LDD_Z::operator()() {
    /* Z is R31:R30 */
    word Z = core->GetRegZ();

    core->SetCoreReg(Rd, core->GetRWMem(Z + K));

    return ((core->flagXMega || core->flagTiny10) && K == 0) ? 1 : 2;
}

avr_op_LDI::avr_op_LDI(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_4(opcode)),
    K(get_K_8(opcode)) {}

unsigned char avr_op_LDI::GetModifiedR() const {
    return R1;
}
int avr_op_LDI::operator()() { 
    core->SetCoreReg(R1, K);

    return 1;
}

avr_op_LDS::avr_op_LDS(word opcode, AvrDevice *c):
    DecodedInstruction(c, true),
    R1(get_rd_5(opcode)) {}

int avr_op_LDS::operator()() {
    /* Get data at k in current data segment and put into Rd */
    word offset = core->Flash->ReadMemWord((core->PC + 1) * 2);
    
    core->SetCoreReg(R1, core->GetRWMem(offset));
    core->PC++;
    
    return 2;
}

avr_op_LD_X::avr_op_LD_X(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)) {}

int avr_op_LD_X::operator()() {
    /* X is R27:R26 */
    word X = core->GetRegX();

    core->SetCoreReg(Rd, core->GetRWMem(X));
    
    return (core->flagXMega || core->flagTiny10) ? 1 : 2;
}

avr_op_LD_X_decr::avr_op_LD_X_decr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)) {}

int avr_op_LD_X_decr::operator()() {
    /* X is R27:R26 */
    word X = core->GetRegX();
    if (Rd == 26 || Rd == 27)
        avr_error( "Result of operation is undefined" );

    /* Perform pre-decrement */
    X--;
    core->SetCoreReg(Rd, core->GetRWMem(X));
    core->SetCoreReg(26, X & 0xff);
    core->SetCoreReg(27, (X >> 8) & 0xff);

    return core->flagTiny10 ? 3 : 2;
}

avr_op_LD_X_incr::avr_op_LD_X_incr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)) {}

int avr_op_LD_X_incr::operator()() {
    /* X is R27:R26 */
    word X = core->GetRegX();
    if (Rd == 26 || Rd == 27)
       avr_error( "Result of operation is undefined" );

    /* Perform post-increment */
    core->SetCoreReg(Rd, core->GetRWMem(X));
    X++;
    core->SetCoreReg(26, X & 0xff);
    core->SetCoreReg(27, (X >> 8) & 0xff);

    return core->flagXMega ? 1 : 2;
}

avr_op_LD_Y_decr::avr_op_LD_Y_decr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)) {}

int avr_op_LD_Y_decr::operator()() {
    /* Y is R29:R28 */
    word Y = core->GetRegY();
    if (Rd == 28 || Rd == 29)
        avr_error( "Result of operation is undefined" );

    /* Perform pre-decrement */
    Y--;
    core->SetCoreReg(Rd, core->GetRWMem(Y));
    core->SetCoreReg(28, Y & 0xff);
    core->SetCoreReg(29, (Y >> 8) & 0xff);

    return core->flagTiny10 ? 3 : 2;
}

avr_op_LD_Y_incr::avr_op_LD_Y_incr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)) {}

int avr_op_LD_Y_incr::operator()() {
    /* Y is R29:R28 */
    word Y = core->GetRegY();
    if (Rd == 28 || Rd == 29)
        avr_error( "Result of operation is undefined" );

    /* Perform post-increment */
    core->SetCoreReg(Rd, core->GetRWMem(Y));
    Y++;
    core->SetCoreReg(28, Y & 0xff);
    core->SetCoreReg(29, (Y >> 8) & 0xff);

    return core->flagXMega ? 1 : 2;
}

avr_op_LD_Z_incr::avr_op_LD_Z_incr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)) {}

int avr_op_LD_Z_incr::operator()() {
    /* Z is R31:R30 */
    word Z = core->GetRegZ();
    if (Rd == 30 || Rd == 31)
        avr_error( "Result of operation is undefined" );

    /* Perform post-increment */
    core->SetCoreReg(Rd, core->GetRWMem(Z));
    Z++;
    core->SetCoreReg(30, Z & 0xff);
    core->SetCoreReg(31, (Z >> 8) & 0xff);

    return core->flagXMega ? 1 : 2;
}

avr_op_LD_Z_decr::avr_op_LD_Z_decr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)) {}

int avr_op_LD_Z_decr::operator()() {
    /* Z is R31:R30 */
    word Z = core->GetRegZ();
    if (Rd == 30 || Rd == 31)
        avr_error( "Result of operation is undefined" );

    /* Perform pre-decrement */
    Z--;
    core->SetCoreReg(Rd, core->GetRWMem(Z));
    core->SetCoreReg(30, Z & 0xff);
    core->SetCoreReg(31, (Z >> 8) & 0xff);

    return core->flagTiny10 ? 3 : 2;
}

avr_op_LPM_Z::avr_op_LPM_Z(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)) {}

int  avr_op_LPM_Z::operator()() {
    /* Z is R31:R30 */
    word Z = core->GetRegZ();

    Z ^= 0x0001;
    core->SetCoreReg(Rd , core->Flash->ReadMem(Z));

    return 3;
}

avr_op_LPM::avr_op_LPM(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_LPM::operator()() {
    /* Z is R31:R30 */
    word Z = core->GetRegZ();
    
    Z ^= 0x0001;
    core->SetCoreReg(0 , core->Flash->ReadMem(Z));

    return 3;
}

avr_op_LPM_Z_incr::avr_op_LPM_Z_incr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)) {}

int avr_op_LPM_Z_incr::operator()() {
    /* Z is R31:R30 */
    word Z = core->GetRegZ();

    core->SetCoreReg(Rd , core->Flash->ReadMem(Z ^ 0x0001));

    Z++;
    core->SetCoreReg(30, Z & 0xff);
    core->SetCoreReg(31, (Z >> 8) & 0xff);

    return 3;
}

avr_op_LSR::avr_op_LSR(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)),
    status(c->status) {}

int avr_op_LSR::operator()() {
    byte rd = core->GetCoreReg(Rd); 

    byte res = (rd >> 1) & 0x7f;

    status->C = rd & 0x1;
    status->N = 0;
    status->V = status->N ^ status->C;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0;

    core->SetCoreReg(Rd, res);

    return 1;
}

avr_op_MOV::avr_op_MOV(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    R2(get_rr_5(opcode)) {}

int avr_op_MOV::operator()() {
    core->SetCoreReg(R1, core->GetCoreReg(R2));
    return 1;
}

avr_op_MOVW::avr_op_MOVW(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd((get_rd_4(opcode) - 16) << 1),
    Rs((get_rr_4(opcode) - 16) << 1) {}

int avr_op_MOVW::operator()() {
    core->SetCoreReg(Rd, core->GetCoreReg(Rs));
    core->SetCoreReg(Rd + 1, core->GetCoreReg(Rs + 1));

    return 1;
}

avr_op_MUL::avr_op_MUL(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)),
    Rr(get_rr_5(opcode)),
    status(c->status) {}

int avr_op_MUL::operator()() {
    byte rd = core->GetCoreReg(Rd);
    byte rr = core->GetCoreReg(Rr);

    word res = rd * rr;

    status->Z = (res & 0xffff) == 0;
    status->C = (res >> 15) & 0x1;

    /* result goes in R1:R0 */
    core->SetCoreReg(0, res & 0xff);
    core->SetCoreReg(1, (res >> 8) & 0xff);

    return 2;
}

avr_op_MULS::avr_op_MULS(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_4(opcode)),
    Rr(get_rr_4(opcode)),
    status(c->status) {}

int avr_op_MULS::operator()() {
    sbyte rd = (sbyte)core->GetCoreReg(Rd);
    sbyte rr = (sbyte)core->GetCoreReg(Rr);

    sword res = rd * rr;

    status->Z = (res & 0xffff) == 0;
    status->C = (res >> 15) & 0x1;

    /* result goes in R1:R0 */
    core->SetCoreReg(0, res & 0xff);
    core->SetCoreReg(1, (res >> 8) & 0xff);

    return 2;
}

avr_op_MULSU::avr_op_MULSU(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_3(opcode)),
    Rr(get_rr_3(opcode)),
    status(c->status) {}

int avr_op_MULSU::operator()() {
    sbyte rd = (sbyte)core->GetCoreReg(Rd);
    byte rr = core->GetCoreReg(Rr);

    sword res = rd * rr;

    status->Z = (res & 0xffff) == 0;
    status->C = (res >> 15) & 0x1;


    /* result goes in R1:R0 */
    core->SetCoreReg(0, res & 0xff);
    core->SetCoreReg(1, (res >> 8) & 0xff);

    return 2;
}

avr_op_NEG::avr_op_NEG(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)),
    status(c->status) {}

int avr_op_NEG::operator()() {
    byte rd  = core->GetCoreReg(Rd);
    byte res = (0x0 - rd) & 0xff;

    status->H = ((res >> 3) | (rd >> 3)) & 0x1;
    status->V = res == 0x80;
    status->N = (res >> 7) & 0x1;
    status->S = status->N ^ status->V;
    status->Z = res == 0x0;
    status->C = res != 0x0;

    core->SetCoreReg(Rd, res);

    return 1;
}

avr_op_NOP::avr_op_NOP(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_NOP::operator()() {
    return 1;
}

avr_op_OR::avr_op_OR(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    Rd(get_rd_5(opcode)),
    Rr(get_rr_5(opcode)),
    status(c->status) {}

int avr_op_OR::operator()() {
    byte res = core->GetCoreReg(Rd) | core->GetCoreReg(Rr);

    status->V = 0;
    status->N = (res >> 7) & 0x1;
    status->S = status->N ^ status->V;
    status->Z = res == 0x0;

    core->SetCoreReg(Rd, res);

    return 1;
}

avr_op_ORI::avr_op_ORI(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_4(opcode)),
    K(get_K_8(opcode)),
    status(c->status) {}

int avr_op_ORI::operator()() {
    byte res = core->GetCoreReg(R1) | K;

    status->V = 0;
    status->N = (res >> 7) & 0x1;
    status->S = status->N ^ status->V;
    status->Z = res == 0x0;

    core->SetCoreReg(R1, res);

    return 1;
}

avr_op_OUT::avr_op_OUT(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    ioreg(get_A_6(opcode)),
    R1(get_rd_5(opcode)) {}

int avr_op_OUT::operator()() {
    core->SetIOReg(ioreg, core->GetCoreReg(R1));

    return 1;
}

avr_op_POP::avr_op_POP(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)) {}

int avr_op_POP::operator()() {
    core->SetCoreReg(R1, core->stack->Pop());

    return 2;
}

avr_op_PUSH::avr_op_PUSH(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)) {}

int avr_op_PUSH::operator()() {
    core->stack->Push(core->GetCoreReg(R1));

    return core->flagXMega ? 1 : 2;
}

avr_op_RCALL::avr_op_RCALL(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    K(n_bit_unsigned_to_signed(get_k_12(opcode), 12)) {}

int avr_op_RCALL::operator()() {
    core->stack->PushAddr(core->PC + 1);
    core->stack->m_ThreadList.OnCall();
    core->DebugOnJump();
    core->PC += K;
    core->PC &= (core->Flash->GetSize() - 1) >> 1;

    if(core->flagTiny10)
        return 4;
    return core->PC_size + (core->flagXMega ? 0 : 1);
    
}

avr_op_RET::avr_op_RET(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_RET::operator()() {
    core->PC = core->stack->PopAddr() - 1;

    return core->PC_size + 2;
}

avr_op_RETI::avr_op_RETI(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    status(c->status) {}

int avr_op_RETI::operator()() {
    core->PC = core->stack->PopAddr() - 1;
    status->I = 1;

    return core->PC_size + 2;
}

avr_op_RJMP::avr_op_RJMP(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    K(n_bit_unsigned_to_signed(get_k_12(opcode), 12)) {}

int avr_op_RJMP::operator()() {
    core->DebugOnJump();
    core->PC += K;
    core->PC &= (core->Flash->GetSize() - 1) >> 1;

    return 2;
}

avr_op_ROR::avr_op_ROR(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    status(c->status) {}

int avr_op_ROR::operator()() {
    byte rd = core->GetCoreReg(R1);

    byte res = (rd >> 1) | ((status->C << 7) & 0x80);

    status->C = rd & 0x1;
    status->N = (res >> 7) & 0x1;
    status->V = status->N ^ status->C;
    status->S = status->N ^ status->V;
    status->Z = res == 0;

    core->SetCoreReg(R1, res);

    return 1;
}


avr_op_SBC::avr_op_SBC(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    R2(get_rr_5(opcode)),
    status(c->status) {}

unsigned char avr_op_SBC::GetModifiedR() const {
    return R1;
}
int avr_op_SBC::operator()() {
    byte rd = core->GetCoreReg(R1);
    byte rr = core->GetCoreReg(R2);

    byte res = rd - rr - status->C;

    status->H = get_sub_carry(res, rd, rr, 3);
    status->V = get_sub_overflow(res, rd, rr);
    status->N = (res >> 7) & 0x1;
    status->S = status->N ^ status->V;
    status->C = get_sub_carry(res, rd, rr, 7);

    if((res & 0xff) != 0)
        status->Z = 0;

    core->SetCoreReg(R1, res);

    return 1;
}

avr_op_SBCI::avr_op_SBCI(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_4(opcode)),
    K(get_K_8(opcode)),
    status(c->status) {}

unsigned char avr_op_SBCI::GetModifiedR() const {
    return R1;
}
int avr_op_SBCI::operator()() {
    byte rd = core->GetCoreReg(R1);

    byte res = rd - K - status->C;

    status->H = get_sub_carry(res, rd, K, 3);
    status->V = get_sub_overflow(res, rd, K);
    status->N = (res >> 7) & 0x1;
    status->S = status->N ^ status->V;
    status->C = get_sub_carry(res, rd, K, 7);

    if((res & 0xff) != 0)
        status->Z = 0;

    core->SetCoreReg(R1, res);

    return 1;
}

avr_op_SBI::avr_op_SBI(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    ioreg(get_A_5(opcode)),
    Kbit(get_reg_bit(opcode)) {}

int avr_op_SBI::operator()() {
    int clks = (core->flagXMega || core->flagTiny10) ? 1 : 2;
    
    core->SetIORegBit(ioreg, Kbit, true);
    
    return clks;
}

avr_op_SBIC::avr_op_SBIC(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    ioreg(get_A_5(opcode)),
    Kbit(get_reg_bit(opcode)) {}

int avr_op_SBIC::operator()() {
    int skip, clks;

    if(core->Flash->DecodedMem[core->PC + 1]->IsInstruction2Words())
        skip = 3;
    else
        skip = 2;

    if((core->GetIOReg(ioreg) & (1 << Kbit)) == 0) {
        core->DebugOnJump();
        core->PC += skip - 1;
        clks = skip;
    } else
        clks = 1;

    if(core->flagXMega)
        clks++;
    
    return clks;
}

avr_op_SBIS::avr_op_SBIS(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    ioreg(get_A_5(opcode)),
    Kbit(get_reg_bit(opcode)) {}

int avr_op_SBIS::operator()() {
    int skip, clks;

    if(core->Flash->DecodedMem[core->PC + 1]->IsInstruction2Words())
        skip = 3;
    else
        skip = 2;

    if((core->GetIOReg(ioreg) & (1 << Kbit)) != 0) {
        core->DebugOnJump();
        core->PC += skip - 1;
        clks = skip;
    } else
        clks = 1;

    if(core->flagXMega)
        clks++;
    
    return clks;
}


avr_op_SBIW::avr_op_SBIW(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_2(opcode)),
    K(get_K_6(opcode)),
    status(c->status) {}

unsigned char avr_op_SBIW::GetModifiedR() const {
    return R1;
}
unsigned char avr_op_SBIW::GetModifiedRHi() const {
    return R1 + 1;
}
int avr_op_SBIW::operator()() {
    byte rdl = core->GetCoreReg(R1);
    byte rdh = core->GetCoreReg(R1 + 1);

    word rd = (rdh << 8) + rdl;
    word res = rd - K;

    status->V = (rdh >> 7 & 0x1) & ~(res >> 15 & 0x1);
    status->N = (res >> 15) & 0x1;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xffff) == 0;
    status->C = (res >> 15 & 0x1) & ~(rdh >> 7 & 0x1);

    core->SetCoreReg(R1, res & 0xff);
    core->SetCoreReg(R1 + 1, (res >> 8) & 0xff);

    return 2;
}

avr_op_SBRC::avr_op_SBRC(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    Kbit(get_reg_bit(opcode)) {}

int avr_op_SBRC::operator()() {
    int skip, clks;

    if(core->Flash->DecodedMem[core->PC + 1]->IsInstruction2Words())
        skip = 3;
    else
        skip = 2;

    if((core->GetCoreReg(R1) & (1 << Kbit)) == 0) {
        core->DebugOnJump();
        core->PC += skip - 1;
        clks = skip;
    } else
        clks = 1;

    return clks;
}

avr_op_SBRS::avr_op_SBRS(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    Kbit(get_reg_bit(opcode)) {}

int avr_op_SBRS::operator()() {
    int skip, clks;

    if(core->Flash->DecodedMem[core->PC + 1]->IsInstruction2Words())
        skip = 3;
    else
        skip = 2;

    if((core->GetCoreReg(R1) & (1 << Kbit)) != 0) {
        core->DebugOnJump();
        core->PC += skip - 1;
        clks = skip;
    } else
        clks = 1;

    return clks;
}

avr_op_SLEEP::avr_op_SLEEP(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_SLEEP::operator()() {
#if 0  //TODO
    MCUCR *mcucr = (MCUCR *)avr_core_get_vdev_by_name( core, "MCUCR" );

    if (mcucr == NULL)
        avr_error( "MCUCR register not installed" );

    /* See if sleep mode is enabled */
    if ( mcucr_get_bit(mcucr, bit_SE) )
    {
        if ( mcucr_get_bit(mcucr, bit_SM) == 0 )
        {
            /* Idle Mode */
            avr_core_set_sleep_mode( core, SLEEP_MODE_IDLE);
        }
        else
        {
            /* Power Down Mode */
            avr_core_set_sleep_mode( core, SLEEP_MODE_PWR_DOWN);
        }
    }
#endif
    return 1;
}

avr_op_SPM::avr_op_SPM(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_SPM::operator()() {
    unsigned char xaddr = 0;
    int cycles = 1;
    if(core->rampz != NULL)
        xaddr = core->rampz->GetRegVal();
    if(core->spmRegister != NULL) {
        unsigned int Z = core->GetRegZ();
        unsigned int D = core->GetCoreReg(0) + (core->GetCoreReg(1) << 8);
        cycles += core->spmRegister->SPM_action(D, xaddr, Z);
    }
    return cycles;
}

avr_op_STD_Y::avr_op_STD_Y(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    K(get_q(opcode)) {}

int avr_op_STD_Y::operator()() {
    /* Y is R29:R28 */
    unsigned int Y = core->GetRegY();

    core->SetRWMem(Y + K, core->GetCoreReg(R1));

    return (K == 0 && (core->flagXMega || core->flagTiny10)) ? 1 : 2;
}

avr_op_STD_Z::avr_op_STD_Z(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    K(get_q(opcode)) {}

int avr_op_STD_Z::operator()() {
    /* Z is R31:R30 */
    int Z = core->GetRegZ();

    core->SetRWMem(Z + K, core->GetCoreReg(R1));

    return (K == 0 && (core->flagXMega || core->flagTiny10)) ? 1 : 2;
}

avr_op_STS::avr_op_STS(word opcode, AvrDevice *c):
    DecodedInstruction(c, true),
    R1(get_rd_5(opcode)) {}

int avr_op_STS::operator()() {
    /* Get data at k in current data segment and put into Rd */
    word k = core->Flash->ReadMemWord((core->PC + 1) * 2);

    core->SetRWMem(k, core->GetCoreReg(R1));
    core->PC++;

    return 2;
}

avr_op_ST_X::avr_op_ST_X(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)) {}

int avr_op_ST_X::operator()() {
    /* X is R27:R26 */
    word X = core->GetRegX();
    
    core->SetRWMem(X, core->GetCoreReg(R1));

    return (core->flagXMega || core->flagTiny10) ? 1 : 2;
}

avr_op_ST_X_decr::avr_op_ST_X_decr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)) {}

int avr_op_ST_X_decr::operator()() {
    /* X is R27:R26 */
    word X = core->GetRegX();
    if (R1 == 26 || R1 == 27)
        avr_error( "Result of operation is undefined" );

    /* Perform pre-decrement */
    X--;
    core->SetCoreReg(26, X & 0xff);
    core->SetCoreReg(27, (X >> 8) & 0xff);
    core->SetRWMem(X, core->GetCoreReg(R1));

    return 2;
}

avr_op_ST_X_incr::avr_op_ST_X_incr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)) {}

int avr_op_ST_X_incr::operator()() {
    /* X is R27:R26 */
    word X = core->GetRegX();
    if (R1 == 26 || R1 == 27)
        avr_error( "Result of operation is undefined" );

    core->SetRWMem(X, core->GetCoreReg(R1));

    /* Perform post-increment */
    X++;
    core->SetCoreReg(26, X & 0xff);
    core->SetCoreReg(27, (X >> 8) & 0xff);

    return (core->flagXMega || core->flagTiny10) ? 1 : 2;
}

avr_op_ST_Y_decr::avr_op_ST_Y_decr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)) {}

int avr_op_ST_Y_decr::operator()() {
    /* Y is R29:R28 */
    word Y = core->GetRegY();
    if (R1 == 28 || R1 == 29)
        avr_error( "Result of operation is undefined" );
 
    /* Perform pre-decrement */
    Y--;
    core->SetCoreReg(28, Y & 0xff);
    core->SetCoreReg(29, (Y >> 8) & 0xff);
    core->SetRWMem(Y, core->GetCoreReg(R1));

    return 2;
}

avr_op_ST_Y_incr::avr_op_ST_Y_incr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)) {}

int avr_op_ST_Y_incr::operator()() {
    /* Y is R29:R28 */
    word Y = core->GetRegY();
    if (R1 == 28 || R1 == 29)
        avr_error( "Result of operation is undefined" );

    core->SetRWMem(Y, core->GetCoreReg(R1));

    /* Perform post-increment */
    Y++;
    core->SetCoreReg(28, Y & 0xff);
    core->SetCoreReg(29, (Y >> 8) & 0xff);

    return (core->flagXMega || core->flagTiny10) ? 1 : 2;
}

avr_op_ST_Z_decr::avr_op_ST_Z_decr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)) {}

int avr_op_ST_Z_decr::operator()() {
    /* Z is R31:R30 */
    word Z = core->GetRegZ();
    if (R1 == 30 || R1 == 31)
        avr_error( "Result of operation is undefined" );
 
    /* Perform pre-decrement */
    Z--;
    core->SetCoreReg(30, Z & 0xff);
    core->SetCoreReg(31, (Z >> 8) & 0xff);
    core->SetRWMem(Z, core->GetCoreReg(R1));

    return 2;
}

avr_op_ST_Z_incr::avr_op_ST_Z_incr(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)) {}

int avr_op_ST_Z_incr::operator()() {
    /* Z is R31:R30 */
    word Z = core->GetRegZ();
    if (R1 == 30 || R1 == 31)
        avr_error( "Result of operation is undefined" );

    core->SetRWMem(Z, core->GetCoreReg(R1));

    /* Perform post-increment */
    Z++;
    core->SetCoreReg(30, Z & 0xff);
    core->SetCoreReg(31, (Z >> 8) & 0xff);

    return (core->flagXMega || core->flagTiny10) ? 1 : 2;
}

avr_op_SUB::avr_op_SUB(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)),
    R2(get_rr_5(opcode)),
    status(c->status) {}

unsigned char avr_op_SUB::GetModifiedR() const {
    return R1;
}
int avr_op_SUB::operator()() {
    byte rd = core->GetCoreReg(R1);
    byte rr = core->GetCoreReg(R2);

    byte res = rd - rr;

    status->H = get_sub_carry(res, rd, rr, 3);
    status->V = get_sub_overflow(res, rd, rr);
    status->N = (res >> 7) & 0x1;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0;
    status->C = get_sub_carry(res, rd, rr, 7);
    
    core->SetCoreReg(R1, res);

    return 1;
}

avr_op_SUBI::avr_op_SUBI(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_4(opcode)),
    status(c->status),
    K(get_K_8(opcode)) {}

unsigned char avr_op_SUBI::GetModifiedR() const {
    return R1;
}
int avr_op_SUBI::operator()() {
    byte rd = core->GetCoreReg(R1);
    byte res = rd - K;

    status->H = get_sub_carry(res, rd, K, 3);
    status->V = get_sub_overflow(res, rd, K);
    status->N = (res >> 7) & 0x1;
    status->S = status->N ^ status->V;
    status->Z = (res & 0xff) == 0;
    status->C = get_sub_carry(res, rd, K, 7);

    core->SetCoreReg(R1, res);

    return 1;
}

avr_op_SWAP::avr_op_SWAP(word opcode, AvrDevice *c):
    DecodedInstruction(c),
    R1(get_rd_5(opcode)) {}

int avr_op_SWAP::operator()() {
    byte rd = core->GetCoreReg(R1);
    byte res = ((rd << 4) & 0xf0) | ((rd >> 4) & 0x0f);

    core->SetCoreReg(R1, res);
    
    return 1;
}

avr_op_WDR::avr_op_WDR(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_WDR::operator()() {
    if(core->wado != NULL)
        core->wado->Wdr();

    return 1;
}

avr_op_BREAK::avr_op_BREAK(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_BREAK::operator()() {
    return BREAK_POINT+1;
}

avr_op_ILLEGAL::avr_op_ILLEGAL(word opcode, AvrDevice *c):
    DecodedInstruction(c) {}

int avr_op_ILLEGAL::operator()() {
    avr_error("Illegal opcode '%02x %02x' executed at PC=0x%x (%d)! Simulation terminated!",
        core->Flash->myMemory[core->PC*2+1], core->Flash->myMemory[core->PC*2], core->PC*2, core->PC);
    return 0;
}

static int get_add_carry( byte res, byte rd, byte rr, int b )
{
    byte resb = res >> b & 0x1;
    byte rdb  = rd  >> b & 0x1;
    byte rrb  = rr  >> b & 0x1;
    return (rdb & rrb) | (rrb & ~resb) | (~resb & rdb);
}

static int get_add_overflow( byte res, byte rd, byte rr )
{
    byte res7 = res >> 7 & 0x1;
    byte rd7  = rd  >> 7 & 0x1;
    byte rr7  = rr  >> 7 & 0x1;
    return (rd7 & rr7 & ~res7) | (~rd7 & ~rr7 & res7);
}

static int get_sub_carry( byte res, byte rd, byte rr, int b )
{
    byte resb = res >> b & 0x1;
    byte rdb  = rd  >> b & 0x1;
    byte rrb  = rr  >> b & 0x1;
    return (~rdb & rrb) | (rrb & resb) | (resb & ~rdb);
}

static int get_sub_overflow( byte res, byte rd, byte rr )
{
    byte res7 = res >> 7 & 0x1;
    byte rd7  = rd  >> 7 & 0x1;
    byte rr7  = rr  >> 7 & 0x1;
    return (rd7 & ~rr7 & ~res7) | (~rd7 & rr7 & res7);
}

static int get_compare_carry( byte res, byte rd, byte rr, int b )
{
    byte resb = res >> b & 0x1;
    byte rdb  = rd  >> b & 0x1;
    byte rrb  = rr  >> b & 0x1;
    return (~rdb & rrb) | (rrb & resb) | (resb & ~rdb);
}

static int get_compare_overflow( byte res, byte rd, byte rr )
{
    byte res7 = res >> 7 & 0x1;
    byte rd7  = rd  >> 7 & 0x1;
    byte rr7  = rr  >> 7 & 0x1;
    /* The atmel data sheet says the second term is ~rd7 for CP
     * but that doesn't make any sense. You be the judge. */
    return (rd7 & ~rr7 & ~res7) | (~rd7 & rr7 & res7);
}
static int n_bit_unsigned_to_signed( unsigned int val, int n ) 
{
    /* Convert n-bit unsigned value to a signed value. */
    unsigned int mask;

    if ( (val & (1 << (n-1))) == 0)
        return (int)val;

    /* manually calculate two's complement */
    mask = (1 << n) - 1; 
    return -1 * ((~val & mask) + 1);
}

/* Get register R24, 26, 28 or 30 - for ADIW and few other */
static int get_rd_2( word opcode )
{
    int reg = ((opcode & mask_Rd_2) >> 4) & 0x3;
    return (reg * 2) + 24;
}

static int get_rd_3( word opcode )
{
    int reg = opcode & mask_Rd_3;
    return ((reg >> 4) & 0x7) + 16;
}

static int get_rd_4( word opcode )
{
    int reg = opcode & mask_Rd_4;
    return ((reg >> 4) & 0xf) + 16;
}

static int get_rd_5( word opcode )
{
    int reg = opcode & mask_Rd_5;
    return ((reg >> 4) & 0x1f);
}

static int get_rr_3( word opcode )
{
    return (opcode & mask_Rr_3) + 16;
}

static int get_rr_4( word opcode )
{
    return (opcode & mask_Rr_4) + 16;
}

static int get_rr_5( word opcode )
{
    int reg = opcode & mask_Rr_5;
    return (reg & 0xf) + ((reg >> 5) & 0x10);
}

static byte get_K_8( word opcode )
{
    int K = opcode & mask_K_8;
    return ((K >> 4) & 0xf0) + (K & 0xf);
}

static byte get_K_6( word opcode )
{
    int K = opcode & mask_K_6;
    return ((K >> 2) & 0x0030) + (K & 0xf);
}

static int get_k_7( word opcode )
{
    return (((opcode & mask_k_7) >> 3) & 0x7f);
}

static int get_k_12( word opcode )
{
    return (opcode & mask_k_12);
}

static int get_k_22( word opcode )
{
    /* Masks only the upper 6 bits of the address, the lower 16 bits
     * are in PC + 1.
     * Examples: "JMP  0xFFFF", "JMP  0x10FF" -> both have opcode: 0x940c
     * Examples: "JMP 0x1FFFF", "JMP 0x110FF" -> both have opcode: 0x940d
     */
    int k = opcode & mask_k_22;
    return ((k >> 3) & 0x003e) + (k & 0x1);
}

static int get_reg_bit( word opcode )
{
    return opcode & mask_reg_bit;
}

static int get_sreg_bit( word opcode )
{
    return (opcode & mask_sreg_bit) >> 4;
}

static int get_q( word opcode )
{
    /* 00q0 qq00 0000 0qqq : Yuck! */
    int q = opcode & mask_q_displ;
    int qq = ( ((q >> 1) & 0x1000) + (q & 0x0c00) ) >> 7;
    return (qq & 0x0038) + (q & 0x7);
}

static int get_A_5( word opcode )
{
    return (opcode & mask_A_5) >> 3;
}

static int get_A_6( word opcode )
{
    int A = opcode & mask_A_6;
    return ((A >> 5) & 0x0030) + (A & 0xf);
}




DecodedInstruction* lookup_opcode( word opcode, AvrDevice *core )
{
    int decode;

    switch (opcode) {
        /* opcodes with no operands */
        case 0x9519:
            if(core->flagEIJMPInstructions)
                return new avr_op_EICALL(opcode, core);                    /* 1001 0101 0001 1001 | EICALL */
            else
                return new avr_op_ILLEGAL(opcode, core);
        case 0x9419:
            if(core->flagEIJMPInstructions)
                return new avr_op_EIJMP(opcode, core);                     /* 1001 0100 0001 1001 | EIJMP */
            else
                return new avr_op_ILLEGAL(opcode, core);
        case 0x95D8:
            if(core->flagELPMInstructions)
                return new avr_op_ELPM(opcode, core);                      /* 1001 0101 1101 1000 | ELPM */
            else
                return new avr_op_ILLEGAL(opcode, core);
        case 0x95F8:
            if(core->flagLPMInstructions)
                return new avr_op_ESPM(opcode, core);                      /* 1001 0101 1111 1000 | ESPM */
            else
                return new avr_op_ILLEGAL(opcode, core);
        case 0x9509:
            if(core->flagIJMPInstructions)
                return new avr_op_ICALL(opcode, core);                     /* 1001 0101 0000 1001 | ICALL */
            else
                return new avr_op_ILLEGAL(opcode, core);
        case 0x9409:
            if(core->flagIJMPInstructions)
                return new avr_op_IJMP(opcode, core);                      /* 1001 0100 0000 1001 | IJMP */
            else
                return new avr_op_ILLEGAL(opcode, core);
        case 0x95C8:
            if(!core->flagTiny10)
                /* except tiny10, all devices provide LPM instruction! */
                return new avr_op_LPM(opcode, core);                       /* 1001 0101 1100 1000 | LPM */
            else
                return new avr_op_ILLEGAL(opcode, core);
        case 0x0000: return new  avr_op_NOP(opcode, core);                 /* 0000 0000 0000 0000 | NOP */
        case 0x9508: return new  avr_op_RET(opcode, core);                 /* 1001 0101 0000 1000 | RET */
        case 0x9518: return new  avr_op_RETI(opcode, core);                /* 1001 0101 0001 1000 | RETI */
        case 0x9588: return new  avr_op_SLEEP(opcode, core);               /* 1001 0101 1000 1000 | SLEEP */
        case 0x95E8:
            if(core->flagLPMInstructions)
                return new avr_op_SPM(opcode, core);                       /* 1001 0101 1110 1000 | SPM */
            else
                return new avr_op_ILLEGAL(opcode, core);
        case 0x95A8: return new  avr_op_WDR(opcode, core);                 /* 1001 0101 1010 1000 | WDR */
        case 0x9598: return new  avr_op_BREAK(opcode, core);               /* 1001 0101 1001 1000 | BREAK */
        default:
                     {
                         /* opcodes with two 5-bit register (Rd and Rr) operands */
                         decode = opcode & ~(mask_Rd_5 | mask_Rr_5);
                         switch ( decode ) {
                             case 0x1C00: return new  avr_op_ADC(opcode, core);         /* 0001 11rd dddd rrrr | ADC or ROL */
                             case 0x0C00: return new  avr_op_ADD(opcode, core);         /* 0000 11rd dddd rrrr | ADD or LSL */
                             case 0x2000: return new  avr_op_AND(opcode, core);         /* 0010 00rd dddd rrrr | AND or TST */
                             case 0x1400: return new  avr_op_CP(opcode, core);          /* 0001 01rd dddd rrrr | CP */
                             case 0x0400: return new  avr_op_CPC(opcode, core);         /* 0000 01rd dddd rrrr | CPC */
                             case 0x1000: return new  avr_op_CPSE(opcode, core);        /* 0001 00rd dddd rrrr | CPSE */
                             case 0x2400: return new  avr_op_EOR(opcode, core);         /* 0010 01rd dddd rrrr | EOR or CLR */
                             case 0x2C00: return new  avr_op_MOV(opcode, core);         /* 0010 11rd dddd rrrr | MOV */
                             case 0x9C00:
                                 if(core->flagMULInstructions)
                                     return new avr_op_MUL(opcode, core);               /* 1001 11rd dddd rrrr | MUL */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x2800: return new  avr_op_OR(opcode, core);          /* 0010 10rd dddd rrrr | OR */
                             case 0x0800: return new  avr_op_SBC(opcode, core);         /* 0000 10rd dddd rrrr | SBC */
                             case 0x1800: return new  avr_op_SUB(opcode, core);         /* 0001 10rd dddd rrrr | SUB */
                         }

                         /* opcode with a single register (Rd) as operand */
                         decode = opcode & ~(mask_Rd_5);
                         switch (decode) {
                             case 0x9405: return new  avr_op_ASR(opcode, core);         /* 1001 010d dddd 0101 | ASR */
                             case 0x9400: return new  avr_op_COM(opcode, core);         /* 1001 010d dddd 0000 | COM */
                             case 0x940A: return new  avr_op_DEC(opcode, core);         /* 1001 010d dddd 1010 | DEC */
                             case 0x9006:
                                 if(core->flagELPMInstructions)
                                     return new avr_op_ELPM_Z(opcode, core);            /* 1001 000d dddd 0110 | ELPM */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x9007:
                                 if(core->flagELPMInstructions)
                                     return new avr_op_ELPM_Z_incr(opcode, core);       /* 1001 000d dddd 0111 | ELPM */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x9403: return new  avr_op_INC(opcode, core);         /* 1001 010d dddd 0011 | INC */
                             case 0x9000: return new  avr_op_LDS(opcode, core);         /* 1001 000d dddd 0000 | LDS */
                             case 0x900C:
                                 if(!core->flagTiny1x)
                                     return new avr_op_LD_X(opcode, core);              /* 1001 000d dddd 1100 | LD */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x900E:
                                 if(!core->flagTiny1x)
                                     return new avr_op_LD_X_decr(opcode, core);         /* 1001 000d dddd 1110 | LD */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x900D:
                                 if(!core->flagTiny1x)
                                     return new avr_op_LD_X_incr(opcode, core);         /* 1001 000d dddd 1101 | LD */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x8008:
                                 if(!core->flagTiny1x)
                                     return new avr_op_LDD_Y(opcode, core);             /* 1000 000d dddd 1000 | LD */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x900A:
                                 if(!core->flagTiny1x)
                                     return new avr_op_LD_Y_decr(opcode, core);         /* 1001 000d dddd 1010 | LD */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x9009:
                                 if(!core->flagTiny1x)
                                     return new avr_op_LD_Y_incr(opcode, core);         /* 1001 000d dddd 1001 | LD */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x8000: return new avr_op_LDD_Z(opcode, core);        /* 1000 000d dddd 0000 | LD */
                             case 0x9002:
                                 if(!core->flagTiny1x)
                                     return new avr_op_LD_Z_decr(opcode, core);         /* 1001 000d dddd 0010 | LD */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x9001:
                                 if(!core->flagTiny1x)
                                     return new avr_op_LD_Z_incr(opcode, core);         /* 1001 000d dddd 0001 | LD */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x9004:
                                 if(core->flagLPMInstructions)
                                     return new avr_op_LPM_Z(opcode, core);             /* 1001 000d dddd 0100 | LPM */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x9005:
                                 if(core->flagLPMInstructions)
                                     return new avr_op_LPM_Z_incr(opcode, core);        /* 1001 000d dddd 0101 | LPM */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x9406: return new  avr_op_LSR(opcode, core);         /* 1001 010d dddd 0110 | LSR */
                             case 0x9401: return new  avr_op_NEG(opcode, core);         /* 1001 010d dddd 0001 | NEG */
                             case 0x900F:
                                 if(!core->flagTiny1x)
                                     return new avr_op_POP(opcode, core);               /* 1001 000d dddd 1111 | POP */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x920F:
                                 if(!core->flagTiny1x)
                                     return new avr_op_PUSH(opcode, core);              /* 1001 001d dddd 1111 | PUSH */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x9407: return new  avr_op_ROR(opcode, core);         /* 1001 010d dddd 0111 | ROR */
                             case 0x9200: return new  avr_op_STS(opcode, core);         /* 1001 001d dddd 0000 | STS */
                             case 0x920C:
                                 if(!core->flagTiny1x)
                                     return new avr_op_ST_X(opcode, core);              /* 1001 001d dddd 1100 | ST */
                             case 0x920E:
                                 if(!core->flagTiny1x)
                                     return new avr_op_ST_X_decr(opcode, core);         /* 1001 001d dddd 1110 | ST */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x920D:
                                 if(!core->flagTiny1x)
                                     return new avr_op_ST_X_incr(opcode, core);         /* 1001 001d dddd 1101 | ST */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x8208:
                                 if(!core->flagTiny1x)
                                     return new avr_op_STD_Y(opcode, core);             /* 1000 001d dddd 1000 | ST */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x920A:
                                 if(!core->flagTiny1x)
                                     return new avr_op_ST_Y_decr(opcode, core);         /* 1001 001d dddd 1010 | ST */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x9209:
                                 if(!core->flagTiny1x)
                                     return new avr_op_ST_Y_incr(opcode, core);         /* 1001 001d dddd 1001 | ST */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x8200: return new  avr_op_STD_Z(opcode, core);       /* 1000 001d dddd 0000 | ST */
                             case 0x9202:
                                 if(!core->flagTiny1x)
                                     return new avr_op_ST_Z_decr(opcode, core);         /* 1001 001d dddd 0010 | ST */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x9201:
                                 if(!core->flagTiny1x)
                                     return new avr_op_ST_Z_incr(opcode, core);         /* 1001 001d dddd 0001 | ST */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x9402: return new  avr_op_SWAP(opcode, core);        /* 1001 010d dddd 0010 | SWAP */
                         }

                         /* opcodes with a register (Rd) and a constant data (K) as operands */
                         decode = opcode & ~(mask_Rd_4 | mask_K_8);
                         switch ( decode ) {
                             case 0x7000: return new  avr_op_ANDI(opcode, core);        /* 0111 KKKK dddd KKKK | CBR or ANDI */
                             case 0x3000: return new  avr_op_CPI(opcode, core);         /* 0011 KKKK dddd KKKK | CPI */
                             case 0xE000: return new  avr_op_LDI(opcode, core);         /* 1110 KKKK dddd KKKK | LDI or SER */
                             case 0x6000: return new  avr_op_ORI(opcode, core);         /* 0110 KKKK dddd KKKK | SBR or ORI */
                             case 0x4000: return new  avr_op_SBCI(opcode, core);        /* 0100 KKKK dddd KKKK | SBCI */
                             case 0x5000: return new  avr_op_SUBI(opcode, core);        /* 0101 KKKK dddd KKKK | SUBI */
                         }

                         /* opcodes with a register (Rd) and a register bit number (b) as operands */
                         decode = opcode & ~(mask_Rd_5 | mask_reg_bit);
                         switch ( decode ) {
                             case 0xF800: return new  avr_op_BLD(opcode, core);         /* 1111 100d dddd 0bbb | BLD */
                             case 0xFA00: return new  avr_op_BST(opcode, core);         /* 1111 101d dddd 0bbb | BST */
                             case 0xFC00: return new  avr_op_SBRC(opcode, core);        /* 1111 110d dddd 0bbb | SBRC */
                             case 0xFE00: return new  avr_op_SBRS(opcode, core);        /* 1111 111d dddd 0bbb | SBRS */
                         }

                         /* opcodes with a relative 7-bit address (k) and a register bit number (b) as operands */
                         decode = opcode & ~(mask_k_7 | mask_reg_bit);
                         switch ( decode ) {
                             case 0xF400: return new  avr_op_BRBC(opcode, core);        /* 1111 01kk kkkk kbbb | BRBC */
                             case 0xF000: return new  avr_op_BRBS(opcode, core);        /* 1111 00kk kkkk kbbb | BRBS */
                         }

                         /* opcodes with a 6-bit address displacement (q) and a register (Rd) as operands */
                         if(!core->flagTiny10 && !core->flagTiny1x) {
                             decode = opcode & ~(mask_Rd_5 | mask_q_displ);
                             switch ( decode ) {
                                 case 0x8008: return new  avr_op_LDD_Y(opcode, core);   /* 10q0 qq0d dddd 1qqq | LDD */
                                 case 0x8000: return new  avr_op_LDD_Z(opcode, core);   /* 10q0 qq0d dddd 0qqq | LDD */
                                 case 0x8208: return new  avr_op_STD_Y(opcode, core);   /* 10q0 qq1d dddd 1qqq | STD */
                                 case 0x8200: return new  avr_op_STD_Z(opcode, core);   /* 10q0 qq1d dddd 0qqq | STD */
                             }
                         }
                         
                         /* opcodes with a absolute 22-bit address (k) operand */
                         decode = opcode & ~(mask_k_22);
                         switch ( decode ) {
                             case 0x940E:
                                 if(core->flagJMPInstructions)
                                     return new avr_op_CALL(opcode, core);              /* 1001 010k kkkk 111k | CALL */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x940C:
                                 if(core->flagJMPInstructions)
                                     return new avr_op_JMP(opcode, core);               /* 1001 010k kkkk 110k | JMP */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                         }

                         /* opcode with a sreg bit select (s) operand */
                         decode = opcode & ~(mask_sreg_bit);
                         switch ( decode ) {
                             /* BCLR takes place of CL{C,Z,N,V,S,H,T,I} */
                             /* BSET takes place of SE{C,Z,N,V,S,H,T,I} */
                             case 0x9488: return new  avr_op_BCLR(opcode, core);        /* 1001 0100 1sss 1000 | BCLR */
                             case 0x9408: return new  avr_op_BSET(opcode, core);        /* 1001 0100 0sss 1000 | BSET */
                         }

                         /* opcodes with a 6-bit constant (K) and a register (Rd) as operands */
                         decode = opcode & ~(mask_K_6 | mask_Rd_2);
                         switch ( decode ) {
                             case 0x9600:
                                 if(core->flagIWInstructions)
                                     return new avr_op_ADIW(opcode, core);              /* 1001 0110 KKdd KKKK | ADIW */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x9700:
                                 if(core->flagIWInstructions)
                                     return new avr_op_SBIW(opcode, core);              /* 1001 0111 KKdd KKKK | SBIW */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                         }

                         /* opcodes with a 5-bit IO Addr (A) and register bit number (b) as operands */
                         decode = opcode & ~(mask_A_5 | mask_reg_bit);
                         switch ( decode ) {
                             case 0x9800: return new  avr_op_CBI(opcode, core);         /* 1001 1000 AAAA Abbb | CBI */
                             case 0x9A00: return new  avr_op_SBI(opcode, core);         /* 1001 1010 AAAA Abbb | SBI */
                             case 0x9900: return new  avr_op_SBIC(opcode, core);        /* 1001 1001 AAAA Abbb | SBIC */
                             case 0x9B00: return new  avr_op_SBIS(opcode, core);        /* 1001 1011 AAAA Abbb | SBIS */
                         }

                         /* opcodes with a 6-bit IO Addr (A) and register (Rd) as operands */
                         decode = opcode & ~(mask_A_6 | mask_Rd_5);
                         switch ( decode ) {
                             case 0xB000: return new  avr_op_IN(opcode, core);          /* 1011 0AAd dddd AAAA | IN */
                             case 0xB800: return new  avr_op_OUT(opcode, core);         /* 1011 1AAd dddd AAAA | OUT */
                         }

                         /* opcodes with a relative 12-bit address (k) operand */
                         decode = opcode & ~(mask_k_12);
                         switch ( decode ) {
                             case 0xD000: return new  avr_op_RCALL(opcode, core);       /* 1101 kkkk kkkk kkkk | RCALL */
                             case 0xC000: return new  avr_op_RJMP(opcode, core);        /* 1100 kkkk kkkk kkkk | RJMP */
                         }

                         /* opcodes with two 4-bit register (Rd and Rr) operands */
                         decode = opcode & ~(mask_Rd_4 | mask_Rr_4);
                         switch ( decode ) {
                             case 0x0100:
                                 if(core->flagMOVWInstruction)
                                     return new avr_op_MOVW(opcode, core);              /* 0000 0001 dddd rrrr | MOVW */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x0200:
                                 if(core->flagMULInstructions)
                                     return new avr_op_MULS(opcode, core);              /* 0000 0010 dddd rrrr | MULS */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                         }

                         /* opcodes with two 3-bit register (Rd and Rr) operands */
                         decode = opcode & ~(mask_Rd_3 | mask_Rr_3);
                         switch ( decode ) {
                             case 0x0300:
                                 if(core->flagMULInstructions)
                                     return new avr_op_MULSU(opcode, core);             /* 0000 0011 0ddd 0rrr | MULSU */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x0308:
                                 if(core->flagMULInstructions)
                                     return new avr_op_FMUL(opcode, core);              /* 0000 0011 0ddd 1rrr | FMUL */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x0380:
                                 if(core->flagMULInstructions)
                                     return new avr_op_FMULS(opcode, core);             /* 0000 0011 1ddd 0rrr | FMULS */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                             case 0x0388:
                                 if(core->flagMULInstructions)
                                     return new avr_op_FMULSU(opcode, core);            /* 0000 0011 1ddd 1rrr | FMULSU */
                                 else
                                     return new avr_op_ILLEGAL(opcode, core);
                         }

                     } /* default */
    } /* first switch */

    //return NULL;
    return new avr_op_ILLEGAL(opcode, core);

} /* decode opcode function */

