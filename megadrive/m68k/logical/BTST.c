#include "BTST.h"


uint8_t BTST_Dyn_Valid_EA[64] = {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
									0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
									1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
									1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
									1, 1, 1, 1, 1, 0, 0, 0 };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>

uint8_t BTST_Sta_Valid_EA[64] = {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
									0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
									1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
									1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
									1, 1, 1, 1, 0, 0, 0, 0 };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>


void mnemo_BTST_Dyn( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( BTST_Dyn_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint8_t Bit_Number = M68k_Context_p->D[((M68k_Context_p->Current_Opcode & 0x0E00) >> 9)];

	if( (EA_Mode & 0x38) == 0)		// Dn
	{
		Bit_Number &= 0x1F;
		if( ((M68k_Context_p->D[EA_Mode] >> Bit_Number) & 1) == 1)
			CCR_CLEAR_Z(M68k_Context_p->Status_Register);
		else
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}
	else
	{
		uint8_t Test_Value = EA_Read_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007);
		if( M68k_Context_p->Interruptions != 0)
			return;

		Bit_Number &= 0x07;
		if( ((Test_Value >> Bit_Number) & 1) == 1)
			CCR_CLEAR_Z(M68k_Context_p->Status_Register);
		else
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	*N_Ticks -= 4;
}

void mnemo_BTST_Sta( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( BTST_Sta_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint8_t Bit_Number;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter + 1, &Bit_Number, 1);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint8_t Test_Value = EA_Read_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007);
	if( M68k_Context_p->Interruptions != 0)
		return;

	Bit_Number &= 0x07;
	if( ((Test_Value >> Bit_Number) & 1) == 1)
		CCR_CLEAR_Z(M68k_Context_p->Status_Register);
	else
		CCR_SET_Z(M68k_Context_p->Status_Register);

	*N_Ticks -= 4;
}
