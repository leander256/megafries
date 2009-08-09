#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include "system_control.h"


uint32_t Read_Control( uint32_t Address, void* Data, uint16_t Data_Length)
{
	memset( Data, 0, Data_Length);
	return 0;
}

uint32_t Write_Control( uint32_t Address, void* Data, uint16_t Data_Length)
{
	return 0;
}
