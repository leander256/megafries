#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "commons.h"


void SwitchToSuperUser( struct M68k_Context* M68k_Context_p)
{
	// if we're not already in super user mode
	if( SR_CHECK_S(M68k_Context_p->Status_Register) == 0)
	{
		SR_SET_S( M68k_Context_p->Status_Register);
		uint32_t SSP = M68k_Context_p->Saved_Stack_Pointer;
		M68k_Context_p->Saved_Stack_Pointer = M68k_Context_p->A[7];
		M68k_Context_p->A[7] = SSP;
	}
	SR_CLEAR_T( M68k_Context_p->Status_Register);
}

void CreateExceptionStackAndSetPC( struct M68k_Context* M68k_Context_p, uint32_t VectorAddress)
{
	// we save the status register before eventually changing S
	uint16_t Status_Register = M68k_Context_p->Status_Register;

	// let's enable super user mode
	SwitchToSuperUser( M68k_Context_p);

	int32_t N_ticks_dummy;

	// we push PC on the system stack
	EA_Write_L_An_PreDec( M68k_Context_p, &N_ticks_dummy, 7, M68k_Context_p->Program_Counter);

	// we push SR on the system stack
	EA_Write_W_An_PreDec( M68k_Context_p, &N_ticks_dummy, 7, Status_Register);

	// now we load the interruption vector in PC
	uint32_t TempVal32;
	if( M68k_Context_p->Read_Memory( VectorAddress, &TempVal32, 4) != 0)
	{
		// That should NEVER be allowed to happen, the ROM should be at the very least 256 bytes large
		printf( "Impossible to read memory at address %#08x !!!\n", VectorAddress);
		abort();
	}
	M68k_Context_p->Program_Counter = ntohl( TempVal32);
}


EA_Compute_Function EA_Compute_Table[64] = {	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 000 xxx Dn
												NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 001 xxx An
												REPEAT8(EA_Comp_An_Ind)								// 010 xxx (An)
												NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 011 xxx (An)+
												NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 100 xxx -(An)
												REPEAT8(EA_Comp_An_Disp)							// 101 xxx (d16,An)
												REPEAT8(EA_Comp_An_Reg_Idx)							// 110 xxx (d8,An,Xn)
												&EA_Comp_Abs_S, &EA_Comp_Abs_L, &EA_Comp_PC_Disp, &EA_Comp_PC_Reg_Idx, NULL, NULL, NULL, NULL };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn)

uint32_t EA_Comp_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	return M68k_Context_p->A[Reg_Number];
}

uint32_t EA_Comp_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	uint16_t Address16;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint32_t Address32 = ntohs(Address16);
	if( (Address32 & 0x00008000) != 0)
		Address32 |= 0xFFFF0000;

	*N_Ticks -= 4;
	return M68k_Context_p->A[Reg_Number] + Address32;
}

uint32_t EA_Comp_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	uint16_t Ext_Word;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Ext_Word, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	Ext_Word = ntohs( Ext_Word);

	uint32_t Disp = Ext_Word & 0x00FF;
	if( (Disp & 0x00000080) != 0)
		Disp |= 0xFFFFFF00;

	uint32_t Index;
	if( (Ext_Word & 0x8000) != 0)
		Index = M68k_Context_p->A[ (Ext_Word & 0x7000) >> 12 ];
	else
		Index = M68k_Context_p->D[ (Ext_Word & 0x7000) >> 12 ];

	if( (Ext_Word & 0x0800) == 0)	// short
	{
		if( (Index & 0x00008000) != 0)
			Index |= 0xFFFF0000;
		else
			Index &= 0x0000FFFF;
	}

	*N_Ticks -= 6;
	return M68k_Context_p->A[Reg_Number] + Disp + Index;
}

uint32_t EA_Comp_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	uint16_t Address16;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint32_t Address32 = ntohs(Address16);
	if( (Address32 & 0x00008000) != 0)
		Address32 |= 0xFFFF0000;

	*N_Ticks -= 4;
	return Address32;
}

