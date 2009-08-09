#include "ADDA.h"


void mnemo_ADDA_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;

	if( EA_Mode > 60)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
			return;
	}

	uint32_t EA_Value = EA_Read_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007);
	if( M68k_Context_p->Interruptions != 0)
		return;

	if( EA_Value & 0x8000)
		EA_Value |= 0xFFFF0000;

	uint16_t RegAn = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	M68k_Context_p->A[RegAn] += EA_Value;

	*N_Ticks -= 4;
}

void mnemo_ADDA_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;

	if( EA_Mode > 60)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
			return;
	}

	uint32_t EA_Value = EA_Read_L_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007);
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint16_t RegAn = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	M68k_Context_p->A[RegAn] += EA_Value;

	*N_Ticks -= 8;
}
