#include "stdafx.h"

void APIENTRY LibEntry(void)
{
	static bool firstEntry = true;

	if (firstEntry) {
		firstEntry = false;

		MessageBoxA(NULL,
			"DllMain from TestDLL",
			"TestDLL Notification",
			MB_OK | MB_ICONINFORMATION) == IDOK ? TRUE : FALSE;
	}
}