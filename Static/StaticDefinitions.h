#pragma once

#include "StaticInclude.h"

#define r_ptr_obfus(p, v) *(DWORD*)(p) = (DWORD)v - (DWORD)(p)
#define r_ptr_deobf(p) *(DWORD*)(p) + (DWORD)(p)

#define R_LUA_TNIL 0
#define R_LUA_TLIGHTUSERDATA 1
#define R_LUA_TNUMBER 2
#define R_LUA_TBOOLEAN 3
#define R_LUA_TSTRING 4
#define R_LUA_TTHREAD 5
#define R_LUA_TFUNCTION 6
#define R_LUA_TTABLE 7
#define R_LUA_TUSERDATA 8
#define R_LUA_TPROTO 9
#define R_LUA_TUPVALUE 10

typedef int(__thiscall* _r_getstate)(DWORD ScriptContext, int Identity);
typedef int(__cdecl* _r_lua_newthread)(DWORD rL);
typedef int(__cdecl* _r_luaV_gettable)(DWORD rL, TValue* A, TValue* B, TValue* C);
typedef int(__cdecl* _r_luaV_settable)(DWORD rL, TValue* A, TValue* B, TValue* C);
typedef DWORD(__cdecl* _r_luaS_newlstr)(DWORD rL, const char* str, size_t B);
typedef signed int(__cdecl* _r_luaD_precall)(DWORD rL, TValue* A, DWORD B);
typedef void*(__cdecl* _r_luaM_realloc_)(DWORD rL, void *block, size_t osize, size_t nsize);
typedef DWORD(__cdecl* _r_luaF_newcclosure)(DWORD rL, DWORD A, DWORD B);
typedef int(__cdecl* _r_lua_pushcclosure)(DWORD rL, void* fn, int nups);
typedef void*(__cdecl* _r_lua_touserdata)(DWORD rL, int idx);
typedef int(__cdecl* _r_lua_pcall)(DWORD rL, int args, int nresults, int handler);
typedef int(__cdecl* _r_luaF_newlclosure)(DWORD rL, DWORD nups, DWORD env);
