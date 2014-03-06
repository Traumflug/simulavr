/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003 Theodore A. Roth,  Klaus Rudolph		
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

#ifndef DECODER
#define DECODER

#include <iostream>

#include "rwmem.h"
#include "types.h"
#include "avrdevice.h"

class AvrFlash;

//! Base class of core instruction
/*! All instruction are derived from this class */
class DecodedInstruction {
    
    protected:
        AvrDevice *core; //!< Link to device instance
        bool size2Word; //!< Flag: true, if instruction has 2 words

    public:
        DecodedInstruction(AvrDevice *c, bool s2w = false): core(c), size2Word(s2w) {}
        virtual ~DecodedInstruction() {}

        //! Returns true, if instruction need 2 words (4byte)
        bool IsInstruction2Words() { return size2Word; } 

        //! Performs instruction
        virtual int operator()() = 0;
        //! Performs instruction and write out instruction mnemonic for trace
        virtual int Trace() = 0;
		//! If this instruction modifies a R0-R31 register then return its number, otherwise -1.
		virtual unsigned char GetModifiedR() const {return -1;}
		//! If this instruction modifies a pair of R0-R31 registers then ...
		virtual unsigned char GetModifiedRHi() const {return -1;}
};

//! Translates an opcode to a instance of DecodedInstruction
DecodedInstruction* lookup_opcode(word opcode, AvrDevice *core);

class avr_op_ADC: public DecodedInstruction {
    /*
     * Add with Carry.
     *       
     * Opcode     : 0001 11rd dddd rrrr 
     * Usage      : ADC  Rd, Rr
     * Operation  : Rd <- Rd + Rr + C
     * Flags      : Z,C,N,V,S,H
     * Num Clocks : 1
     */
    protected:
        unsigned char R1;
        unsigned char R2;
        HWSreg *status;

    public:
        avr_op_ADC(word opcode, AvrDevice *c);
        virtual unsigned char GetModifiedR() const;
        int operator()();
        int Trace(); 
}; //end of class 

class avr_op_ADD: public DecodedInstruction {
    /*
     * Add without Carry.
     *
     * Opcode     : 0000 11rd dddd rrrr 
     * Usage      : ADD  Rd, Rr
     * Operation  : Rd <- Rd + Rr
     * Flags      : Z,C,N,V,S,H
     * Num Clocks : 1
     */
    protected:
        unsigned char R1;
        unsigned char R2;
        HWSreg *status;

    public:
        avr_op_ADD(word opcode, AvrDevice *c); 
        virtual unsigned char GetModifiedR() const;
        int operator()();
        int Trace(); 
}; //end of class 




class avr_op_ADIW: public DecodedInstruction
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

    protected:
        unsigned char Rl;
        unsigned char Rh;
        unsigned char K;
        HWSreg *status;

    public:
        avr_op_ADIW(word opcode, AvrDevice *c);
        virtual unsigned char GetModifiedR() const;
        virtual unsigned char GetModifiedRHi() const;
        int operator()();
        int Trace();
};

class avr_op_AND: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char R2;
        HWSreg *status;

    public:
        avr_op_AND(word opcode, AvrDevice *c); 
        int operator()();
        int Trace();
};

