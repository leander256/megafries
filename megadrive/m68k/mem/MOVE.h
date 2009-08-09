#ifndef MOVE_H_
#define MOVE_H_


#include "../commons.h"


void init_functions_MOVE();

void mnemo_MOVE_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_MOVE_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_MOVE_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);


#endif /*MOVE_H_*/
