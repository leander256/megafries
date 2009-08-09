#include "SUB.h"
#include "SUBX.h"


uint8_t SUB_Valid_EA[2][64] = { {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
									1, 1, 1, 1, 1, 1, 1, 1,		// 001 xxx An
									1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
									1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
									1, 1, 1, 1, 1, 0, 0, 0 },	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>
								{	2, 2, 2, 2, 2, 2, 2, 2,		// 000 xxx Dn
									2, 2, 2, 2, 2, 2, 2, 2,		// 001 xxx An
									1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
									1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
									1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
									1, 1, 0, 0, 0, 0, 0, 0 } };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>


void mnemo_SUB_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	uint8_t DestVal = (M68k_Context_p->Current_Opcode >> 8) & 1;
	switch( SUB_Valid_EA[DestVal][EA_Mode])
	{
		case 0:	M68k_Context_p->Interruptions |= INT_ILLEGAL;
				return;
		case 2: mnemo_SUBX_B( M68k_Context_p, N_Ticks);
				return;
	}

	uint16_t RegDn = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	uint8_t Reg_Value = M68k_Context_p->D[RegDn];

	uint8_t EA_Value;
	uint8_t Source;
	uint8_t Destination;
	if( DestVal)
	{
		EA_Value = EA_Read_B_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
		Destination = EA_Value;
		Source = Reg_Value;
	}
	else
	{
		EA_Value = EA_Read_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007);
		Destination = Reg_Value;
		Source = EA_Value;
		EA_Mode = RegDn;	// if we write into Dn then we use it as EA mode for the writing operation
	}
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint8_t Result = Destination - Source;

	EA_Write_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Result);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_XNZVC(M68k_Context_p->Status_Register);
	if( (Result & 0x80) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Result == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}
	Destination &= 0x80;
	Source &= 0x80;
	Result &= 0x80;

	if( (!Source && Destination && !Result) || (Source && !Destination && Result))
		CCR_SET_V(M68k_Context_p->Status_Register);
	if( (Source && !Destination) || (Result && !Destination) || (Source && Result))
		CCR_SET_XC(M68k_Context_p->Status_Register);

	*N_Ticks -= 4;
}

void mnemo_SUB_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	uint8_t DestVal = (M68k_Context_p->Current_Opcode >> 8) & 1;
	switch( SUB_Valid_EA[DestVal][EA_Mode])
	{
		case 0:	M68k_Context_p->Interruptions |= INT_ILLEGAL;
				return;
		case 2: mnemo_SUBX_W( M68k_Context_p, N_Ticks);
				return;
	}

	uint16_t RegDn = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	uint16_t Reg_Value = M68k_Context_p->D[RegDn];

	uint16_t EA_Value;
	uint16_t Source;
	uint16_t Destination;
	if( DestVal)
	{
		EA_Value = EA_Read_W_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
		Destination = EA_Value;
		Source = Reg_Value;
	}
	else
	{
		EA_Value = EA_Read_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007);
		Destination = Reg_Value;
		Source = EA_Value;
		EA_Mode = RegDn;	// if we write into Dn then we use it as EA mode for the writing operation
	}
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint16_t Result = Destination - Source;

	EA_Write_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Result);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_XNZVC(M68k_Context_p->Status_Register);
	if( (Result & 0x8000) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Result == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}
	Destination &= 0x8000;
	Source &= 0x8000;
	Result &= 0x8000;

	if( (!Source && Destination && !Result) || (Source && !Destination && Result))
		CCR_SET_V(M68k_Context_p->Status_Register);
	if( (Source && !Destination) || (Result && !Destination) || (Source && Result))
		CCR_SET_XC(M68k_Context_p->Status_Register);

	*N_Ticks -= 4;
}

void mnemo_SUB_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	uint8_t DestVal = (M68k_Context_p->Current_Opcode >> 8) & 1;
	switch( SUB_Valid_EA[DestVal][EA_Mode])
	{
		case 0:	M68k_Context_p->Interruptions |= INT_ILLEGAL;
				return;
		case 2: mnemo_SUBX_W( M68k_Context_p, N_Ticks);
				return;
	}

	uint16_t RegDn = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	uint32_t Reg_Value = M68k_Context_p->D[RegDn];

	uint32_t EA_Value;
	uint32_t Source;
	uint32_t Destination;
	if( DestVal)
	{
		EA_Value = EA_Read_L_KeepIntact( M68k_Context_p, N_Ticks, EA_Mode);
		Destination = EA_Value;
		Source = Reg_Value;
	}
	else
	{
		EA_Value = EA_Read_L_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007);
		Destination = Reg_Value;
		Source = EA_Value;
		EA_Mode = RegDn;	// if we write into Dn then we use it as EA mode for the writing operation
	}
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Result = Destination - Source;

	EA_Write_L_Table[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode & 0x0007, Result);
	if( M68k_Context_p->Interruptions != 0)
		return;

	// Now the flags
	CCR_CLEAR_XNZVC(M68k_Context_p->Status_Register);
	if( (Result & 0x80000000) != 0)
		CCR_SET_N(M68k_Context_p->Status_Register);
	else
	{
		if( Result == 0)
			CCR_SET_Z(M68k_Context_p->Status_Register);
	}
	Destination &= 0x80000000;
	Source &= 0x80000000;
	Result &= 0x80000000;

	if( (!Source && Destination && !Result) || (Source && !Destination && Result))
		CCR_SET_V(M68k_Context_p->Status_Register);
	if( (Source && !Destination) || (Result && !Destination) || (Source && Result))
		CCR_SET_XC(M68k_Context_p->Status_Register);

	*N_Ticks -= 8;
}
