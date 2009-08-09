#include "MOVE.h"


void mnemo_MOVE_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t Source_EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( Source_EA_Mode > 0x3C)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint8_t Source_Operand = EA_Read_B_Table[Source_EA_Mode]( M68k_Context_p, N_Ticks, Source_EA_Mode & 0x0007);
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint16_t Destination_EA_Mode = ((M68k_Context_p->Current_Opcode & 0x0E00) >> 9) + ((M68k_Context_p->Current_Opcode & 0x01C0) >> 3);

	EA_Write_B_Table[Destination_EA_Mode]( M68k_Context_p, N_Ticks, Destination_EA_Mode & 0x0007, Source_Operand);
	if( M68k_Context_p->Interruptions != 0)
		return;

 	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);
 	if( (Source_Operand & 0x80) != 0)
 		CCR_SET_N(M68k_Context_p->Status_Register);
 	else
 	{
 		if( Source_Operand == 0)
 			CCR_SET_Z(M68k_Context_p->Status_Register);
 	}

 	*N_Ticks -= 4;
}

void mnemo_MOVE_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t Source_EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( Source_EA_Mode > 0x3C)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint16_t Source_Operand = EA_Read_W_Table[Source_EA_Mode]( M68k_Context_p, N_Ticks, Source_EA_Mode & 0x0007);
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint16_t Destination_EA_Mode = ((M68k_Context_p->Current_Opcode & 0x0E00) >> 9) + ((M68k_Context_p->Current_Opcode & 0x01C0) >> 3);

	EA_Write_W_Table[Destination_EA_Mode]( M68k_Context_p, N_Ticks, Destination_EA_Mode & 0x0007, Source_Operand);
	if( M68k_Context_p->Interruptions != 0)
		return;

 	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);
 	if( (Source_Operand & 0x8000) != 0)
 		CCR_SET_N(M68k_Context_p->Status_Register);
 	else
 	{
 		if( Source_Operand == 0)
 			CCR_SET_Z(M68k_Context_p->Status_Register);
 	}

 	*N_Ticks -= 4;
}

void mnemo_MOVE_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
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

	uint16_t Destination_EA_Mode = ((M68k_Context_p->Current_Opcode & 0x0E00) >> 9) + ((M68k_Context_p->Current_Opcode & 0x01C0) >> 3);

	EA_Write_L_Table[Destination_EA_Mode]( M68k_Context_p, N_Ticks, Destination_EA_Mode & 0x0007, Source_Operand);
	if( M68k_Context_p->Interruptions != 0)
		return;

 	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);
 	if( (Source_Operand & 0x80000000) != 0)
 		CCR_SET_N(M68k_Context_p->Status_Register);
 	else
 	{
 		if( Source_Operand == 0)
 			CCR_SET_Z(M68k_Context_p->Status_Register);
 	}

 	*N_Ticks -= 4;
}
