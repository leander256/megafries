#include "ADD.h"
#include "ADDX.h"


uint8_t ADD_Valid_EA[2][64] = { {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
									1, 1, 1, 1, 1, 1, 1, 1,		// 001 xxx An
									1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
									1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
									1, 1, 1, 1, 1, 0, 0, 0 },	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>
								{	2, 2, 2, 2, 2, 2, 2, 2,		// 000 xxx Dn
									2, 2, 2, 2, 2, 2, 2, 2,		// 001 xxx An
									1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
									1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
									1, 1, 0, 0, 0, 0, 0, 0 } };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>


void mnemo_ADD_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	uint8_t DestVal = (M68k_Context_p->Current_Opcode >> 8) & 1;
	switch( ADD_Valid_EA[DestVal][EA_Mode])
	{
		case 0:	M68k_Context_p->Interruptions |= INT_ILLEGAL;
				return;
		case 2: mnemo_ADDX_B( M68k_Context_p, N_Ticks);
				return;
	}

	uint16_t RegDn = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	uint8_t Reg_Value = M68k_Context_p->D[RegDn];

	uint8_t EA_Value;
	if( DestVal)
		EA_Value = EA_Read_B_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	else
	{
		EA_Value = EA_Read_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7);
		EA_Mode = RegDn;						// if we write into Dn then we use it as EA mode for the writing operation later on
	}
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint8_t Result = Reg_Value + EA_Value;

	EA_Write_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Result);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_XNZVC(M68k_Context_p->Status_Register);
	if( (Result & 0x80) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Result == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}
	Reg_Value &= 0x80;
	EA_Value &= 0x80;
	Result &= 0x80;

	if( (Reg_Value && EA_Value && !Result) || (!Reg_Value && !EA_Value && Result))
		CCR_SET_V(M68k_Context_p->Status_Register);
	if( (Reg_Value && EA_Value) || (Reg_Value && !Result) || (EA_Value && !Result))
		CCR_SET_XC(M68k_Context_p->Status_Register);

	*N_Ticks -= 4;
}

void mnemo_ADD_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	uint16_t DestVal = (M68k_Context_p->Current_Opcode >> 8) & 0x01;
	switch( ADD_Valid_EA[DestVal][EA_Mode])
	{
		case 0:	M68k_Context_p->Interruptions |= INT_ILLEGAL;
				return;
		case 2: mnemo_ADDX_W( M68k_Context_p, N_Ticks);
				return;
	}

	uint16_t RegDn = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	uint16_t Reg_Value = M68k_Context_p->D[RegDn];

	uint16_t EA_Value;
	if( DestVal)
		EA_Value = EA_Read_W_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	else
	{
		EA_Value = EA_Read_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7);
		EA_Mode = RegDn;						// if we write into Dn then we use it as EA mode for the writing operation later on
	}
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint16_t Result = Reg_Value + EA_Value;

	EA_Write_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Result);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_XNZVC(M68k_Context_p->Status_Register);
	if( (Result & 0x8000) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Result == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}
	Reg_Value &= 0x8000;
	EA_Value &= 0x8000;
	Result &= 0x8000;

	if( (Reg_Value && EA_Value && !Result) || (!Reg_Value && !EA_Value && Result))
		CCR_SET_V(M68k_Context_p->Status_Register);
	if( (Reg_Value && EA_Value) || (Reg_Value && !Result) || (EA_Value && !Result))
		CCR_SET_XC(M68k_Context_p->Status_Register);

	*N_Ticks -= 4;
}

void mnemo_ADD_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	uint16_t DestVal = (M68k_Context_p->Current_Opcode >> 8) & 0x01;
	switch( ADD_Valid_EA[DestVal][EA_Mode])
	{
		case 0:	M68k_Context_p->Interruptions |= INT_ILLEGAL;
				return;
		case 2: mnemo_ADDX_L( M68k_Context_p, N_Ticks);
				return;
	}

	uint16_t RegDn = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	uint32_t Reg_Value = M68k_Context_p->D[RegDn];

	uint32_t EA_Value;
	if( DestVal)
		EA_Value = EA_Read_L_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	else
	{
		EA_Value = EA_Read_L_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7);
		EA_Mode = RegDn;						// if we write into Dn then we use it as EA mode for the writing operation later on
	}
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Result = Reg_Value + EA_Value;

	EA_Write_L_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Result);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_XNZVC(M68k_Context_p->Status_Register);
	if( (Result & 0x80000000) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Result == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}
	Reg_Value &= 0x80000000;
	EA_Value &= 0x80000000;
	Result &= 0x80000000;

	if( (Reg_Value && EA_Value && !Result) || (!Reg_Value && !EA_Value && Result))
		CCR_SET_V(M68k_Context_p->Status_Register);
	if( (Reg_Value && EA_Value) || (Reg_Value && !Result) || (EA_Value && !Result))
		CCR_SET_XC(M68k_Context_p->Status_Register);

	*N_Ticks -= 8;
}
