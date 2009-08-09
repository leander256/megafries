#include "MOVEP.h"


void mnemo_MOVEP_W_to_reg( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	M68k_Context_p->Interruptions |= INT_ILLEGAL;
}

void mnemo_MOVEP_W_to_mem( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint32_t Address = M68k_Context_p->A[ M68k_Context_p->Current_Opcode & 7 ];

	uint16_t Disp16;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Disp16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Disp = ntohs( Disp16);
	if( Disp & 0x8000)
		Disp |= 0xFFFF0000;

	Address += Disp;

	uint8_t Reg_Number = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	uint16_t Transfer_Value = M68k_Context_p->D[Reg_Number];

	uint8_t OctetVal = Transfer_Value >> 8;
	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( Address, &OctetVal, 1);
	if( M68k_Context_p->Interruptions != 0)
		return;

	OctetVal = Transfer_Value;		// 16 bits to 8 bits
	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( Address + 2, &OctetVal, 1);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 16;
}

void mnemo_MOVEP_L_to_reg( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	M68k_Context_p->Interruptions |= INT_ILLEGAL;
}

void mnemo_MOVEP_L_to_mem( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	M68k_Context_p->Interruptions |= INT_ILLEGAL;
}
