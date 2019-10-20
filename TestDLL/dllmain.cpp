#include "stdafx.h"
#include "HuntClasses.h"

#include <vector>
#include <string>
#include <sstream>
#include <array>

#include <GdiRadar.h>
#include <Windows.h>

EXTERN_C BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);


#if 0
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
	MSVCRT_ENTRY("_errno"),
	MSVCRT_ENTRY("malloc"), MSVCRT_ENTRY("free"), MSVCRT_ENTRY("_callnewh"),
	MSVCRT_ENTRY("_invalid_parameter_noinfo_noreturn"),
	MSVCRT_ENTRY("abort"), MSVCRT_ENTRY("calloc"), MSVCRT_ENTRY("frexp"),
	MSVCRT_ENTRY("islower"), MSVCRT_ENTRY("isspace"), MSVCRT_ENTRY("isupper"),
	MSVCRT_ENTRY("tolower"),
	MSVCRT_ENTRY("ldexp"), MSVCRT_ENTRY("localeconv"), MSVCRT_ENTRY("__pctype_func"),
	MSVCRT_ENTRY("___lc_locale_name_func"), MSVCRT_ENTRY("___lc_codepage_func"),
	MSVCRT_ENTRY("setlocale"),
	MSVCRT_ENTRY("_wcsdup"), MSVCRT_ENTRY("wcslen"), MSVCRT_ENTRY("wcsnlen")
};
static const SIZE_T resolved_symbols_size =
sizeof(resolved_smybols) / sizeof(resolved_smybols[0]);

enum SymbolIndex {
	SYM_ERRNO,
	SYM_MALLOC, SYM_FREE, SYM_CALLNEWH,
	SYM_INVALID_PARAMETER_NOINFO_NORETURN,
	SYM_ABORT, SYM_CALLOC, SYM_FREXP,
	SYM_ISLOWER, SYM_ISSPACE, SYM_ISUPPER,
	SYM_TOLOWER,
	SYM_LDEXP, SYM_LOCALECONV, SYM_PCTYPE,
	SYM_LC_LOCALE_NAME, SYM_LC_CODEPAGE,
	SYM_SETLOCALE,
	SYM_WCSDUP, SYM_WCSLEN, SYM_WCSNLEN,
	NUMBER_OF_SYMBOLS
};

static_assert(NUMBER_OF_SYMBOLS == resolved_symbols_size, "Invalid number of Symbols in the table/enum");

