#ifndef TST_H_
#define TST_H_


#include "../commons.h"


void init_timings_TST();
void init_functions_TST();
void mnemo_TST( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);

void mnemo_TST_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_TST_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_TST_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);


#endif /*TST_H_*/
