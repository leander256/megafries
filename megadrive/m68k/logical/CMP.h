#ifndef CMP_H_
#define CMP_H_


#include "../commons.h"


void mnemo_CMP_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_CMP_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_CMP_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);


#endif // CMP_H_
