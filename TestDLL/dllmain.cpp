#include "stdafx.h"

#include <vector>
#include <string>
#include <sstream>
#include <array>

#pragma comment(lib, "vcruntime.lib")


struct ResolvedDllEntry {
	const char * const baseDllName;
	const char * const functionName;

	HMODULE moduleBase;
	FARPROC resolvedProc;
};

#define DLL_ENTRY(dll_name, function_name) \
	{ dll_name, function_name, NULL, NULL }
#define MSVCRT_ENTRY(function_name) DLL_ENTRY("msvcrt.dll", function_name)

static struct ResolvedDllEntry resolved_smybols[] = {
	MSVCRT_ENTRY("malloc"), MSVCRT_ENTRY("free"), MSVCRT_ENTRY("_callnewh"),
	MSVCRT_ENTRY("abort"), MSVCRT_ENTRY("calloc"), MSVCRT_ENTRY("frexp"),
	MSVCRT_ENTRY("islower"), MSVCRT_ENTRY("isspace"), MSVCRT_ENTRY("isupper"),
	MSVCRT_ENTRY("ldexp"), MSVCRT_ENTRY("localeconv"),
	MSVCRT_ENTRY("memchr"), MSVCRT_ENTRY("memcmp"), MSVCRT_ENTRY("memcpy")
};
static const SIZE_T resolved_symbols_size =
sizeof(resolved_smybols) / sizeof(resolved_smybols[0]);

enum SymbolIndex {
	SYM_MALLOC, SYM_FREE, SYM_CALLNEWH, SYM_INVALID_PARAMETER_NOINFO_NORETURN,
	SYM_ABORT, SYM_CALLOC, SYM_FREXP,
	SYM_ISLOWER, SYM_ISSPACE, SYM_ISUPPER,
	SYM_LDEXP, SYM_LOCALECONV,
	SYM_MEMCHR, SYM_MEMCMP, SYM_MEMCPY
};

#define WRAPPER_FUNCTION(symbol_index, linker_function_name, return_type, ...) \
	typedef return_type (* symbol_index ## _FN)(__VA_ARGS__); \
	extern "C" \
	return_type linker_function_name(__VA_ARGS__)
#define RUN_REAL_FN(symbol_index, ...) \
	(((symbol_index ## _FN)resolved_smybols[symbol_index].resolvedProc)(__VA_ARGS__))

WRAPPER_FUNCTION(SYM_MALLOC, malloc, void *, size_t n) {
	return RUN_REAL_FN(SYM_MALLOC, n);
}
WRAPPER_FUNCTION(SYM_FREE, free, void, void *p) {
	RUN_REAL_FN(SYM_FREE, p);
}
WRAPPER_FUNCTION(SYM_CALLNEWH, _callnewh, int, size_t n) {
	return RUN_REAL_FN(SYM_CALLNEWH, n);
}
WRAPPER_FUNCTION(SYM_INVALID_PARAMETER_NOINFO_NORETURN,
	_invalid_parameter_noinfo_noreturn, void, void) {
	ExitProcess(1);
}
WRAPPER_FUNCTION(SYM_ABORT, abort, void, void) {
	RUN_REAL_FN(SYM_ABORT);
}
WRAPPER_FUNCTION(SYM_CALLOC, calloc, void *, size_t n, size_t s) {
	return RUN_REAL_FN(SYM_CALLOC, n, s);
}
WRAPPER_FUNCTION(SYM_FREXP, frexp, double, double x, int *expptr) {
	return RUN_REAL_FN(SYM_FREXP, x, expptr);
}
WRAPPER_FUNCTION(SYM_ISLOWER, islower, int, int c) {
	return RUN_REAL_FN(SYM_ISLOWER, c);
}
WRAPPER_FUNCTION(SYM_ISSPACE, isspace, int, int c) {
	return RUN_REAL_FN(SYM_ISSPACE, c);
}
WRAPPER_FUNCTION(SYM_ISUPPER, isupper, int, int c) {
	return RUN_REAL_FN(SYM_ISUPPER, c);
}


extern "C"
static bool resolve_all_symbols(void) {
	bool result = true;

	for (SIZE_T i = 0; i < 3; ++i) {
		resolved_smybols[i].moduleBase = LoadLibraryA(resolved_smybols[i].baseDllName);

		if (!resolved_smybols[i].moduleBase) {
			result = false;
			continue;
		}
		resolved_smybols[i].resolvedProc = GetProcAddress(resolved_smybols[i].moduleBase,
			resolved_smybols[i].functionName);
		if (!resolved_smybols[i].resolvedProc) {
			result = false;
		}
	}

	return result;
}


void APIENTRY LibEntry(PVOID user_ptr)
{
	static bool firstEntry = true;

	if (firstEntry) {
		firstEntry = false;

		if (!resolve_all_symbols()) {
			MessageBoxA(NULL,
				"COULD NOT RESOLVE ALL DYNAMIC DLL SYMBOLS !!!",
				"TestDLL Notification",
				MB_OK | MB_ICONINFORMATION);
			return;
		}
		void *bla = malloc(10);
		free(bla);
#if 1
		std::string text;
		std::vector<DWORD> blubb;
		text = "DllMain from TestDLL: ";
		blubb.push_back(1);
		blubb.push_back(2);
		//std::stringstream muh;
		//muh << "bla" << "," << "blubb";
		MessageBoxA(NULL,
			text.c_str(),
			"TestDLL Notification",
			MB_OK | MB_ICONINFORMATION);
#else
		MessageBoxA(NULL,
			"TEST !!!",
			"TestDLL Notification",
			MB_OK | MB_ICONINFORMATION);
#endif
	}
}