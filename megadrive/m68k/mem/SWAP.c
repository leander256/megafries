#include "SWAP.h"


void mnemo_SWAP( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t Reg_Number = M68k_Context_p->Current_Opcode & 7;

	uint32_t Destination = M68k_Context_p->D[Reg_Number];
	uint32_t Buffer = Destination;

	Destination >>= 16;
	Buffer <<= 16;

	Destination |= Buffer;

	M68k_Context_p->D[Reg_Number] = Destination;

	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);
	if( Destination & 0x80000000)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Destination == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	*N_Ticks -= 4;
}
