#include "BRA.h"


void mnemo_BRA( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint32_t Displacement = M68k_Context_p->Current_Opcode & 0x00FF;

	if( Displacement == 0)
	{
		uint16_t Displacement16;
		M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Displacement16, 2);
		M68k_Context_p->Program_Counter += 2;
		if( M68k_Context_p->Interruptions != 0)
			return;

		Displacement = ntohs( Displacement16);

		if( (Displacement & 0x00008000) != 0)
			Displacement |= 0xFFFF0000;
		Displacement -= 2;

		*N_Ticks -= 4;
	}
	else
	{
		if( (Displacement & 0x00000080) != 0)
			Displacement |= 0xFFFFFF00;
	}

	M68k_Context_p->Program_Counter += Displacement;
	*N_Ticks -= 4;
}
