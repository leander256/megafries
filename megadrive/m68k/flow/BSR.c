#include "BSR.h"


void mnemo_BSR( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t Displacement16 = M68k_Context_p->Current_Opcode & 0x00FF;
	uint32_t Displacement;
	uint8_t Ext_Word = 0;

	if( Displacement16 == 0)
	{
		M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Displacement16, 2);
		M68k_Context_p->Program_Counter += 2;			// NO ! BSR counts from after the opcode, extension word doesn't count
		if( M68k_Context_p->Interruptions != 0)
			return;

		Displacement = ntohs(Displacement16);
		if( (Displacement & 0x00008000) != 0)
			Displacement |= 0xFFFF0000;

		Ext_Word = 1;
	}
	else
	{
		Displacement = Displacement16;
		if( (Displacement & 0x00000080) != 0)
			Displacement |= 0xFFFFFF00;
	}

	// save PC on stack
	M68k_Context_p->A[7] -= 4;
	uint32_t Lit_PC = htonl(M68k_Context_p->Program_Counter);
	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( M68k_Context_p->A[7], &Lit_PC, 4);
	if( M68k_Context_p->Interruptions != 0)
		return;

	if( Ext_Word == 0)
	{
		M68k_Context_p->Program_Counter += Displacement;
		*N_Ticks -= 4;
	}
	else
	{
		M68k_Context_p->Program_Counter += Displacement - 2;
		*N_Ticks -= 8;
	}
}
