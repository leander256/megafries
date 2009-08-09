#include "EXT.h"


void mnemo_EXT_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t Reg_Number = M68k_Context_p->Current_Opcode & 0x3F;
	if( Reg_Number > 7)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	if( M68k_Context_p->D[Reg_Number] & 0x80)
		M68k_Context_p->D[Reg_Number] |= 0x0000FF00;
	else
		M68k_Context_p->D[Reg_Number] &= 0xFFFF00FF;

	*N_Ticks += 4;
}

void mnemo_EXT_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t Reg_Number = M68k_Context_p->Current_Opcode & 0x3F;
	if( Reg_Number > 7)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	if( M68k_Context_p->D[Reg_Number] & 0x8000)
		M68k_Context_p->D[Reg_Number] |= 0xFFFF0000;
	else
		M68k_Context_p->D[Reg_Number] &= 0x0000FFFF;

	*N_Ticks += 4;
}
