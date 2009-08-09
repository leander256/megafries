#ifndef EOR_H_
#define EOR_H_


#include "../commons.h"


void mnemo_EOR_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_EOR_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_EOR_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);


#endif // EOR_H_