uint32_t EA_Comp_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	uint32_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Value, 4);
	M68k_Context_p->Program_Counter += 4;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 8;
	return ntohl(Value);
}

uint32_t EA_Comp_PC_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	uint16_t Displacement16 = 0;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Displacement16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint32_t Value = ntohs( Displacement16);
	if( (Value & 0x8000) != 0)
		Value |= 0xFFFF0000;

	*N_Ticks -= 4;
	return M68k_Context_p->Program_Counter - 2 + Value;			// relative to PC right after the first opcode, not after reading extensions
}

uint32_t EA_Comp_PC_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	uint16_t Ext_Word;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Ext_Word, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	Ext_Word = ntohs( Ext_Word);

	uint32_t Disp = Ext_Word & 0x00FF;
	if( (Disp & 0x00000080) != 0)
		Disp |= 0xFFFFFF00;

	uint32_t Index;
	if( (Ext_Word & 0x8000) != 0)
		Index = M68k_Context_p->A[ (Ext_Word & 0x7000) >> 12 ];
	else
		Index = M68k_Context_p->D[ (Ext_Word & 0x7000) >> 12 ];

	if( (Ext_Word & 0x0800) == 0)	// short
	{
		if( (Index & 0x00008000) != 0)
			Index |= 0xFFFF0000;
		else
			Index &= 0x0000FFFF;
	}

	*N_Ticks -= 6;
	return M68k_Context_p->Program_Counter - 2 + Disp + Index;
}


EA_Read_B_Function EA_Read_B_Table[64] = {	REPEAT8(EA_Read_B_Dn)				// 000 xxx Dn
											REPEAT8(EA_Read_B_An)				// 001 xxx An
											REPEAT8(EA_Read_B_An_Ind)			// 010 xxx (An)
											REPEAT8(EA_Read_B_An_PostInc)		// 011 xxx (An)+
											REPEAT8(EA_Read_B_An_PreDec)		// 100 xxx -(An)
											REPEAT8(EA_Read_B_An_Disp)	 		// 101 xxx (d16,An)
											REPEAT8(EA_Read_B_An_Reg_Idx)		// 110 xxx (d8,An,Xn)
											&EA_Read_B_Abs_S, &EA_Read_B_Abs_L, &EA_Read_B_PC_Disp, &EA_Read_B_An_Reg_Idx, &EA_Read_B_Imm, NULL, NULL, NULL	};		// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>

uint8_t EA_Read_B_Dn( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	return (uint8_t) M68k_Context_p->D[Reg_Number];
}

uint8_t EA_Read_B_An( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	return (uint8_t) M68k_Context_p->A[Reg_Number];
}

uint8_t EA_Read_B_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint8_t Value;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number], &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 4;
	return Value;
}

uint8_t EA_Read_B_An_PreDec( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	M68k_Context_p->A[Reg_Number]--;

	uint8_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number], &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 4;
	return Value;
}

uint8_t EA_Read_B_An_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint8_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number], &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	M68k_Context_p->A[Reg_Number]++;
	*N_Ticks -= 4;
	return Value;
}

uint8_t EA_Read_B_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint16_t Displacement16;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Displacement16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint32_t Displacement = ntohs( Displacement16);
	if( (Displacement & 0x00008000) != 0)
		Displacement |= 0xFFFF0000;

	uint8_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number] + Displacement, &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 8;
	return Value;
}

uint8_t EA_Read_B_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint16_t Ext_Word;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Ext_Word, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;
	Ext_Word = ntohs(Ext_Word);

	uint32_t Displacement = Ext_Word & 0x00FF;
	if( (Displacement & 0x00000080) != 0)
		Displacement |= 0xFFFFFF00;

	uint32_t Index;
	if( Ext_Word & 0x8000)
		Index = M68k_Context_p->A[(Ext_Word >> 12) & 7];
	else
		Index = M68k_Context_p->D[(Ext_Word >> 12) & 7];
	if( (Ext_Word & 0x0800) == 0)
	{
		if( Index & 0x8000)
			Index |= 0xFFFF0000;
		else
			Index &= 0x0000FFFF;
	}

	uint8_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number] + Index + Displacement, &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 10;
	return Value;
}

