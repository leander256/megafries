#include <stddef.h>
#include <string.h>
#include <stdint.h>


#include "system_io.h"


uint16_t (*Read_IO_Table[0x20]) ( uint32_t Address) = { NULL };

uint16_t Read_IO_Version( uint32_t Address);
uint16_t Read_IO_Data_1( uint32_t Address);
uint16_t Read_IO_Data_2( uint32_t Address);
uint16_t Read_IO_Ctrl_E( uint32_t Address);


void Init_IO()
{
	Read_IO_Table[0x0] = Read_IO_Table[0x1] = &Read_IO_Version;

	Read_IO_Table[0x2] = Read_IO_Table[0x3] = &Read_IO_Data_1;
	Read_IO_Table[0x4] = Read_IO_Table[0x5] = &Read_IO_Data_2;

	Read_IO_Table[0xc] = Read_IO_Table[0xd] = &Read_IO_Ctrl_E;
}


uint32_t Read_IO( uint32_t Address, void* Data, uint8_t Data_Length)
{
	Address -= 0x00A10000;

	if( Data_Length == 1)
	{
		uint16_t Result = Read_IO_Table[Address](Address);
		*((unsigned char*)Data) = (unsigned char) Result;
		return 0;
	}
	else
	{
//		while( Data_Length > 0)
		{

//			Data_Length -= 2;
		}

		memset( Data, 0, Data_Length);
		return 0;
	}
}

uint16_t Read_IO_Version( uint32_t Address)
{
	return 0x0060;
}

uint16_t Read_IO_Data_1( uint32_t Address)
{
	return 0;
}

uint16_t Read_IO_Data_2( uint32_t Address)
{
	return 0;
}

uint16_t Read_IO_Ctrl_E( uint32_t Address)
{
	return 0;
}

uint32_t Write_IO( uint32_t Address, void* Data, uint8_t Data_Length)
{
	return 0;
}
