/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003  Theodore A. Roth, Klaus Rudolph		
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
#include <stdlib.h>

//only for debugging
//#include "iostream"
using namespace std;

#include "decoder.h"
#include "avrdevice.h"
#include "ass_do.h"
#include "ass_do_trace.h"
#include "types.h"
#include "global.h"

#include "trace.h"


class AvrDevice;

typedef int (*OpcodeFunc)(AvrDevice *, int, int);
typedef void (*OpcodePrepare)(word, DecodedEntry*);

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
    /* Masks only the upper 6 bits of the address, the other 16 bits
     * are in PC + 1. */
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

/******************************************************************************\
 *
 * Opcode handler functions.
 *
 \******************************************************************************/



void avr_op_ADC( word opcode, DecodedEntry *e) 
{
    /*
     * Add with Carry.
     *       
     * Opcode     : 0001 11rd dddd rrrr 
     * Usage      : ADC  Rd, Rr
     * Operation  : Rd <- Rd + Rr + C
     * Flags      : Z,C,N,V,S,H
     * Num Clocks : 1
     */


    e->p1 = get_rd_5(opcode);
    e->p2 = get_rr_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_ADC_do; else e->OpcodeFunction= avr_op_ADC_do_trace;
}
void avr_op_ADD( word opcode, DecodedEntry *e)
{
    /*
     * Add without Carry.
     *
     * Opcode     : 0000 11rd dddd rrrr 
     * Usage      : ADD  Rd, Rr
     * Operation  : Rd <- Rd + Rr
     * Flags      : Z,C,N,V,S,H
     * Num Clocks : 1
     */


    e->p1 = get_rd_5(opcode);
    e->p2 = get_rr_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_ADD_do; else e->OpcodeFunction= avr_op_ADD_do_trace;
}

void avr_op_ADIW( word opcode, DecodedEntry *e)
{
    /*
     * Add Immediate to Word.
     *
     * Opcode     : 1001 0110 KKdd KKKK 
     * Usage      : ADIW  Rd, K
     * Operation  : Rd+1:Rd <- Rd+1:Rd + K
     * Flags      : Z,C,N,V,S
     * Num Clocks : 2
     */

    e->p2  = get_K_6(opcode);
    e->p1 = get_rd_2(opcode);
    //cout << "ADIW" << e->p1 << " " << e->p2 << endl;


    if (trace_on==0) e->OpcodeFunction= avr_op_ADIW_do; else e->OpcodeFunction= avr_op_ADIW_do_trace;
}

void avr_op_AND( word opcode, DecodedEntry *e)
{ 
    /*
     * Logical AND.
     *
     * Opcode     : 0010 00rd dddd rrrr 
     * Usage      : AND  Rd, Rr
     * Operation  : Rd <- Rd & Rr
     * Flags      : Z,N,V,S
     * Num Clocks : 1
     */
    e->p1 = get_rd_5(opcode);
    e->p2 = get_rr_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_AND_do; else e->OpcodeFunction= avr_op_AND_do_trace;
}

