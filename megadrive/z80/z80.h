#ifndef Z80_H_
#define Z80_H_


#include "../m68k/commons.h"


void Init_z80();

uint32_t Read_z80( uint32_t Address, void* Data, uint16_t Data_Length);
uint32_t Write_z80( uint32_t Address, void* Data, uint16_t Data_Length);


#endif // Z80_H_