uint8_t EA_Read_B_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint16_t Address16;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint32_t Address = ntohs( Address16);
	if( (Address & 0x00008000) != 0)
		Address |= 0xFFFF0000;

	uint8_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( Address, &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 8;
	return Value;
}

uint8_t EA_Read_B_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint32_t Address;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address, 4);
	M68k_Context_p->Program_Counter += 4;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint8_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( ntohl(Address), &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 12;
	return Value;
}

uint8_t EA_Read_B_PC_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint16_t Displacement16;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Displacement16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint32_t Displacement = ntohs( Displacement16);
	if( (Displacement & 0x00008000) != 0)
		Displacement |= 0xFFFF0000;

	uint8_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter - 2 + Displacement, &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 8;
	return Value;
}

uint8_t EA_Read_B_PC_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint32_t Address = EA_Comp_PC_Reg_Idx( M68k_Context_p, N_Ticks, Reg_Number);

	uint8_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( Address, &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 4;
	return Value;
}

uint8_t EA_Read_B_Imm( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint8_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter + 1, &Value, 1);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 4;
	return Value;
}


EA_Write_B_Function EA_Write_B_Table[64] = {	REPEAT8(EA_Write_B_Dn)									// Dn
												NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,			// 001 xxx An
												REPEAT8(EA_Write_B_An_Ind)								// 010 xxx (An)
												REPEAT8(EA_Write_B_An_PostInc)							// 011 xxx (An)+
												REPEAT8(EA_Write_B_An_PreDec)							// 100 xxx -(An)
												REPEAT8(EA_Write_B_An_Disp)								// 101 xxx (d16,An)
												REPEAT8(EA_Write_B_An_Reg_Idx)							// 110 xxx (d8,An,Xn)
												&EA_Write_B_Abs_S, &EA_Write_B_Abs_L, NULL, NULL, NULL, NULL, NULL, NULL	};		// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>

void EA_Write_B_Dn( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	M68k_Context_p->D[Reg_Number] &= 0xFFFFFF00;
	M68k_Context_p->D[Reg_Number] |= Value;
}

void EA_Write_B_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_Number], &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 4;
}

void EA_Write_B_An_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_Number], &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return;

	M68k_Context_p->A[Reg_Number]++;

	*N_Ticks -= 4;
}

void EA_Write_B_An_PreDec( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	M68k_Context_p->A[Reg_Number]--;

	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_Number], &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 4;
}

void EA_Write_B_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint16_t Disp16;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Disp16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Disp32 = ntohs( Disp16);
	if( (Disp32 & 0x00008000) != 0)
		Disp32 |= 0xFFFF0000;

	M68k_Context_p->Interruptions = M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_Number] + Disp32, &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 8;
}

void EA_Write_B_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint32_t Address = EA_Comp_An_Reg_Idx( M68k_Context_p, N_Ticks, Reg_Number);
	M68k_Context_p->Interruptions = M68k_Context_p->Write_Memory( Address, &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 4;
}

void EA_Write_B_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint16_t Address16;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Address = ntohs( Address16);
	if( (Address & 0x00008000) != 0)
		Address |= 0xFFFF0000;

	M68k_Context_p->Interruptions = M68k_Context_p->Write_Memory( Address, &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 8;
}

void EA_Write_B_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint32_t Address;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address, 4);
	M68k_Context_p->Program_Counter += 4;
	if( M68k_Context_p->Interruptions != 0)
		return;

	M68k_Context_p->Interruptions = M68k_Context_p->Write_Memory( ntohl(Address), &Value, 1);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 12;
}


