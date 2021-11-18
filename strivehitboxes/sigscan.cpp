#include "sigscan.h"
#include <stdexcept>
#include <fstream>
#include <ios>
#include <iomanip>
#include <Windows.h>
#include <Psapi.h>

bool failedScan = false;

const sigscan &sigscan::get()
{
	static sigscan instance("GGST-Win64-Shipping.exe");
	return instance;
}

sigscan::sigscan(const char *name)
{
	const auto module = GetModuleHandle(name);
	if (module == nullptr)
		throw std::runtime_error("Module not found");

	MODULEINFO info;
	GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info));
	start = (uintptr_t)(info.lpBaseOfDll);
	end = start + info.SizeOfImage;
}

uintptr_t sigscan::scan(const char *sig, const char *mask) const
{
	const auto last_scan = end - strlen(mask) + 1;
	for (auto addr = start; addr < last_scan; addr++) {
		for (size_t i = 0;; i++) {
			if (mask[i] == '\0')
				return addr;
			if (mask[i] != '?' && sig[i] != *(char*)(addr + i))
				break;
		}
	}

	std::ofstream f("hitboxesfailed.log", std::ios_base::app);
	f << std::hex << std::setfill('0');
	for (int i = 0; i < strlen(mask); ++i)
		if (mask[i] == '?') f << "? ";
		else f << std::setw(2) << ((int)sig[i] & 0xff) << " ";
	f << std::endl;

	//throw std::runtime_error("Sigscan failed");
	failedScan = true;
	return 0;
}

