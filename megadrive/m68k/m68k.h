#ifndef M68K_H_
#define M68K_H_


#include "commons.h"


struct M68k_Context* M68k_Init( Memory_Map_Function Read_Memory_p,
								Memory_Map_Function Write_Memory_p,
								Int_Ctrl_Function Read_Int_Ctrl_p);

void M68k_Reset( struct M68k_Context* M68k_Context_p);

void SwitchToSuperUser( struct M68k_Context* M68k_Context_p);

void M68k_Run_N_Ticks( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks_p);


#endif /*M68K_H_*/
