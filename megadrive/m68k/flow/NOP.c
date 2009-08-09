#include "NOP.h"


void mnemo_NOP( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	*N_Ticks -= 4;
}