EA_Read_W_Function EA_Read_W_Table[64] = {	REPEAT8(EA_Read_W_Dn)						// 000 xxx Dn
											REPEAT8(EA_Read_W_An)						// 001 xxx An
											REPEAT8(EA_Read_W_An_Ind)					// 010 xxx (An)
											REPEAT8(EA_Read_W_An_PostInc)				// 011 xxx (An)+
											REPEAT8(EA_Read_W_An_PreDec)				// 100 xxx -(An)
											REPEAT8(EA_Read_W_An_Disp)					// 101 xxx (d16,An)
											REPEAT8(EA_Read_W_An_Reg_Idx)				// 110 xxx (d8,An,Xn)
											&EA_Read_W_Abs_S, &EA_Read_W_Abs_L, &EA_Read_W_PC_Disp, &EA_Read_W_PC_Reg_Idx, &EA_Read_W_Imm, NULL, NULL, NULL	};		// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>

uint16_t EA_Read_W_Dn( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	return (uint16_t) M68k_Context_p->D[Reg_Number];
}

uint16_t EA_Read_W_An( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	return (uint16_t) M68k_Context_p->A[Reg_Number];
}

uint16_t EA_Read_W_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint16_t Value;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number], &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 4;
	return ntohs(Value);
}

uint16_t EA_Read_W_An_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint16_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number], &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	M68k_Context_p->A[Reg_Number] += 2;
	*N_Ticks -= 4;
	return ntohs(Value);
}

uint16_t EA_Read_W_An_PreDec( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	M68k_Context_p->A[Reg_Number] -= 2;

	uint16_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number], &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 4;
	return ntohs(Value);
}

uint16_t EA_Read_W_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint16_t Displacement16;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Displacement16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint32_t Displacement = ntohs( Displacement16);
	if( (Displacement & 0x00008000) != 0)
		Displacement |= 0xFFFF0000;

	uint16_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number] + Displacement, &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 8;
	return ntohs(Value);
}

uint16_t EA_Read_W_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint16_t Ext_Word;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Ext_Word, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;
	Ext_Word = ntohs(Ext_Word);

	uint32_t Displacement = Ext_Word & 0x00FF;
	if( (Displacement & 0x00000080) != 0)
		Displacement |= 0xFFFFFF00;

	uint32_t Index;
	if( Ext_Word & 0x8000)
		Index = M68k_Context_p->A[(Ext_Word >> 12) & 7];
	else
		Index = M68k_Context_p->D[(Ext_Word >> 12) & 7];
	if( (Ext_Word & 0x0800) == 0)
	{
		if( Index & 0x8000)
			Index |= 0xFFFF0000;
		else
			Index &= 0x0000FFFF;
	}

	uint16_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number] + Index + Displacement, &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 10;
	return ntohs(Value);
}

uint16_t EA_Read_W_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint16_t Address16;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint32_t Address = ntohs( Address16);
	if( (Address & 0x00008000) != 0)
		Address |= 0xFFFF0000;

	uint16_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( Address, &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 8;
	return ntohs(Value);
}

uint16_t EA_Read_W_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint32_t Address;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address, 4);
	M68k_Context_p->Program_Counter += 4;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint16_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( ntohl(Address), &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 12;
	return ntohs(Value);
}

uint16_t EA_Read_W_PC_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint16_t Displacement16;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Displacement16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint32_t Displacement = ntohs( Displacement16);
	if( (Displacement & 0x00008000) != 0)
		Displacement |= 0xFFFF0000;

	uint16_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter + Displacement, &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 8;
	return ntohs(Value);
}

uint16_t EA_Read_W_PC_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint16_t Ext_Word;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Ext_Word, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	Ext_Word = ntohs( Ext_Word);

	uint32_t Disp = Ext_Word & 0x00FF;
	if( (Disp & 0x00000080) != 0)
		Disp |= 0xFFFFFF00;

	uint32_t Index;
	if( (Ext_Word & 0x8000) != 0)
		Index = M68k_Context_p->A[ (Ext_Word & 0x7000) >> 12 ];
	else
		Index = M68k_Context_p->D[ (Ext_Word & 0x7000) >> 12 ];

	if( (Ext_Word & 0x0800) == 0)	// short
	{
		if( (Index & 0x00008000) != 0)
			Index |= 0xFFFF0000;
		else
			Index &= 0x0000FFFF;
	}

	uint16_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter - 2 + Disp + Index , &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 10;
	return ntohs(Value);
}

