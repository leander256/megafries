#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "vdp.h"
#include "vdp_render.h"
#include "../mmu/bus.h"


uint32_t (*Read_VDP_Table[0x12]) ( uint32_t,void*,int16_t) = { NULL };

uint32_t Read_VDP_Data( uint32_t Address, void* Data, int16_t Size);
uint32_t Read_VDP_Control( uint32_t Address, void* Data, int16_t Size);


uint32_t (*Write_VDP_Table[0x12]) ( uint32_t,void*,uint16_t) = { NULL };

uint32_t Write_VDP_Data( uint32_t Address, void* Data, uint16_t Size);
uint32_t Write_VDP_Control( uint32_t Address, void* Data, uint16_t Size);
uint32_t Write_VDP_PSG( uint32_t Address, void* Data, uint16_t Size);


uint32_t VDP_VRAM_R( void* Data);
uint32_t VDP_CRAM_R( void* Data);
uint32_t VDP_VSRAM_R( void* Data);

uint32_t VDP_VRAM_W( uint8_t Octet1, uint8_t Octet2);
uint32_t VDP_CRAM_W( uint8_t Octet1, uint8_t Octet2);
uint32_t VDP_VSRAM_W( uint8_t Octet1, uint8_t Octet2);

uint32_t VDP_DMA_Prep();
uint32_t VDP_DMA_Fill( uint8_t Octet1, uint8_t Octet2);
uint32_t VDP_DMA_Copy();
uint32_t VDP_DMA_68k_to_VDP();


uint8_t VDP_Control_Direction[16] = {	1,				2,				0,		2,
										1,				2,				0,		0,
										1,				0,				0,		0,
										0,				0,				0,		0 };

uint32_t (*VDP_Control_Read[16]) (void*)
									= { &VDP_VRAM_R,	NULL,			NULL,	NULL,
										&VDP_VSRAM_R,	NULL,			NULL,	NULL,
										&VDP_CRAM_R,	NULL,			NULL,	NULL,
										NULL,			NULL,			NULL,	NULL };

uint32_t (*VDP_Control_Write[16]) (uint8_t, uint8_t)
									= { NULL,			&VDP_VRAM_W,	NULL,	&VDP_CRAM_W,
										NULL,			&VDP_VSRAM_W,	NULL,	NULL,
										NULL,			NULL,			NULL,	NULL,
										NULL,			NULL,			NULL,	NULL };


void Init_VDP()
{
	memset( &(Un_VDP_Context.WO_Reg[0]), 0, 24);
	Un_VDP_Context.RO_Reg[0] = 2;
	Un_VDP_Context.RO_Reg[1] = 1;

	Un_VDP_Context.CTRL_Port_Got_Word = 0;
	Un_VDP_Context.Current_Internal_Command = 0;
	Un_VDP_Context.Current_Internal_Address = 0;

	for( int i = 0; i < 4; i++)
	{
		Read_VDP_Table[i] = &Read_VDP_Data;
		Write_VDP_Table[i] = &Write_VDP_Data;
	}

	for( int i = 4; i < 8; i++)
	{
		Read_VDP_Table[i] = &Read_VDP_Control;
		Write_VDP_Table[i] = &Write_VDP_Control;
	}

	Write_VDP_Table[0x11] = &Write_VDP_PSG;

	if( Init_Renderer() != 0)
		abort();
}


uint32_t Read_VDP( uint32_t Address, void* Data, uint16_t Data_Length)
{
	Address -= 0x00C00000;
	if( Address >= 0x12)
		return INT_BUS_ERROR;

	if( Data_Length == 1)
		return Read_VDP_Table[Address](Address, Data, Data_Length);
	else
	{
		while( Data_Length > 0)
		{
			uint32_t Return_Code = Read_VDP_Table[Address](Address, Data, Data_Length);
			if( Return_Code != 0)
				return Return_Code;

			Data = (unsigned char*)Data + 2;
			Data_Length -= 2;
		}

		return 0;
	}
}

uint32_t Read_VDP_Data( uint32_t Address, void* Data, int16_t Size)
{
	if( Size == 1)
		return INT_ADDRESS_ERROR;

	Un_VDP_Context.CTRL_Port_Got_Word = 0;

	uint8_t Index = Un_VDP_Context.Current_Internal_Command & 0xF;
	if( VDP_Control_Direction[Index] != 1)
		return 0;

	return VDP_Control_Read[Index]( Data);
}

uint32_t Read_VDP_Control( uint32_t Address, void* Data, int16_t Size)
{
	memcpy( Data, Un_VDP_Context.RO_Reg + (Address & 1), Size);

	Un_VDP_Context.CTRL_Port_Got_Word = 0;

	return 0;
}

uint32_t VDP_VRAM_R( void* Data)
{
	// modulo and A0 masking on the address
	memcpy( Data, Un_VDP_Context.VRAM + (Un_VDP_Context.Current_Internal_Address & 0xFFFE), 2);
	Un_VDP_Context.Current_Internal_Address += Un_VDP_Context.WO_Reg[15];

	return 0;
}

