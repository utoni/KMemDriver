#include "stdafx.h"

#include <vector>
#include <string>
#include <sstream>
#include <array>

#include <Windows.h>

EXTERN_C BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);


#define SHOW_WARNING(format, ...) \
    do { char errbuf[128]; \
        snprintf(errbuf, sizeof errbuf, "WARNING: " # format, __VA_ARGS__); \
        MessageBoxA(NULL, errbuf, "Hunted WARNING",	MB_OK | MB_ICONINFORMATION); \
	} while (0);


/* function signature depends on used shellcode */
void APIENTRY LibEntry(/* void * arg */)
{
	static bool firstEntry = true;

	if (firstEntry) {
		firstEntry = false;

		HINSTANCE addr = GetModuleHandle(NULL);
		_CRT_INIT(addr, DLL_PROCESS_ATTACH, NULL);

		AllocConsole();
		FILE * conout = NULL;
		freopen_s(&conout, "CONOUT$", "w", stdout);

		printf("Welcome.\n");
	}

	/* ... */
}