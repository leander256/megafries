#include "ORI.h"
#include "ORItoSR.h"


uint8_t ORI_Valid_EA[64] = {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
								0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
								1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
								1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
								1, 1, 0, 0, 2, 0, 0, 0 };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>
// 2 only for W


void mnemo_ORI_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( ORI_Valid_EA[EA_Mode] != 1)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint8_t Immediate_Value;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter + 1, &Immediate_Value, 1);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint8_t Destination_Value = EA_Read_B_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	Destination_Value |= Immediate_Value;

	EA_Write_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Destination_Value);
	if( M68k_Context_p->Interruptions != 0)
		return;

	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);
	if( (Destination_Value & 0x80) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Destination_Value == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	*N_Ticks -= 8;
}

void mnemo_ORI_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	switch( ORI_Valid_EA[EA_Mode])
	{
		case 0: M68k_Context_p->Interruptions |= INT_ILLEGAL;
				return;
		case 2: mnemo_ORItoSR( M68k_Context_p, N_Ticks);
				return;
	}

	uint16_t Immediate_Value;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Immediate_Value, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint16_t Destination_Value = EA_Read_W_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	Destination_Value |= ntohs(Immediate_Value);

	EA_Write_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Destination_Value);
	if( M68k_Context_p->Interruptions != 0)
		return;

	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);
	if( (Destination_Value & 0x8000) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Destination_Value == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	*N_Ticks -= 8;
}

void mnemo_ORI_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	if( ORI_Valid_EA[EA_Mode] != 1)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint32_t Immediate_Value;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Immediate_Value, 4);
	M68k_Context_p->Program_Counter += 4;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Destination_Value = EA_Read_L_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
	if( M68k_Context_p->Interruptions != 0)
		return;

	Destination_Value |= ntohl(Immediate_Value);

	EA_Write_L_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Destination_Value);
	if( M68k_Context_p->Interruptions != 0)
		return;

	CCR_CLEAR_NZVC(M68k_Context_p->Status_Register);
	if( (Destination_Value & 0x80000000) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Destination_Value == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}

	*N_Ticks -= 12;
}
