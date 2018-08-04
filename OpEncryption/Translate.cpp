
#include "Translate.h"
#include "../Utilities/Retcheck.h"

#define LUAVM_DAX_MO 0x1451AFB// // ORIGINALLY MO, AO, ME
#define LUAVM_DAX_ME 0x1A7D575 // ORIGINALLY ME, MO, AO
#define LUAVM_DAX_AO 0x1C6B438 // ORIGINALLY AO, MO, ME

//#define LUAVM_DAX_ME 0x1A7D575 // ORIGINALLY MO, AO, ME
//#define LUAVM_DAX_AO 0x1C6B438 // ORIGINALLY ME, MO, AO
//#define LUAVM_DAX_MO 0x1451AFB // ORIGINALLY AO, MO, ME
//#define LUAVM_DAX_AE 0x62A0B4E3
#define LUAVM_ENCODEINSN(insn, key) ((insn) * key)
#define LUAVM_DECODEINSN(insn, key) ((insn).v * key)

Retcheck tret;
_r_lua_pcall r_lua_pcall = (_r_lua_pcall)tret.unprotect((BYTE*)(Lunar_Offset(LUNAR_IDABASE, LUNAR_RLUA_PCALL)));
_r_luaV_gettable r_luaV_gettable = (_r_luaV_gettable)(Lunar_Offset(LUNAR_IDABASE, LUNAR_RLUAV_GETTABLE));
_r_luaF_newlclosure r_luaF_newlclosure = (_r_luaF_newlclosure)(Lunar_Offset(LUNAR_IDABASE, LUNAR_RGETLCLOSURE));
_r_luaS_newlstr r_luaS_newlstr = (_r_luaS_newlstr)(Lunar_Offset(LUNAR_IDABASE, LUNAR_RLUAS_NEWLSTR));
_r_luaM_realloc_ r_luaM_realloc_ = (_r_luaM_realloc_)(Lunar_Offset(LUNAR_IDABASE, LUNAR_RLUAM_REALLOC));

enum ArgType
{
	t_iA,
	t_iAB,
	t_iAC,
	t_iABC,
	t_iABx,
	t_isBx,
	t_iAsBx
};

struct opcode_info
{
	UCHAR native;
	UCHAR rbx;
	ArgType t;
};

const opcode_info op_table[] =
{
	{ 0, 6, t_iAB }, // OP_MOVE
	{ 1, 4, t_iABx }, // OP_LOADK
	{ 2, 0, t_iABC }, // OP_LOADBOOL
	{ 3, 7, t_iAB }, // OP_LOADNIL
	{ 4, 2, t_iAB }, // OP_GETUPVAL
	{ 5, 8, t_iABx }, // OP_GETGLOBAL
	{ 6, 1, t_iABC }, // OP_GETTABLE
	{ 7, 3, t_iABx }, // OP_SETGLOBAL
	{ 8, 5, t_iAB }, // OP_SETUPVAL
	{ 9, 0xF, t_iABC }, // OP_SETTABLE
	{ 10, 0xD, t_iABC }, // OP_NEWTABLE
	{ 11, 9, t_iABC }, // OP_SELF
	{ 12, 0x10, t_iABC }, // OP_ADD
	{ 13, 0xB, t_iABC }, // OP_SUB
	{ 14, 0x11, t_iABC }, // OP_MUL
	{ 15, 0xA, t_iABC }, // OP_DIV
	{ 16, 0xC, t_iABC }, // OP_MOD
	{ 17, 0xE, t_iABC }, // OP_POW
	{ 18, 0x18, t_iAB }, // OP_UNM
	{ 19, 0x16, t_iAB }, // OP_NOT
	{ 20, 0x12, t_iAB }, // OP_LEN
	{ 21, 0x19, t_iABC }, // OP_CONCAT
	{ 22, 0x14, t_isBx }, // OP_JMP
	{ 23, 0x1A, t_iABC }, // OP_EQ
	{ 24, 0x13, t_iABC }, // OP_LT
	{ 25, 0x15, t_iABC }, // OP_LE
	{ 26, 0x17, t_iAC }, // OP_TEST
	{ 27, 0x21, t_iABC }, // OP_TESTSET
	{ 28, 0x1F, t_iABC }, // OP_CALL
	{ 29, 0x1B, t_iABC }, // OP_TAILCALL
	{ 30, 0x22, t_iAB }, // OP_RETURN
	{ 31, 0x1D, t_iAsBx }, // OP_FORLOOP
	{ 32, 0x23, t_iAsBx }, // OP_FORPREP
	{ 33, 0x1C, t_iAC }, // OP_TFORLOOP
	{ 34, 0x1E, t_iABC }, // OP_SETLIST
	{ 35, 0x20, t_iA }, // OP_CLOSE
	{ 36, 0x25, t_iABx }, // OP_CLOSURE
	{ 37, 0x24, t_iAB } // OP_VARARG
};

