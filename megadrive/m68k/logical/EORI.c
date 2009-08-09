#include "EORI.h"


uint8_t EORI_Valid_EA[3][64] = { {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
									0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
									1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
									1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
									1, 1, 0, 0, 0, 0, 0, 0 },	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>
								{	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
									0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
									1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
									1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
									1, 1, 0, 0, 0, 0, 0, 0 },	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>
								{	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
									0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
									1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
									1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
									1, 1, 0, 0, 0, 0, 0, 0 } };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>


void mnemo_EORI_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( EORI_Valid_EA[0][EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint8_t Imm_Value;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter + 1, &Imm_Value, 1);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint8_t Destination = EA_Read_B_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	Destination ^= Imm_Value;

	EA_Write_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Destination);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);
	if( (Destination & 0x80) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Destination == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	if( (EA_Mode & 0x0038) == 0)
		*N_Ticks -= 8;
	else
		*N_Ticks -= 12;
}

void mnemo_EORI_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( EORI_Valid_EA[0][EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint16_t Imm_Value;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Imm_Value, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint16_t Destination = EA_Read_W_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	Destination ^= Imm_Value;

	EA_Write_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Destination);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);
	if( (Destination & 0x8000) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Destination == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	if( (EA_Mode & 0x0038) == 0)
		*N_Ticks -= 8;
	else
		*N_Ticks -= 12;
}

void mnemo_EORI_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( EORI_Valid_EA[2][EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint32_t Imm_Value;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Imm_Value, 4);
	M68k_Context_p->Program_Counter += 4;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Destination = EA_Read_L_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	Destination ^= Imm_Value;

	EA_Write_L_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Destination);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);
	if( (Destination & 0x80000000) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Destination == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	if( (EA_Mode & 0x0038) == 0)
		*N_Ticks -= 16;
	else
		*N_Ticks -= 20;
}
