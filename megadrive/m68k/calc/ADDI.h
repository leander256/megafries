#ifndef ADDI_H_
#define ADDI_H_


#include "../commons.h"


void mnemo_ADDI_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_ADDI_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_ADDI_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);


#endif // ADDI_H_