uint16_t EA_Read_W_Imm( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint16_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Value, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 4;
	return ntohs(Value);
}

EA_Write_W_Function EA_Write_W_Table[64] = {	REPEAT8(EA_Write_W_Dn)									// 000 xxx Dn
												NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,			// 001 xxx An
												REPEAT8(EA_Write_W_An_Ind)								// 010 xxx (An)
												REPEAT8(EA_Write_W_An_PostInc)							// 011 xxx (An)+
												REPEAT8(EA_Write_W_An_PreDec)							// 100 xxx -(An)
												REPEAT8(EA_Write_W_An_Disp)								// 101 xxx (d16,An)
												REPEAT8(EA_Write_W_An_Reg_Idx)							// 110 xxx (d8,An,Xn)
												&EA_Write_W_Abs_S, &EA_Write_W_Abs_L, NULL, NULL, NULL, NULL, NULL, NULL	};		// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>

void EA_Write_W_Dn( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	M68k_Context_p->D[Reg_Number] &= 0xFFFF0000;
	M68k_Context_p->D[Reg_Number] |= Value;
}

void EA_Write_W_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	Value = htons( Value);
	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_Number], &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 4;
}

void EA_Write_W_An_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	Value = htons( Value);
	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_Number], &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return;

	M68k_Context_p->A[Reg_Number] += 2;

	*N_Ticks -= 4;
}

void EA_Write_W_An_PreDec( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	M68k_Context_p->A[Reg_Number] -= 2;

	Value = htons( Value);
	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_Number], &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 6;
}

void EA_Write_W_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint16_t Disp16;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Disp16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Disp32 = ntohs( Disp16);
	if( (Disp32 & 0x00008000) != 0)
		Disp32 |= 0xFFFF0000;

	Value = htons( Value);
	M68k_Context_p->Interruptions = M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_Number] + Disp32, &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 8;
}

void EA_Write_W_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint32_t Address = EA_Comp_An_Reg_Idx( M68k_Context_p, N_Ticks, Reg_Number);
	Value = htons( Value);
	M68k_Context_p->Interruptions = M68k_Context_p->Write_Memory( Address, &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 4;
}

void EA_Write_W_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint16_t Address16;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Address = ntohs( Address16);
	if( (Address & 0x00008000) != 0)
		Address |= 0xFFFF0000;

	Value = htons( Value);
	M68k_Context_p->Interruptions = M68k_Context_p->Write_Memory( Address, &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 8;
}

void EA_Write_W_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint32_t Address;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address, 4);
	M68k_Context_p->Program_Counter += 4;
	if( M68k_Context_p->Interruptions != 0)
		return;

	Value = htons( Value);
	M68k_Context_p->Interruptions = M68k_Context_p->Write_Memory( ntohl(Address), &Value, 2);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 12;
}

EA_Read_L_Function EA_Read_L_Table[64] = {	REPEAT8(EA_Read_L_Dn)				// 000 xxx Dn
											REPEAT8(EA_Read_L_An)				// 001 xxx An
											REPEAT8(EA_Read_L_An_Ind)			// 010 xxx (An)
											REPEAT8(EA_Read_L_An_PostInc)		// 011 xxx (An)+
											REPEAT8(EA_Read_L_An_PreDec)		// 100 xxx -(An)
											REPEAT8(EA_Read_L_An_Disp)			// 101 xxx (d16,An)
											REPEAT8(EA_Read_L_An_Reg_Idx)		// 110 xxx (d8,An,Xn)
											&EA_Read_L_Abs_S, &EA_Read_L_Abs_L, &EA_Read_L_PC_Disp, &EA_Read_L_PC_Reg_Idx, &EA_Read_L_Imm, NULL, NULL, NULL	};		// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>

