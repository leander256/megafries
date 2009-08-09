#include <stdlib.h>

#include "ilt.h"

#include "bcd/ABCD.h"
#include "calc/ADD.h"
#include "calc/ADDA.h"
#include "calc/ADDI.h"
#include "calc/ADDQ.h"
#include "logical/AND.h"
#include "logical/ANDI.h"
#include "logical/ASd.h"
#include "flow/Bcc.h"
#include "logical/BCHG.h"
#include "logical/BCLR.h"
#include "flow/BRA.h"
#include "logical/BSET.h"
#include "flow/BSR.h"
#include "logical/BTST.h"
#include "logical/CLR.h"
#include "logical/CMP.h"
#include "logical/CMPA.h"
#include "logical/CMPI.h"
#include "calc/DIVS.h"
#include "calc/DIVU.h"
#include "logical/EOR.h"
#include "logical/EORI.h"
#include "flow/JMP.h"
#include "flow/JSR.h"
#include "mem/LEA.h"
#include "logical/LSd.h"
#include "mem/MOVE.h"
#include "mem/MOVEA.h"
#include "mem/MOVEM.h"
#include "mem/MOVEQ.h"
#include "mem/MOVE_USP.h"
#include "mem/MOVEtoCCR.h"
#include "mem/MOVEfromSR.h"
#include "mem/MOVEtoSR.h"
#include "calc/MULS.h"
#include "calc/MULU.h"
#include "flow/NOP.h"
#include "calc/NEG.h"
#include "logical/NOT.h"
#include "logical/OR.h"
#include "logical/ORI.h"
#include "mem/PEA.h"
#include "logical/ROd.h"
#include "logical/ROXd.h"
#include "flow/RTE.h"
#include "flow/RTS.h"
#include "logical/Scc.h"
#include "calc/SUB.h"
#include "calc/SUBA.h"
#include "calc/SUBI.h"
#include "calc/SUBQ.h"
#include "flow/TRAP.h"
#include "logical/TST.h"


#define INIT_TIMINGS(Dest) init_timings_##Dest();
#define INIT_FUNCTIONS(Dest) init_functions_##Dest();


Instruction_Method Instruction_Lookup_Table[1024] = { NULL };

Instruction_Method ILT_0100111001[64] = { NULL };
Instruction_Method ILT_1110xxxxSS[32] = { NULL };

uint8_t Table_is_Initialized = 0;


void ILT_Indirect_0100111001( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	Instruction_Method Current_Instruction = ILT_0100111001[M68k_Context_p->Current_Opcode & 0x003F];

	if( Current_Instruction == NULL)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	Current_Instruction( M68k_Context_p, N_Ticks);
}

void ILT_Indirect_1110xxxxSS( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	Instruction_Method Current_Instruction = ILT_1110xxxxSS[(M68k_Context_p->Current_Opcode >> 3) & 0x001F];

	if( Current_Instruction == NULL)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	Current_Instruction( M68k_Context_p, N_Ticks);
}

void Add_ILT( uint16_t Offset, void (*Func_Add)(struct M68k_Context*, int32_t*))
{
	if( Instruction_Lookup_Table[Offset] != 0)
		abort();

	Instruction_Lookup_Table[Offset] = Func_Add;
}

