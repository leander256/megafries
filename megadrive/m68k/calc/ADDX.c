#include "ADDX.h"

#include "../commons.h"


void mnemo_ADDX_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode_Only = (M68k_Context_p->Current_Opcode & 0x0008) << 2;
	uint16_t EA_Source_Reg = M68k_Context_p->Current_Opcode & 7;
	uint16_t EA_Dest_Reg = (M68k_Context_p->Current_Opcode >> 9) & 7;

	uint8_t Source = EA_Read_B_Table[EA_Mode_Only | EA_Source_Reg]( M68k_Context_p, N_Ticks, EA_Source_Reg);

	uint8_t Destination;
	if( EA_Mode_Only)
		Destination = EA_Read_B_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode_Only | EA_Dest_Reg);
	else
		Destination = EA_Read_B_Table[EA_Mode_Only | EA_Dest_Reg]( M68k_Context_p, N_Ticks, EA_Dest_Reg);
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint8_t Result = Destination + Source;
	if( CCR_CHECK_X(M68k_Context_p->Status_Register))
		Result++;

	EA_Write_B_Table[EA_Mode_Only | EA_Dest_Reg]( M68k_Context_p, N_Ticks, EA_Dest_Reg, Result);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_XNVC(M68k_Context_p->Status_Register);
	if( (Result & 0x80) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	if( Result != 0)
		CCR_CLEAR_Z(M68k_Context_p->Status_Register);

	Source &= 0x80;
	Destination &= 0x80;
	Result &= 0x80;

	if( (Source && Destination && !Result) || (!Source && !Destination && Result))
		CCR_SET_V(M68k_Context_p->Status_Register);
	if( (Source && Destination) || (Source && !Result) || (Destination && !Result))
		CCR_SET_XC(M68k_Context_p->Status_Register);

	*N_Ticks -= 4;
}

void mnemo_ADDX_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	// TODO
	M68k_Context_p->Interruptions |= INT_ILLEGAL;
}

void mnemo_ADDX_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	// TODO
	M68k_Context_p->Interruptions |= INT_ILLEGAL;
}