uint32_t VDP_CRAM_R( void* Data)
{
	// modulo and A0 masking on the address
	memcpy( Data, Un_VDP_Context.CRAM + (Un_VDP_Context.Current_Internal_Address & 0x7E), 2);
	Un_VDP_Context.Current_Internal_Address += Un_VDP_Context.WO_Reg[15];

	return 0;
}

uint32_t VDP_VSRAM_R( void* Data)
{
	// modulo and A0 masking on the address, if >= 80 we just return 0
	uint16_t MaskedAddress = Un_VDP_Context.Current_Internal_Address & 0x7E;
	if( MaskedAddress < 80)
		memcpy( Data, Un_VDP_Context.VSRAM + MaskedAddress, 2);
	else
		memset( Data, 0, 2);
	Un_VDP_Context.Current_Internal_Address += Un_VDP_Context.WO_Reg[15];

	return 0;
}

uint32_t Write_VDP( uint32_t Address, void* Data, uint16_t Data_Length)
{
	Address -= 0x00C00000;
	if( Address >= 0x12)
		return INT_BUS_ERROR;

	if( Data_Length == 1)
	{
		uint16_t Result = Write_VDP_Table[Address](Address, Data, 1);
		*((unsigned char*)Data) = (unsigned char) Result;
		return 0;
	}
	else
	{
		uint32_t Result;

		while( Data_Length > 0)
		{
			Result = Write_VDP_Table[Address](Address, Data, 2);
			if( Result != 0)
				return Result;

			Data = (unsigned char*)Data + 2;
			Data_Length -= 2;
		}

		return 0;
	}
}

uint32_t Write_VDP_Data( uint32_t Address, void* Data, uint16_t Size)
{
	Un_VDP_Context.CTRL_Port_Got_Word = 0;

	uint8_t Octet1;
	uint8_t Octet2;

	if( Size == 1)
		Octet1 = Octet2 = (*(uint8_t*) Data);		// Octet is replicated to form a Word
	else
	{
		Octet1 = *((uint8_t*) Data);
		Octet2 = *(((uint8_t*) Data) + 1);
	}

	if( Un_VDP_Context.Current_Internal_Command & 0x20)
	{
		if( (Un_VDP_Context.WO_Reg[23] & 0xC0) != 0x80)
			return 0;

		return VDP_DMA_Fill( Octet1, Octet2);
	}

	uint8_t Index = Un_VDP_Context.Current_Internal_Command & 0xF;
	if( VDP_Control_Direction[Index] != 2)
		return 0;

	return VDP_Control_Write[Index]( Octet1, Octet2);
}

uint32_t Write_VDP_Control( uint32_t Address, void* Data, uint16_t Size)
{
	uint16_t Opcode;

	if( Size == 1)
		Opcode = ((*(uint8_t*) Data) << 8) | (*(uint8_t*) Data);		// Octet is replicated to form a Word
	else
	{
		memcpy( &Opcode, Data, 2);
		Opcode = ntohs( Opcode);
	}

	if( (Opcode & 0xC000) == 0x8000)	// VDP Register
	{
		Un_VDP_Context.WO_Reg[ (Opcode >> 8) & 0x1F] = Opcode;
		Un_VDP_Context.CTRL_Port_Got_Word = 0;
		Un_VDP_Context.Current_Internal_Command = 0;
		return 0;
	}

	if( ! Un_VDP_Context.CTRL_Port_Got_Word)
	{
		Un_VDP_Context.CTRL_Port_Got_Word = 1;

		Un_VDP_Context.Current_Internal_Address &= 0xC000;
		Un_VDP_Context.Current_Internal_Address |= (Opcode & 0x3FFF);

		Un_VDP_Context.Current_Internal_Command &= 0xFC;
		Un_VDP_Context.Current_Internal_Command |= (Opcode >> 14);

		return 0;
	}

	Un_VDP_Context.CTRL_Port_Got_Word = 0;

	Un_VDP_Context.Current_Internal_Address &= 0x3FFF;
	Un_VDP_Context.Current_Internal_Address |= (Opcode << 14);

	Un_VDP_Context.Current_Internal_Command &= 0x03;
	Un_VDP_Context.Current_Internal_Command |= ((Opcode & 0x00F0) >> 2);

	if( Un_VDP_Context.Current_Internal_Command & 0x20)								// is it DMA ?
		return VDP_DMA_Prep();

	return 0;
}

uint32_t Write_VDP_PSG( uint32_t Address, void* Data, uint16_t Size)
{
	return 0;
}

uint32_t VDP_VRAM_W( uint8_t Octet1, uint8_t Octet2)
{
	if( (Un_VDP_Context.Current_Internal_Address & 1) == 0)
	{
		Un_VDP_Context.VRAM[Un_VDP_Context.Current_Internal_Address] = Octet1;
		Un_VDP_Context.VRAM[Un_VDP_Context.Current_Internal_Address+1] = Octet2;
	}
	else
	{
		Un_VDP_Context.VRAM[Un_VDP_Context.Current_Internal_Address] = Octet2;
		Un_VDP_Context.VRAM[Un_VDP_Context.Current_Internal_Address+1] = Octet1;
	}
	Un_VDP_Context.Current_Internal_Address += Un_VDP_Context.WO_Reg[15];

	return 0;
}

