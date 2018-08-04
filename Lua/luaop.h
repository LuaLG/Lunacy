#pragma once

using Instruction = LUAI_UINT32;
enum OpMode { iABC, iABx, iAsBx };

#ifndef cast
#define cast(t, exp)    ((t)(exp))
#endif

#define cast_byte(i)    cast(lu_byte, (i))
#define cast_num(i)     cast(lua_Number, (i))
#define cast_int(i)     cast(int, (i))

#define SIZE_C          9
#define SIZE_B          9
#define SIZE_Bx         (SIZE_C + SIZE_B)
#define SIZE_A          8
#define SIZE_OP         6
#define POS_OP          0
#define POS_A           (POS_OP + SIZE_OP)
#define POS_C           (POS_A + SIZE_A)
#define POS_B           (POS_C + SIZE_C)
#define POS_Bx          POS_C

#if SIZE_Bx < LUAI_BITSINT-1
#define MAXARG_Bx        ((1<<SIZE_Bx)-1)
#define MAXARG_sBx        (MAXARG_Bx>>1)
#else
#define MAXARG_Bx        MAX_INT
#define MAXARG_sBx        MAX_INT
#endif

#define MAXARG_A        ((1<<SIZE_A)-1)
#define MAXARG_B        ((1<<SIZE_B)-1)
#define MAXARG_C        ((1<<SIZE_C)-1)

#define MASK1(n,p)      ((~((~(Instruction)0)<<n))<<p)
#define MASK0(n,p)      (~MASK1(n,p))

#define GET_OPCODE(i)   (cast(OpCode, ((i)>>POS_OP) & MASK1(SIZE_OP,0)))
#define SET_OPCODE(i,o) ((i) = (((i)&MASK0(SIZE_OP,POS_OP)) | \
                ((cast(Instruction, o)<<POS_OP)&MASK1(SIZE_OP,POS_OP))))

#define GETARG_A(i)     (cast(int, ((i)>>POS_A) & MASK1(SIZE_A,0)))
#define SETARG_A(i,u)   ((i) = (((i)&MASK0(SIZE_A,POS_A)) | \
                ((cast(Instruction, u)<<POS_A)&MASK1(SIZE_A,POS_A))))

#define GETARG_B(i)     (cast(int, ((i)>>POS_B) & MASK1(SIZE_B,0)))
#define SETARG_B(i,b)   ((i) = (((i)&MASK0(SIZE_B,POS_B)) | \
                ((cast(Instruction, b)<<POS_B)&MASK1(SIZE_B,POS_B))))

#define GETARG_C(i)     (cast(int, ((i)>>POS_C) & MASK1(SIZE_C,0)))
#define SETARG_C(i,b)   ((i) = (((i)&MASK0(SIZE_C,POS_C)) | \
                ((cast(Instruction, b)<<POS_C)&MASK1(SIZE_C,POS_C))))

#define GETARG_Bx(i)    (cast(int, ((i)>>POS_Bx) & MASK1(SIZE_Bx,0)))
#define SETARG_Bx(i,b)  ((i) = (((i)&MASK0(SIZE_Bx,POS_Bx)) | \
                ((cast(Instruction, b)<<POS_Bx)&MASK1(SIZE_Bx,POS_Bx))))

#define GETARG_sBx(i)   (GETARG_Bx(i)-MAXARG_sBx)
#define SETARG_sBx(i,b) SETARG_Bx((i),cast(unsigned int, (b)+MAXARG_sBx))

#define CREATE_ABC(o,a,b,c)     ((cast(Instruction, o)<<POS_OP) \
                        | (cast(Instruction, a)<<POS_A) \
                        | (cast(Instruction, b)<<POS_B) \
                        | (cast(Instruction, c)<<POS_C))

#define CREATE_ABx(o,a,bc)      ((cast(Instruction, o)<<POS_OP) \
                        | (cast(Instruction, a)<<POS_A) \
                        | (cast(Instruction, bc)<<POS_Bx))

#define BITRK           (1 << (SIZE_B - 1))
#define ISK(x)          ((x) & BITRK)
#define INDEXK(r)       ((int)(r) & ~BITRK)
#define MAXINDEXRK      (BITRK - 1)
#define RKASK(x)        ((x) | BITRK)
#define NO_REG          MAXARG_A

