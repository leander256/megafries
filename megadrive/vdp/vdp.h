#ifndef VDP_H_
#define VDP_H_


#include "../m68k/commons.h"


// Internal settings

struct VDP_Context
{
	uint8_t WO_Reg[24];
	uint8_t RO_Reg[2];	// there's one but let's pretend it's 2 bytes wide (since we can read bytes from 0xC00004)

	uint8_t CTRL_Port_Got_Word;
	uint8_t Current_Internal_Command;
	uint16_t Current_Internal_Address;

	struct
	{
		uint32_t Source_Address;
		uint16_t Nb_Iter;
	} VDP_DMA_All;


	uint8_t VRAM[65536];
	uint8_t CRAM[64 * 2];
	uint8_t VSRAM[40 * 2 + 24 * 2]; // it's 80 bytes long but to make things easier we allocate a power of 2 sized table
									// we must just be careful when *reading* from it

} Un_VDP_Context;

// Check register values
#define VDP_ENABLED_REG00_IE1 ((Un_VDP_Context.WO_Reg[0] & 0x10) != 0)

#define VDP_ENABLED_REG01_DISP ((Un_VDP_Context.WO_Reg[1] & 0x40) != 0)
#define VDP_ENABLED_REG01_M1 ((Un_VDP_Context.WO_Reg[1] & 0x10) != 0)
#define VDP_ENABLED_REG01_M2 ((Un_VDP_Context.WO_Reg[1] & 0x08) != 0)

#define VDP_ENABLED_REG12_RS0 ((Un_VDP_Context.WO_Reg[12] & 0x80) != 0)
#define VDP_ENABLED_REG12_RS1 ((Un_VDP_Context.WO_Reg[12] & 0x01) != 0)

// Mod register values
#define VDP_CLEAR_REG01_M1 Un_VDP_Context.WO_Reg[1] &= 0xEF;

#define VDP_RAISE_REGST_VBLANK Un_VDP_Context.RO_Reg[1] |= 0x08;
#define VDP_CLEAR_REGST_VBLANK Un_VDP_Context.RO_Reg[1] &= 0xF7;
#define VDP_RAISE_REGST_HBLANK Un_VDP_Context.RO_Reg[1] |= 0x04;
#define VDP_CLEAR_REGST_HBLANK Un_VDP_Context.RO_Reg[1] &= 0xFB;
#define VDP_RAISE_REGST_DMA_BUSY Un_VDP_Context.RO_Reg[1] |= 0x02;
#define VDP_CLEAR_REGST_DMA_BUSY Un_VDP_Context.RO_Reg[1] &= 0xFD;


void Init_VDP();

uint32_t Read_VDP( uint32_t Address, void* Data, uint16_t Data_Length);
uint32_t Write_VDP( uint32_t Address, void* Data, uint16_t Data_Length);

void VDP_Render_Screen();


uint8_t Debug_Patterns;


#endif // VDP_H_
