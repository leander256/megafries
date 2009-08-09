#include "NOT.h"

uint8_t NOT_Valid_EA[64] = {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
								0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
								1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
								1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
								1, 1, 0, 0, 0, 0, 0, 0 };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>


void mnemo_NOT_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( NOT_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint8_t EA_Value = EA_Read_B_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	EA_Value = ~EA_Value;

	EA_Write_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, EA_Value);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);
	if( (EA_Value & 0x80) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( EA_Value == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	*N_Ticks -= 4;
}

void mnemo_NOT_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( NOT_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint16_t EA_Value = EA_Read_W_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	EA_Value = ~EA_Value;

	EA_Write_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, EA_Value);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);
	if( (EA_Value & 0x8000) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( EA_Value == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	*N_Ticks -= 4;
}

void mnemo_NOT_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	M68k_Context_p->Interruptions |= INT_ILLEGAL;
}