void avr_op_ANDI( word opcode, DecodedEntry *e)
{
    /*
     * Logical AND with Immed.
     *
     * Opcode     : 0111 KKKK dddd KKKK 
     * Usage      : ANDI  Rd, K
     * Operation  : Rd <- Rd & K
     * Flags      : Z,N,V,S
     * Num Clocks : 1
     */

    e->p1 = get_rd_4(opcode);
    e->p2 = get_K_8(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_ANDI_do; else e->OpcodeFunction= avr_op_ANDI_do_trace;
}

void avr_op_ASR( word opcode, DecodedEntry *e)
{
    /*
     * Arithmetic Shift Right.
     *
     * Opcode     : 1001 010d dddd 0101 
     * Usage      : ASR  Rd
     * Operation  : Rd(n) <- Rd(n+1), n=0..6
     * Flags      : Z,C,N,V,S
     * Num Clocks : 1
     */
    e->p1 = get_rd_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_ASR_do; else e->OpcodeFunction= avr_op_ASR_do_trace;
}

void avr_op_BCLR( word opcode, DecodedEntry *e)
{
    /*
     * Clear a single flag or bit in SREG.
     *
     * Opcode     : 1001 0100 1sss 1000 
     * Usage      : BCLR  
     * Operation  : SREG(s) <- 0
     * Flags      : SREG(s)
     * Num Clocks : 1
     */
    e->p1 = get_sreg_bit(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_BCLR_do; else e->OpcodeFunction= avr_op_BCLR_do_trace;
}

void avr_op_BLD( word opcode, DecodedEntry *e)
{
    /* Bit load from T to Register.
     *
     * Opcode     : 1111 100d dddd 0bbb 
     * Usage      : BLD  Rd, b
     * Operation  : Rd(b) <- T
     * Flags      : None
     * Num Clocks : 1
     */
    e->p1 = get_rd_5(opcode);
    e->p2 = get_reg_bit(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_BLD_do; else e->OpcodeFunction= avr_op_BLD_do_trace;
}

void avr_op_BRBC( word opcode, DecodedEntry *e)
{ 
    /*
     * Branch if Status Flag Cleared.
     *
     * Pass control directly to the specific bit operation.
     *
     * Opcode     : 1111 01kk kkkk ksss 
     * Usage      : BRBC  s, k
     * Operation  : if (SREG(s) = 0) then PC <- PC + k + 1
     * Flags      : None
     * Num Clocks : 1 / 2
     *
     * k is an relative address represented in two's complements.
     * (64 < k <= 64)
     */
    e->p1 = get_reg_bit(opcode);
    e->p2 = n_bit_unsigned_to_signed( get_k_7(opcode), 7 );
    if (trace_on==0) e->OpcodeFunction= avr_op_BRBC_do; else e->OpcodeFunction= avr_op_BRBC_do_trace;
}

void avr_op_BRBS( word opcode, DecodedEntry *e)
{ 
    /*
     * Branch if Status Flag Set.
     *
     * Pass control directly to the specific bit operation.
     *
     * Opcode     : 1111 00kk kkkk ksss 
     * Usage      : BRBS  s, k
     * Operation  : if (SREG(s) = 1) then PC <- PC + k + 1
     * Flags      : None
     * Num Clocks : 1 / 2
     *
     * k is an relative address represented in two's complements.
     * (64 < k <= 64)
     */
    e->p1 = get_reg_bit(opcode);
    e->p2= n_bit_unsigned_to_signed( get_k_7(opcode), 7 );
    if (trace_on==0) e->OpcodeFunction= avr_op_BRBS_do; else e->OpcodeFunction= avr_op_BRBS_do_trace;
}

void avr_op_BSET( word opcode, DecodedEntry *e)
{ 
    /*
     * Set a single flag or bit in SREG.
     *
     * Opcode     : 1001 0100 0sss 1000 
     * Usage      : BSET
     * Operation  : SREG(s) <- 1
     * Flags      : SREG(s)
     * Num Clocks : 1
     */
    e->p1=get_sreg_bit(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_BSET_do; else e->OpcodeFunction= avr_op_BSET_do_trace;
}

void avr_op_BST( word opcode, DecodedEntry *e)
{
    /*
     * Bit Store from Register to T.
     *
     * Opcode     : 1111 101d dddd 0bbb 
     * Usage      : BST  Rd, b
     * Operation  : T <- Rd(b)
     * Flags      : T
     * Num Clocks : 1
     */
    e->p1 = get_rd_5(opcode);
    e->p2 = get_reg_bit(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_BST_do; else e->OpcodeFunction= avr_op_BST_do_trace;
}

void avr_op_CALL( word opcode, DecodedEntry *e)
{
    /*
     * Call Subroutine.
     *
     * Opcode     : 1001 010k kkkk 111k kkkk kkkk kkkk kkkk
     * Usage      : CALL  k
     * Operation  : PC <- k
     * Flags      : None
     * Num Clocks : 4 / 5
     */
#ifdef TODO //hier gibts noch keinen Trick zum lesen des naechsten Bytes
    int kh = get_k_22(opcode);
    int kl = flash_read( core->flash, pc+1 );

    int k = (kh << 16) + kl;

    if ( (pc_bytes == 2) && ( k > 0xffff) )
        avr_error( "Address out of allowed range: 0x%06x", k );

    avr_core_stack_push( core, pc_bytes, pc+2 );

    avr_core_PC_set( core, k );
    avr_core_inst_CKS_set( core, pc_bytes+2 );
#endif

    if (trace_on==0) e->OpcodeFunction= avr_op_CALL_do; else e->OpcodeFunction= avr_op_CALL_do_trace;
}

void avr_op_CBI( word opcode, DecodedEntry *e)
{
    /*
     * Clear Bit in I/O Register.
     *
     * Opcode     : 1001 1000 AAAA Abbb 
     * Usage      : CBI  A, b
     * Operation  : I/O(A, b) <- 0
     * Flags      : None
     * Num Clocks : 2
     */
    e->p1= get_A_5(opcode);
    e->p2= get_reg_bit(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_CBI_do; else e->OpcodeFunction= avr_op_CBI_do_trace;
}

void avr_op_COM( word opcode, DecodedEntry *e)
{
    /*
     * One's Complement.
     *
     * Opcode     : 1001 010d dddd 0000 
     * Usage      : COM  Rd
     * Operation  : Rd <- $FF - Rd
     * Flags      : Z,C,N,V,S
     * Num Clocks : 1
     */
    e->p1 = get_rd_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_COM_do; else e->OpcodeFunction= avr_op_COM_do_trace;
}

void avr_op_CP( word opcode, DecodedEntry *e)
{
    /*
     * Compare.
     *
     * Opcode     : 0001 01rd dddd rrrr 
     * Usage      : CP  Rd, Rr
     * Operation  : Rd - Rr
     * Flags      : Z,C,N,V,S,H
     * Num Clocks : 1
     */

    e->p1 = get_rd_5(opcode);
    e->p2 = get_rr_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_CP_do; else e->OpcodeFunction= avr_op_CP_do_trace;
}

void avr_op_CPC( word opcode, DecodedEntry *e)
{
    /*
     * Compare with Carry.
     *
     * Opcode     : 0000 01rd dddd rrrr 
     * Usage      : CPC  Rd, Rr
     * Operation  : Rd - Rr - C
     * Flags      : Z,C,N,V,S,H
     * Num Clocks : 1
     */

    e->p1 = get_rd_5(opcode);
    e->p2 = get_rr_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_CPC_do; else e->OpcodeFunction= avr_op_CPC_do_trace;
}

void avr_op_CPI( word opcode, DecodedEntry *e)
{
    /*
     * Compare with Immediate.
     *
     * Opcode     : 0011 KKKK dddd KKKK 
     * Usage      : CPI  Rd, K
     * Operation  : Rd - K
     * Flags      : Z,C,N,V,S,H
     * Num Clocks : 1
     */

    e->p1 = get_rd_4(opcode);
    e->p2 = get_K_8(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_CPI_do; else e->OpcodeFunction= avr_op_CPI_do_trace;
}

void avr_op_CPSE( word opcode, DecodedEntry *e)
{
    /*
     * Compare, Skip if Equal.
     *
     * Opcode     : 0001 00rd dddd rrrr 
     * Usage      : CPSE  Rd, Rr
     * Operation  : if (Rd = Rr) PC <- PC + 2 or 3
     * Flags      : None
     * Num Clocks : 1 / 2 / 3
     */

    e->p1 = get_rd_5(opcode);
    e->p2 = get_rr_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_CPSE_do; else e->OpcodeFunction= avr_op_CPSE_do_trace;
}

void avr_op_DEC( word opcode, DecodedEntry *e)
{
    /*
     * Decrement.
     *
     * Opcode     : 1001 010d dddd 1010 
     * Usage      : DEC  Rd
     * Operation  : Rd <- Rd - 1
     * Flags      : Z,N,V,S
     * Num Clocks : 1
     */

    e->p1= get_rd_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_DEC_do; else e->OpcodeFunction= avr_op_DEC_do_trace;
}

void avr_op_EICALL( word opcode, DecodedEntry *e)
{ 
    /*
     * Extended Indirect Call to (Z).
     *
     * Opcode     : 1001 0101 0001 1001 
     * Usage      : EICALL  
     * Operation  : PC(15:0) <- Z, PC(21:16) <- EIND
     * Flags      : None
     * Num Clocks : 4
     */
    if (trace_on==0) e->OpcodeFunction= avr_op_EICALL_do; else e->OpcodeFunction= avr_op_EICALL_do_trace;
}

void avr_op_EIJMP( word opcode, DecodedEntry *e)
{
    /*
     * Extended Indirect Jmp to (Z).
     *
     * Opcode     : 1001 0100 0001 1001 
     * Usage      : EIJMP  
     * Operation  : PC(15:0) <- Z, PC(21:16) <- EIND
     * Flags      : None
     * Num Clocks : 2
     */

    /* Z is R31:R30 */
    if (trace_on==0) e->OpcodeFunction= avr_op_EIJMP_do; else e->OpcodeFunction= avr_op_EIJMP_do_trace;
}

void avr_op_ELPM_Z( word opcode, DecodedEntry *e)
{
    /*
     * Extended Load Program Memory.
     *
     * Opcode     : 1001 000d dddd 0110 
     * Usage      : ELPM  Rd, Z
     * Operation  : R <- (RAMPZ:Z)
     * Flags      : None
     * Num Clocks : 3
     */

    e->p1 = get_rd_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_ELPM_Z_do; else e->OpcodeFunction= avr_op_ELPM_Z_do_trace;
}

void avr_op_ELPM_Z_incr( word opcode, DecodedEntry *e)
{
    /*
     * Extended Ld Prg Mem and Post-Incr.
     *
     * Opcode     : 1001 000d dddd 0111 
     * Usage      : ELPM  Rd, Z+
     * Operation  : Rd <- (RAMPZ:Z), Z <- Z + 1
     * Flags      : None
     * Num Clocks : 3
     */
    e->p1 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_ELPM_Z_incr_do; else e->OpcodeFunction= avr_op_ELPM_Z_incr_do_trace;
}

void avr_op_ELPM( word opcode, DecodedEntry *e)
{
    /*
     * Extended Load Program Memory.
     *
     * This is the same as avr_op_ELPM_Z with Rd = R0.
     *
     * Opcode     : 1001 0101 1101 1000 
     * Usage      : ELPM  
     * Operation  : R0 <- (RAMPZ:Z)
     * Flags      : None
     * Num Clocks : 3
     */
    if (trace_on==0) e->OpcodeFunction= avr_op_ELPM_do; else e->OpcodeFunction= avr_op_ELPM_do_trace;
}

void avr_op_EOR( word opcode, DecodedEntry *e)
{
    /*
     * Exclusive OR.
     *
     * Opcode     : 0010 01rd dddd rrrr 
     * Usage      : EOR  Rd, Rr
     * Operation  : Rd <- Rd ^ Rr
     * Flags      : Z,N,V,S
     * Num Clocks : 1
     */

    e->p1 = get_rd_5(opcode);
    e->p2 = get_rr_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_EOR_do; else e->OpcodeFunction= avr_op_EOR_do_trace;
}

void avr_op_ESPM( word opcode, DecodedEntry *e)
{
    /*
     * Extended Store Program Memory.
     *
     * Opcode     : 1001 0101 1111 1000 
     * Usage      : ESPM  
     * Operation  : (RAMPZ:Z) <- R1:R0
     * Flags      : None
     * Num Clocks : -
     */
    //avr_error( "This opcode is not implemented yet: 0x%04x", opcode );
    if (trace_on==0) e->OpcodeFunction= avr_op_ESPM_do; else e->OpcodeFunction= avr_op_ESPM_do_trace;
}

/**
 ** I don't know how this Fractional Multiplication works.
 ** If someone wishes to enlighten me, I write these.
 **/

void avr_op_FMUL( word opcode, DecodedEntry *e)
{
    /*
     * Fractional Mult Unsigned.
     *
     * Opcode     : 0000 0011 0ddd 1rrr 
     * Usage      : FMUL  Rd, Rr
     * Operation  : R1:R0 <- (Rd * Rr)<<1 (UU)
     * Flags      : Z,C
     * Num Clocks : 2
     */
    e->p1 = get_rd_3(opcode);
    e->p2 = get_rr_3(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_FMUL_do; else e->OpcodeFunction= avr_op_FMUL_do_trace;
}

void avr_op_FMULS( word opcode, DecodedEntry *e)
{
    /*
     * Fractional Mult Signed.
     *
     * Opcode     : 0000 0011 1ddd 0rrr 
     * Usage      : FMULS  Rd, Rr
     * Operation  : R1:R0 <- (Rd * Rr)<<1 (SS)
     * Flags      : Z,C
     * Num Clocks : 2
     */
    e->p1 = get_rd_3(opcode);
    e->p2 = get_rr_3(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_FMULS_do; else e->OpcodeFunction= avr_op_FMULS_do_trace;
}

void avr_op_FMULSU( word opcode, DecodedEntry *e)
{
    /*
     * Fract Mult Signed w/ Unsigned.
     *
     * Opcode     : 0000 0011 1ddd 1rrr 
     * Usage      : FMULSU  Rd, Rr
     * Operation  : R1:R0 <- (Rd * Rr)<<1 (SU)
     * Flags      : Z,C
     * Num Clocks : 2
     */
    e->p1 = get_rd_3(opcode);
    e->p2 = get_rr_3(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_FMULSU_do; else e->OpcodeFunction= avr_op_FMULSU_do_trace;
}

void avr_op_ICALL( word opcode, DecodedEntry *e)
{
    /*
     * Indirect Call to (Z).
     *
     * Opcode     : 1001 0101 0000 1001 
     * Usage      : ICALL  
     * Operation  : PC(15:0) <- Z, PC(21:16) <- 0
     * Flags      : None
     * Num Clocks : 3 / 4
     */
    if (trace_on==0) e->OpcodeFunction= avr_op_ICALL_do; else e->OpcodeFunction= avr_op_ICALL_do_trace;
}

void avr_op_IJMP( word opcode, DecodedEntry *e)
{
    /*
     * Indirect Jump to (Z).
     *
     * Opcode     : 1001 0100 0000 1001 
     * Usage      : IJMP  
     * Operation  : PC(15:0) <- Z, PC(21:16) <- 0
     * Flags      : None
     * Num Clocks : 2
     */

    /* Z is R31:R30 */

    if (trace_on==0) e->OpcodeFunction= avr_op_IJMP_do; else e->OpcodeFunction= avr_op_IJMP_do_trace;
}

void avr_op_IN( word opcode, DecodedEntry *e)
{
    /*
     * In From I/O Location.
     *
     * Opcode     : 1011 0AAd dddd AAAA 
     * Usage      : IN  Rd, A
     * Operation  : Rd <- I/O(A)
     * Flags      : None
     * Num Clocks : 1
     */
    e->p1 = get_rd_5(opcode);
    e->p2 = get_A_6(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_IN_do; else e->OpcodeFunction= avr_op_IN_do_trace;
}

void avr_op_INC( word opcode, DecodedEntry *e)
{
    /*
     * Increment.
     *
     * Opcode     : 1001 010d dddd 0011 
     * Usage      : INC  Rd
     * Operation  : Rd <- Rd + 1
     * Flags      : Z,N,V,S
     * Num Clocks : 1
     */

    e->p1= get_rd_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_INC_do; else e->OpcodeFunction= avr_op_INC_do_trace;
}

void avr_op_JMP( word opcode, DecodedEntry *e)
{
    /*
     * Jump.
     *
     * Opcode     : 1001 010k kkkk 110k kkkk kkkk kkkk kkkk
     * Usage      : JMP  k
     * Operation  : PC <- k
     * Flags      : None
     * Num Clocks : 3
     */
    //TODO we must read a second word from flash (wuerg!)
    e->p1 = get_k_22(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_JMP_do; else e->OpcodeFunction= avr_op_JMP_do_trace;
}

void avr_op_LDD_Y( word opcode, DecodedEntry *e)
{
    /*
     * Load Indirect with Displacement using index Y.
     *
     * Opcode     : 10q0 qq0d dddd 1qqq 
     * Usage      : LDD  Rd, Y+q
     * Operation  : Rd <- (Y + q)
     * Flags      : None
     * Num Clocks : 2
     */
    e->p1 = get_rd_5(opcode);
    e->p2 = get_q(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_LDD_Y_do; else e->OpcodeFunction= avr_op_LDD_Y_do_trace;
}

void avr_op_LDD_Z( word opcode, DecodedEntry *e)
{
    /*
     * Load Indirect with Displacement using index Z.
     *
     * Opcode     : 10q0 qq0d dddd 0qqq 
     * Usage      : LDD  Rd, Z+q
     * Operation  : Rd <- (Z + q)
     * Flags      : None
     * Num Clocks : 2
     */

    e->p1 = get_rd_5(opcode);
    e->p2 = get_q(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_LDD_Z_do; else e->OpcodeFunction= avr_op_LDD_Z_do_trace;
}

void avr_op_LDI( word opcode, DecodedEntry *e)
{
    /*
     * Load Immediate.
     *
     * Opcode     : 1110 KKKK dddd KKKK 
     * Usage      : LDI  Rd, K
     * Operation  : Rd  <- K
     * Flags      : None
     * Num Clocks : 1
     */
    e->p1 = get_rd_4(opcode);
    e->p2 = get_K_8(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_LDI_do; else e->OpcodeFunction= avr_op_LDI_do_trace;
}

void avr_op_LDS( word opcode, DecodedEntry *e)
{
    /*
     * Load Direct from data space.
     *
     * Opcode     : 1001 000d dddd 0000 kkkk kkkk kkkk kkkk
     * Usage      : LDS  Rd, k
     * Operation  : Rd <- (k)
     * Flags      : None
     * Num Clocks : 2
     */
    e->p1 = get_rd_5(opcode);
    //TODO we must read a second byte from flash
    if (trace_on==0) e->OpcodeFunction= avr_op_LDS_do; else e->OpcodeFunction= avr_op_LDS_do_trace;
}

void avr_op_LD_X( word opcode, DecodedEntry *e)
{
    /*
     * Load Indirect using index X.
     *
     * Opcode     : 1001 000d dddd 1100 
     * Usage      : LD  Rd, X
     * Operation  : Rd <- (X)
     * Flags      : None
     * Num Clocks : 2
     */

    e->p1 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_LD_X_do; else e->OpcodeFunction= avr_op_LD_X_do_trace;
}

void avr_op_LD_X_decr( word opcode, DecodedEntry *e)
{
    /*
     * Load Indirect and Pre-Decrement using index X.
     *
     * Opcode     : 1001 000d dddd 1110 
     * Usage      : LD  Rd, -X
     * Operation  : X <- X - 1, Rd <- (X)
     * Flags      : None
     * Num Clocks : 2
     */

    e->p1 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_LD_X_decr_do; else e->OpcodeFunction= avr_op_LD_X_decr_do_trace;
}

void avr_op_LD_X_incr( word opcode, DecodedEntry *e)
{
    /*
     * Load Indirect and Post-Increment using index X.
     *
     * Opcode     : 1001 000d dddd 1101 
     * Usage      : LD  Rd, X+
     * Operation  : Rd <- (X), X <- X + 1
     * Flags      : None
     * Num Clocks : 2
     */

    e->p1 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_LD_X_incr_do; else e->OpcodeFunction= avr_op_LD_X_incr_do_trace;
}

void avr_op_LD_Y_decr( word opcode, DecodedEntry *e)
{
    /*
     * Load Indirect and PreDecrement using index Y.
     *
     * Opcode     : 1001 000d dddd 1010 
     * Usage      : LD  Rd, -Y
     * Operation  : Y <- Y - 1, Rd <- (Y)
     * Flags      : None
     * Num Clocks : 2
     */

    e->p1 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_LD_Y_decr_do; else e->OpcodeFunction= avr_op_LD_Y_decr_do_trace;
}

void avr_op_LD_Y_incr( word opcode, DecodedEntry *e)
{
    /*
     * Load Indirect and Post-Increment using index Y.
     *
     * Opcode     : 1001 000d dddd 1001 
     * Usage      : LD  Rd, Y+
     * Operation  : Rd <- (Y), Y <- Y + 1
     * Flags      : None
     * Num Clocks : 2
     */

    e->p1 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_LD_Y_incr_do; else e->OpcodeFunction= avr_op_LD_Y_incr_do_trace;
}

void avr_op_LD_Z_incr( word opcode, DecodedEntry *e)
{
    /*
     * Load Indirect and Post-Increment using index Z.
     *
     * Opcode     : 1001 000d dddd 0001 
     * Usage      : LD  Rd, Z+
     * Operation  : Rd <- (Z), Z <- Z+1
     * Flags      : None
     * Num Clocks : 2
     */

    e->p1 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_LD_Z_incr_do; else e->OpcodeFunction= avr_op_LD_Z_incr_do_trace;
}

void avr_op_LD_Z_decr( word opcode, DecodedEntry *e)
{
    /*
     * Load Indirect and Pre-Decrement using index Z.
     *
     * Opcode     : 1001 000d dddd 0010 
     * Usage      : LD  Rd, -Z
     * Operation  : Z <- Z - 1, Rd <- (Z)
     * Flags      : None
     * Num Clocks : 2
     */

    e->p1 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_LD_Z_decr_do; else e->OpcodeFunction= avr_op_LD_Z_decr_do_trace;
}

void avr_op_LPM_Z( word opcode, DecodedEntry *e)
{
    /*
     * Load Program Memory.
     *
     * Opcode     : 1001 000d dddd 0100 
     * Usage      : LPM  Rd, Z
     * Operation  : Rd <- (Z)
     * Flags      : None
     * Num Clocks : 3
     */
    e->p1 = get_rd_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_LPM_Z_do; else e->OpcodeFunction= avr_op_LPM_Z_do_trace;
}

void avr_op_LPM( word opcode, DecodedEntry *e)
{
    /* Load Program Memory.
     *
     * This the same as avr_op_LPM_Z with Rd = R0.
     *
     * Opcode     : 1001 0101 1100 1000 
     * Usage      : LPM  
     * Operation  : R0 <- (Z)
     * Flags      : None
     * Num Clocks : 3
     */
    //return avr_op_LPM_Z( core, 0x9004 );
    e->p1=0; //the same as LPM_Z with Rd=0!!
    if (trace_on==0) e->OpcodeFunction= avr_op_LPM_Z_do; else e->OpcodeFunction= avr_op_LPM_Z_do_trace;
}

void avr_op_LPM_Z_incr( word opcode, DecodedEntry *e)
{
    /*
     * Load Program Memory and Post-Incr.
     *
     * Opcode     : 1001 000d dddd 0101 
     * Usage      : LPM  Rd, Z+
     * Operation  : Rd <- (Z), Z <- Z + 1
     * Flags      : None
     * Num Clocks : 3
     */

    e->p1 = get_rd_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_LPM_Z_incr_do; else e->OpcodeFunction= avr_op_LPM_Z_incr_do_trace;
}

void avr_op_LSR( word opcode, DecodedEntry *e)
{
    /*
     * Logical Shift Right.
     *
     * Opcode     : 1001 010d dddd 0110 
     * Usage      : LSR  Rd
     * Operation  : Rd(n) <- Rd(n+1), Rd(7) <- 0, C <- Rd(0)
     * Flags      : Z,C,N,V,S
     * Num Clocks : 1
     */

    e->p1= get_rd_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_LSR_do; else e->OpcodeFunction= avr_op_LSR_do_trace;
}

void avr_op_MOV( word opcode, DecodedEntry *e)
{
    /* Copy Register.
     *
     * Opcode     : 0010 11rd dddd rrrr 
     * Usage      : MOV  Rd, Rr
     * Operation  : Rd <- Rr
     * Flags      : None
     * Num Clocks : 1
     */
    e->p1 = get_rd_5(opcode);
    e->p2 = get_rr_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_MOV_do; else e->OpcodeFunction= avr_op_MOV_do_trace;
}

void avr_op_MOVW( word opcode, DecodedEntry *e)
{
    /*
     *Copy Register Pair.
     *
     * Opcode     : 0000 0001 dddd rrrr 
     * Usage      : MOVW  Rd, Rr
     * Operation  : Rd+1:Rd <- Rr+1:Rr
     * Flags      : None
     * Num Clocks : 1
     */
    e->p1 = get_rd_4(opcode);
    e->p2 = get_rr_4(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_MOVW_do; else e->OpcodeFunction= avr_op_MOVW_do_trace;
}


void avr_op_MUL( word opcode, DecodedEntry *e)
{
    /*
     * Mult Unsigned.
     *
     * Opcode     : 1001 11rd dddd rrrr 
     * Usage      : MUL  Rd, Rr
     * Operation  : R1:R0 <- Rd * Rr (UU)
     * Flags      : Z,C
     * Num Clocks : 2
     */
    e->p1 = get_rd_5(opcode);
    e->p2 = get_rr_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_MUL_do; else e->OpcodeFunction= avr_op_MUL_do_trace;
}

void avr_op_MULS( word opcode, DecodedEntry *e)
{
    /*
     * Mult Signed.
     *
     * Opcode     : 0000 0010 dddd rrrr 
     * Usage      : MULS  Rd, Rr
     * Operation  : R1:R0 <- Rd * Rr (SS)
     * Flags      : Z,C
     * Num Clocks : 2
     */
    e->p1 = get_rd_4(opcode);
    e->p2 = get_rr_4(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_MULS_do; else e->OpcodeFunction= avr_op_MULS_do_trace;
}

void avr_op_MULSU( word opcode, DecodedEntry *e)
{
    /*
     * Mult Signed with Unsigned.
     * 
     * Rd(unsigned),Rr(signed), result (signed)
     *
     * Opcode     : 0000 0011 0ddd 0rrr 
     * Usage      : MULSU  Rd, Rr
     * Operation  : R1:R0 <- Rd * Rr (SU)
     * Flags      : Z,C
     * Num Clocks : 2
     */
    e->p1 = get_rd_3(opcode);
    e->p2 = get_rr_3(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_MULSU_do; else e->OpcodeFunction= avr_op_MULSU_do_trace;
}

void avr_op_NEG( word opcode, DecodedEntry *e)
{
    /*
     * Two's Complement.
     *
     * Opcode     : 1001 010d dddd 0001 
     * Usage      : NEG  Rd
     * Operation  : Rd <- $00 - Rd
     * Flags      : Z,C,N,V,S,H
     * Num Clocks : 1
     */

    e->p1 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_NEG_do; else e->OpcodeFunction= avr_op_NEG_do_trace;
}

void avr_op_NOP( word opcode, DecodedEntry *e)
{
    /*
     * No Operation.
     *
     * Opcode     : 0000 0000 0000 0000 
     * Usage      : NOP  
     * Operation  : None
     * Flags      : None
     * Num Clocks : 1
     */
    if (trace_on==0) e->OpcodeFunction= avr_op_NOP_do; else e->OpcodeFunction= avr_op_NOP_do_trace;
}

void avr_op_OR( word opcode, DecodedEntry *e)
{
    /*
     * Logical OR.
     *
     * Opcode     : 0010 10rd dddd rrrr 
     * Usage      : OR  Rd, Rr
     * Operation  : Rd <- Rd or Rr
     * Flags      : Z,N,V,S
     * Num Clocks : 1
     */

    e->p1 = get_rd_5(opcode);
    e->p2 = get_rr_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_OR_do; else e->OpcodeFunction= avr_op_OR_do_trace;
}

void avr_op_ORI( word opcode, DecodedEntry *e)
{
    /*
     * Logical OR with Immed.
     *
     * Opcode     : 0110 KKKK dddd KKKK 
     * Usage      : ORI  Rd, K
     * Operation  : Rd <- Rd or K
     * Flags      : Z,N,V,S
     * Num Clocks : 1
     */

    e->p1 = get_rd_4(opcode);
    e->p2 = get_K_8(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_ORI_do; else e->OpcodeFunction= avr_op_ORI_do_trace;
}

void avr_op_OUT( word opcode, DecodedEntry *e)
{
    /*
     * Out To I/O Location.
     *
     * Opcode     : 1011 1AAd dddd AAAA 
     * Usage      : OUT  A Rd
     * Operation  : I/O(A) <- Rd
     * Flags      : None
     * Num Clocks : 1
     */
    e->p1 = get_A_6(opcode);
    e->p2 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_OUT_do; else e->OpcodeFunction= avr_op_OUT_do_trace;
}

void avr_op_POP( word opcode, DecodedEntry *e)
{
    /*
     * Pop Register from Stack.
     *
     * Opcode     : 1001 000d dddd 1111 
     * Usage      : POP  Rd
     * Operation  : Rd <- STACK
     * Flags      : None
     * Num Clocks : 2
     */
    e->p1 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_POP_do; else e->OpcodeFunction= avr_op_POP_do_trace;
}

void avr_op_PUSH( word opcode, DecodedEntry *e)
{
    /*
     * Push Register on Stack.
     *
     * Opcode     : 1001 001d dddd 1111 
     * Usage      : PUSH  Rd
     * Operation  : STACK <- Rd
     * Flags      : None
     * Num Clocks : 2
     */
    e->p1 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_PUSH_do; else e->OpcodeFunction= avr_op_PUSH_do_trace;
}

void avr_op_RCALL( word opcode, DecodedEntry *e)
{
    /*
     * Relative Call Subroutine.
     *
     * Opcode     : 1101 kkkk kkkk kkkk 
     * Usage      : RCALL  k
     * Operation  : PC <- PC + k + 1
     * Flags      : None
     * Num Clocks : 3 / 4
     */
    e->p1 = n_bit_unsigned_to_signed( get_k_12(opcode), 12 );

    if (trace_on==0) e->OpcodeFunction= avr_op_RCALL_do; else e->OpcodeFunction= avr_op_RCALL_do_trace;
}

void avr_op_RET( word opcode, DecodedEntry *e)
{
    /*
     * Subroutine Return.
     *
     * Opcode     : 1001 0101 0000 1000 
     * Usage      : RET  
     * Operation  : PC <- STACK
     * Flags      : None
     * Num Clocks : 4 / 5
     */
    if (trace_on==0) e->OpcodeFunction= avr_op_RET_do; else e->OpcodeFunction= avr_op_RET_do_trace;
}

void avr_op_RETI( word opcode, DecodedEntry *e)
{
    /*
     * Interrupt Return.
     *
     * Opcode     : 1001 0101 0001 1000 
     * Usage      : RETI  
     * Operation  : PC <- STACK
     * Flags      : I
     * Num Clocks : 4 / 5
     */
    if (trace_on==0) e->OpcodeFunction= avr_op_RETI_do; else e->OpcodeFunction= avr_op_RETI_do_trace;
}

void avr_op_RJMP( word opcode, DecodedEntry *e)
{
    /*
     * Relative Jump.
     *
     * Opcode     : 1100 kkkk kkkk kkkk 
     * Usage      : RJMP  k
     * Operation  : PC <- PC + k + 1
     * Flags      : None
     * Num Clocks : 2
     */
    e->p1 = n_bit_unsigned_to_signed( get_k_12(opcode), 12 );

    if (trace_on==0) e->OpcodeFunction= avr_op_RJMP_do; else e->OpcodeFunction= avr_op_RJMP_do_trace;
}

void avr_op_ROR( word opcode, DecodedEntry *e)
{
    /*
     * Rotate Right Though Carry.
     *
     * Opcode     : 1001 010d dddd 0111 
     * Usage      : ROR  Rd
     * Operation  : Rd(7) <- C, Rd(n) <- Rd(n+1), C <- Rd(0)
     * Flags      : Z,C,N,V,S
     * Num Clocks : 1
     */
    e->p1 = get_rd_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_ROR_do; else e->OpcodeFunction= avr_op_ROR_do_trace;
}

void avr_op_SBC( word opcode, DecodedEntry *e)
{
    /*
     * Subtract with Carry.
     *
     * Opcode     : 0000 10rd dddd rrrr 
     * Usage      : SBC  Rd, Rr
     * Operation  : Rd <- Rd - Rr - C
     * Flags      : Z,C,N,V,S,H
     * Num Clocks : 1
     */

    e->p1 = get_rd_5(opcode);
    e->p2 = get_rr_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_SBC_do; else e->OpcodeFunction= avr_op_SBC_do_trace;
}

void avr_op_SBCI( word opcode, DecodedEntry *e)
{
    /*
     * Subtract Immediate with Carry.
     *
     * Opcode     : 0100 KKKK dddd KKKK 
     * Usage      : SBCI  Rd, K
     * Operation  : Rd <- Rd - K - C
     * Flags      : Z,C,N,V,S,H
     * Num Clocks : 1
     */

    e->p1 = get_rd_4(opcode);
    e->p2 = get_K_8(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_SBCI_do; else e->OpcodeFunction= avr_op_SBCI_do_trace;
}

void avr_op_SBI( word opcode, DecodedEntry *e)
{
    /*
     * Set Bit in I/O Register.
     *
     * Opcode     : 1001 1010 AAAA Abbb 
     * Usage      : SBI  A, b
     * Operation  : I/O(A, b) <- 1
     * Flags      : None
     * Num Clocks : 2
     */
    e->p1= get_A_5(opcode);
    e->p2= get_reg_bit(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_SBI_do; else e->OpcodeFunction= avr_op_SBI_do_trace;
}

void avr_op_SBIC( word opcode, DecodedEntry *e)
{
    /*
     * Skip if Bit in I/O Reg Cleared.
     *
     * Opcode     : 1001 1001 AAAA Abbb 
     * Usage      : SBIC  A, b
     * Operation  : if (I/O(A,b) = 0) PC <- PC + 2 or 3
     * Flags      : None
     * Num Clocks : 1 / 2 / 3
     */

    e->p1 = get_A_5(opcode);
    e->p2 = get_reg_bit(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_SBIC_do; else e->OpcodeFunction= avr_op_SBIC_do_trace;
}

void avr_op_SBIS( word opcode, DecodedEntry *e)
{
    /*
     * Skip if Bit in I/O Reg Set.
     *
     * Opcode     : 1001 1011 AAAA Abbb 
     * Usage      : SBIS  A, b
     * Operation  : if (I/O(A,b) = 1) PC <- PC + 2 or 3
     * Flags      : None
     * Num Clocks : 1 / 2 / 3
     */

    e->p1 = get_A_5(opcode);
    e->p2 = get_reg_bit(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_SBIS_do; else e->OpcodeFunction= avr_op_SBIS_do_trace;
}

void avr_op_SBIW( word opcode, DecodedEntry *e)
{
    /*
     * Subtract Immed from Word.
     *
     * Opcode     : 1001 0111 KKdd KKKK 
     * Usage      : SBIW  Rd, K
     * Operation  : Rd+1:Rd <- Rd+1:Rd - K
     * Flags      : Z,C,N,V,S
     * Num Clocks : 2
     */

    e->p1 = get_rd_2(opcode);
    e->p2 = get_K_6(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_SBIW_do; else e->OpcodeFunction= avr_op_SBIW_do_trace;
}

void avr_op_SBRC( word opcode, DecodedEntry *e)
{
    /*
     * Skip if Bit in Reg Cleared.
     *
     * Opcode     : 1111 110d dddd 0bbb 
     * Usage      : SBRC  Rd, b
     * Operation  : if (Rd(b) = 0) PC <- PC + 2 or 3
     * Flags      : None
     * Num Clocks : 1 / 2 / 3
     */

    e->p1 = get_rd_5(opcode);
    e->p2 = get_reg_bit(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_SBRC_do; else e->OpcodeFunction= avr_op_SBRC_do_trace;
}

void avr_op_SBRS( word opcode, DecodedEntry *e)
{
    /*
     * Skip if Bit in Reg Set.
     *
     * Opcode     : 1111 111d dddd 0bbb 
     * Usage      : SBRS  Rd, b
     * Operation  : if (Rd(b) = 1) PC <- PC + 2 or 3
     * Flags      : None
     * Num Clocks : 1 / 2 / 3
     */

    e->p1 = get_rd_5(opcode);
    e->p2 = get_reg_bit(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_SBRS_do; else e->OpcodeFunction= avr_op_SBRS_do_trace;
}

void avr_op_SLEEP( word opcode, DecodedEntry *e)
{
    /*
     * Sleep.
     *
     * This is device specific and should be overridden by sub-class.
     *
     * Opcode     : 1001 0101 1000 1000 
     * Usage      : SLEEP  
     * Operation  : (see specific hardware specification for Sleep)
     * Flags      : None
     * Num Clocks : 1
     */
    if (trace_on==0) e->OpcodeFunction= avr_op_SLEEP_do; else e->OpcodeFunction= avr_op_SLEEP_do_trace;
}

void avr_op_SPM( word opcode, DecodedEntry *e)
{
    /*
     * Store Program Memory.
     *
     * Opcode     : 1001 0101 1110 1000 
     * Usage      : SPM  
     * Operation  : (Z) <- R1:R0
     * Flags      : None
     * Num Clocks : -
     */
    if (trace_on==0) e->OpcodeFunction= avr_op_SPM_do; else e->OpcodeFunction= avr_op_SPM_do_trace;
}

void avr_op_STD_Y( word opcode, DecodedEntry *e)
{
    /*
     * Store Indirect with Displacement.
     *
     * Opcode     : 10q0 qq1d dddd 1qqq 
     * Usage      : STD  Y+q, Rd
     * Operation  : (Y + q) <- Rd
     * Flags      : None
     * Num Clocks : 2
     */

    e->p1 = get_q(opcode);
    e->p2 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_STD_Y_do; else e->OpcodeFunction= avr_op_STD_Y_do_trace;
}

void avr_op_STD_Z( word opcode, DecodedEntry *e)
{
    /*
     * Store Indirect with Displacement.
     *
     * Opcode     : 10q0 qq1d dddd 0qqq 
     * Usage      : STD  Z+q, Rd
     * Operation  : (Z + q) <- Rd
     * Flags      : None
     * Num Clocks : 2
     */

    e->p1 = get_q(opcode);
    e->p2 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_STD_Z_do; else e->OpcodeFunction= avr_op_STD_Z_do_trace;
}

void avr_op_STS( word opcode, DecodedEntry *e)
{
    /*
     * Store Direct to data space.
     *
     * Opcode     : 1001 001d dddd 0000 kkkk kkkk kkkk kkkk
     * Usage      : STS  k, Rd
     * Operation  : (k) <- Rd
     * Flags      : None
     * Num Clocks : 2
     */
    e->p2 = get_rd_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_STS_do; else e->OpcodeFunction= avr_op_STS_do_trace;
}

void avr_op_ST_X( word opcode, DecodedEntry *e)
{
    /*
     * Store Indirect using index X.
     *
     * Opcode     : 1001 001d dddd 1100 
     * Usage      : ST  X, Rd
     * Operation  : (X) <- Rd
     * Flags      : None
     * Num Clocks : 2
     */

    e->p2 = get_rd_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_ST_X_do; else e->OpcodeFunction= avr_op_ST_X_do_trace;
}

void avr_op_ST_X_decr( word opcode, DecodedEntry *e)
{
    /*
     * Store Indirect and Pre-Decrement using index X.
     *
     * Opcode     : 1001 001d dddd 1110 
     * Usage      : ST  -X, Rd
     * Operation  : X <- X - 1, (X) <- Rd
     * Flags      : None
     * Num Clocks : 2
     */

    e->p2 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_ST_X_decr_do; else e->OpcodeFunction= avr_op_ST_X_decr_do_trace;
}

void avr_op_ST_X_incr( word opcode, DecodedEntry *e)
{
    /*
     * Store Indirect and Post-Increment using index X.
     *
     * Opcode     : 1001 001d dddd 1101 
     * Usage      : ST  X+, Rd
     * Operation  : (X) <- Rd, X <- X + 1
     * Flags      : None
     * Num Clocks : 2
     */

    e->p2 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_ST_X_incr_do; else e->OpcodeFunction= avr_op_ST_X_incr_do_trace;
}

void avr_op_ST_Y_decr( word opcode, DecodedEntry *e)
{
    /*
     * Store Indirect and Pre-Decrement using index Y.
     *
     * Opcode     : 1001 001d dddd 1010 
     * Usage      : ST  -Y, Rd
     * Operation  : Y <- Y - 1, (Y) <- Rd
     * Flags      : None
     * Num Clocks : 2
     */

    e->p2 = get_rd_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_ST_Y_decr_do; else e->OpcodeFunction= avr_op_ST_Y_decr_do_trace;
}

void avr_op_ST_Y_incr( word opcode, DecodedEntry *e)
{
    /*
     * Store Indirect and Post-Increment using index Y.
     *
     * Opcode     : 1001 001d dddd 1001 
     * Usage      : ST  Y+, Rd
     * Operation  : (Y) <- Rd, Y <- Y + 1
     * Flags      : None
     * Num Clocks : 2
     */
    e->p2 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_ST_Y_incr_do; else e->OpcodeFunction= avr_op_ST_Y_incr_do_trace;
}

void avr_op_ST_Z_decr( word opcode, DecodedEntry *e)
{
    /*
     * Store Indirect and Pre-Decrement using index Z.
     *
     * Opcode     : 1001 001d dddd 0010 
     * Usage      : ST  -Z, Rd
     * Operation  : Z <- Z - 1, (Z) <- Rd
     * Flags      : None
     * Num Clocks : 2
     */
    e->p2 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_ST_Z_decr_do; else e->OpcodeFunction= avr_op_ST_Z_decr_do_trace;
}

void avr_op_ST_Z_incr( word opcode, DecodedEntry *e)
{
    /*
     * Store Indirect and Post-Increment using index Z.
     *
     * Opcode     : 1001 001d dddd 0001 
     * Usage      : ST  Z+, Rd
     * Operation  : (Z) <- Rd, Z <- Z + 1
     * Flags      : None
     * Num Clocks : 2
     */

    e->p2 = get_rd_5(opcode);

    if (trace_on==0) e->OpcodeFunction= avr_op_ST_Z_incr_do; else e->OpcodeFunction= avr_op_ST_Z_incr_do_trace;
}

void avr_op_SUB( word opcode, DecodedEntry *e)
{
    /*
     * Subtract without Carry.
     *
     * Opcode     : 0001 10rd dddd rrrr 
     * Usage      : SUB  Rd, Rr
     * Operation  : Rd <- Rd - Rr
     * Flags      : Z,C,N,V,S,H
     * Num Clocks : 1
     */

    e->p1 = get_rd_5(opcode);
    e->p2 = get_rr_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_SUB_do; else e->OpcodeFunction= avr_op_SUB_do_trace;
}

void avr_op_SUBI( word opcode, DecodedEntry *e)
{ 
    /*
     * Subtract Immediate.
     *
     * Opcode     : 0101 KKKK dddd KKKK 
     * Usage      : SUBI  Rd, K
     * Operation  : Rd <- Rd - K
     * Flags      : Z,C,N,V,S,H
     * Num Clocks : 1
     */
    e->p1 = get_rd_4(opcode);
    e->p2 = get_K_8(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_SUBI_do; else e->OpcodeFunction= avr_op_SUBI_do_trace;
}

void avr_op_SWAP( word opcode, DecodedEntry *e)
{
    /*
     * Swap Nibbles.
     * 
     * Opcode     : 1001 010d dddd 0010 
     * Usage      : SWAP  Rd
     * Operation  : Rd(3..0) <--> Rd(7..4)
     * Flags      : None
     * Num Clocks : 1
     */
    e->p1 = get_rd_5(opcode);
    if (trace_on==0) e->OpcodeFunction= avr_op_SWAP_do; else e->OpcodeFunction= avr_op_SWAP_do_trace;
}

void avr_op_WDR( word opcode, DecodedEntry *e)
{ 
    /* 
     * Watchdog Reset.
     * 
     * This is device specific and must be overridden by sub-class.
     *
     * Opcode     : 1001 0101 1010 1000 
     * Usage      : WDR  
     * Operation  : (see specific hardware specification for WDR)
     * Flags      : None
     * Num Clocks : 1
     */
    if (trace_on==0) e->OpcodeFunction= avr_op_WDR_do; else e->OpcodeFunction= avr_op_WDR_do_trace;
}

OpcodePrepare lookup_opcode( word opcode )
{
    int decode;

    switch (opcode) {
        /* opcodes with no operands */
        case 0x9519: return avr_op_EICALL;              /* 1001 0101 0001 1001 | EICALL */
        case 0x9419: return avr_op_EIJMP;               /* 1001 0100 0001 1001 | EIJMP */
        case 0x95D8: return avr_op_ELPM;                /* 1001 0101 1101 1000 | ELPM */
        case 0x95F8: return avr_op_ESPM;                /* 1001 0101 1111 1000 | ESPM */
        case 0x9509: return avr_op_ICALL;               /* 1001 0101 0000 1001 | ICALL */
        case 0x9409: return avr_op_IJMP;                /* 1001 0100 0000 1001 | IJMP */
        case 0x95C8: return avr_op_LPM;                 /* 1001 0101 1100 1000 | LPM */
        case 0x0000: return avr_op_NOP;                 /* 0000 0000 0000 0000 | NOP */
        case 0x9508: return avr_op_RET;                 /* 1001 0101 0000 1000 | RET */
        case 0x9518: return avr_op_RETI;                /* 1001 0101 0001 1000 | RETI */
        case 0x9588: return avr_op_SLEEP;               /* 1001 0101 1000 1000 | SLEEP */
        case 0x95E8: return avr_op_SPM;                 /* 1001 0101 1110 1000 | SPM */
        case 0x95A8: return avr_op_WDR;                 /* 1001 0101 1010 1000 | WDR */
        default:
                     {
                         /* opcodes with two 5-bit register (Rd and Rr) operands */
                         decode = opcode & ~(mask_Rd_5 | mask_Rr_5);
                         switch ( decode ) {
                             case 0x1C00: return avr_op_ADC;         /* 0001 11rd dddd rrrr | ADC or ROL */
                             case 0x0C00: return avr_op_ADD;         /* 0000 11rd dddd rrrr | ADD or LSL */
                             case 0x2000: return avr_op_AND;         /* 0010 00rd dddd rrrr | AND or TST */
                             case 0x1400: return avr_op_CP;          /* 0001 01rd dddd rrrr | CP */
                             case 0x0400: return avr_op_CPC;         /* 0000 01rd dddd rrrr | CPC */
                             case 0x1000: return avr_op_CPSE;        /* 0001 00rd dddd rrrr | CPSE */
                             case 0x2400: return avr_op_EOR;         /* 0010 01rd dddd rrrr | EOR or CLR */
                             case 0x2C00: return avr_op_MOV;         /* 0010 11rd dddd rrrr | MOV */
                             case 0x9C00: return avr_op_MUL;         /* 1001 11rd dddd rrrr | MUL */
                             case 0x2800: return avr_op_OR;          /* 0010 10rd dddd rrrr | OR */
                             case 0x0800: return avr_op_SBC;         /* 0000 10rd dddd rrrr | SBC */
                             case 0x1800: return avr_op_SUB;         /* 0001 10rd dddd rrrr | SUB */
                         }

                         /* opcode with a single register (Rd) as operand */
                         decode = opcode & ~(mask_Rd_5);
                         switch (decode) {
                             case 0x9405: return avr_op_ASR;         /* 1001 010d dddd 0101 | ASR */
                             case 0x9400: return avr_op_COM;         /* 1001 010d dddd 0000 | COM */
                             case 0x940A: return avr_op_DEC;         /* 1001 010d dddd 1010 | DEC */
                             case 0x9006: return avr_op_ELPM_Z;      /* 1001 000d dddd 0110 | ELPM */
                             case 0x9007: return avr_op_ELPM_Z_incr; /* 1001 000d dddd 0111 | ELPM */
                             case 0x9403: return avr_op_INC;         /* 1001 010d dddd 0011 | INC */
                             case 0x9000: return avr_op_LDS;         /* 1001 000d dddd 0000 | LDS */
                             case 0x900C: return avr_op_LD_X;        /* 1001 000d dddd 1100 | LD */
                             case 0x900E: return avr_op_LD_X_decr;   /* 1001 000d dddd 1110 | LD */
                             case 0x900D: return avr_op_LD_X_incr;   /* 1001 000d dddd 1101 | LD */
                             case 0x900A: return avr_op_LD_Y_decr;   /* 1001 000d dddd 1010 | LD */
                             case 0x9009: return avr_op_LD_Y_incr;   /* 1001 000d dddd 1001 | LD */
                             case 0x9002: return avr_op_LD_Z_decr;   /* 1001 000d dddd 0010 | LD */
                             case 0x9001: return avr_op_LD_Z_incr;   /* 1001 000d dddd 0001 | LD */
                             case 0x9004: return avr_op_LPM_Z;       /* 1001 000d dddd 0100 | LPM */
                             case 0x9005: return avr_op_LPM_Z_incr;  /* 1001 000d dddd 0101 | LPM */
                             case 0x9406: return avr_op_LSR;         /* 1001 010d dddd 0110 | LSR */
                             case 0x9401: return avr_op_NEG;         /* 1001 010d dddd 0001 | NEG */
                             case 0x900F: return avr_op_POP;         /* 1001 000d dddd 1111 | POP */
                             case 0x920F: return avr_op_PUSH;        /* 1001 001d dddd 1111 | PUSH */
                             case 0x9407: return avr_op_ROR;         /* 1001 010d dddd 0111 | ROR */
                             case 0x9200: return avr_op_STS;         /* 1001 001d dddd 0000 | STS */
                             case 0x920C: return avr_op_ST_X;        /* 1001 001d dddd 1100 | ST */
                             case 0x920E: return avr_op_ST_X_decr;   /* 1001 001d dddd 1110 | ST */
                             case 0x920D: return avr_op_ST_X_incr;   /* 1001 001d dddd 1101 | ST */
                             case 0x920A: return avr_op_ST_Y_decr;   /* 1001 001d dddd 1010 | ST */
                             case 0x9209: return avr_op_ST_Y_incr;   /* 1001 001d dddd 1001 | ST */
                             case 0x9202: return avr_op_ST_Z_decr;   /* 1001 001d dddd 0010 | ST */
                             case 0x9201: return avr_op_ST_Z_incr;   /* 1001 001d dddd 0001 | ST */
                             case 0x9402: return avr_op_SWAP;        /* 1001 010d dddd 0010 | SWAP */
                         }

                         /* opcodes with a register (Rd) and a constant data (K) as operands */
                         decode = opcode & ~(mask_Rd_4 | mask_K_8);
                         switch ( decode ) {
                             case 0x7000: return avr_op_ANDI;        /* 0111 KKKK dddd KKKK | CBR or ANDI */
                             case 0x3000: return avr_op_CPI;         /* 0011 KKKK dddd KKKK | CPI */
                             case 0xE000: return avr_op_LDI;         /* 1110 KKKK dddd KKKK | LDI or SER */
                             case 0x6000: return avr_op_ORI;         /* 0110 KKKK dddd KKKK | SBR or ORI */
                             case 0x4000: return avr_op_SBCI;        /* 0100 KKKK dddd KKKK | SBCI */
                             case 0x5000: return avr_op_SUBI;        /* 0101 KKKK dddd KKKK | SUBI */
                         }

                         /* opcodes with a register (Rd) and a register bit number (b) as operands */
                         decode = opcode & ~(mask_Rd_5 | mask_reg_bit);
                         switch ( decode ) {
                             case 0xF800: return avr_op_BLD;         /* 1111 100d dddd 0bbb | BLD */
                             case 0xFA00: return avr_op_BST;         /* 1111 101d dddd 0bbb | BST */
                             case 0xFC00: return avr_op_SBRC;        /* 1111 110d dddd 0bbb | SBRC */
                             case 0xFE00: return avr_op_SBRS;        /* 1111 111d dddd 0bbb | SBRS */
                         }

                         /* opcodes with a relative 7-bit address (k) and a register bit number (b) as operands */
                         decode = opcode & ~(mask_k_7 | mask_reg_bit);
                         switch ( decode ) {
                             case 0xF400: return avr_op_BRBC;        /* 1111 01kk kkkk kbbb | BRBC */
                             case 0xF000: return avr_op_BRBS;        /* 1111 00kk kkkk kbbb | BRBS */
                         }

                         /* opcodes with a 6-bit address displacement (q) and a register (Rd) as operands */
                         decode = opcode & ~(mask_Rd_5 | mask_q_displ);
                         switch ( decode ) {
                             case 0x8008: return avr_op_LDD_Y;       /* 10q0 qq0d dddd 1qqq | LDD */
                             case 0x8000: return avr_op_LDD_Z;       /* 10q0 qq0d dddd 0qqq | LDD */
                             case 0x8208: return avr_op_STD_Y;       /* 10q0 qq1d dddd 1qqq | STD */
                             case 0x8200: return avr_op_STD_Z;       /* 10q0 qq1d dddd 0qqq | STD */
                         }

                         /* opcodes with a absolute 22-bit address (k) operand */
                         decode = opcode & ~(mask_k_22);
                         switch ( decode ) {
                             case 0x940E: return avr_op_CALL;        /* 1001 010k kkkk 111k | CALL */
                             case 0x940C: return avr_op_JMP;         /* 1001 010k kkkk 110k | JMP */
                         }

                         /* opcode with a sreg bit select (s) operand */
                         decode = opcode & ~(mask_sreg_bit);
                         switch ( decode ) {
                             /* BCLR takes place of CL{C,Z,N,V,S,H,T,I} */
                             /* BSET takes place of SE{C,Z,N,V,S,H,T,I} */
                             case 0x9488: return avr_op_BCLR;        /* 1001 0100 1sss 1000 | BCLR */
                             case 0x9408: return avr_op_BSET;        /* 1001 0100 0sss 1000 | BSET */
                         }

                         /* opcodes with a 6-bit constant (K) and a register (Rd) as operands */
                         decode = opcode & ~(mask_K_6 | mask_Rd_2);
                         switch ( decode ) {
                             case 0x9600: return avr_op_ADIW;        /* 1001 0110 KKdd KKKK | ADIW */
                             case 0x9700: return avr_op_SBIW;        /* 1001 0111 KKdd KKKK | SBIW */
                         }

                         /* opcodes with a 5-bit IO Addr (A) and register bit number (b) as operands */
                         decode = opcode & ~(mask_A_5 | mask_reg_bit);
                         switch ( decode ) {
                             case 0x9800: return avr_op_CBI;         /* 1001 1000 AAAA Abbb | CBI */
                             case 0x9A00: return avr_op_SBI;         /* 1001 1010 AAAA Abbb | SBI */
                             case 0x9900: return avr_op_SBIC;        /* 1001 1001 AAAA Abbb | SBIC */
                             case 0x9B00: return avr_op_SBIS;        /* 1001 1011 AAAA Abbb | SBIS */
                         }

                         /* opcodes with a 6-bit IO Addr (A) and register (Rd) as operands */
                         decode = opcode & ~(mask_A_6 | mask_Rd_5);
                         switch ( decode ) {
                             case 0xB000: return avr_op_IN;          /* 1011 0AAd dddd AAAA | IN */
                             case 0xB800: return avr_op_OUT;         /* 1011 1AAd dddd AAAA | OUT */
                         }

                         /* opcodes with a relative 12-bit address (k) operand */
                         decode = opcode & ~(mask_k_12);
                         switch ( decode ) {
                             case 0xD000: return avr_op_RCALL;       /* 1101 kkkk kkkk kkkk | RCALL */
                             case 0xC000: return avr_op_RJMP;        /* 1100 kkkk kkkk kkkk | RJMP */
                         }

                         /* opcodes with two 4-bit register (Rd and Rr) operands */
                         decode = opcode & ~(mask_Rd_4 | mask_Rr_4);
                         switch ( decode ) {
                             case 0x0100: return avr_op_MOVW;        /* 0000 0001 dddd rrrr | MOVW */
                             case 0x0200: return avr_op_MULS;        /* 0000 0010 dddd rrrr | MULS */
                         }

                         /* opcodes with two 3-bit register (Rd and Rr) operands */
                         decode = opcode & ~(mask_Rd_3 | mask_Rr_3);
                         switch ( decode ) {
                             case 0x0300: return avr_op_MULSU;       /* 0000 0011 0ddd 0rrr | MULSU */
                             case 0x0308: return avr_op_FMUL;        /* 0000 0011 0ddd 1rrr | FMUL */
                             case 0x0380: return avr_op_FMULS;       /* 0000 0011 1ddd 0rrr | FMULS */
                             case 0x0388: return avr_op_FMULSU;      /* 0000 0011 1ddd 1rrr | FMULSU */
                         }

                     } /* default */
    } /* first switch */

    return NULL;

} /* decode opcode function */

static int ass_do_InvalidOpcode(AvrDevice *, int p1, int p2) {
    if (trace_on) {
        traceOut<< "INVALID INSTRUCTION" << endl;
    } else {
        cerr << "INVALID INSTRUCTION" << endl;
    }


    return INVALID_OPCODE;
}

void decode_single_instruction( unsigned char* MemSrc, DecodedEntry *decoded_mem, /*int currentSize,*/ int addr) {
    addr/=2;

    word *MemPtr=(word*)(MemSrc);
    word opcode;

    opcode=((MemPtr[addr])>>8)+((MemPtr[addr]&0xff)<<8);
    OpcodePrepare x=lookup_opcode(opcode);
    if (x!=0) {
        (x)(opcode, &decoded_mem[addr]);
    } else {
        decoded_mem[addr].OpcodeFunction=ass_do_InvalidOpcode;
        decoded_mem[addr].p1=0;
        decoded_mem[addr].p2=0;
    }
}

