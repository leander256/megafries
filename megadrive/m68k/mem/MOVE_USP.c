#include "MOVE_USP.h"


void mnemo_MOVE_USP( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	if( SR_CHECK_S(M68k_Context_p->Status_Register) == 0)
	{
		M68k_Context_p->Interruptions |= INT_PRIVILEGE;
		return;
	}

	if( (M68k_Context_p->Current_Opcode & 0x0008) == 0)
		M68k_Context_p->Saved_Stack_Pointer = M68k_Context_p->A[M68k_Context_p->Current_Opcode & 0x0007];
	else
		M68k_Context_p->A[M68k_Context_p->Current_Opcode & 0x0007] = M68k_Context_p->Saved_Stack_Pointer;

	*N_Ticks -= 4;
}
