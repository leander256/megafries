#include "DBcc.h"


void mnemo_DBcc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t Reg_Number = M68k_Context_p->Current_Opcode & 7;

	uint16_t Displacement16;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Displacement16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	if( CC_Function[ (M68k_Context_p->Current_Opcode & 0x0F00) >> 8] (M68k_Context_p->Status_Register) == 1)
	{
		*N_Ticks -= 6;
		return;
	}

	uint16_t Counter = M68k_Context_p->D[Reg_Number];
	Counter--;
	EA_Write_W_Dn( M68k_Context_p, N_Ticks, Reg_Number, Counter);

	if( Counter == 0xFFFF)
	{
		*N_Ticks -= 14;
		return;
	}

	uint32_t Displacement32 = ntohs(Displacement16);
	if( Displacement32 & 0x00008000)
		Displacement32 |= 0xFFFF0000;

	M68k_Context_p->Program_Counter += (Displacement32 - 2);

	*N_Ticks -= 10;
}
