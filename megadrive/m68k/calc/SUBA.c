#include "SUBA.h"


void mnemo_SUBA( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( EA_Mode > 0x3C)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint32_t Source;
	if( M68k_Context_p->Current_Opcode & 0x0100)
		Source = EA_Read_L_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007);
	else
	{
		Source = EA_Read_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007);
		if( Source & 0x8000)
			Source |= 0xFFFF0000;
	}
	if( M68k_Context_p->Interruptions != 0)
		return;

	M68k_Context_p->A[ (M68k_Context_p->Current_Opcode & 0x0E00) >> 9 ] -= Source;
	*N_Ticks -= 8;
}
