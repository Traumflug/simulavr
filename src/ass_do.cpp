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
#include "types.h"
#include "decoder.h"
#include "avrdevice.h"
#include "ass_do.h"
#include "ass_do_trace.h"

//only needed for debugging
//#include <iostream>
#include "flash.h"
#include "hwstack.h"
#include "rwmem.h"
#include "hwsreg.h"
#include "hwwado.h"




/******************************************************************************\
 *
 * Helper functions for calculating the status register bit values.
 * See the Atmel data sheet for the instuction set for more info.
 *
 ******************************************************************************/
//Rebuild these function !! TODO This is ugly here
bool is_next_inst_2_words(AvrDevice *core) {
	int (*fp)(AvrDevice *, int p1, int p2)= core->Flash->DecodedMem[(core->PC)+1].OpcodeFunction;
	return ( (fp == avr_op_CALL_do) ||
			(fp == avr_op_JMP_do)  ||
			(fp == avr_op_LDS_do)  ||
			(fp == avr_op_STS_do)  ||
			(fp == avr_op_CALL_do_trace) ||
			(fp == avr_op_JMP_do_trace)  ||
			(fp == avr_op_LDS_do_trace)  ||
			(fp == avr_op_STS_do_trace) );

}

void avr_core_stack_push( AvrDevice *core, int cnt, long val) {
	for (int tt=0; tt<cnt; tt++) {
		core->stack->Push(val&0xff);
		val>>=8;
	}
}

dword avr_core_stack_pop( AvrDevice *core, int cnt) {
	dword val=0;
	for (int tt=0; tt<cnt; tt++) {
		val=val<<8;
		val+=core->stack->Pop();
	}
	return val;
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
int avr_op_ADC_do( AvrDevice *core, int p1, int p2) 
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


	byte rd = (*(core->R))[p1];
	byte rr = (*(core->R))[p2]; 

	unsigned char res = rd + rr + core->status->C;

	core->status->H = get_add_carry( res, rd, rr, 3 ) ;
	core->status->V = get_add_overflow( res, rd, rr ) ;
	core->status->N = ((res >> 7) & 0x1)              ;
	core->status->S = (core->status->N ^ core->status->V)   ;
	core->status->Z = ((res & 0xff) == 0)             ;
	core->status->C = get_add_carry( res, rd, rr, 7 ) ;

	(*(core->R))[p1]=res;

	return 1;	//used clocks
}

int avr_op_ADD_do( AvrDevice *core, int p1, int p2 )
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

	byte rd = (*(core->R))[p1];
	byte rr = (*(core->R))[p2];
	byte res = rd + rr;

	core->status->H = get_add_carry( res, rd, rr, 3 ) ;
	core->status->V = get_add_overflow( res, rd, rr ) ;
	core->status->N = ((res >> 7) & 0x1)              ;
	core->status->S = (core->status->N ^ core->status->V)   ;
	core->status->Z = ((res & 0xff) == 0)             ;
	core->status->C = get_add_carry( res, rd, rr, 7 ) ;

	(*(core->R))[p1]=res;
	return 1; 
}

int avr_op_ADIW_do( AvrDevice *core, int p1, int p2 )
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
	byte rdl = (*(core->R))[p1];
	byte rdh = (*(core->R))[p1+1];

	word rd = (rdh << 8) + rdl;
	word res = rd + p2;

	core->status->V = (~(rdh >> 7 & 0x1) & (res >> 15 & 0x1)) ;
	core->status->N = ((res >> 15) & 0x1)                     ;
	core->status->S = (core->status->N ^ core->status->V)                                 ;
	core->status->Z = ((res & 0xffff) == 0)                   ;
	core->status->C = (~(res >> 15 & 0x1) & (rdh >> 7 & 0x1)) ;

	(*(core->R))[p1]=res &0xff;
	(*(core->R))[p1+1]= res>>8;

	return 2; 
}

int avr_op_AND_do( AvrDevice *core, int p1, int p2 )
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
	byte rd = (*(core->R))[p1]; 
	byte rr = (*(core->R))[p2];
	byte res = rd & rr;

	core->status->V = 0                   ;
	core->status->N = ((res >> 7) & 0x1)  ;
	core->status->S = (core->status->N ^ core->status->V)             ;
	core->status->Z = ((res & 0xff) == 0) ;

	(*(core->R))[p1]=res;

	return 1; 
}

int avr_op_ANDI_do( AvrDevice *core, int p1, int p2 )
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
	byte rd = (*(core->R))[p1];
	byte res = rd & p2;

	core->status->V = 0                   ;
	core->status->N = ((res >> 7) & 0x1)  ;
	core->status->S = (core->status->N ^ core->status->V)             ;
	core->status->Z = ((res & 0xff) == 0) ;

	(*(core->R))[p1]=res;

	return 1; 
}

int avr_op_ASR_do( AvrDevice *core, int p1, int p2 )
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

	byte rd = (*(core->R))[p1]; 
	byte res = (rd >> 1) + (rd & 0x80);

	core->status->N = ((res >> 7) & 0x1) ;
	core->status->C = (rd & 0x1) ;
	core->status->V = (core->status->N ^ core->status->C) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = ((res & 0xff) == 0) ;

	(*(core->R))[p1]=res;

	return 1;
}

