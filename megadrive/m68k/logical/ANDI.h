#ifndef ANDI_H_
#define ANDI_H_


#include "../commons.h"


void mnemo_ANDI_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_ANDI_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_ANDI_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);


#endif /*ANDI_H_*/
