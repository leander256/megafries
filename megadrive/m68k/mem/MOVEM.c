#include <assert.h>

#include "MOVEM.h"
#include "../calc/EXT.h"
#include "../iet.h"


// Time to fetch 2 WORDs
#define TIME_BASE 8

const uint8_t Valid_EA_MOVEM[2][64] = {	{	2, 2, 2, 2, 2, 2, 2, 2,		// 000 xxx Dn
											2, 2, 2, 2, 2, 2, 2, 2,		// 001 xxx An
											1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
											0, 0, 0, 0, 0, 0, 0, 0,		// 011 xxx (An)+
											1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
											1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
											1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
											1, 1, 0, 0, 0, 0, 0, 0 },	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn)

										{	0, 0, 0, 0, 0, 0, 0, 0,		// 000 xxx Dn
											0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
											1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
											1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
											0, 0, 0, 0, 0, 0, 0, 0,		// 100 xxx -(An)
											1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
											1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
											1, 1, 1, 1, 0, 0, 0, 0 } };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn)


typedef void (*MOVEM_EA_to_mem_Method) (struct M68k_Context*,int32_t*,uint16_t, uint8_t,void*);

void MOVEM_EA_to_mem_W_An_PreDec(struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode, uint8_t Number_Registers, void* Data);
void MOVEM_EA_to_mem_W_AbsShort(struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode, uint8_t Number_Registers, void* Data);

MOVEM_EA_to_mem_Method MOVEM_EA_to_mem_W[64] = {	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 000 xxx Dn
													NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 001 xxx An
													NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 010 xxx (An)
													NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 011 xxx (An)+
													REPEAT8(MOVEM_EA_to_mem_W_An_PreDec)				// 100 xxx -(An)
													NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 101 xxx (d16,An)
													NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 110 xxx (d8,An,Xn)
													&MOVEM_EA_to_mem_W_AbsShort, NULL, NULL, NULL, NULL, NULL, NULL, NULL };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn)

void MOVEM_EA_to_mem_L_An_Ind(struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode, uint8_t Number_Registers, void* Data);
void MOVEM_EA_to_mem_L_An_PreDec(struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode, uint8_t Number_Registers, void* Data);
void MOVEM_EA_to_mem_L_AbsShort(struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode, uint8_t Number_Registers, void* Data);

MOVEM_EA_to_mem_Method MOVEM_EA_to_mem_L[64] = {	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 000 xxx Dn
													NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 001 xxx An
													REPEAT8(MOVEM_EA_to_mem_L_An_Ind)					// 010 xxx (An)
													NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 011 xxx (An)+
													REPEAT8(MOVEM_EA_to_mem_L_An_PreDec)				// 100 xxx -(An)
													NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 101 xxx (d16,An)
													NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 110 xxx (d8,An,Xn)
													&MOVEM_EA_to_mem_L_AbsShort, NULL, NULL, NULL, NULL, NULL, NULL, NULL };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn)


typedef void (*MOVEM_Get_EA_Method) (struct M68k_Context*,int32_t*,void*,uint16_t,uint16_t,uint8_t);

void MOVEM_Get_Values_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, void* Data, uint16_t EA_Mode, uint16_t Size, uint8_t Number_Registers);
void MOVEM_Get_Values_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, void* Data, uint16_t EA_Mode, uint16_t Size, uint8_t Number_Registers);
void MOVEM_Get_Values_AbsShort( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, void* Data, uint16_t EA_Mode, uint16_t Size, uint8_t Number_Registers);

MOVEM_Get_EA_Method MOVEM_Get_EA[64] = {	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 000 xxx Dn
											NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 001 xxx An
											REPEAT8(MOVEM_Get_Values_Ind)						// 010 xxx (An)
											REPEAT8(MOVEM_Get_Values_PostInc)					// 011 xxx (An)+
											NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 100 xxx -(An)
											NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 101 xxx (d16,An)
											NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,		// 110 xxx (d8,An,Xn)
											&MOVEM_Get_Values_AbsShort, NULL, NULL, NULL, NULL, NULL, NULL, NULL };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn)


