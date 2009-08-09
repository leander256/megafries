#include "NEG.h"

uint8_t NEG_Valid_EA[64] = {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
								0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
								1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
								1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
								1, 1, 0, 0, 0, 0, 0, 0 };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>


void mnemo_NEG_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( NEG_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint8_t EA_Value = EA_Read_B_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint8_t Result = - EA_Value;

	EA_Write_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Result);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_XNZVC(M68k_Context_p->Status_Register);

	if( (Result & 0x80) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	if( Result == 0)
		CCR_SET_Z(M68k_Context_p->Status_Register);
	else
		CCR_SET_XC(M68k_Context_p->Status_Register);

	if( (EA_Value & 0x80) && (Result & 0x80))
		CCR_SET_V(M68k_Context_p->Status_Register);

	if( EA_Mode < 8)
		*N_Ticks -= 4;
	else
		*N_Ticks -= 8;
}

void mnemo_NEG_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( NEG_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint16_t EA_Value = EA_Read_W_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint16_t Result = - EA_Value;

	EA_Write_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Result);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_XNZVC(M68k_Context_p->Status_Register);

	if( (Result & 0x8000) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	if( Result == 0)
		CCR_SET_Z(M68k_Context_p->Status_Register);
	else
		CCR_SET_XC(M68k_Context_p->Status_Register);

	if( (EA_Value & 0x8000) && (Result & 0x8000))
		CCR_SET_V(M68k_Context_p->Status_Register);

	if( EA_Mode < 8)
		*N_Ticks -= 4;
	else
		*N_Ticks -= 8;
}

void mnemo_NEG_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( NEG_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint32_t EA_Value = EA_Read_L_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Result = - EA_Value;

	EA_Write_L_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Result);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_XNZVC(M68k_Context_p->Status_Register);

	if( (Result & 0x80000000) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	if( Result == 0)
		CCR_SET_Z(M68k_Context_p->Status_Register);
	else
		CCR_SET_XC(M68k_Context_p->Status_Register);

	if( (EA_Value & 0x80000000) && (Result & 0x80000000))
		CCR_SET_V(M68k_Context_p->Status_Register);

	if( EA_Mode < 8)
		*N_Ticks -= 8;
	else
		*N_Ticks -= 12;
}