#define LUAVM_SHUFFLE_COMMA ,
typedef enum {
	/*----------------------------------------------------------------------
	name		args	description
	------------------------------------------------------------------------*/
	LUAVM_SHUFFLE9(LUAVM_SHUFFLE_COMMA,
	rOP_MOVE,/*	A B	R(A) := R(B)					*/
	rOP_LOADK,/*	A Bx	R(A) := Kst(Bx)					*/
	rOP_LOADBOOL,/*	A B C	R(A) := (Bool)B; if (C) pc++			*/
	rOP_LOADNIL,/*	A B	R(A) := ... := R(B) := nil			*/
	rOP_GETUPVAL,/*	A B	R(A) := UpValue[B]				*/

	rOP_GETGLOBAL,/*	A Bx	R(A) := Gbl[Kst(Bx)]				*/
	rOP_GETTABLE,/*	A B C	R(A) := R(B)[RK(C)]				*/

	rOP_SETGLOBAL,/*	A Bx	Gbl[Kst(Bx)] := R(A)				*/
	rOP_SETUPVAL),/*	A B	UpValue[B] := R(A)				*/

	LUAVM_SHUFFLE9(LUAVM_SHUFFLE_COMMA,
	rOP_SETTABLE,/*	A B C	R(A)[RK(B)] := RK(C)				*/

	rOP_NEWTABLE,/*	A B C	R(A) := {} (size = B,C)				*/

	rOP_SELF,/*	A B C	R(A+1) := R(B); R(A) := R(B)[RK(C)]		*/

	rOP_ADD,/*	A B C	R(A) := RK(B) + RK(C)				*/
	rOP_SUB,/*	A B C	R(A) := RK(B) - RK(C)				*/
	rOP_MUL,/*	A B C	R(A) := RK(B) * RK(C)				*/
	rOP_DIV,/*	A B C	R(A) := RK(B) / RK(C)				*/
	rOP_MOD,/*	A B C	R(A) := RK(B) % RK(C)				*/
	rOP_POW),/*	A B C	R(A) := RK(B) ^ RK(C)				*/

	LUAVM_SHUFFLE9(LUAVM_SHUFFLE_COMMA,
	rOP_UNM,/*	A B	R(A) := -R(B)					*/
	rOP_NOT,/*	A B	R(A) := not R(B)				*/
	rOP_LEN,/*	A B	R(A) := length of R(B)				*/

	rOP_CONCAT,/*	A B C	R(A) := R(B).. ... ..R(C)			*/

	rOP_JMP,/*	sBx	pc+=sBx					*/

	rOP_EQ,/*	A B C	if ((RK(B) == RK(C)) ~= A) then pc++		*/
	rOP_LT,/*	A B C	if ((RK(B) <  RK(C)) ~= A) then pc++  		*/
	rOP_LE,/*	A B C	if ((RK(B) <= RK(C)) ~= A) then pc++  		*/
	rOP_TEST),/*	A C	if not (R(A) <=> C) then pc++			*/

	LUAVM_SHUFFLE9(LUAVM_SHUFFLE_COMMA,
	rOP_TESTSET,/*	A B C	if (R(B) <=> C) then R(A) := R(B) else pc++	*/

	rOP_CALL,/*	A B C	R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1)) */
	rOP_TAILCALL,/*	A B C	return R(A)(R(A+1), ... ,R(A+B-1))		*/
	rOP_RETURN,/*	A B	return R(A), ... ,R(A+B-2)	(see note)	*/

	rOP_FORLOOP,/*	A sBx	R(A)+=R(A+2);
			   if R(A) <?= R(A+1) then { pc+=sBx; R(A+3)=R(A) }*/
		rOP_FORPREP,/*	A sBx	R(A)-=R(A+2); pc+=sBx				*/

		rOP_TFORLOOP,/*	A C	R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2));
					if R(A+3) ~= nil then R(A+2)=R(A+3) else pc++	*/
		rOP_SETLIST,/*	A B C	R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B	*/
		rOP_CLOSE),/*	A 	close all variables in the stack up to (>=) R(A)*/

		LUAVM_SHUFFLE2(LUAVM_SHUFFLE_COMMA,
		rOP_CLOSURE,/*	A Bx	R(A) := closure(KPROTO[Bx], R(A), ... ,R(A+n))	*/

		rOP_VARARG),/*	A B	R(A), R(A+1), ..., R(A+B-1) = vararg		*/

		rOP___COUNT
} rOpCode;

