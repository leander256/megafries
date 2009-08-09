#include "CMPA.h"
#include "CMP_common.h"


void mnemo_CMPA( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x3F;
	if( EA_Mode > 0x3C)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint32_t Source;
	if( M68k_Context_p->Current_Opcode & 0x0100)
	{
		Source = EA_Read_L_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7);
		if( M68k_Context_p->Interruptions != 0)
			return;
	}
	else
	{
		uint16_t Source16 = EA_Read_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7);
		if( M68k_Context_p->Interruptions != 0)
			return;

		Source = Source16;
		if( Source & 0x8000)
			Source |= 0xFFFF0000;
	}

	M68k_Context_p->Status_Register &= 0xFFF0;
	M68k_Context_p->Status_Register |= CMP_L( M68k_Context_p->A[ (M68k_Context_p->Current_Opcode >> 9) & 7], Source);

	*N_Ticks -= 6;
}
