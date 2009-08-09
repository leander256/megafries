#ifndef CMPI_H_
#define CMPI_H_


#include "../commons.h"


void mnemo_CMPI_B( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_CMPI_W( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);
void mnemo_CMPI_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks);


#endif // CMPI_H_