const opcode_info* Lunar_DaxGetOpcode(int op)
{
	for (int i = 0; i < 38; i++)
		if (op_table[i].native == op)
			return &op_table[i];
	return NULL;
}

const opcode_info* Lunar_DaxGetROpcode(int op)
{
	for (int i = 0; i < 38; i++)
		if (op_table[i].rbx == op)
			return &op_table[i];
	return NULL;
}

uint32_t Lunar_DaxEncodeOp(uint32_t x, uint32_t mulEven, uint32_t addEven, uint32_t mulOdd, uint32_t addOdd)
{
	uint32_t result = 0;
	uint32_t mask = 1;
	for (size_t i = 0; i < 8 * sizeof(uint32_t); ++i)
	{
		uint32_t bitDesired = mask & x;
		uint32_t bitOdd = mask & (result*mulOdd + addOdd);
		uint32_t bitEven = mask & (result*mulEven + addEven);
		if ((bitEven ^ bitOdd) != bitDesired)
		{
			result |= mask;
		}
		mask <<= 1;
	}
	return result;
}

uint32_t Lunar_DaxTranslateCode(unsigned int i, int pc, signed int key)
{
	Instruction enc = i;
	Instruction op = Lunar_DaxGetROpcode(R_GET_OPCODE(i))->native;
	switch (op) 
	{
	case OP_CALL:
	case OP_TAILCALL:
	case OP_RETURN:
	case OP_CLOSURE:
		enc = Lunar_DaxEncodeOp(i, LUAVM_DAX_ME, pc, LUAVM_DAX_MO, LUAVM_DAX_AO);
		R_SET_OPCODE(enc, R_GET_OPCODE(i));
		break;
	case OP_MOVE:
		R_SET_ARGC(enc, (pc | 1));
		break;
	default:
		break;
	}
	return LUAVM_ENCODEINSN(enc, key);
}

R_Proto* Lunar_NewProto(DWORD rL)
{
	R_Proto* f = (R_Proto*)(r_luaM_realloc_(rL, NULL, NULL, sizeof(R_Proto)));
	f->k = NULL;
	f->sizek = 0;
	f->p = NULL;
	f->sizep = 0;
	f->code = NULL;
	f->sizecode = 0;
	f->sizelineinfo = 0;
	f->sizeupvalues = 0;
	f->nups = 0;
	f->upvalues = NULL;
	f->numparams = 0;
	f->is_vararg = 0;
	f->maxstacksize = 0;
	f->lineinfo = NULL;
	f->sizelocvars = 0;
	f->locvars = NULL;
	f->linedefined = 0;
	f->lastlinedefined = 0;
	f->source = NULL;
	return f;
}

R_UpVal* Lunar_NewUpVal(DWORD rL)
{
	R_UpVal* uv = (R_UpVal*)r_luaM_realloc_(rL, NULL, NULL, sizeof(R_UpVal));
	ZeroMemory(uv->v, sizeof(TValue));
	return uv;
}

double Lunar_ConvertFloat(double From)
{
	DWORD* xorconst = (DWORD*)(Lunar_Offset(LUNAR_IDABASE, LUNAR_RLUAV_XORCONST));
	double newnumber;
	void* oldnumber = &From;
	__asm
	{
		mov edx, oldnumber
		mov eax, xorconst
		movsd xmm1, qword ptr[edx]
		xorpd xmm1, [eax]
		movsd newnumber, xmm1
	}
	return newnumber;
}

BOOL Lunar_ConvertTV(DWORD rL, TValue* From, TValue* To)
{
	switch (From->tt)
	{
	case LUA_TNIL:
		To->tt = R_LUA_TNIL;
		To->value.p = NULL;
		break;
	case LUA_TBOOLEAN:
		To->tt = R_LUA_TBOOLEAN;
		To->value.b = From->value.b;
		break;
	case LUA_TNUMBER:
	{
		double T = Lunar_ConvertFloat(From->value.n);
		To->tt = R_LUA_TNUMBER;
		To->value.n = Lunar_ConvertFloat(From->value.n);
		break;
	}
	case LUA_TSTRING:
	{
		TString* ts = (TString*)(From->value.gc);
		const char* str = (const char*)(ts + 1);
		To->tt = R_LUA_TSTRING;
		To->value.gc = (GCObject*)(r_luaS_newlstr(rL, str, ts->tsv.len));
		break;
	}
	}

	return TRUE;
}

