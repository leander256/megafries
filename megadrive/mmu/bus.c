#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL/SDL.h>

#include "system_io.h"
#include "system_control.h"
#include "../vdp/vdp.h"
#include "../z80/z80.h"
#include "bus.h"


Memory_Map_Function Read_Map[256] = { NULL };
Memory_Map_Function Write_Map[256] = { NULL };


void* Cartridge = NULL;

uint8_t RAM[65536] = { 0 };

struct M68k_Context* my_M68k = NULL;

uint8_t Int_Autovector;

int32_t Nb_Ticks_System;
int32_t Nb_Ticks_Given_to_CPU;
int32_t Nb_Ticks_CPU;


uint32_t Read_From_Memory( uint32_t Address, void* Data, uint16_t Data_Length)
{
	assert( Data != NULL);
	assert( Data_Length == 1 || (((Data_Length & 0xFFFE) != 0) && ((Data_Length & 0x01) == 0)));

	// 24 bits bus
	Address &= 0x00FFFFFF;

	if( Data_Length > 1 && (Address & 0x00000001) != 0)
		return INT_ADDRESS_ERROR;

	Memory_Map_Function my_Function = Read_Map[Address >> 16];

	if( my_Function == NULL)
		return INT_BUS_ERROR;
	else
		return my_Function( Address, Data, Data_Length);
}

uint32_t Read_Cartridge( uint32_t Address, void* Data, uint16_t Data_Length)
{
	if( Address + Data_Length > 0x00400000)
		return INT_BUS_ERROR;

	memcpy( Data, (char*)Cartridge + Address, Data_Length);

	return 0;
}

uint32_t Read_A1( uint32_t Address, void* Data, uint16_t Data_Length)
{
	if( Address < 0x00A10020)
		return Read_IO( Address, Data, Data_Length);

	if( (Address >= 0x00A11000) && (Address < 0x00A11202))
		return Read_Control( Address, Data, Data_Length);

	return INT_BUS_ERROR;
}

uint32_t Read_RAM( uint32_t Address, void* Data, uint16_t Data_Length)
{
	// if somehow a game wants to read from FEFFFE to FF0001, it's supposed to work (?)
	if( Address + Data_Length > 0x01000000)
		return INT_BUS_ERROR;

	memcpy( Data, RAM + (Address & 0x0000FFFF), Data_Length);

	return 0;
}

uint32_t Write_To_Memory( uint32_t Address, void* Data, uint16_t Data_Length)
{
	assert( Data != NULL);
	assert( Data_Length == 1 || (((Data_Length & 0xFE) != 0) && ((Data_Length & 0x01) == 0)));

	// 24 bits bus
	Address &= 0x00FFFFFF;

	if( Data_Length > 1 && (Address & 0x00000001) != 0)
		return INT_ADDRESS_ERROR;

	Memory_Map_Function my_Function = Write_Map[Address >> 16];

	if( my_Function == NULL)
		return INT_BUS_ERROR;
	else
		return my_Function( Address, Data, Data_Length);
}

uint32_t Write_A1( uint32_t Address, void* Data, uint16_t Data_Length)
{
	if( Address < 0x00A10020)
		return Write_IO( Address, Data, Data_Length);

	if( (Address >= 0x00A11000) && (Address < 0x00A11202))
		return Write_Control( Address, Data, Data_Length);

	return INT_BUS_ERROR;
}

uint32_t Write_RAM( uint32_t Address, void* Data, uint16_t Data_Length)
{
	// if somehow a game wants to write from 0x00FEFFFE to 0x00FF0001, it's supposed to work (?) so we don't test lower boundaries
	if( Address + Data_Length > 0x01000000)
		return INT_BUS_ERROR;

	memcpy( RAM + (Address & 0x0000FFFF), Data, Data_Length);

	return 0;
}

uint8_t Read_Interrupt_Controler()
{
	int NumVector = 7;

	while( NumVector > 0 && ((Int_Autovector >> NumVector) & 1) == 0)
		NumVector--;

	//Int_Autovector &= ~(1 << NumVector);
	Int_Autovector = 0;
	return NumVector;
}