int avr_op_BCLR_do( AvrDevice *core, int p1, int p2 )
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
	*(core->status)=*(core->status)&~(1<<p1);

	return 1;
}

int avr_op_BLD_do( AvrDevice *core, int p1, int p2 )
{
	/* Bit load from T to Register.
	 *
	 * Opcode     : 1111 100d dddd 0bbb 
	 * Usage      : BLD  Rd, b
	 * Operation  : Rd(b) <- T
	 * Flags      : None
	 * Num Clocks : 1
	 */

	int bit=p2;

	byte rd  = (*(core->R))[p1];
	int  T   = core->status->T; 
	byte res;

	if (T == 0)
		res = rd & ~(1 << bit);
	else
		res = rd | (1 << bit);

	(*(core->R))[p1]=res;

	return 1;
}

int avr_op_BRBC_do( AvrDevice *core, int p1, int p2 )
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

	int k= p2;
	int clks;

    if  (((1<<p1) & (*(core->status)))==0)
	{
		core->PC+=k;
		clks=2;
	}
	else
	{
		clks=1;
	}

	return clks;
}

int avr_op_BRBS_do( AvrDevice *core, int p1, int p2 )
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
	int k= p2;
	int clks;

    if  (((1<<p1) & (*(core->status)))!=0)
	{
		core->PC+=k;
		clks=2;
	}
	else 
	{
		clks=1;
	}

	return clks;
}

int avr_op_BSET_do( AvrDevice *core, int p1, int p2 )
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
	*(core->status)=*(core->status)|1<<p1;
	return 1;
}

int avr_op_BST_do( AvrDevice *core, int p1, int p2 )
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

	core->status->T= (((*(core->R))[p1])>>p2)&0x01;

	return 1;
}

int avr_op_CALL_do( AvrDevice *core, int p1, int p2 )
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
	int kh=p1;
	word *MemPtr=(word*)core->Flash->myMemory;
	word offset=MemPtr[(core->PC)+1];         //this is k!
	offset=(offset>>8)+((offset&0xff)<<8);
	int kl=offset;
	int k=(kh<<16)+kl;
	int pc_bytes=core->PC_size;

	avr_core_stack_push( core, pc_bytes, core->PC+2 );
	core->PC=k-1;

	return pc_bytes+2;
}

int avr_op_CBI_do( AvrDevice *core, int p1, int p2 )
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

	byte val= (*(core->ioreg))[p1];
	byte b= p2;


	(*(core->ioreg))[p1]= (val & ~(1<<b) );


	return 2;
}

int avr_op_COM_do( AvrDevice *core, int p1, int p2 )
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

	byte rd  = (*(core->R))[p1];
	byte res = 0xff - rd;


	core->status->N = ((res >> 7) & 0x1) ;
	core->status->C = 1 ;
	core->status->V = 0 ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = ((res & 0xff) == 0) ;

	(*(core->R))[p1]= res;

	return 1;
}

int avr_op_CP_do( AvrDevice *core, int p1, int p2 )
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

	byte rd  = (*(core->R))[p1];
	byte rr  = (*(core->R))[p2];
	byte res = rd - rr;


	core->status->H = get_compare_carry( res, rd, rr, 3 ) ;
	core->status->V = get_compare_overflow( res, rd, rr ) ;
	core->status->N = ((res >> 7) & 0x1); 
	core->status->S = (core->status->N ^ core->status->V);
	core->status->Z = ((res & 0xff) == 0) ;
	core->status->C = get_compare_carry( res, rd, rr, 7 ) ;

	return 1;
}

int avr_op_CPC_do( AvrDevice *core, int p1, int p2 )
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

	byte rd  = (*(core->R))[p1];
	byte rr  = (*(core->R))[p2];
	byte res = rd - rr - core->status->C;


	core->status->H = get_compare_carry( res, rd, rr, 3 ) ;
	core->status->V = get_compare_overflow( res, rd, rr ) ;
	core->status->N = ((res >> 7) & 0x1) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->C = get_compare_carry( res, rd, rr, 7 ) ;

	/* Previous value remains unchanged when result is 0; cleared otherwise */
	bool Z = ((res & 0xff) == 0);
	bool prev_Z = core->status->Z; 
	core->status->Z = Z && prev_Z ;

	return 1;
}

int avr_op_CPI_do( AvrDevice *core, int p1, int p2 )
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

	byte K = p2; 

	byte rd  = (*(core->R))[p1];
	byte res = rd - K;


	core->status->H = get_compare_carry( res, rd, K, 3 ) ;
	core->status->V = get_compare_overflow( res, rd, K ) ;
	core->status->N = ((res >> 7) & 0x1) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = ((res & 0xff) == 0) ;
	core->status->C = get_compare_carry( res, rd, K, 7 ) ;

	return 1;
}

int avr_op_CPSE_do( AvrDevice *core, int p1, int p2 )
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
	int skip;

	byte rd = (*(core->R))[p1];
	byte rr = (*(core->R))[p2];
	int clks;

	if ( is_next_inst_2_words(core) ) {
		skip = 3;
	} else {
		skip = 2;
	}

	if (rd == rr)
	{
		core->PC+=skip-1;
		clks=skip;
	}
	else 
	{
		clks=1;
	}

	return clks;
}

