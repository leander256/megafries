#ifndef MOVEP_H_
#define MOVEP_H_


#include "../commons.h"


void mnemo_MOVEP_W_to_reg( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_MOVEP_W_to_mem( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_MOVEP_L_to_reg( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_MOVEP_L_to_mem( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);


#endif // MOVEP_H_
