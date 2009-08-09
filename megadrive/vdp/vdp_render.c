#include <xcb/xcb.h>
#include <string.h>
#include <stdlib.h>
#include <SDL/SDL.h>


#include "vdp.h"
#include "vdp_render.h"


uint8_t Debug_Patterns = 0;
uint8_t DP_x = 19;
uint8_t DP_y = 10;
uint32_t DP_Colour;


xcb_connection_t*			VDP_XCB_Connection;
xcb_screen_t*				VDP_XCB_Screen;
xcb_window_t				VDP_XCB_Window;
xcb_pixmap_t				VDP_XCB_Pixmap;
xcb_gcontext_t				VDP_XCB_GC;
xcb_colormap_t				VDP_XCB_ColourMap;
xcb_alloc_color_reply_t*	VDP_XCB_ColourTable[64];


SDL_Surface*		VDP_SDL_Screen;
uint32_t			VDP_SDL_ColourMap[65];


uint8_t Rendered_Screen[4][320*240];


uint32_t Init_Renderer_XCB()
{
	VDP_XCB_Connection = xcb_connect( NULL, NULL);
	VDP_XCB_Screen = xcb_setup_roots_iterator( xcb_get_setup( VDP_XCB_Connection)).data;

	/* Create the window */
	VDP_XCB_Window = xcb_generate_id( VDP_XCB_Connection);
	xcb_create_window(	VDP_XCB_Connection,						/* Connection          */
						XCB_COPY_FROM_PARENT,					/* depth (same as root)*/
						VDP_XCB_Window,							/* window Id           */
						VDP_XCB_Screen->root,					/* parent window       */
						160, 120,								// x, y
						320, 240,								/* width, height       */
						2,										/* border_width        */
						XCB_WINDOW_CLASS_INPUT_OUTPUT,			/* class               */
						VDP_XCB_Screen->root_visual,			/* visual              */
						XCB_CW_BACK_PIXEL,
						&(VDP_XCB_Screen->black_pixel));

	/* Map the window on the screen */
	xcb_map_window( VDP_XCB_Connection, VDP_XCB_Window);

	VDP_XCB_Pixmap = xcb_generate_id( VDP_XCB_Connection);
	xcb_create_pixmap(	VDP_XCB_Connection,
						VDP_XCB_Screen->root_depth,
						VDP_XCB_Pixmap,
						VDP_XCB_Window,
						320,
						240);

	VDP_XCB_ColourMap = xcb_generate_id( VDP_XCB_Connection);
	xcb_create_colormap(	VDP_XCB_Connection,
							XCB_COLORMAP_ALLOC_NONE,
							VDP_XCB_ColourMap,
							VDP_XCB_Window,
							VDP_XCB_Screen->root_visual);

	VDP_XCB_GC = xcb_generate_id( VDP_XCB_Connection);
	xcb_create_gc(	VDP_XCB_Connection,
					VDP_XCB_GC,
					VDP_XCB_Window,
					0, NULL );

	xcb_flush( VDP_XCB_Connection);

	memset( VDP_XCB_ColourTable, 0, 64);

	return 0;
}

uint32_t Init_Renderer_SDL()
{
	// now we try SDL
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("Unable to init SDL: %s\n", SDL_GetError());
		abort();
	}
	atexit( SDL_Quit);

	VDP_SDL_Screen = SDL_SetVideoMode( 640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if( VDP_SDL_Screen == NULL)
	{
		printf( "Unable to set SDL video: %s\n", SDL_GetError());
		abort();
	}

	VDP_SDL_ColourMap[64] = SDL_MapRGB( VDP_SDL_Screen->format, 255, 0, 0);

	return 0;
}

uint32_t Init_Renderer()
{
	//Init_Renderer_XCB();
	Init_Renderer_SDL();

	memset( Rendered_Screen, 0, 320 * 240 * 4);

	return 0;
}


uint8_t Reverse4[2][4] = { { 0, 1, 2, 3}, { 3, 2, 1, 0 } };
uint8_t Reverse8[2][8] = { { 0, 1, 2, 3, 4, 5, 6, 7}, { 7, 6, 5, 4, 3, 2, 1, 0 } };

