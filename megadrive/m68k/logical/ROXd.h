#ifndef ROXD_H_
#define ROXD_H_


#include "../commons.h"


void mnemo_ROXd_REG_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_ROXd_REG_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_ROXd_REG_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);


#endif // ROXD_H_