void mnemo_MOVEM_to_mem_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t Flags, uint16_t EA_Mode, uint8_t Number_Registers);
void mnemo_MOVEM_to_mem_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t Flags, uint16_t EA_Mode, uint8_t Number_Registers);
void mnemo_MOVEM_to_reg( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t Flags, uint16_t EA_Mode, uint16_t Size, uint8_t Number_Registers);

void mnemo_MOVEM_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	uint8_t Direction = (M68k_Context_p->Current_Opcode & 0x0400) >> 10;

	switch( Valid_EA_MOVEM[Direction][EA_Mode])
	{
		case 0:	M68k_Context_p->Interruptions |= INT_ILLEGAL;
				return;
		case 2: mnemo_EXT_W( M68k_Context_p, N_Ticks);
				return;
	}

	uint16_t Flags;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Flags, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;
	Flags = htons( Flags);

	uint8_t Number_Registers = 0;
	for(int i = 0; i < 16; i++)
		if( ((1 << i) & Flags) != 0)
			Number_Registers++;

	if( Number_Registers == 0)
		return;

	if( Direction == 1)
		mnemo_MOVEM_to_reg( M68k_Context_p, N_Ticks, Flags, EA_Mode, 2, Number_Registers);
	else
	{
		if( (EA_Mode & 0x0038) == 0x20)		// predecrement, reverse flags
		{
			uint16_t Tempflags = Flags;
			Flags = 0;

			for( int i = 0; i < 16; i++)
				if( ((1 << i) & Tempflags) != 0)
					Flags |= (1 << (15 - i));
		}
		mnemo_MOVEM_to_mem_W( M68k_Context_p, N_Ticks, Flags, EA_Mode, Number_Registers);
	}
}

void mnemo_MOVEM_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;
	uint8_t Direction = (M68k_Context_p->Current_Opcode & 0x0400) >> 10;

	switch( Valid_EA_MOVEM[Direction][EA_Mode])
	{
		case 0:	M68k_Context_p->Interruptions |= INT_ILLEGAL;
				return;
		case 2: mnemo_EXT_L( M68k_Context_p, N_Ticks);
				return;
	}

	uint16_t Flags;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Flags, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;
	Flags = htons( Flags);

	uint8_t Number_Registers = 0;
	for(int i = 0; i < 16; i++)
		if( ((1 << i) & Flags) != 0)
			Number_Registers++;

	if( Number_Registers == 0)
		return;

	if( Direction == 1)
		mnemo_MOVEM_to_reg( M68k_Context_p, N_Ticks, Flags, EA_Mode, 4, Number_Registers);
	else
	{
		if( (EA_Mode & 0x0038) == 0x20)		// predecrement, reverse flags
		{
			uint16_t Tempflags = Flags;
			Flags = 0;

			for( int i = 0; i < 16; i++)
				if( ((1 << i) & Tempflags) != 0)
					Flags |= (1 << (15 - i));
		}

		mnemo_MOVEM_to_mem_L( M68k_Context_p, N_Ticks, Flags, EA_Mode, Number_Registers);
	}
}

void mnemo_MOVEM_to_mem_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t Flags, uint16_t EA_Mode, uint8_t Number_Registers)
{
	assert( Number_Registers <= 16);

	uint16_t Collected_Data[16];

	int Number_Collected = 0;
	for( int i = 0; i < 8; i++, Flags >>= 1)
		if( (Flags & 1) != 0)
			Collected_Data[Number_Collected++] = htons((uint16_t) M68k_Context_p->D[i]);

	for( int i = 0; i < 8; i++, Flags >>= 1)
			if( (Flags & 1) != 0)
				Collected_Data[Number_Collected++] = htons((uint16_t) M68k_Context_p->A[i]);

	MOVEM_EA_to_mem_W[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode, Number_Registers, &Collected_Data);
}

