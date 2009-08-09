#include "BCLR.h"
#include "../mem/MOVEP.h"


uint8_t BCLR_Valid_EA[64] = {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
								2, 2, 2, 2, 2, 2, 2, 2,		// 001 xxx An
								1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
								1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
								1, 1, 0, 0, 0, 0, 0, 0 };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>


void mnemo_BCLR( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	switch( BCLR_Valid_EA[EA_Mode])
	{
		case 0: M68k_Context_p->Interruptions |= INT_ILLEGAL;
				return;
		case 2: mnemo_MOVEP_W_to_mem( M68k_Context_p, N_Ticks);
				return;
	}

	uint8_t Bit_Number;
	if( M68k_Context_p->Current_Opcode & 0x0100)
		Bit_Number = M68k_Context_p->D[((M68k_Context_p->Current_Opcode & 0x0E00) >> 9)];
	else
	{
		M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter + 1, &Bit_Number, 1);
		M68k_Context_p->Program_Counter += 2;
		if( M68k_Context_p->Interruptions != 0)
			return;
	}

	if( (EA_Mode & 0x38) == 0)		// Dn
	{
		Bit_Number &= 0x1F;
		uint32_t Mask = 1 << Bit_Number;

		if( M68k_Context_p->D[EA_Mode] & Mask)
			CCR_CLEAR_Z(M68k_Context_p->Status_Register);
		else
			CCR_SET_Z(M68k_Context_p->Status_Register);

		M68k_Context_p->D[EA_Mode] &= ~Mask;
	}
	else
	{
		uint8_t Destination = EA_Read_B_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
		if( M68k_Context_p->Interruptions != 0)
			return;

		Bit_Number &= 0x07;
		uint32_t Mask = 1 << Bit_Number;

		if( Destination & Mask)
			CCR_CLEAR_Z(M68k_Context_p->Status_Register);
		else
			CCR_SET_Z(M68k_Context_p->Status_Register);
		Destination &= ~Mask;

		EA_Write_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 7, Destination);
		if( M68k_Context_p->Interruptions != 0)
			return;
	}

	*N_Ticks -= 8;
}
