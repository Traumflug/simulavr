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
#ifndef ASS_DO_TRACE
#define ASS_DO_TRACE
#include "avrdevice.h"
#include "types.h"
int avr_op_ADC_do_trace( AvrDevice *core, int, int );
int avr_op_ADD_do_trace( AvrDevice *core, int, int );
int avr_op_ADIW_do_trace( AvrDevice *core, int, int );
int avr_op_AND_do_trace( AvrDevice *core, int, int );
int avr_op_ANDI_do_trace( AvrDevice *core, int, int );
int avr_op_ASR_do_trace( AvrDevice *core, int, int );
int avr_op_BCLR_do_trace( AvrDevice *core, int, int );
int avr_op_BLD_do_trace( AvrDevice *core, int, int );
int avr_op_BRBC_do_trace( AvrDevice *core, int, int );
int avr_op_BRBS_do_trace( AvrDevice *core, int, int );
int avr_op_BSET_do_trace( AvrDevice *core, int, int );
int avr_op_BST_do_trace( AvrDevice *core, int, int );
int avr_op_CALL_do_trace( AvrDevice *core, int, int );
int avr_op_CBI_do_trace( AvrDevice *core, int, int );
int avr_op_COM_do_trace( AvrDevice *core, int, int );
int avr_op_CP_do_trace( AvrDevice *core, int, int );
int avr_op_CPC_do_trace( AvrDevice *core, int, int );
int avr_op_CPI_do_trace( AvrDevice *core, int, int );
int avr_op_CPSE_do_trace( AvrDevice *core, int, int );
int avr_op_DEC_do_trace( AvrDevice *core, int, int );
int avr_op_EICALL_do_trace( AvrDevice *core, int, int );
int avr_op_EIJMP_do_trace( AvrDevice *core, int, int );
int avr_op_ELPM_Z_do_trace( AvrDevice *core, int, int );
int avr_op_ELPM_Z_incr_do_trace( AvrDevice *core, int, int );
int avr_op_ELPM_do_trace( AvrDevice *core, int, int );
int avr_op_EOR_do_trace( AvrDevice *core, int, int );
int avr_op_ESPM_do_trace( AvrDevice *core, int, int );
int avr_op_FMUL_do_trace( AvrDevice *core, int, int );
int avr_op_FMULS_do_trace( AvrDevice *core, int, int );
int avr_op_FMULSU_do_trace( AvrDevice *core, int, int );
int avr_op_ICALL_do_trace( AvrDevice *core, int, int );
int avr_op_IJMP_do_trace( AvrDevice *core, int, int );
int avr_op_IN_do_trace( AvrDevice *core, int, int );
int avr_op_INC_do_trace( AvrDevice *core, int, int );
int avr_op_JMP_do_trace( AvrDevice *core, int, int );
int avr_op_LDD_Y_do_trace( AvrDevice *core, int, int );
int avr_op_LDD_Z_do_trace( AvrDevice *core, int, int );
int avr_op_LDI_do_trace( AvrDevice *core, int, int );
int avr_op_LDS_do_trace( AvrDevice *core, int, int );
int avr_op_LD_X_do_trace( AvrDevice *core, int, int );
int avr_op_LD_X_decr_do_trace( AvrDevice *core, int, int );
int avr_op_LD_X_incr_do_trace( AvrDevice *core, int, int );
int avr_op_LD_Y_decr_do_trace( AvrDevice *core, int, int );
int avr_op_LD_Y_incr_do_trace( AvrDevice *core, int, int );
int avr_op_LD_Z_incr_do_trace( AvrDevice *core, int, int );
int avr_op_LD_Z_decr_do_trace( AvrDevice *core, int, int );
int avr_op_LPM_Z_do_trace( AvrDevice *core, int, int );
int avr_op_LPM_do_trace( AvrDevice *core, int, int );
int avr_op_LPM_Z_incr_do_trace( AvrDevice *core, int, int );
int avr_op_LSR_do_trace( AvrDevice *core, int, int );
int avr_op_MOV_do_trace( AvrDevice *core, int, int );
int avr_op_MOVW_do_trace( AvrDevice *core, int, int );
int avr_op_MUL_do_trace( AvrDevice *core, int, int );
int avr_op_MULS_do_trace( AvrDevice *core, int, int );
int avr_op_MULSU_do_trace( AvrDevice *core, int, int );
int avr_op_NEG_do_trace( AvrDevice *core, int, int );
int avr_op_NOP_do_trace( AvrDevice *core, int, int );
int avr_op_OR_do_trace( AvrDevice *core, int, int );
int avr_op_ORI_do_trace( AvrDevice *core, int, int );
int avr_op_OUT_do_trace( AvrDevice *core, int, int );
int avr_op_POP_do_trace( AvrDevice *core, int, int );
int avr_op_PUSH_do_trace( AvrDevice *core, int, int );
int avr_op_RCALL_do_trace( AvrDevice *core, int, int );
int avr_op_RET_do_trace( AvrDevice *core, int, int );
int avr_op_RETI_do_trace( AvrDevice *core, int, int );
int avr_op_RJMP_do_trace( AvrDevice *core, int, int );
int avr_op_ROR_do_trace( AvrDevice *core, int, int );
int avr_op_SBC_do_trace( AvrDevice *core, int, int );
int avr_op_SBCI_do_trace( AvrDevice *core, int, int );
int avr_op_SBI_do_trace( AvrDevice *core, int, int );
int avr_op_SBIC_do_trace( AvrDevice *core, int, int );
int avr_op_SBIS_do_trace( AvrDevice *core, int, int );
int avr_op_SBIW_do_trace( AvrDevice *core, int, int );
int avr_op_SBRC_do_trace( AvrDevice *core, int, int );
int avr_op_SBRS_do_trace( AvrDevice *core, int, int );
int avr_op_SLEEP_do_trace( AvrDevice *core, int, int );
int avr_op_SPM_do_trace( AvrDevice *core, int, int );
int avr_op_STD_Y_do_trace( AvrDevice *core, int, int );
int avr_op_STD_Z_do_trace( AvrDevice *core, int, int );
int avr_op_STS_do_trace( AvrDevice *core, int, int );
int avr_op_ST_X_do_trace( AvrDevice *core, int, int );
int avr_op_ST_X_decr_do_trace( AvrDevice *core, int, int );
int avr_op_ST_X_incr_do_trace( AvrDevice *core, int, int );
int avr_op_ST_Y_decr_do_trace( AvrDevice *core, int, int );
int avr_op_ST_Y_incr_do_trace( AvrDevice *core, int, int );
int avr_op_ST_Z_decr_do_trace( AvrDevice *core, int, int );
int avr_op_ST_Z_incr_do_trace( AvrDevice *core, int, int );
int avr_op_SUB_do_trace( AvrDevice *core, int, int );
int avr_op_SUBI_do_trace( AvrDevice *core, int, int );
int avr_op_SWAP_do_trace( AvrDevice *core, int, int );
int avr_op_WDR_do_trace( AvrDevice *core, int, int );
#endif
