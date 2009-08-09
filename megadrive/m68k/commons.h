#ifndef COMMONS_H_
#define COMMONS_H_


#include <stddef.h>
#include <arpa/inet.h>
#include <stdint.h>


struct M68k_Context;

typedef uint8_t (*Instruction_Init) ( struct M68k_Context*);
typedef void (*Instruction_Method) ( struct M68k_Context*, int32_t*);
typedef uint32_t (*Memory_Map_Function) (uint32_t,void*,uint16_t);
typedef uint8_t (*Int_Ctrl_Function) ();


// exceptions ... TRAP # stored in the lowest byte
#define INT_RESET			0x8000
#define INT_ADDRESS_ERROR	0x4000
#define INT_BUS_ERROR		0x2000

#define INT_TRACE			0x1000
#define INT_INTAUTOVEC		0x0800
#define	INT_ILLEGAL			0x0400
#define INT_PRIVILEGE		0x0200

#define INT_TRAP			0x0100
#define INT_TRAPV			0x0080
#define INT_CHK				0x0040
#define INT_ZERO_DIVIDE		0x0020

void SwitchToSuperUser( struct M68k_Context* M68k_Context_p);
void CreateExceptionStackAndSetPC( struct M68k_Context* M68k_Context_p, uint32_t VectorAddress);


struct M68k_Context
{
	// registers
	uint32_t D[8];
	uint32_t A[8];
	uint32_t Saved_Stack_Pointer;		// the unused one between USP and SSP
	uint32_t Program_Counter;
	uint16_t Status_Register;

	// main memory
	Memory_Map_Function Read_Memory;
	Memory_Map_Function Write_Memory;

	// current instruction
	uint16_t Current_Opcode;
	uint32_t Address_Opcode;

	// bitmask of interruptions
	uint16_t Interruptions;
	Int_Ctrl_Function Read_Interrupt_Controler;
};


// Condition Code Register macros, to apply on unsigned short
#define CCR_CHECK_C(Dest) (((Dest) & 0x0001) != 0)
#define CCR_CHECK_V(Dest) (((Dest) & 0x0002) != 0)
#define CCR_CHECK_Z(Dest) (((Dest) & 0x0004) != 0)
#define CCR_CHECK_N(Dest) (((Dest) & 0x0008) != 0)
#define CCR_CHECK_X(Dest) (((Dest) & 0x0010) != 0)
#define CCR_CHECK_OR_ZC(Dest) (((Dest) & 0x0005) != 0)

#define CCR_CLEAR_Z(Dest) ((Dest) &= 0xFFFB)
#define CCR_CLEAR_NZ(Dest) ((Dest) &= 0xFFF3)
#define CCR_CLEAR_NZVC(Dest) ((Dest) &= 0xFFF0)
#define CCR_CLEAR_X(Dest) ((Dest) &= 0xFFEF)
#define CCR_CLEAR_XC(Dest) ((Dest) &= 0xFFEE)
#define CCR_CLEAR_XNVC(Dest) ((Dest) &= 0xFFEB)
#define CCR_CLEAR_XNZVC(Dest) ((Dest) &= 0xFFE0)

#define CCR_SET_C(Dest) ((Dest) |= 0x0001)
#define CCR_SET_V(Dest) ((Dest) |= 0x0002)
#define CCR_SET_Z(Dest) ((Dest) |= 0x0004)
#define CCR_SET_N(Dest) ((Dest) |= 0x0008)
#define CCR_SET_XC(Dest) ((Dest) |= 0x0011)

// Supervisor Register macros, to apply on unsigned short
#define SR_CHECK_S(Dest) ((Dest) & 0x2000)

#define SR_CLEAR_T(Dest) ((Dest) &= 0xBFFF)
#define SR_CLEAR_S(Dest) ((Dest) &= 0xDFFF)

#define SR_SET_T(Dest) ((Dest) |= 0x4000)
#define SR_SET_S(Dest) ((Dest) |= 0x2000)

#define SR_SET_INT(Dest,Val) ( (Dest) = ((Dest) & 0xF0FF) | (Val))

// Conditions testing
uint8_t (*CC_Function[16]) ( uint16_t);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Effective Address computation and operand retrieval
#define REPEAT8(Dest) &(Dest), &(Dest), &(Dest), &(Dest), &(Dest), &(Dest), &(Dest), &(Dest),

