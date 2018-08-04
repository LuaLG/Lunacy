#pragma once

#include <iostream>
#include <Windows.h>
#include <vector>

struct scan_item {
	bool success = false;

	DWORD offsetreg = NULL;
	DWORD protection = NULL;
	DWORD* result = nullptr;

	PBYTE scanbytes;
	std::string mask = "";
};

class scanner {
private:
	std::vector<scan_item*> item_queue;

	DWORD regionbytepattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char *szMask);
	bool comparebytes(const BYTE *pData, const BYTE *bMask, const char *szMask);
	std::vector<std::string> split(std::string s, char delim);
	int fromhex(std::string);

public:

	void queuescan(PBYTE scanbytes, std::string mask = "", DWORD* result = nullptr, DWORD offsetreg = NULL, DWORD protection = PAGE_EXECUTE_READ);
	void queueidastylescan(std::string scanbytes, DWORD* result = nullptr, DWORD offsetreg = NULL, DWORD protection = PAGE_EXECUTE_READ);
	int scanregion(DWORD region = 0, DWORD regionsize = 0x7FFFFFFF);
};