void VDP_Render_Pattern( uint16_t Pattern_Address, uint8_t FlipX, uint8_t FlipY, uint8_t ScreenNumber, uint16_t x, uint16_t y, uint8_t Palette_Number)
{
	uint8_t ColourHi;
	uint8_t ColourLo;

	for( int j = 0; j < 8; j++)
		for( int i = 0; i < 4; i++)
		{
			ColourHi = *(Un_VDP_Context.VRAM + Pattern_Address + (Reverse8[FlipY][j] * 4) + Reverse4[FlipX][i]);
			ColourLo = (ColourHi & 0x0F);
			ColourHi >>= 4;

			if( ColourHi != 0)
				Rendered_Screen[ScreenNumber][ (y + j) * 320 + x + i * 2 + FlipX] = ColourHi | Palette_Number;
			if( ColourLo != 0)
				Rendered_Screen[ScreenNumber][ (y + j) * 320 + x + i * 2 + (1 - FlipX) ] = ColourLo | Palette_Number;
		}
}

void VDP_Render_Scroll_Plane( uint8_t Nb_H_Cells, uint8_t Nb_V_Cells, uint8_t Plane_Length, uint8_t Plane_Height, uint8_t* PNT_Scroll_p, uint8_t ScreenNumber, uint8_t OffsetType)
{
	uint16_t Pattern_Value;
	uint16_t Pattern_Address;
	uint8_t Palette_Number;

	uint8_t FlipX;
	uint8_t FlipY;

	for( int y = 0; y < Nb_V_Cells; y++)
		for( int x = 0; x < Nb_H_Cells; x++)
		{
			Pattern_Value = *(uint16_t*) (PNT_Scroll_p + ((y * Plane_Length + x) * 2));
			Pattern_Value = ntohs( Pattern_Value);

			Palette_Number = (Pattern_Value >> 9) & 0x30;
			Pattern_Address = Pattern_Value << 5;

			FlipX = (Pattern_Value & 0x0800) >> 11;
			FlipY = (Pattern_Value & 0x1000) >> 12;

			VDP_Render_Pattern( Pattern_Address, FlipX, FlipY, ScreenNumber, x << 3, y << 3, Palette_Number);
		}
}

void VDP_Render_Sprite( uint8_t* PNT_Sprite, uint8_t* Current_Sprite, const uint16_t Max_H, const uint16_t Max_V, const uint8_t ScreenNumber)
{
	uint16_t Next_Sprite = (*(Current_Sprite + 3)) & 0x7F;
	if( Next_Sprite != 0)
		VDP_Render_Sprite( PNT_Sprite, PNT_Sprite + (Next_Sprite << 3), Max_H, Max_V, ScreenNumber);

	uint16_t H_Pos = ntohs(*(uint16_t*) (Current_Sprite + 6)) & 0x01FF;
	uint16_t V_Pos = ntohs(*(uint16_t*) Current_Sprite) & 0x03FF;

	uint8_t H_Size = (((*(Current_Sprite + 2)) >> 2) & 3) + 1;
	uint8_t V_Size = ((*(Current_Sprite + 2)) &  3) + 1;

	uint8_t Palette_Number = ((*(Current_Sprite + 4)) & 0x60) >> 1;
	uint16_t Pattern_Address = ntohs(*(uint16_t*) (Current_Sprite + 4)) << 5;

	uint16_t Rel_H_Pos;
	uint16_t Rel_V_Pos;
	uint16_t Current_Pattern_Address;

	uint8_t FlipX = ((*(Current_Sprite + 4)) & 0x08) >> 3;
	uint8_t FlipY = ((*(Current_Sprite + 4)) & 0x10) >> 4;

	uint8_t Real_x;
	uint8_t Real_y;

	for( int y = 0; y < V_Size; y++)
		for( int x = 0; x < H_Size; x++)
		{
			Real_x = FlipX ? (H_Size - 1 - x) : x;
			Real_y = FlipY ? (V_Size - 1 - y) : y;

			if( H_Pos + x * 8 >= 0x80 && H_Pos + (x + 1) * 8 <= Max_H)
				if( V_Pos + y * 8 >= 0x80 && V_Pos + (y + 1) * 8 <= Max_V)
				{
					Rel_H_Pos = H_Pos + x * 8 - 0x80;
					Rel_V_Pos = V_Pos + y * 8 - 0x80;

					Current_Pattern_Address = Pattern_Address + (Real_y + Real_x * V_Size) * 32;		// ! Sprites are generated column by column, not line by line
					VDP_Render_Pattern( Current_Pattern_Address, FlipX, FlipY, ScreenNumber, Rel_H_Pos, Rel_V_Pos, Palette_Number);
				}
		}

}

