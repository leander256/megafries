#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "m68k.h"
#include "ilt.h"
#include "calc/ADDQ.h"	// temp


struct M68k_Context* M68k_Init( Memory_Map_Function Read_Memory_p,
								Memory_Map_Function Write_Memory_p,
								Int_Ctrl_Function Read_Int_Ctrl_p)
{
	Init_ILT();

	struct M68k_Context* my_M68k_Context = (struct M68k_Context*) malloc( sizeof(struct M68k_Context));
	assert( my_M68k_Context != NULL);

	my_M68k_Context->Read_Memory = Read_Memory_p;
	my_M68k_Context->Write_Memory = Write_Memory_p;
	my_M68k_Context->Read_Interrupt_Controler = Read_Int_Ctrl_p;

	memset( my_M68k_Context->D, 0, sizeof( my_M68k_Context->D));
	memset( my_M68k_Context->A, 0, sizeof( my_M68k_Context->A));
	my_M68k_Context->Status_Register = 0;

	M68k_Reset( my_M68k_Context);

	return my_M68k_Context;
}

void M68k_Reset( struct M68k_Context* M68k_Context_p)
{
	assert( M68k_Context_p != NULL);

	M68k_Context_p->Current_Opcode = 0;
	M68k_Context_p->Interruptions = 0;

	// STEP 1 : set S-bit, load PC and SSP from memory

	SR_SET_S(M68k_Context_p->Status_Register);

	// The Stack Pointer A7 is read from memory address 0x0
	uint32_t TempVar;
	if( M68k_Context_p->Read_Memory( 0, &TempVar, 4) != 0)
	{
		//TODO something
	}
	M68k_Context_p->A[7] = ntohl( TempVar) & 0x00FFFFFF;

	// The Program Counter register is read from memory address 0x4
	if( M68k_Context_p->Read_Memory( 4, &TempVar, 4) != 0)
	{
		//TODO something
	}
	M68k_Context_p->Program_Counter = ntohl( TempVar) & 0x00FFFFFF;

	// STEP 2 : Enable interruptions and clear the T-bit
	SR_SET_INT(M68k_Context_p->Status_Register,7);
	SR_CLEAR_T(M68k_Context_p->Status_Register);
}


void M68k_Run_N_Ticks( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks_p)
{
	assert( M68k_Context_p != NULL);
	assert( N_Ticks_p != 0);

	while( *N_Ticks_p > 0)
	{
		if( M68k_Context_p->Interruptions != 0)
		{
			// check for exceptions to process first
			if( (M68k_Context_p->Interruptions & INT_BUS_ERROR) != 0)
			{
				printf( "Exception : Illegal bus address\n");

				CreateExceptionStackAndSetPC( M68k_Context_p, 0x008);

				// let's unmark the interruption and let the procedure do its work
				M68k_Context_p->Interruptions &= ~INT_BUS_ERROR;

				*N_Ticks_p -= (50 - 4);		// 50 includes time to fetch the next instruction, but we'll count that later so 46
			}

			if( (M68k_Context_p->Interruptions & INT_ADDRESS_ERROR) != 0)
			{
				printf( "Exception : Illegal unaligned address\n");

				CreateExceptionStackAndSetPC( M68k_Context_p, 0x00C);

				// let's unmark the interruption and let the procedure do its work
				M68k_Context_p->Interruptions &= ~INT_ADDRESS_ERROR;

				*N_Ticks_p -= (50 - 4);		// 50 includes time to fetch the next instruction, but we'll count that later so 46
			}

			if( (M68k_Context_p->Interruptions & INT_INTAUTOVEC) != 0)
			{
				int NumVector = M68k_Context_p->Read_Interrupt_Controler();

				if( NumVector > ((M68k_Context_p->Status_Register >> 8) & 7) || NumVector == 7)
				{
					//printf( "Exception : Interruption Autovector #%d\n", NumVector);

					CreateExceptionStackAndSetPC( M68k_Context_p, 0x060 + 4 * NumVector);

					// We modify SR to prevent lower interruptions from messing with us
					M68k_Context_p->Status_Register &= 0xF8FF;
					M68k_Context_p->Status_Register |= (NumVector << 8);

					*N_Ticks_p -= (44 - 4);		// 44 includes time to fetch the next instruction, but we'll count that later so 40
				}

				M68k_Context_p->Interruptions &= ~INT_INTAUTOVEC;
			}

			if( (M68k_Context_p->Interruptions & INT_ILLEGAL) != 0)
			{
				printf("Exception : bad opcode %#06x at address %#010x\n", M68k_Context_p->Current_Opcode, M68k_Context_p->Address_Opcode);
				//return N_Ticks_p;
			}

			if( (M68k_Context_p->Interruptions & INT_PRIVILEGE) != 0)
			{
				printf("Exception : privileged opcode\n");
				//return N_Ticks_p;
			}

			if( M68k_Context_p->Interruptions != 0)
				abort();

			if( *N_Ticks_p <= 0)		// because exception trapping is CPU expensive
				break;
		}

		uint16_t Opcode;
		M68k_Context_p->Address_Opcode = M68k_Context_p->Program_Counter;
		M68k_Context_p->Interruptions |= M68k_Context_p->Read_Memory( M68k_Context_p->Program_Counter, &Opcode, 2);
		M68k_Context_p->Program_Counter += 2;
		if( M68k_Context_p->Interruptions != 0)
			continue;

		M68k_Context_p->Current_Opcode = ntohs(Opcode);
		Instruction_Method Current_Instruction = Instruction_Lookup_Table[M68k_Context_p->Current_Opcode >> 6];

//#ifndef DEBUG
	//	printf( "PC %#010x / %010d : %#06x\n", M68k_Context_p->Address_Opcode, M68k_Context_p->Address_Opcode, M68k_Context_p->Current_Opcode);
//#endif // ! DEBUG

		if( Current_Instruction == NULL)
		{
			M68k_Context_p->Interruptions |= INT_ILLEGAL;
			continue;
		}
		Current_Instruction( M68k_Context_p, N_Ticks_p);
	}
}
