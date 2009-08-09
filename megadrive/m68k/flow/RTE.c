#include "RTE.h"


void mnemo_RTE( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	if( SR_CHECK_S(M68k_Context_p->Status_Register) == 0)
	{
		M68k_Context_p->Interruptions |= INT_PRIVILEGE;
		return;
	}

	M68k_Context_p->Status_Register = EA_Read_W_An_PostInc( M68k_Context_p, N_Ticks, 7);
	if( M68k_Context_p->Interruptions != 0)
		return;

	M68k_Context_p->Program_Counter = EA_Read_L_An_PostInc( M68k_Context_p, N_Ticks, 7);
	if( M68k_Context_p->Interruptions != 0)
		return;

	if( SR_CHECK_S(M68k_Context_p->Status_Register) == 0)
	{
		uint32_t USP = M68k_Context_p->Saved_Stack_Pointer;
		M68k_Context_p->Saved_Stack_Pointer = M68k_Context_p->A[7];
		M68k_Context_p->A[7] = USP;
	}

	*N_Ticks -= 8;
}