uint32_t EA_Read_L_Dn( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	return M68k_Context_p->D[Reg_Number];
}

uint32_t EA_Read_L_An( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	return M68k_Context_p->A[Reg_Number];
}

uint32_t EA_Read_L_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint32_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number], &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 8;
	return ntohl(Value);
}

uint32_t EA_Read_L_An_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint32_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number], &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	M68k_Context_p->A[Reg_Number] += 4;
	*N_Ticks -= 8;
	return ntohl(Value);
}

uint32_t EA_Read_L_An_PreDec( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	M68k_Context_p->A[Reg_Number] -= 4;

	uint32_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number], &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 8;
	return ntohl(Value);
}

uint32_t EA_Read_L_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint16_t Displacement16;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Displacement16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint32_t Displacement = ntohs( Displacement16);
	if( (Displacement & 0x00008000) != 0)
		Displacement |= 0xFFFF0000;

	uint32_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number] + Displacement, &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 12;
	return ntohl(Value);
}

uint32_t EA_Read_L_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint16_t Ext_Word;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Ext_Word, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;
	Ext_Word = ntohs(Ext_Word);

	uint32_t Displacement = Ext_Word & 0x00FF;
	if( (Displacement & 0x00000080) != 0)
		Displacement |= 0xFFFFFF00;

	uint32_t Index;
	if( Ext_Word & 0x8000)
		Index = M68k_Context_p->A[(Ext_Word >> 12) & 7];
	else
		Index = M68k_Context_p->D[(Ext_Word >> 12) & 7];
	if( (Ext_Word & 0x0800) == 0)
	{
		if( Index & 0x8000)
			Index |= 0xFFFF0000;
		else
			Index &= 0x0000FFFF;
	}

	uint32_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number] + Index + Displacement, &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 10;
	return ntohl(Value);
}

uint32_t EA_Read_L_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint16_t Address16;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint32_t Address32 = ntohs(Address16);
	if( (Address32 & 0x00008000) != 0)
		Address32 |= 0xFFFF0000;

	uint32_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( Address32, &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 12;
	return ntohl(Value);
}

uint32_t EA_Read_L_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint32_t Address;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address, 4);
	M68k_Context_p->Program_Counter += 4;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint32_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( ntohl(Address), &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 16;
	return ntohl(Value);
}

uint32_t EA_Read_L_PC_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint16_t Displacement16;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Displacement16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	uint32_t Displacement = ntohs( Displacement16);
	if( (Displacement & 0x00008000) != 0)
		Displacement |= 0xFFFF0000;

	uint32_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter + Displacement, &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 12;
	return ntohl(Value);
}

uint32_t EA_Read_L_PC_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint16_t Ext_Word;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Ext_Word, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	Ext_Word = ntohs( Ext_Word);

	uint32_t Disp = Ext_Word & 0x00FF;
	if( Disp & 0x80)
		Disp |= 0xFFFFFF00;

	uint32_t Index;
	if( Ext_Word & 0x8000)
		Index = M68k_Context_p->A[(Ext_Word >> 12) & 7];
	else
		Index = M68k_Context_p->D[(Ext_Word >> 12) & 7];
	if( (Ext_Word & 0x0800) == 0)
	{
		if( Index & 0x8000)
			Index |= 0xFFFF0000;
		else
			Index &= 0x0000FFFF;
	}

	uint32_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter - 2 + Disp + Index, &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 14;

	return ntohl(Value);
}

uint32_t EA_Read_L_Imm( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint32_t Value;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Value, 4);
	M68k_Context_p->Program_Counter += 4;
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	*N_Ticks -= 8;
	return ntohl(Value);
}

