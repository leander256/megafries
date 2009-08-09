#include "Bcc.h"


void mnemo_Bcc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint32_t Displacement = M68k_Context_p->Current_Opcode & 0x00FF;
	uint32_t Ticks_16bits;

	if( Displacement == 0)
	{
		uint16_t Disp16;
		M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Disp16, 2);
		M68k_Context_p->Program_Counter += 2;
		if( M68k_Context_p->Interruptions != 0)
			return;

		Displacement = ntohs(Disp16);
		if( (Displacement & 0x00008000) != 0)
			Displacement |= 0xFFFF0000;

		Displacement -= 2;	// because PC should be 2 bytes before
		Ticks_16bits = 4;
	}
	else
	{
		if( (Displacement & 0x00000080) != 0)
			Displacement |= 0xFFFFFF00;

		Ticks_16bits = 0;
	}

	if( CC_Function[ (M68k_Context_p->Current_Opcode & 0x0F00) >> 8] (M68k_Context_p->Status_Register) == 1)
	{
		M68k_Context_p->Program_Counter += Displacement;
		*N_Ticks -= 10;
	}
	else
		*N_Ticks -= 8 + Ticks_16bits;
}
