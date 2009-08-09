#include "ORItoSR.h"


void mnemo_ORItoSR( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	if( SR_CHECK_S(M68k_Context_p->Status_Register) == 0)
	{
		M68k_Context_p->Interruptions |= INT_PRIVILEGE;
		return;
	}

	uint16_t Immediate_Value;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Immediate_Value, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	Immediate_Value = ntohs( Immediate_Value);
	Immediate_Value &= 0xA71F;		// we keep only the relevant bits
	M68k_Context_p->Status_Register |= Immediate_Value;

	*N_Ticks -= 20;
}