EA_Write_L_Function EA_Write_L_Table[64];
EA_Write_L_Function EA_Write_L_Table[64] = {	REPEAT8(EA_Write_L_Dn)									// 000 xxx Dn
												REPEAT8(EA_Write_L_An)									// 001 xxx An
												REPEAT8(EA_Write_L_An_Ind)								// 010 xxx (An)
												REPEAT8(EA_Write_L_An_PostInc)							// 011 xxx (An)+
												REPEAT8(EA_Write_L_An_PreDec)							// 100 xxx -(An)
												REPEAT8(EA_Write_L_An_Disp)								// 101 xxx (d16,An)
												REPEAT8(EA_Write_L_An_Reg_Idx)							// 110 xxx (d8,An,Xn)
												&EA_Write_L_Abs_S, &EA_Write_L_Abs_L, NULL, NULL, NULL, NULL, NULL, NULL	};		// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>

void EA_Write_L_Dn( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	M68k_Context_p->D[Reg_Number] = Value;
}

void EA_Write_L_An( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	M68k_Context_p->A[Reg_Number] = Value;
}

void EA_Write_L_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	Value = htonl( Value);
	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_Number], &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 8;
}

void EA_Write_L_An_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	Value = htonl( Value);
	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_Number], &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return;

	M68k_Context_p->A[Reg_Number] += 4;

	*N_Ticks -= 8;
}

void EA_Write_L_An_PreDec( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	M68k_Context_p->A[Reg_Number] -= 4;

	Value = htonl( Value);
	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_Number], &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 8;
}

void EA_Write_L_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint16_t Disp16;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Disp16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Disp32 = ntohs( Disp16);
	if( (Disp32 & 0x00008000) != 0)
		Disp32 |= 0xFFFF0000;

	Value = htonl( Value);
	M68k_Context_p->Interruptions = M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_Number] + Disp32, &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 12;
}

void EA_Write_L_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);
	assert( Reg_Number < 8);

	uint32_t Address = EA_Comp_An_Reg_Idx( M68k_Context_p, N_Ticks, Reg_Number);
	Value = htonl( Value);
	M68k_Context_p->Interruptions = M68k_Context_p->Write_Memory( Address, &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 8;
}

void EA_Write_L_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint16_t Address16;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Address = ntohs( Address16);
	if( (Address & 0x00008000) != 0)
		Address |= 0xFFFF0000;

	Value = htonl( Value);
	M68k_Context_p->Interruptions = M68k_Context_p->Write_Memory( Address, &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 12;
}

void EA_Write_L_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks != NULL);

	uint32_t Address;
	M68k_Context_p->Interruptions = M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address, 4);
	M68k_Context_p->Program_Counter += 4;
	if( M68k_Context_p->Interruptions != 0)
		return;

	Value = htonl( Value);
	M68k_Context_p->Interruptions = M68k_Context_p->Write_Memory( ntohl( Address), &Value, 4);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= 16;
}

uint8_t EA_Read_B_KeepIntact( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode)
{
	uint16_t Mode_Number = EA_Mode & 0x38;
	uint16_t Reg_Number = EA_Mode & 0x07;

	uint32_t SavedPC = M68k_Context_p->Program_Counter;

	uint8_t EA_Value = EA_Read_B_Table[EA_Mode]( M68k_Context_p, N_Ticks, Reg_Number);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	if( Mode_Number == 0x18)
		M68k_Context_p->A[Reg_Number]--;
	else
	{
		if( Mode_Number == 0x20)
			M68k_Context_p->A[Reg_Number]++;
	}

	M68k_Context_p->Program_Counter = SavedPC;

	return EA_Value;
}

uint16_t EA_Read_W_KeepIntact( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode)
{
	uint16_t Mode_Number = EA_Mode & 0x38;
	uint16_t Reg_Number = EA_Mode & 0x07;

	uint32_t SavedPC = M68k_Context_p->Program_Counter;

	uint16_t EA_Value = EA_Read_W_Table[EA_Mode]( M68k_Context_p, N_Ticks, Reg_Number);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	if( Mode_Number == 0x18)
		M68k_Context_p->A[Reg_Number] -= 2;
	else
	{
		if( Mode_Number == 0x20)
			M68k_Context_p->A[Reg_Number] += 2;
	}

	M68k_Context_p->Program_Counter = SavedPC;

	return EA_Value;
}

