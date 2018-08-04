
#include "Utilities\Scan.h"
#include "Utilities\Instance.h"
#include "Static\StaticInclude.h"
#include "Static\StaticAddresses.h"
#include "OpEncryption\Translate.h"

DWORD r_DataModel;
DWORD r_ScriptContext;
_r_getstate r_getstate = (_r_getstate)(Lunar_Offset(LUNAR_IDABASE, LUNAR_RGETSTATE));

DWORD Lunar_Offset(DWORD SubValue, DWORD Address)
{
	static DWORD ProcessBase;
	if (!ProcessBase)
		ProcessBase = (DWORD)(GetModuleHandleA(NULL));

	return (Address - SubValue) + ProcessBase;
}

BOOL Lunar_OpenConsole()
{
	DWORD nOldProtect;
	if (!VirtualProtect(&FreeConsole, 1, PAGE_EXECUTE_READWRITE, &nOldProtect))
		return FALSE;
	*(BYTE*)&FreeConsole = 0xC3;
	if (!VirtualProtect(&FreeConsole, 1, nOldProtect, &nOldProtect))
		return FALSE;

	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	SetConsoleTitleA("Lunacy Debug Console");

	return TRUE;
}

VOID Lunar_Entrypoint()
{
	DWORD ScanFor = (Lunar_Offset(LUNAR_IDABASE, LUNAR_SCANFORSERVICE));

	Lunar_OpenConsole();
	Lunar_DebugPrint("Lunacy - Initiating...");
	r_DataModel = getParent(Memory::Scan(PAGE_READWRITE, (char*)&ScanFor, "xxxx"));
	r_ScriptContext = getChildFromClassName(r_DataModel, "ScriptContext");
	Lunar_DebugPrint(" OK\n");

	while (FindWindowW(NULL, L"ROBLOX"))
	{
		printf(">");
		std::string si;
		std::getline(std::cin, si);
		Lunar_RunScript(r_getstate(r_ScriptContext, 2), si.c_str());
	}
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID Reserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Lunar_Entrypoint, NULL, NULL, NULL);
	return TRUE;
}