#ifndef LSD_H_
#define LSD_H_


#include "../commons.h"


void mnemo_LSd_REG_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_LSd_REG_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_LSd_REG_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);


#endif // LSD_H_