int avr_op_DEC_do( AvrDevice *core, int p1, int p2 )
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

	byte res = ((*(core->R))[p1])-1; 

	core->status->N = ((res >> 7) & 0x1) ;
	core->status->V = (res == 0x7f) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = ((res & 0xff) == 0) ;

	(*(core->R))[p1]=res;

	return 1;
}

int avr_op_EICALL_do( AvrDevice *core, int p1, int p2 )
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

	int pc_bytes = 3;

	/* Z is R31:R30 */
	/*
	int new_pc = ((core->EIND & 0x3f) << 16) + 
	((*(core->R))[31] << 8) +
	(*(core->R))[30];
	*/
#define EIND 0x10 //TODO
	int new_PC=(*(core->R))[30]+(*(core->R))[31]<<8+(*(core->ioreg))[EIND]<<16;
	//avr_warning( "needs serious code review\n" );

	avr_core_stack_push( core, pc_bytes, (core->PC)+1 );

	core->PC=new_PC;

	return 4;
}

int avr_op_EIJMP_do( AvrDevice *core, int p1, int p2 )
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
	core->PC = (((*(core->ioreg))[EIND] & 0x3f) << 16) + 
		((*(core->R))[31]) << 8+ 
		((*(core->R))[30]);

	return 2;
}

int avr_op_ELPM_Z_do( AvrDevice *core, int p1, int p2 )
{
	/*
	 * Extended Load Program Memory.
	 *
	 * Opcode     : 1001 000d dddd 0110 
	 * Usage      : ELPM  Rd, Z
	 * Operation  : R[ p1 ]<- (RAMPZ:Z)
	 * Flags      : None
	 * Num Clocks : 3
	 */
	int Z, flash_addr;
	word data;

	Z = ((core->GetRampz() & 0x3f) << 16) +
		(((*(core->R))[31]) << 8) +
		(*(core->R))[30];


	flash_addr = Z ^ 0x0001; 
	data = core->Flash->myMemory[flash_addr];
	(*(core->R))[p1]=data;

	return 3;
}

int avr_op_ELPM_Z_incr_do( AvrDevice *core, int p1, int p2 )
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
	int Z, flash_addr;
	word data;

	Z = ((core->GetRampz() & 0x3f) << 16) +
		(((*(core->R))[31]) << 8) +
		((*(core->R))[30]);

	flash_addr = Z ^ 0x0001; 

	data = core->Flash->myMemory[flash_addr];
	(*(core->R))[p1]=data;

	/* post increment Z */
	Z += 1;
	core->SetRampz((Z >> 16) & 0x3f);
	(*(core->R))[30]=Z&0xff;
	(*(core->R))[31]=(Z>>8)&0xff;

	return 1;
}

int avr_op_ELPM_do( AvrDevice *core, int p1, int p2 )
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
	
	int Z, flash_addr;
	word data;

	Z = ((core->GetRampz() & 0x3f) << 16) +
		(((*(core->R))[31]) << 8) +
		((*(core->R))[30]);

	flash_addr = Z ^ 0x0001; 

	data = core->Flash->myMemory[flash_addr];
	(*(core->R))[0]=data;
	return 3;
}

int avr_op_EOR_do( AvrDevice *core, int p1, int p2 )
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

	byte rd = (*(core->R))[p1]; 
	byte rr = (*(core->R))[p2];

	byte res = rd ^ rr;


	core->status->V = 0 ;
	core->status->N = ((res >> 7) & 0x1) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = ((res & 0xff) == 0 ) ;

	(*(core->R))[p1]=res;

	return 1;
}

int avr_op_ESPM_do( AvrDevice *core, int p1, int p2 )
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
	//TODO
	//avr_error( "This opcode is not implemented yet: 0x%04x", opcode );
	
	return 0;
}

/**
 ** I don't know how this Fractional Multiplication works.
 ** If someone wishes to enlighten me, I write these.
 **/

int avr_op_FMUL_do( AvrDevice *core, int p1, int p2 )
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

	byte rd =  (*(core->R))[p1];
	byte rr =  (*(core->R))[p2];

	word resp = rd * rr;  
	word res = resp << 1;


	core->status->Z=((res & 0xffff) == 0) ;
	core->status->C=((resp >> 15) & 0x1) ;


	/* result goes in R1:R0 */
	(*(core->R))[0]=res&0xff;
	(*(core->R))[1]=res>>8;

	return 2;
}

int avr_op_FMULS_do( AvrDevice *core, int p1, int p2 )
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

	sbyte rd = (*(core->R))[p1];
	sbyte rr = (*(core->R))[p2];

	word resp = rd * rr;  
	word res = resp << 1;


	core->status->Z = ((res & 0xffff) == 0) ;
	core->status->C = ((resp >> 15) & 0x1) ;

	/* result goes in R1:R0 */
	(*(core->R))[0]= res& 0xff;
	(*(core->R))[1]= res>>8;

	return 2;
}