typedef uint32_t (*EA_Compute_Function) (struct M68k_Context*, int32_t*, uint8_t); // context, ticks, reg #
EA_Compute_Function EA_Compute_Table[64];
uint32_t EA_Comp_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Comp_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Comp_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Comp_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Comp_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Comp_PC_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Comp_PC_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);


// 8 bits
typedef uint8_t (*EA_Read_B_Function) (struct M68k_Context*, int32_t*, uint8_t); // context, ticks, reg #
EA_Read_B_Function EA_Read_B_Table[64];
uint8_t EA_Read_B_Dn( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint8_t EA_Read_B_An( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint8_t EA_Read_B_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint8_t EA_Read_B_An_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint8_t EA_Read_B_An_PreDec( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint8_t EA_Read_B_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint8_t EA_Read_B_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint8_t EA_Read_B_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint8_t EA_Read_B_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint8_t EA_Read_B_PC_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint8_t EA_Read_B_Imm( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);

typedef void (*EA_Write_B_Function) (struct M68k_Context*, int32_t*, uint8_t, uint8_t); // context, ticks, reg #, value
EA_Write_B_Function EA_Write_B_Table[64];
void EA_Write_B_Dn( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value);
void EA_Write_B_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value);
void EA_Write_B_An_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value);
void EA_Write_B_An_PreDec( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value);
void EA_Write_B_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value);
void EA_Write_B_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value);
void EA_Write_B_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value);
void EA_Write_B_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint8_t Value);

// 16 bits
typedef uint16_t (*EA_Read_W_Function) (struct M68k_Context*, int32_t*, uint8_t); // context, ticks, reg #
EA_Read_W_Function EA_Read_W_Table[64];
uint16_t EA_Read_W_Dn( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint16_t EA_Read_W_An( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint16_t EA_Read_W_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint16_t EA_Read_W_An_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint16_t EA_Read_W_An_PreDec( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint16_t EA_Read_W_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint16_t EA_Read_W_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint16_t EA_Read_W_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint16_t EA_Read_W_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint16_t EA_Read_W_PC_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint16_t EA_Read_W_PC_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint16_t EA_Read_W_Imm( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);

typedef void (*EA_Write_W_Function) (struct M68k_Context*, int32_t*, uint8_t, uint16_t); // context, ticks, reg #, value
EA_Write_W_Function EA_Write_W_Table[64];
void EA_Write_W_Dn( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value);
void EA_Write_W_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value);
void EA_Write_W_An_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value);
void EA_Write_W_An_PreDec( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value);
void EA_Write_W_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value);
void EA_Write_W_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value);
void EA_Write_W_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value);
void EA_Write_W_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint16_t Value);

// 32 bits
typedef uint32_t (*EA_Read_L_Function) (struct M68k_Context*, int32_t*, uint8_t); // context, ticks, reg #
EA_Read_L_Function EA_Read_L_Table[64];
uint32_t EA_Read_L_Dn( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Read_L_An( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Read_L_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Read_L_An_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Read_L_An_PreDec( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Read_L_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Read_L_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Read_L_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Read_L_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Read_L_PC_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Read_L_PC_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);
uint32_t EA_Read_L_Imm( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number);

typedef void (*EA_Write_L_Function) (struct M68k_Context*, int32_t*, uint8_t, uint32_t); // context, ticks, reg #, value
EA_Write_L_Function EA_Write_L_Table[64];
void EA_Write_L_Dn( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value);
void EA_Write_L_An( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value);
void EA_Write_L_An_Ind( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value);
void EA_Write_L_An_PostInc( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value);
void EA_Write_L_An_PreDec( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value);
void EA_Write_L_An_Disp( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value);
void EA_Write_L_An_Reg_Idx( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value);
void EA_Write_L_Abs_S( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value);
void EA_Write_L_Abs_L( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint8_t Reg_Number, uint32_t Value);


uint8_t EA_Read_B_KeepIntact( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode);
uint16_t EA_Read_W_KeepIntact( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode);
uint32_t EA_Read_L_KeepIntact( struct M68k_Context* M68k_Context_p, int32_t* N_Ticks, uint16_t EA_Mode);

#endif /*COMMONS_H_*/
