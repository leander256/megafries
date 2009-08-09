#include "CMPI.h"
#include "CMP_common.h"


uint8_t CMPI_Valid_EA[64] = {	1, 1, 1, 1, 1, 1, 1, 1,		// 000 xxx Dn
								0, 0, 0, 0, 0, 0, 0, 0,		// 001 xxx An
								1, 1, 1, 1, 1, 1, 1, 1,		// 010 xxx (An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 011 xxx (An)+
								1, 1, 1, 1, 1, 1, 1, 1,		// 100 xxx -(An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 101 xxx (d16,An)
								1, 1, 1, 1, 1, 1, 1, 1,		// 110 xxx (d8,An,Xn)
								1, 1, 1, 1, 0, 0, 0, 0 };	// 111 000 (xxx).W, 111 001 (xxx).L, 111 010 (d16,PC), 111 011 (d8,PC,Xn), 111 100 #<xxx>


void mnemo_CMPI_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;

	if( CMPI_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint8_t Source;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter + 1, &Source, 1);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint8_t Destination = EA_Read_B_Table[EA_Mode](M68k_Context_p, N_Ticks, EA_Mode & 7);
	if( M68k_Context_p->Interruptions != 0)
		return;

	M68k_Context_p->Status_Register &= 0xFFF0;
	M68k_Context_p->Status_Register |= CMP_B( Destination, Source);

	*N_Ticks -= 8;
}

void mnemo_CMPI_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;

	if( CMPI_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint16_t Source;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Source, 2);
	M68k_Context_p->Program_Counter += 2;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint16_t Destination = EA_Read_W_Table[EA_Mode](M68k_Context_p, N_Ticks, EA_Mode & 7);
	if( M68k_Context_p->Interruptions != 0)
		return;

	M68k_Context_p->Status_Register &= 0xFFF0;
	M68k_Context_p->Status_Register |= CMP_W( Destination, ntohs(Source));

	*N_Ticks -= 8;
}

void mnemo_CMPI_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t EA_Mode = M68k_Context_p->Current_Opcode & 0x003F;

	if( CMPI_Valid_EA[EA_Mode] == 0)
	{
		M68k_Context_p->Interruptions |= INT_ILLEGAL;
		return;
	}

	uint32_t Source;
	M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Source, 4);
	M68k_Context_p->Program_Counter += 4;
	if( M68k_Context_p->Interruptions != 0)
		return;

	uint32_t Destination = EA_Read_L_Table[EA_Mode](M68k_Context_p, N_Ticks, EA_Mode & 7);
	if( M68k_Context_p->Interruptions != 0)
		return;

	M68k_Context_p->Status_Register &= 0xFFF0;
	M68k_Context_p->Status_Register |= CMP_L( Destination, ntohl(Source));

	*N_Ticks -= 8;
}