int avr_op_FMULSU_do( AvrDevice *core, int p1, int p2 )
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

	sbyte rd = (*(core->R))[p1];
	byte rr = (*(core->R))[p2];

	word resp = rd * rr;  
	word res = resp << 1;


	core->status->Z=((res & 0xffff) == 0) ;
	core->status->C=((resp >> 15) & 0x1) ;
	(*(core->R))[0]= res& 0xff;
	(*(core->R))[1]= res>>8;

	return 2;
}

int avr_op_ICALL_do( AvrDevice *core, int p1, int p2 )
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
	int pc = core->PC;
	int pc_bytes = core->PC_size;

	/* Z is R31:R30 */
	int new_pc = ((*(core->R))[31] << 8) + (*(core->R))[30];

	avr_core_stack_push( core, pc_bytes, pc+1 );

	core->PC=new_pc-1;

	return pc_bytes+1;
}

int avr_op_IJMP_do( AvrDevice *core, int p1, int p2 )
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
	int new_pc = ((*(core->R))[31] << 8) + ((*(core->R))[30]);
	core->PC=new_pc-1;

	return 2;
}

int avr_op_IN_do( AvrDevice *core, int p1, int p2 )
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

	(*(core->R))[p1]=(*(core->ioreg))[p2];

	return 1;
}

int avr_op_INC_do( AvrDevice *core, int p1, int p2 )
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

	byte rd  = (*(core->R))[p1];
	byte res = rd + 1;

	core->status->N = ((res >> 7) & 0x1) ;
	core->status->V = (rd == 0x7f) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = ((res & 0xff) == 0) ;

	(*(core->R))[p1]=res;

	return 1;
}

int avr_op_JMP_do( AvrDevice *core, int p1, int p2 )
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

	word *MemPtr=(word*)core->Flash->myMemory;
	word offset=MemPtr[(core->PC)+1];         //this is k!
	offset=(offset>>8)+((offset&0xff)<<8);

	core->PC=offset-1;

	return 3;
}

int avr_op_LDD_Y_do( AvrDevice *core, int p1, int p2 )
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

	word Y;

	/* Y is R29:R28 */
	Y = ((*(core->R))[29] << 8) + ((*(core->R))[28]);
	(*(core->R))[p1]= (*(core->Sram))[Y+p2];

	return 2;
}

int avr_op_LDD_Z_do( AvrDevice *core, int p1, int p2 )
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
	word Z;

	int q=p2;


	/* Z is R31:R30 */
	Z = ((*(core->R))[31] << 8) + ((*(core->R))[30]);

	(*(core->R))[p1]= (*(core->Sram))[Z+q];

	return 2;
}

int avr_op_LDI_do( AvrDevice *core, int p1, int p2 )
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
	(*(core->R))[p1]=p2;

	return 1;
}

int avr_op_LDS_do( AvrDevice *core, int p1, int p2 )
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

	/* Get data at k in current data segment and put into Rd */
	word *MemPtr=(word*)core->Flash->myMemory;
	word offset=MemPtr[(core->PC)+1];         //this is k!
	offset=(offset>>8)+((offset&0xff)<<8);
	(*(core->R))[p1]=(*(core->Sram))[offset];
	core->PC++; //2 word instr

	return 2;
}

int avr_op_LD_X_do( AvrDevice *core, int p1, int p2 )
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
	word X;


	/* X is R27:R26 */
	X = ((*(core->R))[27] << 8) + ((*(core->R))[26]);
	(*(core->R))[p1]= (*(core->Sram))[X];

	return 2;
}

int avr_op_LD_X_decr_do( AvrDevice *core, int p1, int p2 )
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
	word X;

	//TODO
	//if ( (Rd == 26) || (Rd == 27) )
	//	avr_error( "Results of operation are undefined" );

	/* X is R27:R26 */
	X = ((*(core->R))[27] << 8) + ((*(core->R))[26]);

	/* Perform pre-decrement */
	X -= 1;
	(*(core->R))[p1]= (*(core->Sram))[X];
	(*(core->R))[26]=X&0xff;
	(*(core->R))[27]=X>>8;

	return 2;
}

int avr_op_LD_X_incr_do( AvrDevice *core, int p1, int p2 )
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
	word X;


	//TODO
	//if ( (Rd == 26) || (Rd == 27) )
	//	avr_error( "Results of operation are undefined" );

	/* X is R27:R26 */
	X = ((*(core->R))[27] << 8) + ((*(core->R))[26]);

	(*(core->R))[p1]= (*(core->Sram))[X];

	/* Perform post-increment */
	X += 1;
	(*(core->R))[26]=X&0xff;
	(*(core->R))[27]=X>>8;

	return 2;
}

int avr_op_LD_Y_decr_do( AvrDevice *core, int p1, int p2 )
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
	word Y;


	//TODO
	//if ( (p1 == 28) || (p1 == 29) )
	//	avr_error( "Results of operation are undefined" );

	/* Y is R29:R28 */
	Y = ((*(core->R))[ 29] << 8) + (*(core->R))[ 28];

	/* Perform pre-decrement */
	Y -= 1;
	(*(core->R))[p1]=(*(core->Sram))[Y];
	(*(core->R))[28]=Y&0xff;
	(*(core->R))[29]=Y>>8;

	return 2;
}

