#ifndef MMU_H_
#define MMU_H_


#include "../m68k/m68k.h"


uint32_t Read_From_Memory( uint32_t Address, void* Data, uint16_t Data_Length);	// declared here for VDP DMA

void Init_MMU( void* Cartridge_p);


#endif /*MMU_H_*/