void mnemo_MOVEM_to_mem_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t Flags, uint16_t EA_Mode, uint8_t Number_Registers)
{
	assert( Number_Registers <= 16);

	uint32_t Collected_Data[16];

	int Number_Collected = 0;
	for( int i = 0; i < 8; i++, Flags >>= 1)
		if( (Flags & 1) != 0)
			Collected_Data[Number_Collected++] = htonl(M68k_Context_p->D[i]);

	for( int i = 0; i < 8; i++, Flags >>= 1)
			if( (Flags & 1) != 0)
				Collected_Data[Number_Collected++] = htonl(M68k_Context_p->A[i]);

	MOVEM_EA_to_mem_L[EA_Mode]( M68k_Context_p, N_Ticks, EA_Mode, Number_Registers, &Collected_Data);
}

void mnemo_MOVEM_to_reg( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t Flags, uint16_t EA_Mode, uint16_t Size, uint8_t Number_Registers)
{
	assert( Size == 2 || Size == 4);
	assert( Number_Registers <= 16);

	uint32_t Collected_Data[16];

	if( Size == 2)
	{
		uint16_t Collected_Data16[16];
		MOVEM_Get_EA[EA_Mode]( M68k_Context_p, N_Ticks, Collected_Data16, EA_Mode, 2, Number_Registers);
		if( M68k_Context_p->Interruptions != 0)
			return;

		for( int i = 0; i < Number_Registers; i++)
		{
			uint32_t Temp = ntohs(Collected_Data16[i]);
			if( (Temp & 0x8000) != 0)
				Temp |= 0xFFFF0000;
			Collected_Data[i] = Temp;
		}
	}
	else
	{
		MOVEM_Get_EA[EA_Mode]( M68k_Context_p, N_Ticks, Collected_Data, EA_Mode, 4, Number_Registers);
		if( M68k_Context_p->Interruptions != 0)
			return;

		for( int i = 0; i < Number_Registers; i++)
			Collected_Data[i] = ntohl( Collected_Data[i]);
	}

	int Index_Collected_Data = 0;
	for( int i = 0; i < 8; i++, Flags >>= 1)			// D0 to D7
		if( (Flags & 1) != 0)
			M68k_Context_p->D[i] = Collected_Data[Index_Collected_Data++];

	uint32_t Saved_An = 0x0ABCDEF0;
	uint8_t An_Saved = 8;
	if( (EA_Mode & 0x0038) == 0x0018)
	{
		uint8_t An = EA_Mode & 0x0007;
		if( ((1 << An) & Flags) != 0)
		{
			An_Saved = An;
			Saved_An = M68k_Context_p->A[An];
		}
	}

	for( int i = 0; i < 8; i++, Flags >>= 1)			// A0 to A7
		if( (Flags & 1) != 0)
			M68k_Context_p->A[i] = Collected_Data[Index_Collected_Data++];

	if( (An_Saved & 0x08) == 0)
		M68k_Context_p->A[An_Saved] = Saved_An;
}


void MOVEM_EA_to_mem_W_An_PreDec(struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode, uint8_t Number_Registers, void* Data)
{
	uint16_t Reg_An = EA_Mode & 7;

	// We decrement first so we store the registers in the right order;
	M68k_Context_p->A[Reg_An] -= 2 * Number_Registers;

	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_An], Data, Number_Registers << 1);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= TIME_BASE + EACT_Word[EA_Mode] * Number_Registers;
}

void MOVEM_EA_to_mem_W_AbsShort(struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode, uint8_t Number_Registers, void* Data)
{
	uint16_t Address16;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Address = ntohs( Address16);
	if( (Address & 0x00008000) != 0)
		Address &= 0xFFFF0000;

	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( Address, Data, Number_Registers << 1);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= TIME_BASE + EACT_Word[EA_Mode] * Number_Registers;
}