int avr_op_LD_Y_incr_do( AvrDevice *core, int p1, int p2 )
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
	word Y;


	//TODO
	//if ( (Rd == 28) || (Rd == 29) )
	//	avr_error( "Results of operation are undefined" );

	/* Y is R29:R28 */
	Y = ((*(core->R))[ 29] << 8) + (*(core->R))[ 28];
	(*(core->R))[p1]=(*(core->Sram))[Y];
	Y += 1;
	(*(core->R))[28]=Y&0xff;
	(*(core->R))[29]=Y>>8;

	return 2;
}

int avr_op_LD_Z_incr_do( AvrDevice *core, int p1, int p2 )
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
	word Z;


	//TODO
	////if ( (Rd == 30) || (Rd == 31) )
	//	avr_error( "Results of operation are undefined" );

	/* Z is R31:R30 */
	Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];

	/* Perform post-increment */
	(*(core->R))[p1]=(*(core->Sram))[Z];
	Z += 1;
	(*(core->R))[30]=Z&0xff;
	(*(core->R))[31]=Z>>8;

	return 2;
}

int avr_op_LD_Z_decr_do( AvrDevice *core, int p1, int p2 )
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
	word Z;

	//TODO
	//if ( (Rd == 30) || (Rd == 31) )
	//	avr_error( "Results of operation are undefined" );

	/* Z is R31:R30 */
	Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];

	/* Perform pre-decrement */
	Z -= 1;
	(*(core->R))[p1]=(*(core->Sram))[Z];
	(*(core->R))[30]=Z&0xff;
	(*(core->R))[31]=Z>>8;

	return 2;
}

int avr_op_LPM_Z_do( AvrDevice *core, int p1, int p2 )
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
	word Z;
	word data;


	/* Z is R31:R30 */
	Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];

    Z^=0x0001;
    data = core->Flash->myMemory[Z];

	(*(core->R))[p1]=data;

	return 3;
}

int avr_op_LPM_do( AvrDevice *core, int p1, int p2 )
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
	word Z;
	word data;


	/* Z is R31:R30 */
	Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];

    Z^=0x0001;
    data = core->Flash->myMemory[Z];

	(*(core->R))[0]=data;
	return 3;
}

int avr_op_LPM_Z_incr_do( AvrDevice *core, int p1, int p2 )
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
	word Z, flashAddr;
	word data;


	Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];
    flashAddr= Z^ 0x0001;
    data = core->Flash->myMemory[flashAddr];

	(*(core->R))[p1]=data;
	Z += 1;
	(*(core->R))[30]=Z&0xff;
	(*(core->R))[31]=Z>>8;

	return 3;
}

int avr_op_LSR_do( AvrDevice *core, int p1, int p2 )
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
	//int Rd = get_rd_5(opcode);
	byte rd = (*(core->R))[ p1 ];

	byte res = (rd >> 1) & 0x7f;


	core->status->C = (rd & 0x1) ;
	core->status->N = (0) ;
	core->status->V = (core->status->N ^ core->status->C) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = ((res & 0xff) == 0) ;

	(*(core->R))[p1]=res;

	return 1;
}

int avr_op_MOV_do( AvrDevice *core, int p1, int p2 )
{
	/* Copy Register.
	 *
	 * Opcode     : 0010 11rd dddd rrrr 
	 * Usage      : MOV  Rd, Rr
	 * Operation  : Rd <- Rr
	 * Flags      : None
	 * Num Clocks : 1
	 */

	(*(core->R))[p1]=(*(core->R))[p2];
	return 1;
}

int avr_op_MOVW_do( AvrDevice *core, int p1, int p2 )
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

	p1 = (p1 - 16) * 2;
	p2 = (p2 - 16) * 2;
	(*(core->R))[p1]=(*(core->R))[p2];
	(*(core->R))[p1+1]=(*(core->R))[p2+1];

	return 1;
}


int avr_op_MUL_do( AvrDevice *core, int p1, int p2 )
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

	byte rd = (*(core->R))[ p1 ];
	byte rr = (*(core->R))[ p2 ];

	word res = rd * rr;

	core->status->Z=((res & 0xffff) == 0) ;
	core->status->C=((res >> 15) & 0x1) ;


	/* result goes in R1:R0 */
	(*(core->R))[0]=res&0xff;
	(*(core->R))[1]=res>>8;

	return 2;
}

int avr_op_MULS_do( AvrDevice *core, int p1, int p2 )
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

	sbyte rd = (sbyte)(*(core->R))[ p1 ];
	sbyte rr = (sbyte)(*(core->R))[ p2 ];
	sword res = rd * rr;


	core->status->Z=((res & 0xffff) == 0) ;
	core->status->C=((res >> 15) & 0x1) ;

	/* result goes in R1:R0 */
	(*(core->R))[0]=res&0xff;
	(*(core->R))[1]=res>>8;

	return 2;
}

