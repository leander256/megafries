#ifndef ASD_H_
#define ASD_H_


#include "../commons.h"


void mnemo_ASd_REG_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_ASd_REG_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_ASd_REG_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);


#endif // ASD_H_