void MOVEM_EA_to_mem_L_An_Ind(struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode, uint8_t Number_Registers, void* Data)
{
	uint16_t Reg_An = EA_Mode & 7;

	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_An], Data, Number_Registers << 2);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= TIME_BASE + EACT_Long[EA_Mode] * Number_Registers;
}

void MOVEM_EA_to_mem_L_An_PreDec(struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode, uint8_t Number_Registers, void* Data)
{
	uint16_t Reg_An = EA_Mode & 7;

	// We decrement first so we store the registers in the right order;
	M68k_Context_p->A[Reg_An] -= 4 * Number_Registers;

	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( M68k_Context_p->A[Reg_An], Data, Number_Registers << 2);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= TIME_BASE + EACT_Long[EA_Mode] * Number_Registers;
}

void MOVEM_EA_to_mem_L_AbsShort(struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode, uint8_t Number_Registers, void* Data)
{
	uint16_t Address16;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Address16, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Address = ntohs( Address16);
	if( (Address & 0x00008000) != 0)
		Address |= 0xFFFF0000;

	M68k_Context_p->Interruptions |= M68k_Context_p->Write_Memory( Address, Data, Number_Registers << 2);
	if( M68k_Context_p->Interruptions != 0)
		return;

	*N_Ticks -= TIME_BASE + EACT_Long[EA_Mode] * Number_Registers;
}

void MOVEM_Get_Values_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, void* Data, uint16_t EA_Mode, uint16_t Size, uint8_t Number_Registers)
{
	assert( Size == 2 || Size == 4);

	uint16_t Reg_Number = EA_Mode & 0x0007;
	uint8_t Mem_Size;
	if( Size == 2)
		Mem_Size = Number_Registers << 1;
	else
		Mem_Size = Number_Registers << 2;

	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number], Data, Mem_Size);
	if( M68k_Context_p->Interruptions != 0)
		return;

	if( Size == 2)
		*N_Ticks -= (TIME_BASE + EACT_Word[EA_Mode] * Number_Registers);
	else
		*N_Ticks -= (TIME_BASE + EACT_Long[EA_Mode] * Number_Registers);
}

void MOVEM_Get_Values_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, void* Data, uint16_t EA_Mode, uint16_t Size, uint8_t Number_Registers)
{
	assert( Size == 2 || Size == 4);

	uint16_t Reg_Number = EA_Mode & 0x0007;
	uint8_t Mem_Size;
	if( Size == 2)
		Mem_Size = Number_Registers << 1;
	else
		Mem_Size = Number_Registers << 2;

	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->A[Reg_Number], Data, Mem_Size);
	if( M68k_Context_p->Interruptions != 0)
		return;

	M68k_Context_p->A[Reg_Number] += Mem_Size;

	if( Size == 2)
		*N_Ticks -= (TIME_BASE + EACT_Word[EA_Mode] * Number_Registers);
	else
		*N_Ticks -= (TIME_BASE + EACT_Long[EA_Mode] * Number_Registers);
}

void MOVEM_Get_Values_AbsShort( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, void* Data, uint16_t EA_Mode, uint16_t Size, uint8_t Number_Registers)
{
	assert( Size == 2 || Size == 4);

	uint16_t Reg_Number = EA_Mode & 0x0007;
	uint8_t Mem_Size;
	if( Size == 2)
		Mem_Size = Number_Registers << 1;
	else
		Mem_Size = Number_Registers << 2;

	int32_t Dummy = 0;
	uint32_t Address = EA_Comp_Abs_S( M68k_Context_p, &Dummy, Reg_Number);

	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( Address, Data, Mem_Size);
	if( M68k_Context_p->Interruptions != 0)
		return;

	if( Size == 2)
		*N_Ticks -= (TIME_BASE + EACT_Word[EA_Mode] * Number_Registers);
	else
		*N_Ticks -= (TIME_BASE + EACT_Long[EA_Mode] * Number_Registers);
}
