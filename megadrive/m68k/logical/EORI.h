#ifndef EORI_H_
#define EORI_H_


#include "../commons.h"


void mnemo_EORI_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_EORI_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_EORI_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);


#endif // EORI_H_
