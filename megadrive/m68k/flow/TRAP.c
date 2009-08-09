#include "TRAP.h"


void mnemo_TRAP( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	CreateExceptionStackAndSetPC( M68k_Context_p, ((M68k_Context_p->Current_Opcode & 0xF) + 32) << 2);

	*N_Ticks -= 38;
}
