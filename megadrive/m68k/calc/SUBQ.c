#include "SUBQ.h"

uint8_t SUBQ_Valid_EA[64] = {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
								1, 1, 1, 1, 1, 1, 1, 1,		// 001 xxx An
								1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
								1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
								1, 1, 0, 0, 0, 0, 0, 0 };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>


void mnemo_SUBQ_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	if( (M68k_Context_p->Current_Opcode & 0x0038) == 0x0008)
	{
		//mnemo_SUBQ_L( M68k_Context_p, N_Ticks);
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( SUBQ_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint8_t Source_Value = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	if( Source_Value == 0)
		Source_Value = 8;

	uint8_t Destination_Value = EA_Read_B_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint8_t Result = Destination_Value - Source_Value;

	EA_Write_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Result);
	if( M68k_Context_p->Interruptions != 0)
		return;

	if( (EA_Mode & 0x0038) != 0x0008)	// if dest is not An
	{
		CCR_CLEAR_XNZVC(M68k_Context_p->Status_Register);

		if( (Result & 0x80) != 0)
		{
			CCR_SET_N(M68k_Context_p->Status_Register);

			if( (Destination_Value & 0x80) == 0)		// simplified C formula since Source is always between 0 and 8
				CCR_SET_XC(M68k_Context_p->Status_Register);
		}
		else
		{
			if( (Destination_Value & 0x80) != 0)		// simplified C formula since Source is always between 0 and 8
				CCR_SET_V(M68k_Context_p->Status_Register);

			if( Result == 0)
				CCR_SET_Z(M68k_Context_p->Status_Register);
		}
	}

	*N_Ticks -= 4;
}

void mnemo_SUBQ_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	if( (M68k_Context_p->Current_Opcode & 0x0038) == 0x0008)
	{
		mnemo_SUBQ_L( M68k_Context_p, N_Ticks);
		return;
	}

	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( SUBQ_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint16_t Source_Value = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	if( Source_Value == 0)
		Source_Value = 8;

	uint16_t Destination_Value = EA_Read_W_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint16_t Result = Destination_Value - Source_Value;

	EA_Write_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Result);
	if( M68k_Context_p->Interruptions != 0)
		return;

	if( (EA_Mode & 0x0038) != 0x0008)	// if dest is not An
	{
		CCR_CLEAR_XNZVC(M68k_Context_p->Status_Register);

		if( (Result & 0x8000) != 0)
		{
			CCR_SET_N(M68k_Context_p->Status_Register);

			if( (Destination_Value & 0x8000) == 0)		// simplified C formula since Source is always between 0 and 8
				CCR_SET_XC(M68k_Context_p->Status_Register);
		}
		else
		{
			if( (Destination_Value & 0x8000) != 0)		// simplified C formula since Source is always between 0 and 8
				CCR_SET_V(M68k_Context_p->Status_Register);

			if( Result == 0)
				CCR_SET_Z(M68k_Context_p->Status_Register);
		}
	}

	*N_Ticks -= 4;
}

void mnemo_SUBQ_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( SUBQ_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint32_t Source_Value = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	if( Source_Value == 0)
		Source_Value = 8;

	uint32_t Destination_Value = EA_Read_L_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Result = Destination_Value - Source_Value;

	EA_Write_L_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Result);
	if( M68k_Context_p->Interruptions != 0)
		return;

	if( (EA_Mode & 0x0038) != 0x0008)	// if dest is not An
	{
		CCR_CLEAR_XNZVC(M68k_Context_p->Status_Register);

		if( (Result & 0x80000000) != 0)
		{
			CCR_SET_N(M68k_Context_p->Status_Register);

			if( (Destination_Value & 0x80000000) == 0)		// simplified C formula since Source is always between 0 and 8
				CCR_SET_XC(M68k_Context_p->Status_Register);
		}
		else
		{
			if( (Destination_Value & 0x80000000) != 0)		// simplified C formula since Source is always between 0 and 8
				CCR_SET_V(M68k_Context_p->Status_Register);

			if( Result == 0)
				CCR_SET_Z(M68k_Context_p->Status_Register);
		}
	}

	*N_Ticks -= 8;
}
