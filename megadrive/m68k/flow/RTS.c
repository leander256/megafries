#include <assert.h>

#include "RTS.h"


void mnemo_RTS( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	assert( M68k_Context_p != 0);
	assert( N_Ticks != 0);

	uint32_t NewPC;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->A[7], &NewPC, 4);
	if( M68k_Context_p->Interruptions != 0)
		return;

	M68k_Context_p->Program_Counter = ntohl( NewPC);
	M68k_Context_p->A[7] += 4;

	*N_Ticks -= 16;
}