#define WRAPPER_FUNCTION(symbol_index, linker_function_name, return_type, ...) \
	typedef return_type (* symbol_index ## _FN)(__VA_ARGS__); \
	extern "C" \
	return_type linker_function_name(__VA_ARGS__)
#define RUN_REAL_FN(symbol_index, ...) \
	(((symbol_index ## _FN)resolved_smybols[symbol_index].resolvedProc)(__VA_ARGS__))

int* __cdecl _errno(void) {
	typedef int*(*SYM_ERRNO_FN)();
	return (((SYM_ERRNO_FN)resolved_smybols[SYM_ERRNO].resolvedProc)());
}
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
WRAPPER_FUNCTION(SYM_TOLOWER, tolower, int, int c) {
	return RUN_REAL_FN(SYM_TOLOWER, c);
}
WRAPPER_FUNCTION(SYM_LDEXP, ldexp, double, double x, int exp) {
	return RUN_REAL_FN(SYM_LDEXP, x, exp);
}
WRAPPER_FUNCTION(SYM_LOCALECONV, localeconv, struct lconv *, void) {
	return RUN_REAL_FN(SYM_LOCALECONV);
}
WRAPPER_FUNCTION(SYM_PCTYPE, __pctype_func, const unsigned short *, void) {
	return RUN_REAL_FN(SYM_PCTYPE);
}
WRAPPER_FUNCTION(SYM_LC_LOCALE_NAME, ___lc_locale_name_func, wchar_t **, void) {
	return RUN_REAL_FN(SYM_LC_LOCALE_NAME);
}
WRAPPER_FUNCTION(SYM_LC_CODEPAGE, ___lc_codepage_func, UINT, void) {
	return RUN_REAL_FN(SYM_LC_CODEPAGE);
}
WRAPPER_FUNCTION(SYM_SETLOCALE, setlocale, char *, int category, const char *locale) {
	return RUN_REAL_FN(SYM_SETLOCALE, category, locale);
}
WRAPPER_FUNCTION(SYM_WCSDUP, _wcsdup, wchar_t *, const wchar_t *src) {
	return RUN_REAL_FN(SYM_WCSDUP, src);
}
WRAPPER_FUNCTION(SYM_WCSLEN, _wcslen, size_t, const wchar_t *str) {
	return RUN_REAL_FN(SYM_WCSLEN, str);
}
WRAPPER_FUNCTION(SYM_WCSNLEN, wcsnlen, size_t, const wchar_t *str, size_t n) {
	return RUN_REAL_FN(SYM_WCSNLEN, str, n);
}

extern "C"
void __vcrt_initialize() {}
extern "C"
void __vcrt_uninitialize() {}
extern "C"
void __vcrt_uninitialize_critical() {}
extern "C"
void __vcrt_thread_attach() {}
extern "C"
void __vcrt_thread_detach() {}
extern "C"
void __acrt_initialize() {}
extern "C"
void __acrt_uninitialize() {}
extern "C"
void __acrt_uninitialize_critical() {}
extern "C"
void __acrt_thread_attach() {}
extern "C"
void __acrt_thread_detach() {}

extern "C"
static bool resolve_all_symbols(void) {
	bool result = true;

	for (SIZE_T i = 0; i < 3; ++i) {
		if (resolved_smybols[i].moduleBase) {
			result = false;
		}
		resolved_smybols[i].moduleBase = LoadLibraryA(resolved_smybols[i].baseDllName);
		if (!resolved_smybols[i].moduleBase) {
			result = false;
			continue;
		}
		if (resolved_smybols[i].resolvedProc) {
			result = false;
		}
		resolved_smybols[i].resolvedProc = GetProcAddress(resolved_smybols[i].moduleBase,
			resolved_smybols[i].functionName);
		if (!resolved_smybols[i].resolvedProc) {
			result = false;
		}
	}

	return result;
}
#endif

static gdi_radar_context * ctx = NULL;
static UINT64 pEntSys = 0x0;
static IEntitySystem * iEnt = NULL;


#define SHOW_WARNING(format, ...) \
    do { char errbuf[128]; \
        snprintf(errbuf, sizeof errbuf, "WARNING: " # format, __VA_ARGS__); \
        MessageBoxA(NULL, errbuf, "Hunted WARNING",	MB_OK | MB_ICONINFORMATION); \
	} while (0);

static bool ConfigureAndInitGDI(void)
{
	SetWindowTextA(GetConsoleWindow(), "Hunted");

	gdi_radar_config cfg = {};
	cfg.className = L"HR";
	cfg.windowName = L"HRWND";
	cfg.minimumUpdateTime = 0.25f;
	cfg.maximumRedrawFails = 5;
	cfg.reservedEntities = 16;
	cfg.drawAngles = true;

	printf("Configure.\n");
	ctx = gdi_radar_configure(&cfg, gdi_radar_get_fake_hinstance());
	if (!ctx)
	{
		printf("Configure failed.\n");
		return false;
	}

	gdi_radar_set_game_dimensions(ctx, 1020.0f, 1020.0f);

	if (!gdi_radar_init(ctx))
	{
		printf("Init failed.\n");
		return false;
	}

	return true;
}

static bool InitAndCheckPtr(PVOID user_ptr)
{
	char reserved_stack_space[256];

	pEntSys = *(UINT64*)user_ptr;
	iEnt = *(IEntitySystem **)user_ptr;

	ZeroMemory(&reserved_stack_space[0], sizeof reserved_stack_space);
	if (iEnt->GetNumEntities() > 65535) {
		char errbuf[128];
		snprintf(errbuf, sizeof errbuf,
			"WARNING: Invalid number of Entities: VALUE[%d] > 65535\n",
			iEnt->GetNumEntities());
		MessageBoxA(NULL,
			errbuf,
			"Hunted WARNING",
			MB_OK | MB_ICONINFORMATION);
		return false;
	}
#define PENTITYSYSTEM_ISYSTEM_OFFSET 104
	if ((PVOID)(*(UINT64*)(pEntSys + PENTITYSYSTEM_ISYSTEM_OFFSET)) != iEnt->GetSystem()) {
		char errbuf[128];
		snprintf(errbuf, sizeof errbuf,
			"WARNING: ISystem interface instance not equal: MEMBER[%p] != GETSYSTEM[%p]\n",
			(PVOID)(*(UINT64*)(pEntSys + PENTITYSYSTEM_ISYSTEM_OFFSET)), iEnt->GetSystem());
		MessageBoxA(NULL,
			errbuf,
			"Hunted WARNING",
			MB_OK | MB_ICONINFORMATION);
		return false;
	}
	if (iEnt->GetSystem()->GetLogicalCPUCount() < 1 ||
		iEnt->GetSystem()->GetLogicalCPUCount() > 32)
	{
		SHOW_WARNING("GetLogicalCPUCount returned an invalid value: %u",
			iEnt->GetSystem()->GetLogicalCPUCount());
		return false;
	}
	if (iEnt->GetSystem()->IsQuitting() ||
		iEnt->GetSystem()->IsRelaunch())
	{
		SHOW_WARNING("IsQuitting/IsRelaunch returned invalid values: %u/%u",
			iEnt->GetSystem()->IsQuitting(), iEnt->GetSystem()->IsRelaunch());
		return false;
	}
	if (iEnt->GetSystem()->GetHWND() > (PVOID)((ULONG_PTR)0xFFFFFFFF))
	{
		SHOW_WARNING("GetHWND returned an invalid window handle: %p",
			iEnt->GetSystem()->GetHWND());
		return false;
	}
	if ((PVOID)pEntSys != iEnt->GetSystem()->GetIEntitySystem()) {
		char errbuf[128];
		snprintf(errbuf, sizeof errbuf,
			"WARNING: IEntitySystem interface instance not equal: GLOBAL[%p] != GETENTITYSYSTEM[%p]\n",
			(PVOID)pEntSys, iEnt->GetSystem()->GetIEntitySystem());
		MessageBoxA(NULL,
			errbuf,
			"Hunted WARNING",
			MB_OK | MB_ICONINFORMATION);
		return false;
	}
	if ((PVOID)pEntSys != iEnt->GetSystem()->GetGlobalEnvironment()->pEntitySystem) {
		char errbuf[128];
		snprintf(errbuf, sizeof errbuf,
			"WARNING: IEntitySystem interface instance not equal: GLOBAL[%p] != pEntitySystem[%p]\n",
			(PVOID)pEntSys, iEnt->GetSystem()->GetGlobalEnvironment()->pEntitySystem);
		MessageBoxA(NULL,
			errbuf,
			"Hunted WARNING",
			MB_OK | MB_ICONINFORMATION);
		return false;
	}
	if (iEnt->GetSystem() != iEnt->GetSystem()->GetGlobalEnvironment()->pGameFramework->GetISystem()) {
		char errbuf[128];
		snprintf(errbuf, sizeof errbuf,
			"WARNING: ISystem interface instance not equal: IEntitySystem[%p] != pGameFramework[%p]\n",
			iEnt->GetSystem(), iEnt->GetSystem()->GetGlobalEnvironment()->pGameFramework->GetISystem());
		MessageBoxA(NULL,
			errbuf,
			"Hunted WARNING",
			MB_OK | MB_ICONINFORMATION);
		return false;
	}
	if (iEnt->GetSystem() != iEnt->GetSystem()->GetGlobalEnvironment()->pSystem) {
		char errbuf[128];
		snprintf(errbuf, sizeof errbuf,
			"WARNING: ISystem interface instance not equal: IEntitySystem[%p] != pSystem[%p]\n",
			iEnt->GetSystem(), iEnt->GetSystem()->GetGlobalEnvironment()->pSystem);
		MessageBoxA(NULL,
			errbuf,
			"Hunted WARNING",
			MB_OK | MB_ICONINFORMATION);
		return false;
	}
	if (iEnt->GetSystem()->GetGlobalEnvironment()->pRenderer != iEnt->GetSystem()->GetIRenderer()) {
		char errbuf[128];
		snprintf(errbuf, sizeof errbuf,
			"WARNING: ISystem interface instance not equal: IEntitySystem[%p] != pSystem[%p]\n",
			iEnt->GetSystem(), iEnt->GetSystem()->GetGlobalEnvironment()->pSystem);
		MessageBoxA(NULL,
			errbuf,
			"Hunted WARNING",
			MB_OK | MB_ICONINFORMATION);
		return false;
	}

	return true;
}

void APIENTRY LibEntry(PVOID user_ptr)
{
	static bool firstEntry = true;

	if (firstEntry) {
		firstEntry = false;
#if 0
		if (!resolve_all_symbols()) {
			MessageBoxA(NULL,
				"COULD NOT RESOLVE ALL DYNAMIC DLL SYMBOLS !!!",
				"TestDLL Notification",
				MB_OK | MB_ICONINFORMATION);
			return;
		}
		void *bla = malloc(10);
		free(bla);
#endif

		HINSTANCE addr = GetModuleHandle(NULL);
		_CRT_INIT(addr, DLL_PROCESS_ATTACH, NULL);

		if (!InitAndCheckPtr(user_ptr))
		{
			return;
		}

		AllocConsole();
		FILE * conout = NULL;
		freopen_s(&conout, "CONOUT$", "w", stdout);

		printf("Welcome.\n");
		printf("[used memory: %u][cpu flags: %u][user name: %s][cpu count: %d]\n",
			iEnt->GetSystem()->GetUsedMemory(),
			iEnt->GetSystem()->GetCPUFlags(),
			iEnt->GetSystem()->GetUserName(),
			iEnt->GetSystem()->GetLogicalCPUCount());

		if (!ConfigureAndInitGDI()) {
			return;
		}
	}

	if (!iEnt || iEnt->GetSystem()->GetGlobalEnvironment()->pGameFramework->IsInLevelLoad()) {
		return;
	}

#if 1
	static UINT64 exec_counter = 0;
	if ((++exec_counter) % 500 == 0) {
		printf("---%d---%d---%s---%.2f %.2f %.2f---\n",
			iEnt->GetSystem()->GetGlobalEnvironment()->pGameFramework->GetClientActorId(),
			iEnt->GetSystem()->GetGlobalEnvironment()->pGameFramework->GetClientEntityId(),
			iEnt->GetSystem()->GetGlobalEnvironment()->pGameFramework->GetLevelName(),
			iEnt->GetSystem()->GetGlobalEnvironment()->pGameFramework->GetClientEntity()->GetWorldAngles().x,
			iEnt->GetSystem()->GetGlobalEnvironment()->pGameFramework->GetClientEntity()->GetWorldAngles().y,
			iEnt->GetSystem()->GetGlobalEnvironment()->pGameFramework->GetClientEntity()->GetWorldAngles().z
		);
	}
#endif

	gdi_radar_clear_entities(ctx);

	SIZE_T i = 1;
	IEntityItPtr pEntIt = iEnt->GetEntityIterator();
	while (IEntity* pEnt = pEntIt->Next()) {
		if (!pEnt->IsInitialized() || pEnt->IsGarbage()) {
			continue;
		}
#if 0
		if (pEnt->GetFlags() != (ENTITY_FLAG_CASTSHADOW | ENTITY_FLAG_SEND_RENDER_EVENT)) {
			continue;
		}
#endif
		const char *name = pEnt->GetName();
		if (strlen(name) < 4) {
			continue;
		}
		if (name[0] != 'H' || name[1] != 'u' || name[2] != 'n' || name[3] != 't') {
			continue;
		}

		enum entity_color entCol = entity_color::EC_RED;
		if (pEnt->GetFlags() & ENTITY_FLAG_LOCAL_PLAYER) {
			entCol = entity_color::EC_BLUE;
		}
		else if ((pEnt->GetFlags() & ENTITY_ENEMY_CHECK) == 0) {
			entCol = entity_color::EC_BLACK;
		}

		Vec3 entPos = pEnt->GetPos();
		entPos.x -= 500.0f;
		entPos.y -= 500.0f;
		entPos.y = 1020.0f - entPos.y;
		float entAngle = pEnt->GetWorldAngles().z;
		entAngle *= -1.0f;
		entAngle -= 1.5707963267948966192313216916398f; /* pi/2 == 90deg */
		entity radar_entity{ (int)entPos.x, (int)entPos.y, entAngle, 100.0f, entCol, "test" };
		gdi_radar_add_entity(ctx, &radar_entity);

		i++;
	}

	static UINT64 redraw_retry = 0;
	if (!gdi_radar_redraw_if_necessary(ctx) &&
		((++redraw_retry) % 250 == 0))
	{
		printf("Reint (redraw failed).\n");
		gdi_radar_close_and_cleanup(&ctx);
		ConfigureAndInitGDI();
		return;
	}

	gdi_radar_process_window_events_nonblocking(ctx);
}