uint32_t EA_Read_L_KeepIntact( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode)
{
	uint16_t Mode_Number = EA_Mode & 0x38;
	uint16_t Reg_Number = EA_Mode & 0x07;

	uint32_t SavedPC = M68k_Context_p->Program_Counter;

	uint32_t EA_Value = EA_Read_L_Table[EA_Mode]( M68k_Context_p, N_Ticks, Reg_Number);
	if( M68k_Context_p->Interruptions != 0)
		return 0;

	if( Mode_Number == 0x18)
		M68k_Context_p->A[Reg_Number] -= 4;
	else
	{
		if( Mode_Number == 0x20)
			M68k_Context_p->A[Reg_Number] += 4;
	}

	M68k_Context_p->Program_Counter = SavedPC;

	return EA_Value;
}

///////////////////////////////////////////////////////////////////////////////
// Conditions testing

uint8_t CC_T( uint16_t Condition_Code_Register)
{
	return 1;
}

uint8_t CC_F( uint16_t Condition_Code_Register)
{
	return 0;
}

uint8_t CC_HI( uint16_t Condition_Code_Register)
{
	if( CCR_CHECK_OR_ZC(Condition_Code_Register))
		return 0;
	else
		return 1;
}

uint8_t CC_LS( uint16_t Condition_Code_Register)
{
	if( CCR_CHECK_OR_ZC(Condition_Code_Register))
		return 1;
	else
		return 0;
}

uint8_t CC_CC( uint16_t Condition_Code_Register)
{
	if( CCR_CHECK_C(Condition_Code_Register))
		return 0;
	else
		return 1;
}


uint8_t CC_CS( uint16_t Condition_Code_Register)
{
	if( CCR_CHECK_C(Condition_Code_Register))
		return 1;
	else
		return 0;
}

uint8_t CC_EQ( uint16_t Condition_Code_Register)
{
	if( CCR_CHECK_Z(Condition_Code_Register))
		return 1;
	else
		return 0;
}

uint8_t CC_NE( uint16_t Condition_Code_Register)
{
	if( CCR_CHECK_Z(Condition_Code_Register))
		return 0;
	else
		return 1;
}

uint8_t CC_PL( uint16_t Condition_Code_Register)
{
	if( CCR_CHECK_N(Condition_Code_Register))
		return 0;
	else
		return 1;
}

uint8_t CC_MI( uint16_t Condition_Code_Register)
{
	if( CCR_CHECK_N(Condition_Code_Register))
		return 1;
	else
		return 0;
}

uint8_t CC_GE( uint16_t Condition_Code_Register)
{
	if( CCR_CHECK_N(Condition_Code_Register) == CCR_CHECK_V(Condition_Code_Register))
		return 1;
	else
		return 0;
}

uint8_t CC_LT( uint16_t Condition_Code_Register)
{
	if( CCR_CHECK_N(Condition_Code_Register) !=  CCR_CHECK_V(Condition_Code_Register))
		return 1;
	else
		return 0;
}

uint8_t CC_GT( uint16_t Condition_Code_Register)
{
	if( (Condition_Code_Register & 0x000E) == 0x000A || (Condition_Code_Register & 0x000E) == 0x0000)
		return 1;
	else
		return 0;
}

uint8_t CC_LE( uint16_t Condition_Code_Register)
{
	if( CCR_CHECK_Z(Condition_Code_Register) || (CCR_CHECK_N(Condition_Code_Register) != CCR_CHECK_V(Condition_Code_Register)) )
		return 1;
	else
		return 0;
}

uint8_t (*CC_Function[16]) ( uint16_t) = {	&CC_T,	&CC_F,	&CC_HI,	&CC_LS,
											&CC_CC,	&CC_CS,	&CC_NE,	&CC_EQ,
											NULL,	NULL,	&CC_PL,	&CC_MI,
											&CC_GE,	&CC_LT,	&CC_GT,	&CC_LE };
