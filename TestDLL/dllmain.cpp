#include "stdafx.h"

#include <vector>
#include <string>
#include <sstream>

extern "C"
BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

#pragma comment(lib, "vcruntime.lib")

typedef void(*myfree_t)(void *ptr);
typedef void*(*mymalloc_t)(size_t n);
typedef int(*_mycallnewh_t)(size_t n);

static myfree_t myfree;
static mymalloc_t mymalloc;
static _mycallnewh_t _mycallnewh;

void _invalid_parameter_noinfo_noreturn(void) {
	ExitProcess(1);
}

extern "C"
void * malloc(size_t n) {
	return mymalloc(n);
}

extern "C"
void free(void *ptr) {
	myfree(ptr);
}

extern "C"
int _callnewh(size_t n) {
	return _mycallnewh(n);
}

void MyFnResolve(void) {

}

void APIENTRY LibEntry(PVOID user_ptr)
{
	static bool firstEntry = true;

	if (firstEntry) {
		firstEntry = false;

		HMODULE msvcrtModule = LoadLibraryA("msvcrt.dll");
		mymalloc = (mymalloc_t) GetProcAddress(msvcrtModule, "malloc");
		myfree = (myfree_t) GetProcAddress(msvcrtModule, "free");
		_mycallnewh = (_mycallnewh_t)GetProcAddress(msvcrtModule, "_callnewh");
		if (!mymalloc || !myfree || !_mycallnewh) {
			return;
		}
#if 1
		std::string text;
		std::vector<DWORD> blubb;
		text = "DllMain from TestDLL: ";
		blubb.push_back(1);
		blubb.push_back(2);
		//std::wstringstream muh;
		//muh << "bla" << "," << "blubb";
#endif
		MessageBoxA(NULL,
			text.c_str(),
			"TestDLL Notification",
			MB_OK | MB_ICONINFORMATION);
#if 0
		if (firstEntry &&
			!_CRT_INIT(NULL, DLL_PROCESS_ATTACH, NULL)) {
			MessageBoxA(NULL,
				"DllMain _CRT_INIT failed",
				"TestDLL Notification",
				MB_OK | MB_ICONINFORMATION);
		}
#endif
	}
}