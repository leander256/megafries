#include "ROd.h"


void mnemo_ROd_REG_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
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

	uint8_t Destination = M68k_Context_p->D[Reg_Number];

	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);

	if( Number != 0)
	{
		uint16_t TempNumber = Number & 7;
		uint8_t Buffer = Destination;

		if( Direction == 0)
		{
			Destination >>= TempNumber;
			Buffer <<= (8 - TempNumber);
			Destination |= Buffer;

			if( (Destination & 0x80) != 0)
				CCR_SET_C(M68k_Context_p->Status_Register);
		}
		else
		{
			Destination <<= TempNumber;
			Buffer >>= (8 - TempNumber);
			Destination |= Buffer;

			if( (Destination & 1) != 0)
				CCR_SET_C(M68k_Context_p->Status_Register);
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
	*N_Ticks -= (4 + 2 * Number);
}

void mnemo_ROd_REG_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
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

	uint16_t Destination = M68k_Context_p->D[Reg_Number];

	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);

	if( Number != 0)
	{
		uint16_t TempNumber = Number & 0xF;
		uint8_t Buffer = Destination;

		if( Direction == 0)
		{
			Destination >>= TempNumber;
			Buffer <<= (8 - TempNumber);
			Destination |= Buffer;

			if( (Destination & 0x8000) != 0)
				CCR_SET_C(M68k_Context_p->Status_Register);
		}
		else
		{
			Destination <<= TempNumber;
			Buffer >>= (8 - TempNumber);
			Destination |= Buffer;

			if( (Destination & 1) != 0)
				CCR_SET_C(M68k_Context_p->Status_Register);
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
	*N_Ticks -= (4 + 2 * Number);
}

void mnemo_ROd_REG_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
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

	uint32_t Destination = M68k_Context_p->D[Reg_Number];

	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);

	if( Number != 0)
	{
		uint16_t TempNumber = Number & 0x001F;
		uint32_t Buffer = Destination;

		if( Direction == 0)
		{
			Destination >>= TempNumber;
			Buffer <<= (32 - TempNumber);
			Destination |= Buffer;

			if( (Destination & 0x80000000) != 0)
				CCR_SET_C(M68k_Context_p->Status_Register);
		}
		else
		{
			Destination <<= TempNumber;
			Buffer >>= (32 - TempNumber);
			Destination |= Buffer;

			if( (Destination & 1) != 0)
				CCR_SET_C(M68k_Context_p->Status_Register);
		}
	}

	if( Destination & 0x80000000)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Destination == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	M68k_Context_p->D[Reg_Number] = Destination;
	*N_Ticks -= (8 + 2 * Number);
}
