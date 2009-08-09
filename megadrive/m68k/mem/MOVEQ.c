#include "MOVEQ.h"


void mnemo_MOVEQ( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);

	uint32_t Value = M68k_Context_p->Current_Opcode & 0x00FF;
	if( (Value & 0x00000080) != 0)
	{
		CCR_SET_N(M68k_Context_p->Status_Register);
		Value |= 0xFFFFFF00;
	}
	else
	{
		if( Value == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	M68k_Context_p->D[((M68k_Context_p->Current_Opcode & 0x0E00) >> 9)] = Value;

	*N_Ticks -= 4;
}