typedef enum {
	/*----------------------------------------------------------------------
	name            args    description
	------------------------------------------------------------------------*/
	OP_MOVE,/*      A B     R(A) := R(B)                                    */
	OP_LOADK,/*     A Bx    R(A) := Kst(Bx)                                 */
	OP_LOADBOOL,/*  A B C   R(A) := (Bool)B; if (C) pc++                    */
	OP_LOADNIL,/*   A B     R(A) := ... := R(B) := nil                      */
	OP_GETUPVAL,/*  A B     R(A) := UpValue[B]                              */

	OP_GETGLOBAL,/* A Bx    R(A) := Gbl[Kst(Bx)]                            */
	OP_GETTABLE,/*  A B C   R(A) := R(B)[RK(C)]                             */

	OP_SETGLOBAL,/* A Bx    Gbl[Kst(Bx)] := R(A)                            */
	OP_SETUPVAL,/*  A B     UpValue[B] := R(A)                              */
	OP_SETTABLE,/*  A B C   R(A)[RK(B)] := RK(C)                            */

	OP_NEWTABLE,/*  A B C   R(A) := {} (size = B,C)                         */

	OP_SELF,/*      A B C   R(A+1) := R(B); R(A) := R(B)[RK(C)]             */

	OP_ADD,/*       A B C   R(A) := RK(B) + RK(C)                           */
	OP_SUB,/*       A B C   R(A) := RK(B) - RK(C)                           */
	OP_MUL,/*       A B C   R(A) := RK(B) * RK(C)                           */
	OP_DIV,/*       A B C   R(A) := RK(B) / RK(C)                           */
	OP_MOD,/*       A B C   R(A) := RK(B) % RK(C)                           */
	OP_POW,/*       A B C   R(A) := RK(B) ^ RK(C)                           */
	OP_UNM,/*       A B     R(A) := -R(B)                                   */
	OP_NOT,/*       A B     R(A) := not R(B)                                */
	OP_LEN,/*       A B     R(A) := length of R(B)                          */

	OP_CONCAT,/*    A B C   R(A) := R(B).. ... ..R(C)                       */

	OP_JMP,/*       sBx     pc+=sBx                                 */

	OP_EQ,/*        A B C   if ((RK(B) == RK(C)) ~= A) then pc++            */
	OP_LT,/*        A B C   if ((RK(B) <  RK(C)) ~= A) then pc++            */
	OP_LE,/*        A B C   if ((RK(B) <= RK(C)) ~= A) then pc++            */

	OP_TEST,/*      A C     if not (R(A) <=> C) then pc++                   */
	OP_TESTSET,/*   A B C   if (R(B) <=> C) then R(A) := R(B) else pc++     */

	OP_CALL,/*      A B C   R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1)) */
	OP_TAILCALL,/*  A B C   return R(A)(R(A+1), ... ,R(A+B-1))              */
	OP_RETURN,/*    A B     return R(A), ... ,R(A+B-2)      (see note)      */

	OP_FORLOOP,/*   A sBx   R(A)+=R(A+2);
			   if R(A) <?= R(A+1) then { pc+=sBx; R(A+3)=R(A) }*/
	OP_FORPREP,/*   A sBx   R(A)-=R(A+2); pc+=sBx                           */

	OP_TFORLOOP,/*  A C     R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2));
				if R(A+3) ~= nil then R(A+2)=R(A+3) else pc++   */
	OP_SETLIST,/*   A B C   R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B        */

	OP_CLOSE,/*     A       close all variables in the stack up to (>=) R(A)*/
	OP_CLOSURE,/*   A Bx    R(A) := closure(KPROTO[Bx], R(A), ... ,R(A+n))  */

	OP_VARARG/*     A B     R(A), R(A+1), ..., R(A+B-1) = vararg            */
} OpCode;


#define NUM_OPCODES     (cast(int, OP_VARARG) + 1)

enum OpArgMask
{
	OpArgN,  /* argument is not used */
	OpArgU,  /* argument is used */
	OpArgR,  /* argument is a register or a jump offset */
	OpArgK   /* argument is a constant or register/constant */
};

#define getOpMode(m)    (cast(enum OpMode, luaP_opmodes[m] & 3))
#define getBMode(m)     (cast(enum OpArgMask, (luaP_opmodes[m] >> 4) & 3))
#define getCMode(m)     (cast(enum OpArgMask, (luaP_opmodes[m] >> 2) & 3))
#define testAMode(m)    (luaP_opmodes[m] & (1 << 6))
#define testTMode(m)    (luaP_opmodes[m] & (1 << 7))

#define N_SET_OPCODE(i,o) i |= o
#define N_GET_OPCODE(i) (i&0x3F)
#define R_SET_OPCODE(i,o) i = (i&0x3FFFFFF)|(o<<26)
#define R_GET_OPCODE(i) (i&0xFC000000)>>26

#define N_SET_ARGA(i,o) i = (i&0xFFFFC03F)|(o<<6)
#define N_GET_ARGA(i) (i&0x3FC0)>>6
#define R_SET_ARGA(i,o) i = (i&0xFC03FFFF)|(o<<18)
#define R_GET_ARGA(i)  (i&0x3FC0000)>>18

#define N_SET_ARGB(i,o) i = (i&0x7FFFFF)|(o<<23)
#define N_GET_ARGB(i) (i>>23)
#define R_SET_ARGB(i,o) i |= o
#define R_GET_ARGB(i) (i&0x1FF)

#define N_SET_ARGC(i,o) i = (i&0xFF803FFF)|(o<<14)
#define N_GET_ARGC(i) (i&0x7FC000)>>14
#define R_SET_ARGC(i,o) i = (i&0xFFFC01FF)|(o<<9)
#define R_GET_ARGC(i) (i&0x3FE00)>>9

#define N_SET_ARGBx(i,o) i = (i&0xFC03FFFF)|((o)<<14)
#define N_GET_ARGBx(i) (i>>14)
#define R_SET_ARGBx(i,o) i |= o
#define R_GET_ARGBx(i) (i&0x3FFFF)

#define N_SET_ARGsBx(i,o) N_SET_ARGBx(i,o+131071)
#define N_GET_ARGsBx(i) N_GET_ARGBx(i) - 131071
#define R_SET_ARGsBx(i,o) R_SET_ARGBx(i,o+131071)
#define R_GET_ARGsBx(i) R_GET_ARGBx(i) - 131071