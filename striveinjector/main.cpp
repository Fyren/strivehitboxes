#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>

const auto *exe = "GGST-Win64-Shipping.exe";
const auto *dll = "strivehitboxes.dll";

struct HandleCloser {
	using pointer = HANDLE;
	void operator()(HANDLE h) { ::CloseHandle(h); }
};
using AutoHandle = std::unique_ptr<HANDLE, HandleCloser>;

bool elevate_privileges()
{
	LUID luid;
	if (!LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &luid)) {
		std::cerr << "LookupPrivilegeValue failed\n";
		return false;
	}

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	HANDLE h;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &h)) {
		std::cerr << "OpenProcessToken failed\n";
		return false;
	}
	AutoHandle token(h);

	if (!AdjustTokenPrivileges(token.get(), FALSE, &tp, 0, (TOKEN_PRIVILEGES *)(nullptr), (DWORD *)(nullptr))) {
		std::cerr << "AdjustTokenPrivileges failed\n";
		return false;
	}

	return true;
}

bool inject()
{
	if (!elevate_privileges())
		return false;

	AutoHandle snap(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);

	Process32First(snap.get(), &entry);
	do {
		constexpr auto mask =
			PROCESS_QUERY_INFORMATION |
			PROCESS_CREATE_THREAD |
			PROCESS_VM_OPERATION |
			PROCESS_VM_READ |
			PROCESS_VM_WRITE;

		AutoHandle proc(OpenProcess(mask, false, entry.th32ProcessID));

		char path[MAX_PATH];
		if (GetModuleFileNameEx(proc.get(), nullptr, path, MAX_PATH) == 0) continue;
		if (strcmp(path + strlen(path) - strlen(exe), exe) != 0) continue;

		AutoHandle modules(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(proc.get())));
		if (modules.get() == INVALID_HANDLE_VALUE) {
			std::cerr << "Couldn't get modules for Strive process\n";
			return false;
		}

		MODULEENTRY32 me;
		me.dwSize = sizeof(MODULEENTRY32);
		if (!Module32First(modules.get(), &me)) {
			std::cerr << "Couldn't get modules for Strive process\n";
			return false;
		}

		do {
			if (strcmp(dll, me.szModule) == 0) {
				std::cerr << "Already injected\n";
				return false;
			}
		} while (Module32Next(modules.get(), &me));

		char dll_path[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, dll_path);
		strcat_s(dll_path, MAX_PATH, "/");
		strcat_s(dll_path, MAX_PATH, dll);

		const auto size = strlen(dll_path) + 1;
		auto *buf = VirtualAllocEx(proc.get(), nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if (buf == nullptr) {
			std::cerr << "VirtualAllocEx failed\n";
			return false;
		}

		WriteProcessMemory(proc.get(), buf, dll_path, size, nullptr);
		AutoHandle th(CreateRemoteThread(proc.get(), nullptr, 0, (LPTHREAD_START_ROUTINE)(LoadLibrary), buf, 0, nullptr));
		WaitForSingleObject(th.get(), INFINITE);

		std::cout << "Injected" << std::endl;
		return true;
	} while (Process32Next(snap.get(), &entry));

	std::cerr << "Didn't find process for " << exe << "\n";
	return false;
}

int main()
{
	const auto result = inject();
	return result ? 0 : 1;
}
