#ifndef ROD_H_
#define ROD_H_


#include "../commons.h"


void mnemo_ROd_REG_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_ROd_REG_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_ROd_REG_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);


#endif // ROD_H_
