#include "MULS.h"


uint8_t MULS_Valid_EA[64] = {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
								0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
								1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
								1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
								1, 1, 1, 1, 1, 0, 0, 0 };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>


void mnemo_MULS( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( MULS_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint16_t Reg_Number = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	int32_t Destination = EA_Read_W_Dn( M68k_Context_p, N_Ticks, Reg_Number);

	int32_t Source = EA_Read_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7);
	if( M68k_Context_p->Interruptions != 0)
		return;

	Destination *= Source;

	M68k_Context_p->D[Reg_Number] = Destination;

	CCR_CLEAR_NZVC(M68k_Context_p->Current_Opcode);
	if( Destination < 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
		if( Destination == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);

	int32_t Precise_Ticks = 0;
	Source <<= 1;
	for( int i = 0 ; i < 16; i++)
	{
		if( (Source & 3) == 1 || (Source & 3) == 2)
			Precise_Ticks += 2;
		Source >>= 1;
	}

	*N_Ticks -= 38 + Precise_Ticks;
}
