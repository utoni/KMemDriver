#include "stdafx.h"
#include "HuntClasses.h"

#include <vector>
#include <string>
#include <sstream>
#include <array>

#include <GdiRadar.h>
#include <Windows.h>

EXTERN_C BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);


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
	cfg.minimumUpdateTime = 0.20f;
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

static bool InitAndCheckPtr(struct HuntCtx * HuntCtx)
{
	char reserved_stack_space[256];

	pEntSys = *(UINT64*)(HuntCtx->ppEntSys);
	iEnt = *HuntCtx->ppEntSys;

	ZeroMemory(&reserved_stack_space[0], sizeof reserved_stack_space);
	if (iEnt->GetNumEntities() > 65535) {
		SHOW_WARNING("Invalid number of Entities : VALUE[%u] > 65535\n",
			iEnt->GetNumEntities());
		return false;
	}
	if ((PVOID)(*(UINT64*)(pEntSys + PENTITYSYSTEM_ISYSTEM_OFFSET)) != iEnt->GetSystem()) {
		SHOW_WARNING("ISystem interface instance not equal : MEMBER[%p] != GETSYSTEM[%p]\n",
			(PVOID)(*(UINT64*)(pEntSys + PENTITYSYSTEM_ISYSTEM_OFFSET)), iEnt->GetSystem());
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
		SHOW_WARNING("IEntitySystem interface instance not equal: GLOBAL[%p] != GETENTITYSYSTEM[%p]\n",
			(PVOID)pEntSys, iEnt->GetSystem()->GetIEntitySystem());
		return false;
	}
	if ((PVOID)pEntSys != iEnt->GetSystem()->GetGlobalEnvironment()->pEntitySystem) {
		SHOW_WARNING("IEntitySystem interface instance not equal: GLOBAL[%p] != pEntitySystem[%p]\n",
			(PVOID)pEntSys, iEnt->GetSystem()->GetGlobalEnvironment()->pEntitySystem);
		return false;
	}
	if (*HuntCtx->ppGlobalEnv != iEnt->GetSystem()->GetGlobalEnvironment()) {
		SHOW_WARNING("GlobalEnvironment signature not equals GetGlobalEnvironment() instance: ppGlobalEnv[%p] != GetGlobalEnvironment[%p]\n",
			(PVOID)pEntSys, iEnt->GetSystem()->GetGlobalEnvironment()->pEntitySystem);
		return false;
	}
	if ((*HuntCtx->ppCCryAction)->GetIActorSystem() != iEnt->GetSystem()->GetGlobalEnvironment()->pGameFramework->GetIActorSystem()) {
		SHOW_WARNING("CCryAction->GetIActorSystem() signature not equals GameFramework->GetIActorSystem() instance: ppCCryAction[%p] != pGameFramework[%p]\n",
			(*HuntCtx->ppCCryAction)->GetIActorSystem(), iEnt->GetSystem()->GetGlobalEnvironment()->pGameFramework->GetIActorSystem());
		return false;
	}
	if (iEnt->GetSystem() != iEnt->GetSystem()->GetGlobalEnvironment()->pGameFramework->GetISystem()) {
		SHOW_WARNING("ISystem interface instance not equal: IEntitySystem[%p] != pGameFramework[%p]\n",
			iEnt->GetSystem(), iEnt->GetSystem()->GetGlobalEnvironment()->pGameFramework->GetISystem());
		return false;
	}
	if (iEnt->GetSystem() != iEnt->GetSystem()->GetGlobalEnvironment()->pSystem) {
		SHOW_WARNING("ISystem interface instance not equal: IEntitySystem[%p] != pSystem[%p]\n",
			iEnt->GetSystem(), iEnt->GetSystem()->GetGlobalEnvironment()->pSystem);
		return false;
	}
	if (iEnt->GetSystem()->GetGlobalEnvironment()->pRenderer != iEnt->GetSystem()->GetIRenderer()) {
		SHOW_WARNING("ISystem interface instance not equal: IEntitySystem[%p] != pSystem[%p]\n",
			iEnt->GetSystem(), iEnt->GetSystem()->GetGlobalEnvironment()->pSystem);
		return false;
	}

	return true;
}

void APIENTRY LibEntry(struct HuntCtx * HuntCtx)
{
	static bool firstEntry = true;

	if (!HuntCtx || !HuntCtx->ppEntSys || !HuntCtx->ppGlobalEnv || !HuntCtx->ppCCryAction)
		return;

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

		if (!InitAndCheckPtr(HuntCtx))
		{
			return;
		}

		AllocConsole();
		FILE * conout = NULL;
		freopen_s(&conout, "CONOUT$", "w", stdout);

		printf("Welcome.\n");
		printf("[thread id: %d][used memory: %u][cpu flags: %u][user name: %s][cpu count: %d]\n",
			GetCurrentThreadId(),
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
	if (!gdi_radar_check_if_redraw_necessary(ctx)) {
		return;
	}

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

		Vec3 entPos = pEnt->GetPos();
		entPos.x -= 500.0f;
		entPos.y -= 500.0f;
		entPos.y = 1020.0f - entPos.y;
		float entAngle = pEnt->GetWorldAngles().z;
		entAngle *= -1.0f;
		entAngle -= 1.5707963267948966192313216916398f; /* pi/2 == 90deg */
		entity radar_entity{
			(int)entPos.x, (int)entPos.y, entAngle,
			(entCol == entity_color::EC_BLUE ? 60 : 0),
			100.0f, entCol, "test"
		};
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