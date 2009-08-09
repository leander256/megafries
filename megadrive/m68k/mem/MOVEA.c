#include "MOVEA.h"


void mnemo_MOVEA_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t Source_EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( Source_EA_Mode > 0x3C)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint16_t Source_Operand16 = EA_Read_W_Table[Source_EA_Mode]( M68k_Context_p, N_Ticks, Source_EA_Mode & 0x0007);
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Source_Operand = Source_Operand16;
	if( (Source_Operand & 0x00008000) != 0)
		Source_Operand |= 0xFFFF0000;

	EA_Write_L_Table[ 0x08 ]( M68k_Context_p, N_Ticks, (M68k_Context_p->Current_Opcode & 0x0E00) >> 9, Source_Operand);
	if( M68k_Context_p->Interruptions != 0)
		return;

 	*N_Ticks -= 4;
}

void mnemo_MOVEA_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t Source_EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( Source_EA_Mode > 0x3C)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint32_t Source_Operand = EA_Read_L_Table[Source_EA_Mode]( M68k_Context_p, N_Ticks, Source_EA_Mode & 0x0007);
	if( M68k_Context_p->Interruptions != 0)
		return;

	EA_Write_L_Table[ 0x08 ]( M68k_Context_p, N_Ticks, (M68k_Context_p->Current_Opcode & 0x0E00) >> 9, Source_Operand);
	if( M68k_Context_p->Interruptions != 0)
		return;

 	*N_Ticks -= 4;
}