void Init_ILT()
{
	if( Table_is_Initialized != 0)
		return;

	// ADD/ADDX - 1101 | REG(3) | DIR(1) | SIZE E [00,01,10]
	for( int i = 0; i < 8; i++)
		for( int j = 0; j < 2; j++)
		{
			Add_ILT( 0x340 | (i << 3) | (j << 2) , &mnemo_ADD_B);
			Add_ILT( 0x341 | (i << 3) | (j << 2) , &mnemo_ADD_W);
			Add_ILT( 0x342 | (i << 3) | (j << 2) , &mnemo_ADD_L);
		}

	// ADDA - 1101 | REG(3) | SIZE(1) | 11
	for( int i = 0; i < 8; i++)
	{
		Add_ILT( 0x343 | (i << 3) , &mnemo_ADDA_W);
		Add_ILT( 0x347 | (i << 3) , &mnemo_ADDA_L);
	}

	// ADDI - 00000110 | SIZE(2) E [00,01,10]
	Add_ILT( 0x018 , &mnemo_ADDI_B);
	Add_ILT( 0x019 , &mnemo_ADDI_W);
	Add_ILT( 0x01A , &mnemo_ADDI_L);

	// ADDQ - 0101 | DATA(3) | 0 | SIZE(2) E [00,10]
	for( int i = 0; i < 8; i++)
	{
		Add_ILT( 0x140 | (i << 3) , &mnemo_ADDQ_B);
		Add_ILT( 0x141 | (i << 3) , &mnemo_ADDQ_W);
		Add_ILT( 0x142 | (i << 3) , &mnemo_ADDQ_L);
	}

	// AND - 1100 | REG(3) | DIR(1) | SIZE(2) E [00,01,10]
	// ABCD - 1100 | REG(3) | 100
	for( int i = 0; i < 8; i++)
		for( int j = 0; j < 2; j++)
		{
			Add_ILT( 0x300 | (i << 3) | (j << 2) , &mnemo_AND_B);
			Add_ILT( 0x301 | (i << 3) | (j << 2) , &mnemo_AND_W);
			Add_ILT( 0x302 | (i << 3) | (j << 2) , &mnemo_AND_L);
		}

	// ANDI - 00000010 | SIZE(2) E [00,10]
	Add_ILT( 0x008 , &mnemo_ANDI_B);
	Add_ILT( 0x009 , &mnemo_ANDI_W);
	Add_ILT( 0x00A , &mnemo_ANDI_L);

	// Bcc - 0110 | COND(4) E [0010,1111] | OFFSET(2)    the two higher offset bits are part of the opcode
	for( int i = 0; i < 4; i++)
		for( int j = 2; j < 16; j++)
			Add_ILT( 0x180 | (j << 2) | i , &mnemo_Bcc);

	// BCHG - 0000 | REG(3) | 101
	//        0000100001
	for( int i = 0; i < 8; i++)
		Add_ILT( 0x005 | (i << 3) , &mnemo_BCHG);
	Add_ILT( 0x021 , &mnemo_BCHG);

	// BCLR - 0000 | REG(3) | 110
	//        0000100010
	for( int i = 0; i < 8; i++)
		Add_ILT( 0x006 | (i << 3) , &mnemo_BCLR);
	Add_ILT( 0x022 , &mnemo_BCLR);

	// BRA - 01100000 | OFFSET(2)	the two higher offset bits are part of the opcode
	for( int i = 0; i < 4; i++)
		Add_ILT( 0x180 | i , &mnemo_BRA);

	// BSET - 0000 | REG(3) | 111
	//        0000100011
	for( int i = 0; i < 8; i++)
		Add_ILT( 0x007 | (i << 3) , &mnemo_BSET);
	Add_ILT( 0x023 , &mnemo_BSET);

	// BSR - 01100001 | OFFSET(2)	the two higher offset bits are part of the opcode
	for( int i = 0; i < 4; i++)
		Add_ILT( 0x184 | i , &mnemo_BSR);

	// BTST - 0000 | REG(3) | 100
	//        0000100000
	for( int i = 0; i < 8; i++)
		Add_ILT( 0x004 | (i << 3) , &mnemo_BTST_Dyn);
	Add_ILT( 0x020 , &mnemo_BTST_Sta);

	// CLR - 01000010 | SIZE(2) E [00,01,10]
	Add_ILT( 0x108 , &mnemo_CLR_B);
	Add_ILT( 0x109 , &mnemo_CLR_W);
	Add_ILT( 0x10A , &mnemo_CLR_L);

	// CMP - 1011 | REG(3) | 0 | SIZE(2) E [00,01,10]
	for( int i = 0; i < 8; i++)
	{
		Add_ILT( 0x2C0 | (i << 3) , &mnemo_CMP_B);
		Add_ILT( 0x2C1 | (i << 3) , &mnemo_CMP_W);
		Add_ILT( 0x2C2 | (i << 3) , &mnemo_CMP_L);
	}

	// CMPA - 1011 | REG(3) | SIZE(1) | 11
	for( int i = 0; i < 16; i++)
		Add_ILT( 0x2C3 | (i << 2) , &mnemo_CMPA);

	// CMPI - 00001100 | SIZE(2) E [00,01,10]
	Add_ILT( 0x030 , &mnemo_CMPI_B);
	Add_ILT( 0x031 , &mnemo_CMPI_W);
	Add_ILT( 0x032 , &mnemo_CMPI_L);

	// DIVS - 1000 | REG(3) | 111
	for( int i = 0; i < 8; i++)
		Add_ILT( 0x207 | (i << 3) , &mnemo_DIVS);

	// DIVU - 1000 | REG(3) | 011
	for( int i = 0; i < 8; i++)
		Add_ILT( 0x203 | (i << 3) , &mnemo_DIVU);

	// EOR - 1011 | REG(3) | 1 | SIZE(2) E [00,01,10]
	for( int i = 0; i < 8; i++)
	{
		Add_ILT( 0x2c4 | (i << 3) , &mnemo_EOR_B);
		Add_ILT( 0x2c5 | (i << 3) , &mnemo_EOR_W);
		Add_ILT( 0x2c6 | (i << 3) , &mnemo_EOR_L);
	}

	// EORI - 00001010 | SIZE(2) E [00,01,10]
	Add_ILT( 0x028 , &mnemo_EORI_B);
	Add_ILT( 0x029 , &mnemo_EORI_W);
	Add_ILT( 0x02A , &mnemo_EORI_L);

	// JMP - 0100111011
	Add_ILT( 0x13b , &mnemo_JMP);

	// JSR - 0100111010
	Add_ILT( 0x13a , &mnemo_JSR);

	// LEA - 0100 | REG(3) | 111
	for( int i = 0; i < 8; i++)
		Add_ILT( 0x107 | (i << 3) , &mnemo_LEA);

	// MOVE - 00 | SIZE(2) E [00,10] | DEST(6) E [000000,111001] - [001000,001111]  BUT dest is written backwards so register then mode!
	for( int i = 0; i < 58; i++)
		if( (i >> 3) != 1)
		{
			int j = ((i & 7) << 3) + (i >> 3);
			Add_ILT( 0x040 | j , &mnemo_MOVE_B);
			Add_ILT( 0x0C0 | j , &mnemo_MOVE_W);
			Add_ILT( 0x080 | j , &mnemo_MOVE_L);
		}

	// MOVEA - 00 | SIZE(3) E { 11, 10 } | REG(3) | 001
	for( int i = 0; i < 8; i++)
	{
		Add_ILT( 0x0C1 | (i << 3) , &mnemo_MOVEA_W);
		Add_ILT( 0x081 | (i << 3) , &mnemo_MOVEA_L);
	}

	// MOVEM - 01001 | DR(1) | 001 | SIZE(1)
	// EXT - 010010001 | SIZE(1)                accessed from within MOVEM
	for( int i = 0; i < 2; i++)
	{
		Add_ILT( 0x122 | (i << 4) , &mnemo_MOVEM_W);
		Add_ILT( 0x123 | (i << 4) , &mnemo_MOVEM_L);
	}

	// MOVEQ - 0111 | REG(3) | 0 | DATA(2)
	for( int i = 0; i < 8; i++)
		for( int j = 0; j < 4; j++)
			Add_ILT( 0x1C0 | (i << 3) | j , &mnemo_MOVEQ);

	// MOVE to CCR - 0100010011
	Add_ILT( 0x113 , &mnemo_MOVEtoCCR);

	// MOVE from SR - 0100000011
	Add_ILT( 0x103 , &mnemo_MOVEfromSR);

	// MOVE to SR - 0100011011
	Add_ILT( 0x11b , &mnemo_MOVEtoSR);

	// MULS - 1100 | REG(3) | 111
	for( int i = 0; i < 8; i++)
		Add_ILT( 0x307 | (i << 3) , &mnemo_MULS);

	// MULU - 1100 | REG(3) | 011
	for( int i = 0; i < 8; i++)
		Add_ILT( 0x303 | (i << 3) , &mnemo_MULU);

	// NEG - 01000100 | SIZE(2) E [00,01,10]
	Add_ILT( 0x110 , &mnemo_NEG_B);
	Add_ILT( 0x111 , &mnemo_NEG_W);
	Add_ILT( 0x112 , &mnemo_NEG_L);

	// NOT - 01000110 | SIZE(2) E [00,01,10]
	Add_ILT( 0x118 , &mnemo_NOT_B);
	Add_ILT( 0x119 , &mnemo_NOT_W);
	Add_ILT( 0x11A , &mnemo_NOT_L);

	// OR - 1000 | REG(3) | DIR(1) | SIZE(2) E [00,01,10]
	for( int i = 0; i < 8; i++)
		for( int j = 0; j < 2; j++)
		{
			Add_ILT( 0x200 | (i << 3) | (j << 2) , &mnemo_OR_B);
			Add_ILT( 0x201 | (i << 3) | (j << 2) , &mnemo_OR_W);
			Add_ILT( 0x202 | (i << 3) | (j << 2) , &mnemo_OR_L);
		}

	// ORI - 00000000 | SIZE(2) E [00,01,10]
	Add_ILT( 0x000 , &mnemo_ORI_B);
	Add_ILT( 0x001 , &mnemo_ORI_W);
	Add_ILT( 0x002 , &mnemo_ORI_L);

	// PEA/SWAP - 0100100001
	Add_ILT( 0x121 , &mnemo_PEA);

	// Scc/DBcc - 0101 | COND(4) | 11
	for( int i = 0; i < 16; i++)
		Add_ILT( 0x143 | (i << 2) , &mnemo_Scc);

	// SUB/SUBX - 1001 | REG(3) | DIR(1) | SIZE E [00,01,10]
	for( int i = 0; i < 8; i++)
		for( int j = 0; j < 2; j++)
		{
			Add_ILT( 0x240 | (i << 3) | (j << 2) , &mnemo_SUB_B);
			Add_ILT( 0x241 | (i << 3) | (j << 2) , &mnemo_SUB_W);
			Add_ILT( 0x242 | (i << 3) | (j << 2) , &mnemo_SUB_L);
		}

	// SUBA- 1001 | REG(3) | SIZE(1) | 11
	for( int i = 0; i < 16; i++)
		Add_ILT( 0x243 | (i << 2) , &mnemo_SUBA);

	// SUBQ - 0101 | DATA(3) | 1 | SIZE E [00,01,10]
	for( int i = 0; i < 8; i++)
	{
		Add_ILT( 0x144 | (i << 3) , &mnemo_SUBQ_B);
		Add_ILT( 0x145 | (i << 3) , &mnemo_SUBQ_W);
		Add_ILT( 0x146 | (i << 3) , &mnemo_SUBQ_L);
	}

	// SUBI - 00000100 | SIZE(2) E [00,01,10]
	Add_ILT( 0x010 , &mnemo_SUBI_B);
	Add_ILT( 0x011 , &mnemo_SUBI_W);
	Add_ILT( 0x012 , &mnemo_SUBI_L);

	// TST - 01001010 | SIZE(2) E [00,10]
	Add_ILT( 0x128 , &mnemo_TST_B);
	Add_ILT( 0x129 , &mnemo_TST_W);
	Add_ILT( 0x12A , &mnemo_TST_L);


	////////////////////////////////////////////////////////////////////////////////
	// 0100111001 instructions: MOVE USR, NOP, RTE, RTS, TRAP
	Add_ILT( 0x139 , &ILT_Indirect_0100111001);

	// MOVE USR - 10 | DIR(1) | REG(3)
	for( int i = 0; i < 16; i++)
		ILT_0100111001[ 0x20 | i ] = &mnemo_MOVE_USP;

	// NOP - 110001
	ILT_0100111001[ 0x31 ] = &mnemo_NOP;

	// RTE - 110011
	ILT_0100111001[ 0x33 ] = &mnemo_RTE;

	// RTS - 110101
	ILT_0100111001[ 0x35 ] = &mnemo_RTS;

	// TRAP - 00 | VECTOR(4)
	for( int i = 0; i < 16; i++)
		ILT_0100111001[ i ] = &mnemo_TRAP;

	////////////////////////////////////////////////////////////////////////////////
	// 1110xxxxSS instructions: ASd, LSd, ROd, ROXd
	for( int i = 0; i < 16; i++)
		for( int j = 0; j < 3; j++)
			Add_ILT( 0x380 | (i << 2) | j , &ILT_Indirect_1110xxxxSS);

	// ASd REG - SIZE(2) | I/R(1) | 00
	for( int i = 0; i < 2; i++)
	{
		ILT_1110xxxxSS[ 0x00 | (i << 2) ] = &mnemo_ASd_REG_B;
		ILT_1110xxxxSS[ 0x08 | (i << 2) ] = &mnemo_ASd_REG_W;
		ILT_1110xxxxSS[ 0x10 | (i << 2) ] = &mnemo_ASd_REG_L;
	}

	// LSd REG - SIZE(2) | I/R(1) | 01
	for( int i = 0; i < 2; i++)
	{
		ILT_1110xxxxSS[ 0x01 | (i << 2) ] = &mnemo_LSd_REG_B;
		ILT_1110xxxxSS[ 0x09 | (i << 2) ] = &mnemo_LSd_REG_W;
		ILT_1110xxxxSS[ 0x11 | (i << 2) ] = &mnemo_LSd_REG_L;
	}

	// ROd REG - SIZE(2) | I/R(1) | 11
	for( int i = 0; i < 2; i++)
	{
		ILT_1110xxxxSS[ 0x03 | (i << 2) ] = &mnemo_ROd_REG_B;
		ILT_1110xxxxSS[ 0x0B | (i << 2) ] = &mnemo_ROd_REG_W;
		ILT_1110xxxxSS[ 0x13 | (i << 2) ] = &mnemo_ROd_REG_L;
	}

	// ROXd REG - SIZE(2) | I/R(1) | 10
	for( int i = 0; i < 2; i++)
	{
		ILT_1110xxxxSS[ 0x02 | (i << 2) ] = &mnemo_ROXd_REG_B;
		ILT_1110xxxxSS[ 0x0A | (i << 2) ] = &mnemo_ROXd_REG_W;
		ILT_1110xxxxSS[ 0x12 | (i << 2) ] = &mnemo_ROXd_REG_L;
	}


	Table_is_Initialized = 1;
}