int avr_op_MULSU_do( AvrDevice *core, int p1, int p2 )
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

	sbyte rd = (sbyte)(*(core->R))[ p1 ];
	byte rr = (*(core->R))[ p2 ];

	sword res = rd * rr;       


	core->status->Z=((res & 0xffff) == 0) ;
	core->status->C=((res >> 15) & 0x1) ;


	/* result goes in R1:R0 */
	(*(core->R))[0]=res&0xff;
	(*(core->R))[1]=res>>8;

	return 2;
}

int avr_op_NEG_do( AvrDevice *core, int p1, int p2 )
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

	byte rd  = (*(core->R))[ p1];
	byte res = (0x0 - rd) & 0xff;

	core->status->H = (((res >> 3) | (rd >> 3)) & 0x1) ;
	core->status->V = (res == 0x80) ;
	core->status->N = ((res >> 7) & 0x1) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = (res == 0x0) ;
	core->status->C = (res != 0x0) ;

	(*(core->R))[p1]=res;

	return 1;
}

int avr_op_NOP_do( AvrDevice *core, int p1, int p2 )
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

	return 1;
}

int avr_op_OR_do( AvrDevice *core, int p1, int p2 )
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
	byte res = (*(core->R))[ p1] | (*(core->R))[ p2];

	core->status->V = (0) ;
	core->status->N = ((res >> 7) & 0x1) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = (res == 0x0) ;

	(*(core->R))[p1]= res;

	return 1;
}

int avr_op_ORI_do( AvrDevice *core, int p1, int p2 )
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

	byte res = (*(core->R))[ p1 ] | p2;

	core->status->V = (0) ;
	core->status->N = ((res >> 7) & 0x1) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = (res == 0x0) ;

	(*(core->R))[p1]=res;

	return 1;
}

int avr_op_OUT_do( AvrDevice *core, int p1, int p2 )
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

	(*(core->ioreg))[p1]=(*(core->R))[p2];

	return 1;
}

int avr_op_POP_do( AvrDevice *core, int p1, int p2 )
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
	
    (*(core->R))[p1]= avr_core_stack_pop(core, 1);

	return 2;
}

int avr_op_PUSH_do( AvrDevice *core, int p1, int p2 )
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

	avr_core_stack_push( core, 1, (*(core->R))[ p1 ] );

	return 2;
}

int avr_op_RCALL_do( AvrDevice *core, int p1, int p2 )
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

	int pc       = core->PC;
	int pc_bytes = core->PC_size;
	int k=p1;

	avr_core_stack_push( core, pc_bytes, pc+1 );
	core->PC+=k;

	return pc_bytes+1;
}

int avr_op_RET_do( AvrDevice *core, int p1, int p2 )
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
	int pc_bytes = core->PC_size;
	int pc       = avr_core_stack_pop( core, pc_bytes );

	core->PC=pc-1;

	return pc_bytes+2;
}

int avr_op_RETI_do( AvrDevice *core, int p1, int p2 )
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
	int pc_bytes = core->PC_size;
	int pc = avr_core_stack_pop( core, pc_bytes );

	core->PC=pc-1;
	core->status->I=1;

	return pc_bytes+2;
}

int avr_op_RJMP_do( AvrDevice *core, int p1, int p2 )
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
	core->PC+=p1;

	return 2;
}

int avr_op_ROR_do( AvrDevice *core, int p1, int p2 )
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

	byte rd = (*(core->R))[ p1 ];

	byte res = (rd >> 1) | ((( core->status->C ) << 7) & 0x80);


	core->status->C = (rd & 0x1) ;
	core->status->N = ((res >> 7) & 0x1) ;
	core->status->V = (core->status->N ^ core->status->C) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = (res == 0) ;

	(*(core->R))[p1]=res;

	return 1;
}

int avr_op_SBC_do( AvrDevice *core, int p1, int p2 )
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

	byte rd = (*(core->R))[ p1 ];
	byte rr = (*(core->R))[ p2 ];

	byte res = rd - rr - ( core->status->C );


	core->status->H = (get_sub_carry( res, rd, rr, 3 )) ;
	core->status->V = (get_sub_overflow( res, rd, rr )) ;
	core->status->N = ((res >> 7) & 0x1) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->C = (get_sub_carry( res, rd, rr, 7 )) ;

	if ((res & 0xff) != 0)
		core->status->Z = (0) ;

	(*(core->R))[p1]=res;

	return 1;
}

int avr_op_SBCI_do( AvrDevice *core, int p1, int p2 )
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

	byte K=p2;

	byte rd = (*(core->R))[ p1 ];

	byte res = rd - K - ( core->status->C );


	core->status->H = (get_sub_carry( res, rd, K, 3 )) ;
	core->status->V = (get_sub_overflow( res, rd, K )) ;
	core->status->N = ((res >> 7) & 0x1) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->C = (get_sub_carry( res, rd, K, 7 )) ;

	if ((res & 0xff) != 0)
		core->status->Z = 0 ;

	(*(core->R))[p1]=res;

	return 1;
}

int avr_op_SBI_do( AvrDevice *core, int p1, int p2 )
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

	int A=p1;
	int b=p2;

	byte val =((*(core->ioreg))[A] );
	(*(core->ioreg))[ A]= val | (1 << b) ;

	return 2;
}

