#include "Scc.h"
#include "../flow/DBcc.h"


uint8_t Scc_Valid_EA[64] = {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
								2, 2, 2, 2, 2, 2, 2, 2,		// 001 xxx An
								1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
								1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
								1, 1, 0, 0, 0, 0, 0, 0 };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>


void mnemo_Scc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	switch( Scc_Valid_EA[EA_Mode])
	{
		case 0:	M68k_Context_p->Interruptions |= INT_ILLEGAL;
				return;
		case 2: mnemo_DBcc( M68k_Context_p, N_Ticks);
				return;
	}

	if( EA_Mode > 7)
		EA_Read_B_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);	// we have to read the value first

	int32_t TruthCosts = 0;
	if( CC_Function[(M68k_Context_p->Current_Opcode & 0x0F00) >> 8](M68k_Context_p->Status_Register))
	{
		EA_Write_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7, 0xFF);
		TruthCosts = 2;
	}
	else
		EA_Write_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7, 0);

	if( EA_Mode > 7)
		*N_Ticks -= 4 + TruthCosts;
	else
		*N_Ticks -= 8;
}
