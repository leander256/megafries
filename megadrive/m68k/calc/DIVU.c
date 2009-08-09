#include "DIVU.h"


uint8_t DIVU_Valid_EA[64] = {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
								0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
								1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
								1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
								1, 1, 1, 1, 1, 0, 0, 0 };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>


void mnemo_DIVU( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( DIVU_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint16_t Reg_Number = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	uint32_t Destination = M68k_Context_p->D[Reg_Number];

	uint32_t Source = EA_Read_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7);
	if( M68k_Context_p->Interruptions != 0)
		return;

	CCR_CLEAR_NZVC(M68k_Context_p->Current_Opcode);

	if( Source == 0)
	{
		M68k_Context_p->Interruptions |= INT_ZERO_DIVIDE;
		return;
	}

	uint32_t Quotient = Destination / Source;
	if( Quotient > 65535)
	{
		CCR_SET_V(M68k_Context_p->Status_Register);
		*N_Ticks -= (140 * 65536 / Quotient);
		return;
	}

	if( Quotient & 0x8000)								// ultimately, quotient is 16 bits wide
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
		if( Quotient == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);

	uint32_t Remainder = Destination % Source;

	Remainder <<= 16;
	Quotient &= 0x0000FFFF;
	Quotient |= Remainder;

	M68k_Context_p->D[Reg_Number] = Quotient;
	*N_Ticks -= 140;
}