uint32_t VDP_CRAM_W( uint8_t Octet1, uint8_t Octet2)
{
	// Since A0 is not used to write in CRAM, we must mask it.. at the same time we can "modulo 128" it too
	Un_VDP_Context.CRAM[Un_VDP_Context.Current_Internal_Address & 0x7E] = Octet1;
	Un_VDP_Context.CRAM[(Un_VDP_Context.Current_Internal_Address & 0x7E) + 1] = Octet2;
	Un_VDP_Context.Current_Internal_Address += Un_VDP_Context.WO_Reg[15];

	return 0;
}

uint32_t VDP_VSRAM_W( uint8_t Octet1, uint8_t Octet2)
{
	uint8_t Address = Un_VDP_Context.Current_Internal_Address & 0x7E;
	if( Address < 80)
	{
		Un_VDP_Context.VSRAM[Address] = Octet1;
		Un_VDP_Context.VSRAM[Address+1] = Octet2;
	}
	Un_VDP_Context.Current_Internal_Address += Un_VDP_Context.WO_Reg[15];

	return 0;
}

uint32_t VDP_DMA_Prep()
{
	if( !VDP_ENABLED_REG01_M1)		// DMA is disabled, should not work
		return 0;

	Un_VDP_Context.VDP_DMA_All.Source_Address = (Un_VDP_Context.WO_Reg[23] << 17) | (Un_VDP_Context.WO_Reg[22] << 9) | (Un_VDP_Context.WO_Reg[21] << 1);
	Un_VDP_Context.VDP_DMA_All.Nb_Iter = (Un_VDP_Context.WO_Reg[20] << 8) | Un_VDP_Context.WO_Reg[19];
	if( Un_VDP_Context.VDP_DMA_All.Nb_Iter == 0)
		Un_VDP_Context.VDP_DMA_All.Nb_Iter = 0xFFFF;

	switch( Un_VDP_Context.WO_Reg[23] & 0xC0)
	{
		case 0x80:	return 0;													// nothing to do
		case 0xC0:	if( ! (Un_VDP_Context.Current_Internal_Command & 0x10))		// we check CD4
						return 0;
					return VDP_DMA_Copy();
		default:	return VDP_DMA_68k_to_VDP();
	}
}

uint32_t VDP_DMA_Fill( uint8_t Octet1, uint8_t Octet2)
{
	if( !VDP_ENABLED_REG01_M1)		// DMA is disabled, should not work
		return 0;

	VDP_RAISE_REGST_DMA_BUSY;

	Un_VDP_Context.VRAM[Un_VDP_Context.Current_Internal_Address] = Octet2;

	while( Un_VDP_Context.VDP_DMA_All.Nb_Iter > 0)
	{
		Un_VDP_Context.VRAM[Un_VDP_Context.Current_Internal_Address ^ 1] = Octet1;

		Un_VDP_Context.VDP_DMA_All.Nb_Iter--;
		Un_VDP_Context.Current_Internal_Address += Un_VDP_Context.WO_Reg[15];
	}

	VDP_CLEAR_REGST_DMA_BUSY;

	return 0;
}

uint32_t VDP_DMA_Copy()
{
	abort();
	return 0;
}

uint32_t VDP_DMA_68k_to_VDP()
{
	if( !VDP_ENABLED_REG01_M1)		// DMA is disabled, should not work
		return 0;
	VDP_RAISE_REGST_DMA_BUSY;

	uint8_t* RAM_Address;
	uint16_t Max_Value;
	switch( Un_VDP_Context.Current_Internal_Command & 0xF)
	{
		case 1:	RAM_Address = Un_VDP_Context.VRAM;
				Max_Value = 0xFFFF;
				break;
		case 3:	RAM_Address = Un_VDP_Context.CRAM;
				Max_Value = 0x7F;
				break;
		case 5: RAM_Address = Un_VDP_Context.VSRAM;
				Max_Value = 0x7F;
				break;
		default: return 0;
	}

	// for now, very slow algorithm
	while( Un_VDP_Context.VDP_DMA_All.Nb_Iter && Un_VDP_Context.Current_Internal_Address <= Max_Value)
	{
		Read_From_Memory( Un_VDP_Context.VDP_DMA_All.Source_Address, RAM_Address + Un_VDP_Context.Current_Internal_Address, 2);

		Un_VDP_Context.VDP_DMA_All.Source_Address += 2;
		if( Un_VDP_Context.VDP_DMA_All.Source_Address > 0xFFFFFF)
			Un_VDP_Context.VDP_DMA_All.Source_Address -= 0x010000;
		Un_VDP_Context.Current_Internal_Address += Un_VDP_Context.WO_Reg[15];

		Un_VDP_Context.VDP_DMA_All.Nb_Iter--;
	}

	VDP_CLEAR_REGST_DMA_BUSY;

	return 0;
}
