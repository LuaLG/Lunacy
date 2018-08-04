
#include "../Static/StaticInclude.h"
#include "../Static/StaticDefinitions.h"
#include "../OpEncryption/Translate.h"
#include "../Utilities/Retcheck.h"

Retcheck ret;
_r_lua_newthread r_lua_newthread = (_r_lua_newthread)ret.unprotect((BYTE*)Lunar_Offset(LUNAR_IDABASE, LUNAR_RLUA_NEWTHREAD));

void Lunar_Push(DWORD rL, DWORD Function)
{
	DWORD top = *(DWORD*)(rL + 16);
	*(DWORD*)(top) = Function;
	*(DWORD*)(top + 8) = R_LUA_TFUNCTION;
	*(DWORD*)(rL + 16) += 16;
}

LONG Lunar_CatchException(PEXCEPTION_POINTERS T)
{
	Lunar_DebugPrint("[!] An error occured!\n-> Error code: %d\nEIP: %x\nReturn address: %x\n", GetLastError(), T->ContextRecord->Eip, *(DWORD*)(T->ContextRecord->Ebp + 4));
	return EXCEPTION_EXECUTE_HANDLER;
}

BOOL Lunar_RunScript(DWORD rL, const char* script)
{
	static lua_State* L;
	if (!L)
		L = luaL_newstate();

	DWORD tL = r_lua_newthread(rL);
	//DWORD fKey_ptr = rL + *(DWORD*)(rL + 8) + 36;
	//signed int fKey = *(signed int*)(fKey_ptr + fKey_ptr);

	if (luaL_loadstring(L, script))
	{
		Lunar_DebugPrint("[!] Branch to failure\n");
		return FALSE;
	}
	else
	{
		Lunar_DebugPrint("[!] Branch to success\n");
		LClosure* LFn = (LClosure*)(L->top - 1)->value.gc;
		__try
		{
			DWORD RFn = Lunar_CreateFunction(tL, LFn);
			Lunar_DebugPrint("[#] Executing...\n\n");
			Lunar_RunAsync(rL, RFn);
		}
		__except (Lunar_CatchException(GetExceptionInformation()))
		{
			Lunar_DebugPrint("[#] :(\n\n");
		}

	}
}