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
#ifndef ASS_DO
#define ASS_DO
#include "avrdevice.h"
#include "types.h"
int avr_op_ADC_do( AvrDevice *core, int, int );
int avr_op_ADD_do( AvrDevice *core, int, int );
int avr_op_ADIW_do( AvrDevice *core, int, int );
int avr_op_AND_do( AvrDevice *core, int, int );
int avr_op_ANDI_do( AvrDevice *core, int, int );
int avr_op_ASR_do( AvrDevice *core, int, int );
int avr_op_BCLR_do( AvrDevice *core, int, int );
int avr_op_BLD_do( AvrDevice *core, int, int );
int avr_op_BRBC_do( AvrDevice *core, int, int );
int avr_op_BRBS_do( AvrDevice *core, int, int );
int avr_op_BSET_do( AvrDevice *core, int, int );
int avr_op_BST_do( AvrDevice *core, int, int );
int avr_op_CALL_do( AvrDevice *core, int, int );
int avr_op_CBI_do( AvrDevice *core, int, int );
int avr_op_COM_do( AvrDevice *core, int, int );
int avr_op_CP_do( AvrDevice *core, int, int );
int avr_op_CPC_do( AvrDevice *core, int, int );
int avr_op_CPI_do( AvrDevice *core, int, int );
int avr_op_CPSE_do( AvrDevice *core, int, int );
int avr_op_DEC_do( AvrDevice *core, int, int );
int avr_op_EICALL_do( AvrDevice *core, int, int );
int avr_op_EIJMP_do( AvrDevice *core, int, int );
int avr_op_ELPM_Z_do( AvrDevice *core, int, int );
int avr_op_ELPM_Z_incr_do( AvrDevice *core, int, int );
int avr_op_ELPM_do( AvrDevice *core, int, int );
int avr_op_EOR_do( AvrDevice *core, int, int );
int avr_op_ESPM_do( AvrDevice *core, int, int );
int avr_op_FMUL_do( AvrDevice *core, int, int );
int avr_op_FMULS_do( AvrDevice *core, int, int );
int avr_op_FMULSU_do( AvrDevice *core, int, int );
int avr_op_ICALL_do( AvrDevice *core, int, int );
int avr_op_IJMP_do( AvrDevice *core, int, int );
int avr_op_IN_do( AvrDevice *core, int, int );
int avr_op_INC_do( AvrDevice *core, int, int );
int avr_op_JMP_do( AvrDevice *core, int, int );
int avr_op_LDD_Y_do( AvrDevice *core, int, int );
int avr_op_LDD_Z_do( AvrDevice *core, int, int );
int avr_op_LDI_do( AvrDevice *core, int, int );
int avr_op_LDS_do( AvrDevice *core, int, int );
int avr_op_LD_X_do( AvrDevice *core, int, int );
int avr_op_LD_X_decr_do( AvrDevice *core, int, int );
int avr_op_LD_X_incr_do( AvrDevice *core, int, int );
int avr_op_LD_Y_decr_do( AvrDevice *core, int, int );
int avr_op_LD_Y_incr_do( AvrDevice *core, int, int );
int avr_op_LD_Z_incr_do( AvrDevice *core, int, int );
int avr_op_LD_Z_decr_do( AvrDevice *core, int, int );
int avr_op_LPM_Z_do( AvrDevice *core, int, int );
int avr_op_LPM_do( AvrDevice *core, int, int );
int avr_op_LPM_Z_incr_do( AvrDevice *core, int, int );
int avr_op_LSR_do( AvrDevice *core, int, int );
int avr_op_MOV_do( AvrDevice *core, int, int );
int avr_op_MOVW_do( AvrDevice *core, int, int );
int avr_op_MUL_do( AvrDevice *core, int, int );
int avr_op_MULS_do( AvrDevice *core, int, int );
int avr_op_MULSU_do( AvrDevice *core, int, int );
int avr_op_NEG_do( AvrDevice *core, int, int );
int avr_op_NOP_do( AvrDevice *core, int, int );
int avr_op_OR_do( AvrDevice *core, int, int );
int avr_op_ORI_do( AvrDevice *core, int, int );
int avr_op_OUT_do( AvrDevice *core, int, int );
int avr_op_POP_do( AvrDevice *core, int, int );
int avr_op_PUSH_do( AvrDevice *core, int, int );
int avr_op_RCALL_do( AvrDevice *core, int, int );
int avr_op_RET_do( AvrDevice *core, int, int );
int avr_op_RETI_do( AvrDevice *core, int, int );
int avr_op_RJMP_do( AvrDevice *core, int, int );
int avr_op_ROR_do( AvrDevice *core, int, int );
int avr_op_SBC_do( AvrDevice *core, int, int );
int avr_op_SBCI_do( AvrDevice *core, int, int );
int avr_op_SBI_do( AvrDevice *core, int, int );
int avr_op_SBIC_do( AvrDevice *core, int, int );
int avr_op_SBIS_do( AvrDevice *core, int, int );
int avr_op_SBIW_do( AvrDevice *core, int, int );
int avr_op_SBRC_do( AvrDevice *core, int, int );
int avr_op_SBRS_do( AvrDevice *core, int, int );
int avr_op_SLEEP_do( AvrDevice *core, int, int );
int avr_op_SPM_do( AvrDevice *core, int, int );
int avr_op_STD_Y_do( AvrDevice *core, int, int );
int avr_op_STD_Z_do( AvrDevice *core, int, int );
int avr_op_STS_do( AvrDevice *core, int, int );
int avr_op_ST_X_do( AvrDevice *core, int, int );
int avr_op_ST_X_decr_do( AvrDevice *core, int, int );
int avr_op_ST_X_incr_do( AvrDevice *core, int, int );
int avr_op_ST_Y_decr_do( AvrDevice *core, int, int );
int avr_op_ST_Y_incr_do( AvrDevice *core, int, int );
int avr_op_ST_Z_decr_do( AvrDevice *core, int, int );
int avr_op_ST_Z_incr_do( AvrDevice *core, int, int );
int avr_op_SUB_do( AvrDevice *core, int, int );
int avr_op_SUBI_do( AvrDevice *core, int, int );
int avr_op_SWAP_do( AvrDevice *core, int, int );
int avr_op_WDR_do( AvrDevice *core, int, int );
#endif