DWORD Lunar_ConvertInstruction(DWORD* from_inst, DWORD original_inst, DWORD loc, signed int key)
{
	*from_inst = NULL;
	opcode_info opcode = op_table[N_GET_OPCODE(original_inst)];
	R_SET_OPCODE(*from_inst, opcode.rbx);
	switch (opcode.t)
	{
	case t_iABC:
		R_SET_ARGB(*from_inst, N_GET_ARGB(original_inst));
		R_SET_ARGC(*from_inst, N_GET_ARGC(original_inst));
		break;
	case t_iAB:
		R_SET_ARGB(*from_inst, N_GET_ARGB(original_inst));
		break;
	case t_iAC:
		R_SET_ARGC(*from_inst, N_GET_ARGC(original_inst));
		break;
	case t_iABx:
		R_SET_ARGBx(*from_inst, N_GET_ARGBx(original_inst));
		break;
	case t_iAsBx:
	case t_isBx:
		R_SET_ARGsBx(*from_inst, N_GET_ARGsBx(original_inst));
		break;
	case t_iA:
	default:
		break;
	}
	return Lunar_DaxTranslateCode(*from_inst, loc, key);
}

const char* Lunar_StringFromTS(TString* ts)
{
	return (const char*)(ts + 1);
}

// Big Time

R_Proto* Lunar_Convert(DWORD rL, Proto* p)
{
	R_Proto* fP = Lunar_NewProto(rL);
	fP->nups = p->nups;
	fP->numparams = p->numparams;
	fP->maxstacksize = p->maxstacksize;
	fP->sizelineinfo = p->sizelineinfo;
	fP->sizecode = p->sizecode;
	fP->sizep = p->sizep;
	fP->sizek = p->sizek;
	const char* fnSource = Lunar_StringFromTS(p->source);
	r_ptr_obfus(((DWORD)(fP)) + 12, r_luaS_newlstr(rL, fnSource, p->source->tsv.len));
	signed int key = rL + *(DWORD*)(rL + 8) + 36;
	signed int fKey = *(signed int*)key + key;

	DWORD* CodeVector = (DWORD*)(r_luaM_realloc_(rL, NULL, NULL, p->sizecode * 4));
	r_ptr_obfus(((DWORD)(fP)) + 24, CodeVector);
	for (int i = 0; i < p->sizecode; i++)
		CodeVector[i] = Lunar_ConvertInstruction(&CodeVector[i], p->code[i], i, fKey);

	DWORD* LineInfoVector = (DWORD*)(r_luaM_realloc_(rL, NULL, NULL, p->sizelineinfo * 4));
	r_ptr_obfus(((DWORD)(fP)) + 36, LineInfoVector);
	for (int i = 0; i < p->sizelineinfo; i++)
		LineInfoVector[i] = p->lineinfo[i] ^ (i << 8);

	TValue* ConstantVector = (TValue*)(r_luaM_realloc_(rL, NULL, NULL, p->sizek * sizeof(TValue)));
	r_ptr_obfus(((DWORD)(fP)) + 32, ConstantVector);
	for (int i = 0; i < p->sizek; i++)
		Lunar_ConvertTV(rL, &p->k[i], &ConstantVector[i]);

	if (p->sizep)
	{
		DWORD* ProtoVector = (DWORD*)(r_luaM_realloc_(rL, NULL, NULL, p->sizep * 4));
		r_ptr_obfus(((DWORD)(fP)) + 8, ProtoVector);
		for (int i = 0; i < p->sizep; i++)
			ProtoVector[i] = (DWORD)(Lunar_Convert(rL, p->p[i]));
	}

	return fP;
}

DWORD Lunar_CreateFunction(DWORD rL, LClosure* FromFn)
{
	R_Proto* ffnP = Lunar_Convert(rL, FromFn->p);
	DWORD ffnC = r_luaF_newlclosure(rL, FromFn->nupvalues, *(DWORD*)(rL + 104));
	r_ptr_obfus(ffnC + 16, ffnP);

	if (FromFn->nupvalues)
	{
		DWORD Counter = 0;
		DWORD NupVector = ffnC + 20;
		do
		{
			*(DWORD*)NupVector = (DWORD)(Lunar_NewUpVal(rL));
			++Counter;
			NupVector += 4;
		} while (Counter < FromFn->nupvalues);
	}

	return ffnC;
}

void Lunar_RunAsync(DWORD rL, DWORD Function)
{
	TValue* spwn;
	TValue t_kidx;

	/*spwn = (TValue*)(*(DWORD*)(rL + 16));
	t_kidx.tt = R_LUA_TSTRING;
	t_kidx.value.gc = (GCObject*)(r_luaS_newlstr(rL, "spawn", strlen("spawn")));
	r_luaV_gettable(rL, (StkId)(rL + 104), &t_kidx, spwn);
	*(DWORD*)(rL + 16) += 16;*/
	

	DWORD top = *(DWORD*)(rL + 16);
	*(DWORD*)(top) = Function;
	*(DWORD*)(top + 8) = R_LUA_TFUNCTION;
	*(DWORD*)(rL + 16) += 16;

	Lunar_DebugPrint("test: %x\n", top);
	int resp = r_lua_pcall(rL, 0, 0, 0);
	Lunar_DebugPrint("Pcall Response: %d\n", resp);
}