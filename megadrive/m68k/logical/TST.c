#include "TST.h"


// 0 means invalid mode
const uint8_t TST_Valid_EA[64] = {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
									0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
									1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
									1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
									1, 1, 0, 0, 0, 0, 0, 0 };	// 111 000 (xxx).W and 111 001 (xxx).L


void mnemo_TST_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint8_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( TST_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint8_t Operand = EA_Read_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7);

	CCR_CLEAR_NZVC( M68k_Context_p->Status_Register);
	if( (Operand & 0x80) != 0)
		CCR_SET_N( M68k_Context_p->Status_Register);
	else
		if( Operand == 0)
			CCR_SET_Z( M68k_Context_p->Status_Register);

	*N_Ticks -= 4;
}

void mnemo_TST_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( TST_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint16_t Operand = EA_Read_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7);

	CCR_CLEAR_NZVC( M68k_Context_p->Status_Register);
	if( (Operand & 0x8000) != 0)
		CCR_SET_N( M68k_Context_p->Status_Register);
	else
		if( Operand == 0)
			CCR_SET_Z( M68k_Context_p->Status_Register);

	*N_Ticks -= 4;
}

void mnemo_TST_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( TST_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint32_t Operand = EA_Read_L_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7);

	CCR_CLEAR_NZVC( M68k_Context_p->Status_Register);
	if( (Operand & 0x80000000) != 0)
		CCR_SET_N( M68k_Context_p->Status_Register);
	else
		if( Operand == 0)
			CCR_SET_Z( M68k_Context_p->Status_Register);

	*N_Ticks -= 4;
}