int avr_op_SBIC_do( AvrDevice *core, int p1, int p2 )
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
	int skip;

	int A = p1;
	int b = p2;
	int clks;


	if ( is_next_inst_2_words(core) ) skip = 3;
	else skip = 2;

	if ( ( ((*(core->ioreg))[A]) & (1 << b) ) == 0)
	{
		core->PC+=skip-1;
		clks=skip;
	}
	else
	{
		clks=1;
	}

	return clks;
}

int avr_op_SBIS_do( AvrDevice *core, int p1, int p2 )
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
	int skip;

	int A=p1;
	int b=p2;
	int clks;

	if ( is_next_inst_2_words(core) )
		skip = 3;
	else
		skip = 2;

	if ( ((*(core->ioreg))[A] & (1 << b) ) != 0)
	{
		core->PC+=skip-1;
		clks=skip;
	}
	else
	{
		clks=1;
	}

	return clks;
}

int avr_op_SBIW_do( AvrDevice *core, int p1, int p2 )
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

	byte K=p2;

	byte rdl = (*(core->R))[ p1 ];
	byte rdh = (*(core->R))[ p1+1 ];

	word rd = (rdh << 8) + rdl;

	word res = rd - K;


	core->status->V = ((rdh >> 7 & 0x1) & ~(res >> 15 & 0x1)) ;
	core->status->N = ((res >> 15) & 0x1) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = ((res & 0xffff) == 0) ;
	core->status->C = ((res >> 15 & 0x1) & ~(rdh >> 7 & 0x1)) ;

	(*(core->R))[p1]=res&0xff;
	(*(core->R))[p1+1]=res>>8;

	return 2;
}

int avr_op_SBRC_do( AvrDevice *core, int p1, int p2 )
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
	int skip;
	int clks;
	int b = p2;

	if ( is_next_inst_2_words(core) )
		skip = 3;
	else
		skip = 2;

	if ((((*(core->R))[ p1 ] >> b) & 0x1) == 0)
	{
		core->PC+=skip-1;
		clks=skip;
	}
	else 
	{ 
		clks=1;
	}

	return clks;
}

int avr_op_SBRS_do( AvrDevice *core, int p1, int p2 )
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
	int skip;
	int clks;

	int b = p2;

	if ( is_next_inst_2_words(core) )
		skip = 3;
	else
		skip = 2;

	if ((((*(core->R))[ p1 ] >> b) & 0x1) != 0)
	{
		core->PC+=skip-1;
		clks=skip;
	}
	else
	{
		clks=1;
	}

	return clks;
}

int avr_op_SLEEP_do( AvrDevice *core, int p1, int p2 )
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
#ifdef LLL	//TODO
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

	return 0;
}

int avr_op_SPM_do( AvrDevice *core, int p1, int p2 )
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
	//avr_error( "This opcode is not implemented yet: 0x%04x", opcode );
	return 0;
}

int avr_op_STD_Y_do( AvrDevice *core, int p1, int p2 )
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

	//int Rd = get_rd_5(opcode);
	//int q  = get_q(opcode);
	//int q=p1;

	/* Y is R29:R28 */
	int Y = ((*(core->R))[ 29] << 8) + (*(core->R))[ 28];

	(*(core->Sram))[Y+p1]= (*(core->R))[ p2] ;

	//avr_core_PC_incr( core, 1 );
	//avr_core_inst_CKS_set( core, 2 );

	return 2;
}

int avr_op_STD_Z_do( AvrDevice *core, int p1, int p2 )
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
	int Z;

	//int Rd = get_rd_5(opcode);
	int q  = p1;

	/* Z is R31:R30 */
	Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];

	//avr_core_mem_write( core, Z+q, (*(core->R))[ Rd] );
	(*(core->Sram))[Z+q]=(*(core->R))[p2];

	//avr_core_PC_incr( core, 1 );
	//avr_core_inst_CKS_set( core, 2 );

	return 2;
}

int avr_op_STS_do( AvrDevice *core, int p1, int p2 )
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
	//int Rd = get_rd_5(opcode);

	/* Get data at k in current data segment and put into Rd */
	//int k_pc = avr_core_PC_get(core) + 1;
	//int k    = core->Flash->myMemory[ k_pc];
	word *MemPtr=(word*)core->Flash->myMemory;
	word k=MemPtr[(core->PC)+1];         //this is k!
	k=(k>>8)+((k&0xff)<<8);


	//avr_core_mem_write( core, k, (*(core->R))[ Rd] );

	(*(core->Sram))[k]=(*(core->R))[p2];
	core->PC++;

	//avr_core_PC_incr( core, 2 );
	//avr_core_inst_CKS_set( core, 2 );

	return 2;
}

int avr_op_ST_X_do( AvrDevice *core, int p1, int p2 )
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
	word X;

	//int Rd = get_rd_5(opcode);

	/* X is R27:R26 */
	//X = ((*(core->R))[ 27] << 8) + (*(core->R))[ 26];
	X = ((*(core->R))[27] << 8) + ((*(core->R))[26]);

	//avr_core_mem_write( core, X, (*(core->R))[ Rd] );
	(*(core->Sram))[X]=(*(core->R))[p2];


	//avr_core_PC_incr( core, 1 );
	//avr_core_inst_CKS_set( core, 2 );

	return 2;
}

