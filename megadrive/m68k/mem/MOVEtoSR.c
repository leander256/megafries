#include "MOVEtoSR.h"


uint8_t MOVEtoSR_Valid_EA[64] = {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
									0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
									1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
									1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
									1, 1, 1, 1, 1, 0, 0, 0 };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>


void mnemo_MOVEtoSR( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	if( SR_CHECK_S(M68k_Context_p->Status_Register) == 0)
	{
		M68k_Context_p->Interruptions |= INT_PRIVILEGE;
		return;
	}

	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( MOVEtoSR_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}	

	uint16_t Value = EA_Read_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007);
	if( M68k_Context_p->Interruptions != 0)
		return;

	M68k_Context_p->Status_Register = Value;

	if( SR_CHECK_S(M68k_Context_p->Status_Register) == 0)		// we switch back to user mode
	{
		uint32_t TempStack = M68k_Context_p->A[7];
		M68k_Context_p->A[7] = M68k_Context_p->Saved_Stack_Pointer;
		M68k_Context_p->Saved_Stack_Pointer = TempStack;
	}

	*N_Ticks -= 12;	
}