void Init_MMU( void* Cartridge_p)
{
	assert( Cartridge_p != NULL);

	Cartridge = Cartridge_p;

	// Read_Map
	int i;
	for( i = 0; i < 0x40; i++)
		Read_Map[i] = &Read_Cartridge;

	Read_Map[0xA0] = &Read_z80;
	Read_Map[0xA1] = &Read_A1;
	Read_Map[0xC0] = &Read_VDP;

	Read_Map[0xFF] = &Read_RAM;

	// Write_Map
	Write_Map[0xA0] = &Write_z80;
	Write_Map[0xA1] = &Write_A1;
	Write_Map[0xC0] = &Write_VDP;

	Write_Map[0xFF] = &Write_RAM;

	// Init all components
	Init_IO();
	Init_VDP();

	my_M68k = M68k_Init( &Read_From_Memory, &Write_To_Memory, &Read_Interrupt_Controler);
}

void Start_Machine()
{
	assert( my_M68k != NULL);

	Nb_Ticks_System = 0;
	Nb_Ticks_CPU = 0;
	Nb_Ticks_Given_to_CPU = 130000;

	uint16_t Nb_Rasters_left = VDP_ENABLED_REG01_M2 ? 240 : 224;

	for(;;)
	{
		M68k_Run_N_Ticks( my_M68k, &Nb_Ticks_Given_to_CPU);
		//VDP_Run(130000);

		if( Nb_Rasters_left == 0)
		{
			VDP_Render_Screen();
			Nb_Rasters_left = VDP_ENABLED_REG01_M2 ? 240 : 224;
			VDP_RAISE_REGST_VBLANK;

			my_M68k->Interruptions |= INT_INTAUTOVEC;
			Int_Autovector |= 1 << 6;
			Nb_Ticks_Given_to_CPU += 2500;

			usleep( 10000);
		}
		else
		{
			Nb_Rasters_left--;
			VDP_CLEAR_REGST_VBLANK;

			if( VDP_ENABLED_REG00_IE1)
			{
				my_M68k->Interruptions |= INT_INTAUTOVEC;
				Int_Autovector |= 1 << 4;
			}

			VDP_RAISE_REGST_HBLANK;
			Nb_Ticks_Given_to_CPU += 70;
			M68k_Run_N_Ticks( my_M68k, &Nb_Ticks_Given_to_CPU);
			VDP_CLEAR_REGST_HBLANK;

			Nb_Ticks_Given_to_CPU += 500;
		}

		//////////////////////////////////////////////// SDL events, so we can at least quit
		SDL_Event event;
		while ( SDL_PollEvent(&event) )
		{
			switch (event.type)
			{
				case SDL_MOUSEBUTTONDOWN:
									printf( "Mouse button %d pressed at (%d,%d)\n",
											event.button.button, event.button.x, event.button.y);
									break;
				case SDL_KEYUP:
									if( event.key.keysym.sym == SDLK_SPACE)
										Debug_Patterns = 1;
									break;
				case SDL_QUIT:		return;
			}
		}
		/////////////////////////////////////////////////////////////////////////////////

	}
}


/////////////////////////////////////////////////////////////////////////////////


int main( int argc, char* argv[])
{
	char* FileName;

	if( argc >= 2)
		FileName = argv[1];
	else
		FileName = "Sonic.bin";

	printf( "Loading %s... ", FileName);

	FILE* fin = fopen( FileName, "r");
	if( fin == NULL)
	{
		printf( "Epic FAIL\n");
		return 1;
	}

	struct stat RomFileStats;
	if( stat( FileName, &RomFileStats) == -1)
	{
		printf( "Epic FAIL 3\n");
		return 1;
	}

	void* my_Cartridge = malloc( RomFileStats.st_size);
	printf( "Read %ld bytes... ", RomFileStats.st_size);
	size_t Read_Bytes = fread( my_Cartridge, RomFileStats.st_size, 1, fin);
	if( Read_Bytes != 1)
	{
		printf( "\nEpic FAIL 2\n");
		return 2;
	}
	fclose( fin);
	printf( " Done\n");

	Init_MMU( my_Cartridge);
	Start_Machine();

	return 0;
}