int avr_op_ST_X_decr_do( AvrDevice *core, int p1, int p2 )
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
	word X;


	//TODO
	//if ( (Rd == 26) || (Rd == 27) )
	//	avr_error( "Results of operation are undefined: 0x%04x", opcode );

	/* X is R27:R26 */
	X = ((*(core->R))[27] << 8) + ((*(core->R))[26]);

	/* Perform pre-decrement */
	X -= 1;
	(*(core->R))[26]=X & 0xff;
	(*(core->R))[27]=X >> 8;

	(*(core->Sram))[X]=(*(core->R))[p2];

	return 2;
}

int avr_op_ST_X_incr_do( AvrDevice *core, int p1, int p2 )
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
	word X;

	/* X is R27:R26 */
	X = ((*(core->R))[27] << 8) + ((*(core->R))[26]);

	(*(core->Sram))[X]=(*(core->R))[p2];

	/* Perform post-increment */
	X += 1;
	(*(core->R))[26]=X&0xff;
	(*(core->R))[27]=X>>8;

	return 2;
}

int avr_op_ST_Y_decr_do( AvrDevice *core, int p1, int p2 )
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
	word Y;


	//TODO
	//if ( (Rd == 28) || (Rd == 29) )
	//	avr_error( "Results of operation are undefined: 0x%04x", opcode );

	/* Y is R29:R28 */
	Y = ((*(core->R))[ 29] << 8) + (*(core->R))[ 28];

	/* Perform pre-decrement */
	Y -= 1;
	(*(core->R))[28]=Y&0xff;
	(*(core->R))[29]=Y>>8;

	(*(core->Sram))[Y]=(*(core->R))[p2];


	return 2;
}

int avr_op_ST_Y_incr_do( AvrDevice *core, int p1, int p2 )
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
	word Y;

	//TODO
	//if ( (Rd == 28) || (Rd == 29) )
	//	avr_error( "Results of operation are undefined: 0x%04x", opcode );

	/* Y is R29:R28 */
	Y = ((*(core->R))[ 29] << 8) + (*(core->R))[ 28];

	(*(core->Sram))[Y]=(*(core->R))[p2];

	/* Perform post-increment */
	Y += 1;
	(*(core->R))[28]=Y&0xff;
	(*(core->R))[29]=Y>>8;

	return 2;
}

int avr_op_ST_Z_decr_do( AvrDevice *core, int p1, int p2 )
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
	word Z;


	//TODO
	//if ( (Rd == 30) || (Rd == 31) )
	//	avr_error( "Results of operation are undefined: 0x%04x", opcode );

	/* Z is R31:R30 */
	Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];

	/* Perform pre-decrement */
	Z -= 1;
	(*(core->R))[30]=Z&0xff;
	(*(core->R))[31]=Z>>8;

	(*(core->Sram))[Z]=(*(core->R))[p2];

	return 2;
}

int avr_op_ST_Z_incr_do( AvrDevice *core, int p1, int p2 )
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
	word Z;


	//TODO
	//if ( (Rd == 30) || (Rd == 31) )
	//	avr_error( "Results of operation are undefined: 0x%04x", opcode );

	/* Z is R31:R30 */
	Z = ((*(core->R))[ 31] << 8) + (*(core->R))[ 30];

	(*(core->Sram))[Z]=(*(core->R))[p2];

	/* Perform post-increment */
	Z += 1;
	(*(core->R))[30]=Z&0xff;
	(*(core->R))[31]=Z>>8;

	return 2;
}

int avr_op_SUB_do( AvrDevice *core, int p1, int p2 )
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

	byte rd = (*(core->R))[ p1 ];
	byte rr = (*(core->R))[ p2 ];

	byte res = rd - rr;

	core->status->H = (get_sub_carry( res, rd, rr, 3 )) ;
	core->status->V = (get_sub_overflow( res, rd, rr )) ;
	core->status->N = ((res >> 7) & 0x1) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = ((res & 0xff) == 0) ;
	core->status->C = (get_sub_carry( res, rd, rr, 7 )) ;
	(*(core->R))[p1]=res;

	return 1;
}

int avr_op_SUBI_do( AvrDevice *core, int p1, int p2 )
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

	byte K=p2;

	byte rd = (*(core->R))[ p1 ];

	byte res = rd - K;


	core->status->H = (get_sub_carry( res, rd, K, 3 )) ;
	core->status->V = (get_sub_overflow( res, rd, K )) ;
	core->status->N = ((res >> 7) & 0x1) ;
	core->status->S = (core->status->N ^ core->status->V) ;
	core->status->Z = ((res & 0xff) == 0) ;
	core->status->C = (get_sub_carry( res, rd, K, 7 )) ;

	(*(core->R))[p1]= res;

	return 1;
}

int avr_op_SWAP_do( AvrDevice *core, int p1, int p2 )
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

	byte rd = (*(core->R))[ p1 ];

	(*(core->R))[p1]= ((rd << 4) & 0xf0) | ((rd >> 4) & 0x0f) ;


	return 1;
}

int avr_op_WDR_do( AvrDevice *core, int p1, int p2 )
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

	core->wado->Wdr();

	return 1;
}
