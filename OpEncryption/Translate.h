#pragma once

#include "../Static/StaticInclude.h"
#include "../Static/StaticDefinitions.h"
#include "../Static/StaticAddresses.h"

uint32_t Lunar_DaxEncodeOp(uint32_t x, uint32_t mulEven, uint32_t addEven, uint32_t mulOdd, uint32_t addOdd);
R_Proto* Lunar_Convert(DWORD rL, Proto* p);
DWORD Lunar_CreateFunction(DWORD rL, LClosure* FromFn);
void Lunar_Push(DWORD rL, DWORD Function);
void Lunar_RunAsync(DWORD rL, DWORD Function);