uint32_t VDP_Render_XCB_Palette()
{
	for( int i = 0; i < 64; i++)
		free( VDP_XCB_ColourTable[i]);

	for( int i = 0; i < 64; i++)
		VDP_XCB_ColourTable[i] = xcb_alloc_color_reply(	VDP_XCB_Connection,
														xcb_alloc_color(VDP_XCB_Connection,
																		VDP_XCB_ColourMap,
																		(Un_VDP_Context.CRAM[i*2+1] << 12) & 0xE000,
																		(Un_VDP_Context.CRAM[i*2+1] << 8) & 0xE000,
																		(Un_VDP_Context.CRAM[i*2] << 4) & 0xE000 ),
														NULL);
	return 0;
}

uint32_t VDP_Render_SDL_Palette()
{
	for( int i = 0; i < 64; i++)
		VDP_SDL_ColourMap[i] = SDL_MapRGB(	VDP_SDL_Screen->format,
											(Un_VDP_Context.CRAM[i*2+1] << 4) & 0xE0,
											Un_VDP_Context.CRAM[i*2+1] & 0xE0,
											(Un_VDP_Context.CRAM[i*2] << 4) & 0xE0 );
	return 0;
}

uint32_t VDP_Render_XCB_Display()
{
	xcb_point_t myPoint;
	uint32_t myColour;
	uint32_t myOldColour = VDP_XCB_ColourTable[ Rendered_Screen[0][0] ]->pixel;
	xcb_change_gc(	VDP_XCB_Connection,
					VDP_XCB_GC,
					XCB_GC_FOREGROUND,
					&myOldColour );

	for( myPoint.y = 0; myPoint.y < 240; myPoint.y++)
		for( myPoint.x = 0; myPoint.x < 320; myPoint.x++)
		{
			myColour = VDP_XCB_ColourTable[ Rendered_Screen[0][myPoint.y * 320 + myPoint.x] ]->pixel;
			if( myColour != myOldColour)
			{
				xcb_change_gc(	VDP_XCB_Connection,
								VDP_XCB_GC,
								XCB_GC_FOREGROUND,
								&myColour );
				myOldColour = myColour;
			}

			xcb_poly_point(	VDP_XCB_Connection,
							XCB_COORD_MODE_ORIGIN,
							VDP_XCB_Pixmap,
							VDP_XCB_GC,
							1,
							&myPoint);
		}

	xcb_copy_area(	VDP_XCB_Connection,
					VDP_XCB_Pixmap,
					VDP_XCB_Window,
					VDP_XCB_GC,
					0, 0,
					0, 0,
					320, 240 );

	xcb_flush( VDP_XCB_Connection);

	return 0;
}

uint32_t VDP_Render_SDL_Display()
{
	uint32_t Offset_X;
	uint32_t Offset_Y;
	uint8_t PixCol;

//	for( int i = 0, ki = 0; ki < 2; ki++)
//		for( int j = 0, kj = 0; kj < 2; kj++)
	for( int i = 0; i < 2; i++)
		for( int j = 0; j < 2; j++)
			for( int y = 0; y < 240; y++)
				for( int x = 0; x < 320; x++)
				{
					PixCol = Rendered_Screen[j*2+i][y * 320 + x];
					//if( (PixCol & 0xF) != 0)		// if not transparent
					{
						Offset_X = x + i * 320;
						Offset_Y = (y + j * 240 ) * VDP_SDL_Screen->pitch / 4;

						*( (uint32_t*) VDP_SDL_Screen->pixels + Offset_X + Offset_Y) =
							VDP_SDL_ColourMap[ PixCol ];
					}
				}

//	for( int i = 0; i < 400; i++)
//		*( (uint32_t*) VDP_SDL_Screen->pixels + i + i * VDP_SDL_Screen->pitch / 4) = VDP_SDL_ColourMap[ 1 ];

	SDL_Flip( VDP_SDL_Screen);

	return 0;
}

uint8_t VDP_Scrolling_Screen_Size[] = { 32, 64, 255, 128 };

