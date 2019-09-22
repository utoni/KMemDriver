#include "stdafx.h"

void APIENTRY LibEntry(void)
{
	static bool firstEntry = true;

	if (firstEntry) {
		firstEntry = false;

		MessageBoxA(NULL,
			"DllMain from TestDLL",
			"TestDLL Notification",
			MB_OK | MB_ICONINFORMATION);
	}
}

extern "C" __declspec(dllexport)
DWORD WINAPI LibEntryThreaded(_In_ LPVOID lpParameter)
{
	LibEntry();

	ExitThread(0);
	return 0;
}