class avr_op_ANDI: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char K;
        HWSreg *status;

    public:
        avr_op_ANDI(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ASR:public DecodedInstruction
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

    protected:
        unsigned char R1;
        HWSreg *status;

    public:
        avr_op_ASR(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_BCLR: public DecodedInstruction
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

    protected:
        HWSreg *status;
        unsigned char Kbit;

    public:
        avr_op_BCLR(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};


class avr_op_BLD: public DecodedInstruction
{
    /* Bit load from T to Register.
     *
     * Opcode     : 1111 100d dddd 0bbb 
     * Usage      : BLD  Rd, b
     * Operation  : Rd(b) <- T
     * Flags      : None
     * Num Clocks : 1
     */

    protected:
        unsigned char R1;
        unsigned char Kbit;
        HWSreg *status;

    public:
        avr_op_BLD(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_BRBC: public DecodedInstruction
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

    protected:
        HWSreg *status;
        unsigned char bitmask;
        signed char offset;

    public:
        avr_op_BRBC(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_BRBS: public DecodedInstruction
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

    protected:
        HWSreg *status;
        unsigned char bitmask;
        signed char offset;

    public:
        avr_op_BRBS(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_BSET: public DecodedInstruction
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

    protected:
        HWSreg *status;
        unsigned char Kbit;

    public:
        avr_op_BSET(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_BST: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char Kbit;
        HWSreg *status;

    public:
        avr_op_BST(word opcode, AvrDevice *c);
        int operator()();
        int Trace();

};

class avr_op_CALL: public DecodedInstruction
{
    /*
     * Call Subroutine.
     *
     * Opcode     : 1001 010k kkkk 111k kkkk kkkk kkkk kkkk
     * Usage      : CALL  k
     * Operation  : PC <- k
     * Flags      : None
     * Num Clocks : 3 / 4 / 5
     */

    protected:
        unsigned char KH;

    public:
        avr_op_CALL(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_CBI: public DecodedInstruction
{
    /*
     * Clear Bit in I/O Register.
     *
     * Opcode     : 1001 1000 AAAA Abbb 
     * Usage      : CBI  A, b
     * Operation  : I/O(A, b) <- 0
     * Flags      : None
     * Num Clocks : 1 / 2
     */

    protected:
        unsigned char ioreg;
        unsigned char Kbit;
        HWSreg *status;

    public:
        avr_op_CBI(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_COM: public DecodedInstruction
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

    protected:
        unsigned char R1;
        HWSreg *status;

    public:
        avr_op_COM(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_CP: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char R2;
        HWSreg *status;

    public:
        avr_op_CP(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_CPC: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char R2;
        HWSreg *status;

    public:
        avr_op_CPC(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_CPI: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char K;
        HWSreg *status;

    public:
        avr_op_CPI(word opcode, AvrDevice *c);
        int operator()();
        int Trace();

};

class avr_op_CPSE: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char R2;
        HWSreg *status;

    public:
        avr_op_CPSE(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_DEC: public DecodedInstruction
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

    protected:
        unsigned char R1;
        HWSreg *status;

    public:
        avr_op_DEC(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_EICALL: public DecodedInstruction
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

    public:
        avr_op_EICALL(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_EIJMP: public DecodedInstruction
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

    public:
        avr_op_EIJMP(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ELPM_Z: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_ELPM_Z(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ELPM_Z_incr: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_ELPM_Z_incr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ELPM: public DecodedInstruction
{
    /*
     * Extended Load Program Memory.
     *
     *
     * Opcode     : 1001 0101 1101 1000 
     * Usage      : ELPM  
     * Operation  : R0 <- (RAMPZ:Z)
     * Flags      : None
     * Num Clocks : 3
     */

    public:
        avr_op_ELPM(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_EOR: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char R2;
        HWSreg *status;

    public:
        avr_op_EOR(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ESPM: public DecodedInstruction
{
    /*
     * Extended Store Program Memory.
     * (In datasheet: "SPM #2– Store Program Memory")
     *
     * Opcode     : 1001 0101 1111 1000 
     * Usage      : ESPM  
     * Operation  : (RAMPZ:Z) <- R1:R0
     * Flags      : None
     * Num Clocks : -
     */

    public:
        avr_op_ESPM(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_FMUL:public DecodedInstruction
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

    protected:
        unsigned char Rd;
        unsigned char Rr;
        HWSreg *status;

    public:
        avr_op_FMUL(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_FMULS: public DecodedInstruction
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

    protected:
        unsigned char Rd;
        unsigned char Rr;
        HWSreg *status;

    public:
        avr_op_FMULS(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_FMULSU: public DecodedInstruction
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

    protected:
        unsigned char Rd;
        unsigned char Rr;
        HWSreg *status;

    public:
        avr_op_FMULSU(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ICALL: public DecodedInstruction
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

    public:
        avr_op_ICALL(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_IJMP: public DecodedInstruction
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

    public:
        avr_op_IJMP (word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_IN: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char ioreg;

    public:
        avr_op_IN(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_INC: public DecodedInstruction
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

    protected:
        unsigned char R1;
        HWSreg *status;

    public:
        avr_op_INC(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_JMP: public DecodedInstruction
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
    
    protected:
        unsigned int K;

    public:
        avr_op_JMP (word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LDD_Y: public DecodedInstruction
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

    protected:
        unsigned char Rd;
        unsigned char K;

    public:
        avr_op_LDD_Y(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LDD_Z: public DecodedInstruction
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

    protected:
        unsigned char Rd;
        unsigned char K;

    public:
        avr_op_LDD_Z(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LDI: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char K;

    public:
        avr_op_LDI(word opcode, AvrDevice *c);
        virtual unsigned char GetModifiedR() const;
        int operator()();
        int Trace();
};

class avr_op_LDS: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_LDS(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LD_X: public DecodedInstruction
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

    protected:
        unsigned char Rd;

    public:
        avr_op_LD_X(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LD_X_decr: public DecodedInstruction
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

    protected:
        unsigned char Rd;

    public:
        avr_op_LD_X_decr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LD_X_incr: public DecodedInstruction
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

    protected:
        unsigned char Rd;

    public:
        avr_op_LD_X_incr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LD_Y_decr: public DecodedInstruction
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


    protected:
        unsigned char Rd;

    public:
        avr_op_LD_Y_decr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LD_Y_incr: public DecodedInstruction
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

    protected:
        unsigned char Rd;

    public:
        avr_op_LD_Y_incr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LD_Z_incr: public DecodedInstruction
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

    protected:
        unsigned char Rd;

    public:
        avr_op_LD_Z_incr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LD_Z_decr: public DecodedInstruction
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

    protected:
        unsigned char Rd;

    public:
        avr_op_LD_Z_decr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LPM_Z: public DecodedInstruction
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

    protected:
        unsigned char Rd;

    public:
        avr_op_LPM_Z(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LPM: public DecodedInstruction
{
    /* Load Program Memory.
     *
     * This the same as avr_op_LPM_Z:public DecodedInstruction
     *
     * Opcode     : 1001 0101 1100 1000 
     * Usage      : LPM  
     * Operation  : R0 <- (Z)
     * Flags      : None
     * Num Clocks : 3
     */
    //return avr_op_LPM_Z:public DecodedInstruction

    public:
        avr_op_LPM(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LPM_Z_incr: public DecodedInstruction
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

    protected:
        unsigned char Rd;

    public:
        avr_op_LPM_Z_incr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_LSR: public DecodedInstruction
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

    protected:
        unsigned char Rd;
        HWSreg *status;

    public:
        avr_op_LSR(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_MOV: public DecodedInstruction
{
    /* Copy Register.
     *
     * Opcode     : 0010 11rd dddd rrrr 
     * Usage      : MOV  Rd, Rr
     * Operation  : Rd <- Rr
     * Flags      : None
     * Num Clocks : 1
     */

    protected:
        unsigned char R1;
        unsigned char R2;

    public:
        avr_op_MOV(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_MOVW: public DecodedInstruction
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

    protected:
        unsigned char Rd;
        unsigned char Rs;

    public:
        avr_op_MOVW(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_MUL: public DecodedInstruction
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

    protected:
        unsigned char Rd;
        unsigned char Rr;
        HWSreg *status;

    public:
        avr_op_MUL(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_MULS: public DecodedInstruction
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

    protected:
        unsigned char Rd;
        unsigned char Rr;
        HWSreg *status;

    public:
        avr_op_MULS(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_MULSU: public DecodedInstruction
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


    protected:
        unsigned char Rd;
        unsigned char Rr;
        HWSreg *status;

    public:
        avr_op_MULSU(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_NEG: public DecodedInstruction
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

    protected:
        unsigned char Rd;
        HWSreg *status;

    public:
        avr_op_NEG(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_NOP: public DecodedInstruction
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


    public:
        avr_op_NOP(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_OR:public DecodedInstruction
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

    protected:
        unsigned char Rd;
        unsigned char Rr;
        HWSreg *status;

    public:
        avr_op_OR(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ORI: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char K;
        HWSreg *status;

    public:
        avr_op_ORI(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_OUT: public DecodedInstruction
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

    protected:
        unsigned char ioreg;
        unsigned char R1;

    public:
        avr_op_OUT(word opcode, AvrDevice *c);
        int operator()();
        int Trace();

    friend class AvrFlash;  // AvrFlash::LooksLikeContextSwitch() needs to read ioreg
};

class avr_op_POP: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_POP(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_PUSH: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_PUSH(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_RCALL: public DecodedInstruction
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

    protected:
        signed int K;

    public:
        avr_op_RCALL(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_RET: public DecodedInstruction
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

    public:
        avr_op_RET(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_RETI: public DecodedInstruction
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

    protected:
        HWSreg *status;

    public:
        avr_op_RETI(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_RJMP: public DecodedInstruction
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

    protected:
        signed int K;

    public:
        avr_op_RJMP(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ROR: public DecodedInstruction
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

    protected:
        unsigned char R1;
        HWSreg *status;

    public:
        avr_op_ROR(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_SBC: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char R2;
        HWSreg *status;

    public:
        avr_op_SBC(word opcode, AvrDevice *c);
        virtual unsigned char GetModifiedR() const;
        int operator()();
        int Trace();
};

class avr_op_SBCI: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char K;
        HWSreg *status;

    public:
        avr_op_SBCI(word opcode, AvrDevice *c);
        virtual unsigned char GetModifiedR() const;
        int operator()();
        int Trace();
};

class avr_op_SBI: public DecodedInstruction
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

    protected:
        unsigned char ioreg;
        unsigned char Kbit;

    public:
        avr_op_SBI(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_SBIC: public DecodedInstruction
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

    protected:
        unsigned char ioreg;
        unsigned char Kbit;

    public:
        avr_op_SBIC(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_SBIS: public DecodedInstruction
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

    protected:
        unsigned char ioreg;
        unsigned char Kbit;

    public:
        avr_op_SBIS(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_SBIW: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char K;
        HWSreg *status;

    public:
        avr_op_SBIW(word opcode, AvrDevice *c);
        virtual unsigned char GetModifiedR() const;
        virtual unsigned char GetModifiedRHi() const;
        int operator()();
        int Trace();
};

class avr_op_SBRC: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char Kbit;

    public:
        avr_op_SBRC(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_SBRS: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char Kbit;

    public:
        avr_op_SBRS(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

/*! \todo SLEEP instruction not implemented */
class avr_op_SLEEP: public DecodedInstruction
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


    public:
        avr_op_SLEEP(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_SPM: public DecodedInstruction
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

    public:
        avr_op_SPM(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_STD_Y: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char K;

    public:
        avr_op_STD_Y(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_STD_Z: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char K;

    public:
        avr_op_STD_Z(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_STS: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_STS(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ST_X: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_ST_X(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ST_X_decr: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_ST_X_decr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ST_X_incr: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_ST_X_incr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ST_Y_decr: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_ST_Y_decr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ST_Y_incr: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_ST_Y_incr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ST_Z_decr: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_ST_Z_decr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ST_Z_incr: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_ST_Z_incr(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_SUB: public DecodedInstruction
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

    protected:
        unsigned char R1;
        unsigned char R2;
        HWSreg *status;

    public:
        avr_op_SUB(word opcode, AvrDevice *c);
        virtual unsigned char GetModifiedR() const;
        int operator()();
        int Trace();
};

class avr_op_SUBI: public DecodedInstruction
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

    protected:
        unsigned char R1;
        HWSreg *status;
        unsigned char K;

    public:
        avr_op_SUBI(word opcode, AvrDevice *c);
        virtual unsigned char GetModifiedR() const;
        int operator()();
        int Trace();
};

class avr_op_SWAP: public DecodedInstruction
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

    protected:
        unsigned char R1;

    public:
        avr_op_SWAP(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_WDR: public DecodedInstruction
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

    public:
        avr_op_WDR(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_BREAK: public DecodedInstruction
{
    /*
     * Halts execution
     *
     * TODO Check if execution is continuable on a real MCU
     *
     * Opcode     : 1001 0101 1001 1000
     * Usage      : BREAK
     * Operation  : (see specific hardware specification for BREAK)
     * Flags      : None
     * Num Clocks : N/A
     */

    public:
        avr_op_BREAK(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

class avr_op_ILLEGAL: public DecodedInstruction
{ 
    //illegal instruction

    public:
        avr_op_ILLEGAL(word opcode, AvrDevice *c);
        int operator()();
        int Trace();
};

#endif