void VDP_Render_Screen()
{
	if( ! VDP_ENABLED_REG01_DISP)
		return;

	///////////////////////////////////// Screen mode
	if( VDP_ENABLED_REG12_RS0 != VDP_ENABLED_REG12_RS1)
		return;

	uint8_t Nb_H_Cells = VDP_ENABLED_REG12_RS0 ? 40 : 32;
	uint8_t Nb_V_Cells = VDP_ENABLED_REG01_M2  ? 30 : 28;

	///////////////////////////////////// Palette
	//VDP_Render_XCB_Palette();
	VDP_Render_SDL_Palette();

	///////////////////////////////////// Scrolling screen

	// Scrolling screen size
	uint8_t HSZ = VDP_Scrolling_Screen_Size[ Un_VDP_Context.WO_Reg[16] & 3 ];
	uint8_t VSZ = VDP_Scrolling_Screen_Size[ (Un_VDP_Context.WO_Reg[16] >> 4) & 3 ];

	if( HSZ + VSZ > 160)
		return;

	// HScroll Data Table
	//uint8_t* HSDT_p = Un_VDP_Context.VRAM + ((((uint16_t) Un_VDP_Context.WO_Reg[13]) << 10) & 0xFC00);

	// Pattern Name Table
	uint8_t* PNT_ScrollA = Un_VDP_Context.VRAM + ((((uint16_t) Un_VDP_Context.WO_Reg[2]) << 10 ) & 0xE000);
	uint8_t* PNT_ScrollB = Un_VDP_Context.VRAM + ((((uint16_t) Un_VDP_Context.WO_Reg[4]) << 13 ) & 0xE000);

	// We should use reg #17 and #18 for offset

	// First, the background colour
	memset( Rendered_Screen, Un_VDP_Context.WO_Reg[7] & 0x3F, 320*240*4);

	// Now Scroll B then A
	VDP_Render_Scroll_Plane( Nb_H_Cells, Nb_V_Cells, HSZ, VSZ, PNT_ScrollB, 0, 1);
	VDP_Render_Scroll_Plane( Nb_H_Cells, Nb_V_Cells, HSZ, VSZ, PNT_ScrollA, 1, 0);
	//VDP_Render_Scroll_Plane( Nb_H_Cells, Nb_V_Cells, HSZ, VSZ, PNT_ScrollA + 16, 0);

	// And Window
	uint8_t* PNT_Window;
	if( Nb_H_Cells == 40)
		PNT_Window = Un_VDP_Context.VRAM + ((((uint16_t) Un_VDP_Context.WO_Reg[3]) << 10 ) & 0xF000);
	else
		PNT_Window = Un_VDP_Context.VRAM + ((((uint16_t) Un_VDP_Context.WO_Reg[3]) << 10 ) & 0xF800);
	VDP_Render_Scroll_Plane( Nb_H_Cells, Nb_V_Cells, Nb_H_Cells == 40 ? 64 : 32, Nb_V_Cells, PNT_Window, 2, 2);

	// Sprites must be rendered differently
	uint8_t* PNT_Sprite;
	if( Nb_H_Cells == 40)
		PNT_Sprite = Un_VDP_Context.VRAM + ((((uint16_t) Un_VDP_Context.WO_Reg[5]) << 9 ) & 0xFC00);
	else
		PNT_Sprite = Un_VDP_Context.VRAM + (((uint16_t) Un_VDP_Context.WO_Reg[5]) << 9 );
	VDP_Render_Sprite( PNT_Sprite, PNT_Sprite, Nb_H_Cells == 40 ? 0x1bf : 0x17f, Nb_V_Cells == 30 ? 0x16f : 0x15f, 3);

	if( Debug_Patterns)
	{
		for( int y = 0; y < 240; y++)
			for( int i = 0; i < 4; i++)
				Rendered_Screen[i][y*320 + DP_x*8] = Rendered_Screen[i][y*320 + DP_x*8 + 7] = 64;

		for( int x = 0; x < 320; x++)
			for( int i = 0; i < 4; i++)
				Rendered_Screen[i][DP_y*8*320 + x] = Rendered_Screen[i][(DP_y*8+7)*320 + x] = 64;

		printf( "Debug Patterns, x=%02d, y=%02d\n", DP_x, DP_y);
	}

	// We copy everything to the pixmap pixel by pixel
	//VDP_Render_XCB_Display();
	VDP_Render_SDL_Display();

	if( Debug_Patterns)
	{
		SDL_Event event;
		SDL_WaitEvent(&event);

		switch (event.type)
		{
			case SDL_KEYUP:
							if( event.key.keysym.sym == SDLK_SPACE)
							{
								Debug_Patterns = 0;
								break;
							}
							if( event.key.keysym.sym == SDLK_UP && DP_y > 0)
							{
								DP_y--;
								break;
							}
							if( event.key.keysym.sym == SDLK_DOWN && DP_y < 29)
							{
								DP_y++;
								break;
							}
							if( event.key.keysym.sym == SDLK_LEFT && DP_x > 0)
							{
								DP_x--;
								break;
							}
							if( event.key.keysym.sym == SDLK_RIGHT && DP_x < 39)
							{
								DP_x++;
								break;
							}
		}
	}

}
