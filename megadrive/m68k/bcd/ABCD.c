#include "ABCD.h"


void ABCD_internal( uint8_t Source, uint8_t* Destination, uint16_t* ConditionRegister);


void mnemo_ABCD( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks)
{
	uint16_t Source_Register = (M68k_Context_p->Current_Opcode & 0x0E00) >> 9;
	uint16_t Destination_Register = M68k_Context_p->Current_Opcode & 0x0007;

	// R/M field at bit 3
	if( (M68k_Context_p->Current_Opcode & 0x0008) == 0)
	{
		ABCD_internal( (unsigned char) M68k_Context_p->D[Source_Register], (unsigned char*) &(M68k_Context_p->D[Destination_Register]), &M68k_Context_p->Status_Register);
		*N_Ticks -= 6;
	}
	else
	{
		M68k_Context_p->A[Source_Register]--;
		M68k_Context_p->A[Destination_Register]--;

		uint8_t Source;
		M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->A[Source_Register], &Source, 1);
		if( M68k_Context_p->Interruptions != 0)
			return;

		uint8_t Destination;
		M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->A[Destination_Register], &Destination, 1);
		if( M68k_Context_p->Interruptions != 0)
			return;

		ABCD_internal( Source, &Destination, &(M68k_Context_p->Status_Register));
		*N_Ticks -= 8;
	}
}

void ABCD_internal( uint8_t Source, uint8_t* Destination, uint16_t* ConditionRegister)
{
	uint8_t Source_Decimal = Source & 0x0F;
	uint8_t Destination_Decimal = (*Destination) & 0x0F;

	Source_Decimal += (Source >> 4) * 10;
	Destination_Decimal += ((*Destination) >> 4) * 10;

	Destination_Decimal += Source_Decimal;
	if( CCR_CHECK_X(*ConditionRegister))
		Destination_Decimal++;

	if( Destination_Decimal > 99)
	{
		Destination_Decimal -= 100;
		CCR_SET_XC(*ConditionRegister);
	}
	else
	{
		CCR_CLEAR_XC(*ConditionRegister);

		if( Destination_Decimal != 0)
			CCR_CLEAR_Z(*ConditionRegister);		
	}

	*Destination = (Destination_Decimal % 10) + ((Destination_Decimal / 10) << 4); 
}
