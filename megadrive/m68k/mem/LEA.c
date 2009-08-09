#include "LEA.h"
#include "../iet.h"


const uint8_t LEA_Valid_EA[] = {	0, 0, 0, 0, 0, 0, 0, 0,		// 000 xxx Dn
									0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
									1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
									0, 0, 0, 0, 0, 0, 0, 0,		// 011 xxx (An)+
									0, 0, 0, 0, 0, 0, 0, 0,		// 100 xxx -(An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
									1, 1, 1, 1, 0, 0, 0, 0 };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn)


void mnemo_LEA( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;

	if( LEA_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint32_t Operand = EA_Compute_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7);
	if( M68k_Context_p->Interruptions != 0)
		return;

	M68k_Context_p->A[(M68k_Context_p->Current_Opcode & 0x0FFF) >> 9] = Operand;
}
