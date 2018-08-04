#pragma once
#pragma warning(disable:4996)
#pragma warning(disable:4098)

#include <Windows.h>
#include <stdint.h>
#include <iostream>
#include <string>

#include "../Lua/lua.hpp"
#include "../Lua/luadefs.h"
#include "../Lua/luaop.h"

#define Lunar_Debug

#ifdef Lunar_Debug
#define Lunar_DebugPrint printf
#else
#define Lunar_DebugPrint __noop
#endif

DWORD Lunar_Offset(DWORD SubValue, DWORD Address);
BOOL Lunar_RunScript(DWORD rL, const char* script);