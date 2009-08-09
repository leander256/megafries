#include "ROXd.h"


void mnemo_ROXd_REG_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	// TODO
	M68k_Context_p->Interruptions |= INT_ILLEGAL;
}

void mnemo_ROXd_REG_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
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
		uint16_t TempNumber = Number;
		while( TempNumber > 16)				// modulo 17 !
			TempNumber -= 17;
		uint32_t Buffer = Destination;

		if( TempNumber > 0)
		{
			if( Direction == 0)
			{
				Buffer <<= 1;
				if( CCR_CHECK_X(M68k_Context_p->Status_Register))
					Buffer |= 1;
				Buffer <<= (15 - (TempNumber - 1));

				Destination >>= TempNumber - 1;
				if( Destination & 1)
					CCR_SET_XC(M68k_Context_p->Status_Register);
				else
					CCR_CLEAR_XC(M68k_Context_p->Status_Register);
				Destination >>= 1;

				Destination |= Buffer;
			}
			else
			{
				Destination <<= 1;
				if( CCR_CHECK_X(M68k_Context_p->Status_Register))
					Destination |= 1;
				Destination <<= TempNumber - 1;

				Buffer >>= (16 - TempNumber);
				if( Buffer & 1)
					CCR_SET_XC(M68k_Context_p->Status_Register);
				else
					CCR_CLEAR_XC(M68k_Context_p->Status_Register);
				Buffer >>= 1;

				Destination |= Buffer;
			}
		}
		else
		{
			if( CCR_CHECK_X(M68k_Context_p->Status_Register))
				CCR_SET_C(M68k_Context_p->Status_Register);
		}
	}
	else
		if( CCR_CHECK_X(M68k_Context_p->Status_Register))
			CCR_SET_C(M68k_Context_p->Status_Register);

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

void mnemo_ROXd_REG_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	// TODO
	M68k_Context_p->Interruptions |= INT_ILLEGAL;

}
