#include "ASd.h"


void mnemo_ASd_REG_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t Reg_Number = M68k_Context_p->Current_Opcode & 0x0007;
	uint16_t Direction = M68k_Context_p->Current_Opcode & 0x0100;
	uint16_t IR = M68k_Context_p->Current_Opcode & 0x0020;
	uint16_t Number = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;

	if( IR == 0)
	{
		if( Number == 0)
			Number = 8;
	}
	else
		Number = M68k_Context_p->D[Number] & 0x0000003F;

	int8_t Destination = M68k_Context_p->D[Reg_Number];

	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);

	if( Number != 0)
	{
		CCR_CLEAR_X(M68k_Context_p->Status_Register);
		int8_t Old_Value = Destination;

		if( Direction == 0)
		{
			Destination >>= (Number - 1);
			if( (Destination & 1) != 0)
				CCR_SET_XC(M68k_Context_p->Status_Register);
			Destination >>= 1;
		}
		else
		{
			Destination <<= Number - 1;
			if( Destination & 0x80)
				CCR_SET_XC(M68k_Context_p->Status_Register);
			Destination <<= 1;

			int8_t Mask = 0xFF << (Number - 1);
			if( ((Destination >> Number) ^ Old_Value) & Mask)		// X-OR to the rescue to determine wether some bits are different
				CCR_SET_V(M68k_Context_p->Status_Register);
		}
	}

	if( Destination & 0x80)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Destination == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	EA_Write_B_Dn( M68k_Context_p, N_Ticks, Reg_Number, Destination);

	*N_Ticks -= (6 + 2 * Number);
}

void mnemo_ASd_REG_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t Reg_Number = M68k_Context_p->Current_Opcode & 0x0007;
	uint16_t Direction = M68k_Context_p->Current_Opcode & 0x0100;
	uint16_t IR = M68k_Context_p->Current_Opcode & 0x0020;
	uint16_t Number = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;

	if( IR == 0)
	{
		if( Number == 0)
			Number = 8;
	}
	else
		Number = M68k_Context_p->D[Number] & 0x0000003F;

	int16_t Destination = M68k_Context_p->D[Reg_Number];

	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);

	if( Number != 0)
	{
		CCR_CLEAR_X(M68k_Context_p->Status_Register);
		int16_t Old_Value = Destination;

		if( Direction == 0)
		{
			Destination >>= (Number - 1);
			if( (Destination & 1) != 0)
				CCR_SET_XC(M68k_Context_p->Status_Register);
			Destination >>= 1;
		}
		else
		{
			Destination <<= Number - 1;
			if( Destination & 0x8000)
				CCR_SET_XC(M68k_Context_p->Status_Register);
			Destination <<= 1;

			int16_t Mask = 0xFFFF << (Number - 1);
			if( ((Destination >> Number) ^ Old_Value) & Mask)		// X-OR to the rescue to determine wether some bits are different
				CCR_SET_V(M68k_Context_p->Status_Register);
		}
	}

	if( Destination & 0x8000)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Destination == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	EA_Write_W_Dn( M68k_Context_p, N_Ticks, Reg_Number, Destination);

	*N_Ticks -= (6 + 2 * Number);
}

void mnemo_ASd_REG_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t Reg_Number = M68k_Context_p->Current_Opcode & 0x0007;
	uint16_t Direction = M68k_Context_p->Current_Opcode & 0x0100;
	uint16_t IR = M68k_Context_p->Current_Opcode & 0x0020;
	uint16_t Number = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;

	if( IR == 0)
	{
		if( Number == 0)
			Number = 8;
	}
	else
		Number = M68k_Context_p->D[Number] & 0x0000003F;

	int32_t Destination = M68k_Context_p->D[Reg_Number];

	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);

	if( Number != 0)
	{
		CCR_CLEAR_X(M68k_Context_p->Status_Register);
		int16_t Old_Value = Destination;

		if( Direction == 0)
		{
			Destination >>= (Number - 1);
			if( (Destination & 1) != 0)
				CCR_SET_XC(M68k_Context_p->Status_Register);
			Destination >>= 1;
		}
		else
		{
			Destination <<= Number - 1;
			if( Destination & 0x80000000)
				CCR_SET_XC(M68k_Context_p->Status_Register);
			Destination <<= 1;

			int16_t Mask = 0xFFFFFFFF << (Number - 1);
			if( ((Destination >> Number) ^ Old_Value) & Mask)		// X-OR to the rescue to determine whether some bits are different
				CCR_SET_V(M68k_Context_p->Status_Register);
		}
	}

	if( Destination & 0x80000000)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Destination == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	EA_Write_L_Dn( M68k_Context_p, N_Ticks, Reg_Number, Destination);

	*N_Ticks -= (8 + 2 * Number);
}
