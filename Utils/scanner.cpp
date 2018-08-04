#include "scanner.h"
#include <sstream>

std::vector<std::string> scanner::split(std::string s, char delim) {
	std::vector<std::string> ret;
	std::istringstream iis;
	iis.clear();
	iis.str(s);
	std::string temp;
	while (std::getline(iis, temp, delim))
		ret.push_back(temp);
	return ret;
}


int scanner::fromhex(std::string s) {
	char* p;
	long n = strtol(s.c_str(), &p, 16);
	if (*p != 0)
		throw std::exception("unable to convert hex to decimal");
	return (int)(n);
}

bool scanner::comparebytes(const BYTE *pData, const BYTE *bMask, const char *szMask) {
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask) return 0;
	return (*szMask) == NULL;
}

DWORD scanner::regionbytepattern(DWORD dwAddress, DWORD dwLen, BYTE* bMask, char* szMask) {
	for (int i = 0; i < (int)(dwLen); i++)
		if (comparebytes((BYTE*)(dwAddress + (int)i), bMask, szMask))
			return (int)(dwAddress + i);
	return 0;
}

void scanner::queuescan(PBYTE scanbytes, std::string mask, DWORD* result, DWORD offsetreg, DWORD protection) {
	scan_item* item = new scan_item;
	ZeroMemory(item, sizeof(scan_item));

	item->scanbytes = scanbytes;
	item->mask = mask;
	item->result = result;
	item->offsetreg = offsetreg;
	item->protection = protection;
	item_queue.push_back(item);

}

void scanner::queueidastylescan(std::string scanbytes, DWORD* result, DWORD offsetreg, DWORD protection) {
	std::vector<std::string> bytes = split(scanbytes, ' ');
	std::string mask = "", aob = "";

	for (size_t c = 0; c < bytes.size(); c++)
		mask += bytes[c].substr(0, 1) == "?" ? "?" : "x";

	for (size_t c = 0; c < bytes.size(); c++)
		aob += (char)std::strtol(bytes[c].c_str(), NULL, 16);

	printf("[[%s]] (%s)\n", aob.c_str(), mask.c_str());

	scan_item* item = new scan_item;
	ZeroMemory(item, sizeof(scan_item));
	item->scanbytes = (PBYTE)aob.c_str();
	item->mask = mask;
	item->result = result;
	item->offsetreg = offsetreg;
	item->protection = protection;
	item_queue.push_back(item);
}

int scanner::scanregion(DWORD region, DWORD regionsize) {
	int failures = 0;

	DWORD PageSize;
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	PageSize = si.dwPageSize;
	MEMORY_BASIC_INFORMATION mi;

	for (auto it = item_queue.begin(); it != item_queue.end(); it++) {
		scan_item* item = (*it);
		for (DWORD lpAddr = region + item->offsetreg; lpAddr < regionsize; lpAddr += PageSize) {
			DWORD vq = VirtualQuery((void*)lpAddr, &mi, PageSize);
			if (vq == ERROR_INVALID_PARAMETER || vq == 0) break;
			if (mi.Type == MEM_MAPPED) continue;
			if (mi.Protect == item->protection)
			{
				DWORD addr = regionbytepattern(lpAddr, PageSize, item->scanbytes, (char*)(item->mask.c_str()));
				if (addr != 0)
				{
					(*item->result) = addr;
					item->success = true;
					printf("%s is gud\n", item->mask.c_str());
					goto nextitem;
				}
			}
		}
		printf("%s is bad\n", item->mask.c_str());
		failures++;

	nextitem:
		__asm nop;

	}
	return failures;
}
