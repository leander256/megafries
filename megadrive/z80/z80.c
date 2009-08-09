#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>


#include "z80.h"
#include "../ym2612/ym2612.h"


unsigned char z80_RAM[65536];

uint32_t (*Read_Z80_Table[16])( uint32_t Address, void* Data, uint16_t Data_Length) = { NULL };

uint32_t Read_Sound_Bank( uint32_t Address, void* Data, uint16_t Data_Length);


void Init_z80()
{
}

uint32_t Read_z80( uint32_t Address, void* Data, uint16_t Data_Length)
{
	Address -= 0xA00000;

	if( Address < 0x2000)
	{
		if( Address + Data_Length > 0x2000)
		{
			Read_Sound_Bank( Address, Data, 0x2000 - Address);
			return INT_BUS_ERROR;
		}
		else
		{
			Read_Sound_Bank( Address, Data, Data_Length);
			return 0;
		}
	}

	if( Address < 0x4000)
		abort();//return INT_BUS_ERROR;

	if( Address < 0x4004)
	{
		uint8_t Bus_Err = 0;
		if( Address + Data_Length > 0x4004)
			Bus_Err = 1;

		while( Data_Length > 0)
		{
			*(uint8_t*)Data = Read_ym2612();
			Data = (uint8_t*)Data - 1;
			Data_Length--;
		}

		if( Bus_Err)
			return INT_BUS_ERROR;
		else
			return 0;
	}

	memcpy( Data, z80_RAM + Address, Data_Length);

	return 0;
}

uint32_t Read_Sound_Bank( uint32_t Address, void* Data, uint16_t Data_Length)
{
	memcpy( Data, z80_RAM + Address, Data_Length);

	return 0;
}

uint32_t Write_z80( uint32_t Address, void* Data, uint16_t Data_Length)
{
	Address -= 0xA00000;
	memcpy( z80_RAM + Address, Data, Data_Length);

	return 0;
}
