#include "pch.h"
#include "Driver.h"
#include "KInterface.h"

#include <iostream>
#include <iomanip>
#include <windows.h>

#pragma comment(lib, "User32.lib")

#define WHEXOUT std::setfill(L'0') << std::setw(16) << std::hex

BOOL running = false;
const wchar_t *wName = L"HUNT";

typedef struct SSystemGlobalEnvironment
{
	PVOID pDialogSystem;
	PVOID p3DEngine;
	PVOID pNetwork;
	PVOID pNetContext;
	PVOID pLobby;
	PVOID pScriptSystem;
	PVOID pPhysicalWorld;
	PVOID pFlowSystem;
	PVOID pInput;
	PVOID pStatoscope;
	PVOID pCryPak;
	PVOID pFileChangeMonitor;
	PVOID pProfileLogSystem;
	PVOID pParticleManager;
	PVOID pOpticsManager;
	PVOID pFrameProfileSystem;
	PVOID pTimer;
	PVOID pCryFont;
	PVOID pGameFramework;
	PVOID pLocalMemoryUsage;
	PVOID pEntitySystem;
	PVOID pConsole;
	PVOID pAudioSystem;
	PVOID pSystem;
	PVOID pCharacterManager;
	PVOID pAISystem;
	PVOID pLog;
	PVOID pCodeCheckpointMgr;
	PVOID pMovieSystem;
	PVOID pNameTable;
	PVOID pRenderer;
	PVOID pAuxGeomRenderer;
	PVOID pHardwareMouse;
	PVOID pMaterialEffects;
	PVOID pJobManager;
	PVOID pOverloadSceneManager;
	PVOID pFlashUI;
	PVOID pUIFramework;
	PVOID pServiceNetwork;
	PVOID pRemoteCommandManager;
	PVOID pDynamicResponseSystem;
	PVOID pThreadManager;
	PVOID pScaleformHelper; // nullptr when Scaleform support is not enabled
	PVOID pSchematyc;
	PVOID pSchematyc2;
	PVOID pReflection;

	PVOID pLiveCreateManager;
	PVOID pLiveCreateHost;
	PVOID pMonoRuntime;
	UINT32 mMainThreadId;      //!< The main thread ID is used in multiple systems so should be stored globally.
	UINT32 nMainFrameID;
	const char* szCmdLine = "";       //!< Startup command line.

	//! Generic debug string which can be easily updated by any system and output by the debug handler
	enum { MAX_DEBUG_STRING_LENGTH = 128 };
	char szDebugStatus[MAX_DEBUG_STRING_LENGTH];

	//! Used to tell if this is a server/multiplayer instance
	bool bServer;
	bool bMultiplayer;
	bool bHostMigrating;
	int bDeepProfiling;
	bool bBootProfilerEnabledFrames;
	PVOID callbackStartSection;
	PVOID callbackEndSection;
	//////////////////////////////////////////////////////////////////////////

	//! Whether we are running unattended, disallows message boxes and other blocking events that require human intervention
	bool bUnattendedMode;
	//! Whether we are unit testing
	bool bTesting;

	bool bNoRandomSeed;
} SSystemGlobalEnvironment;



bool consoleHandler(int signal) {
	if (signal == CTRL_C_EVENT) {
		if (!running)
			exit(EXIT_FAILURE);
		running = false;
		std::wcout << L"Waiting for graceful shutdown .." << std::endl;
	}
	return true;
}

void printBuf(UCHAR *buf, SIZE_T siz, SIZE_T bytesBeforeNewline) {
	unsigned int i, j;
	const unsigned char colors[] = { 10,11,12,13,14,15 };
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	for (i = 0, j = 0; i < siz; ++i) {
		if (i % bytesBeforeNewline == 0) {
			SetConsoleTextAttribute(hConsole, colors[j++ % (sizeof colors)]);
			wprintf(L"\n0x%04X: ", i);
		}
		wprintf(L"%02X ", buf[i]);
	}
	wprintf(L"\n");
	SetConsoleTextAttribute(hConsole, 15);
}

BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam)
{
	int length = GetWindowTextLength(hWnd);
	TCHAR* buffer;
	buffer = new TCHAR[length + 1];
	memset(buffer, 0, (length + 1) * sizeof(TCHAR));
	GetWindowText(hWnd, buffer, length + 1);
	if (!wcscmp(buffer, wName))
		*(HWND *)lParam = hWnd;
	delete[] buffer;
	return TRUE;
}

int wmain(int argc, wchar_t **argv)
{
	HANDLE targetPID = 0;
	PVOID buf;
	HANDLE kevent;
	HANDLE uevent;

	KInterface &ki = KInterface::getInstance();
	std::vector<MEMORY_BASIC_INFORMATION> pages;
	std::vector<MODULE_DATA> modules;

	if (argc > 1 && !wcscmp(argv[1], L"test")) {
		return 0;
	}

	if (argc > 1 && !wcscmp(argv[1], L"wnd")) {
		if (argc < 3) {
			std::wcout << L"Window title required!" << std::endl;
			return 1;
		}
		wName = argv[2];
		std::wcout << L"Waiting for window title: '" << wName << L"'" << std::endl;

		HWND targetHWND = NULL;
		while (1) {
			if (!EnumWindows(enumWindowsProc, (LPARAM)&targetHWND)) {
				return 1;
			}
			if (targetHWND) {
				std::wcout << L"Found window '" << wName << L"' with Handle 0x"
					<< std::hex << targetHWND << std::endl;
				break;
			}
			Sleep(1000);
		}
		GetWindowThreadProcessId(targetHWND, (LPDWORD)&targetPID);
	}

	SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleHandler, TRUE);

	if (!ki.Init()) {
		std::wcout << L"Kernel Interface Init() failed" << std::endl;
		return 1;
	}

	try {
		buf = ki.getBuffer();
		kevent = ki.getKHandle();
		uevent = ki.getUHandle();
	}
	catch (std::runtime_error& err) {
		std::wcout << err.what() << std::endl;
		return 1;
	}

	std::wcout << L"Buffer.: " << buf << std::endl;
	std::wcout << L"KHandle: " << kevent << std::endl;
	std::wcout << L"UHandle: " << uevent << std::endl;

	if (!ki.Handshake()) {
		std::wcout << L"Kernel Interface Handshake() failed" << std::endl;
		return 1;
	}
	if (targetPID) {
		if (!ki.Modules(targetPID, modules))
			std::wcout << L"Kernel Interface Modules() failed with 0x"
			<< std::hex << ki.getLastNtStatus() << std::endl;
		else std::wcout << L"Got " << std::dec << modules.size() << L" modules for pid 0x"
			<< std::hex << targetPID << std::endl;
		if (!ki.Pages(targetPID, pages))
			std::wcout << L"Kernel Interface Pages() failed with 0x"
			<< std::hex << ki.getLastNtStatus() << std::endl;
		else std::wcout << L"Got " << std::dec << pages.size() << L" mapped pages for pid 0x"
			<< std::hex << targetPID << std::endl;
	}

	running = TRUE;
	do {
		if (ki.RecvWait() == SRR_TIMEOUT) {
			std::wcout << L"Ping -> ";
			if (!ki.Ping()) {
				std::wcout << L"Got no valid PONG, abort!" << std::endl;
				running = FALSE;
			}
			else std::wcout << L"PONG!" << std::endl;
		}

		if (!running)
			break;

		try {
			if (targetPID) {
				for (MODULE_DATA& md : modules) {
					if (!strncmp(md.BaseDllName, "CrySystem.dll",
						sizeof md.BaseDllName))
					{
						std::wcout << L"CrySystem.dll......: 0x" << WHEXOUT << md.DllBase << std::endl;
						UINT64 g_pEnv = KMemory::Rpm<UINT64>(targetPID,
							(PVOID)((UINT64)md.DllBase + 0xA37708));

						std::wcout << L"g_pEnv.............: 0x" << WHEXOUT << g_pEnv << std::endl;
					}
					else
						if (!strncmp(md.BaseDllName, "CryEntitySystem.dll",
							sizeof md.BaseDllName))
						{
							std::wcout << L"CryEntitySystem.dll: 0x" << std::hex << md.DllBase << std::endl;
#if 1
							/* Found: void CEntitySystem::LoadInternalState(IDataReadStream& reader) */
							UINT64 g_pEnv = KMemory::Rpm<UINT64>(targetPID,
								(PVOID)((UINT64)md.DllBase + 0x28C400));

							std::wcout << L"g_pEnv.............: 0x" << WHEXOUT << g_pEnv << std::endl;
#if 1
							BYTE tmp[sizeof SSystemGlobalEnvironment * 2] = { 0 };
							SSIZE_T siz = KMemoryBuf::Rpm<sizeof tmp>(targetPID, (PVOID)(g_pEnv), tmp);

							SSystemGlobalEnvironment *gEnv = (SSystemGlobalEnvironment *)&tmp[0];
							std::wcout << std::hex
								<< gEnv->p3DEngine << ", "
								<< gEnv->pEntitySystem << ", "
								<< gEnv->bMultiplayer << ", "
								<< gEnv->mMainThreadId << ", "
								<< gEnv->nMainFrameID << ", "
								<< gEnv->szDebugStatus << ", "
								<< gEnv->pMonoRuntime << ", "
								<< gEnv->callbackStartSection << ", "
								<< gEnv->callbackEndSection << ", "
								<< gEnv->bUnattendedMode << ", "
								<< gEnv->bTesting << ", "
								<< gEnv->bNoRandomSeed << ", "
								<< gEnv->bDeepProfiling << ", "
								<< std::endl;

							/* Found: void CEntitySystem::LoadInternalState(IDataReadStream& reader)
							 * search for String "ExistingEntity: '%s' '%s'"
							 */
							UINT64 m_EntityArray = KMemory::Rpm<UINT64>(targetPID,
								(PVOID)((UINT64)gEnv->pEntitySystem + 0x40078));
							//m_EntityArray += 0x40078;
							std::wcout << WHEXOUT
								<< m_EntityArray
								<< std::endl;

							USHORT nops;
							nops = KMemory::Rpm<USHORT>(targetPID,
								(PVOID)((UINT64)md.DllBase + 0x12C3C7));
							std::wcout << std::hex << "+++ " << nops << std::endl;
							nops = 0x9090;
							KMemory::Wpm<USHORT>(targetPID,
								(PVOID)((UINT64)md.DllBase + 0x12C3C7), &nops);
#if 0
							UINT64 tmp2[1024];
							SSIZE_T siz2 = KMemoryBuf::Rpm<sizeof tmp2>(targetPID, (PVOID)(m_EntityArray), (PBYTE)tmp2);
							SIZE_T i = 0;
							for (UINT64 val : tmp2) {
								if (val)
									std::wcout << WHEXOUT << val << ", ";
								if (++i % 16 == 0)
									std::wcout << std::endl;
							}
							/*
							if (siz2 > 0)
								printBuf(tmp2, sizeof tmp2, 64);
							*/
#endif
#endif
#if 0
							SSIZE_T siz3;
							static Diff<65535> diff = { 0 };
							siz3 = KScan::BinDiffSimple(targetPID, (PVOID)(m_EntityArray), &diff);
							if (siz3 > 0) {
								std::wcout << L"Got " << std::dec << diff.diffs.size() << L" diffs" << std::endl;
								for (auto& e : diff.diffs) {
									std::wcout << std::dec << L"0x"
										<< std::hex << (PVOID)(/*(ULONG_PTR)(m_EntityArray)+*/e.first)
										<< " - " << std::hex << e.second
										<< L": ";
									printBuf((UCHAR *)((ULONG_PTR)(diff.current_buffer) + e.first), e.second, e.second);
								}
							}
#endif
#if 0
#if 1
							//UCHAR scan[1024] = { 0 };
							UCHAR scan[] = "anti_alles";
							SSIZE_T found = KScan::ScanSimple<UCHAR, sizeof scan>(targetPID,
								(PVOID)(m_EntityArray), 65535, scan);
#else
							SSIZE_T found = (SSIZE_T)(pIEntitySystem)+8192;
#endif
							if (found >= 0) {
								std::wcout << "FOUND: 0x" << std::hex << (PVOID)found << std::endl;
								UCHAR tmp[4096] = { 0 };
								SSIZE_T siz = KMemoryBuf::Rpm<UCHAR, sizeof tmp>(targetPID, (PVOID)found, tmp);
								if (siz > 0) {
									printBuf(tmp, sizeof tmp, 64);
#if 0
									UINT64 i;
									for (i = 0; i < sizeof tmp; i += 8) {
										UINT64 value = *(UINT64 *)&tmp[i];
										if (value)
											printf("0x%p ", (PVOID)value);
									}
									printf("\nGot %llu entities ..\n", i);
#endif
								}
							}
#endif
#endif
						}
#if 0
						else if (!strncmp(md.BaseDllName, "CryRenderD3D11.dll",
							sizeof md.BaseDllName))
						{
							UINT32 displayWidth = KMemory::Rpm<UINT32>(targetPID,
								(PVOID)((ULONGLONG)md.DllBase + /* 0x19F0FE */ 0x5EA870));
							UINT32 displayHeight = KMemory::Rpm<UINT32>(targetPID,
								(PVOID)((ULONGLONG)md.DllBase + /* 0x19F0F0 */ 0x5EA9DC));
							std::wcout << L"Display.........: " << std::dec << displayWidth
								<< " x " << displayHeight << std::endl;
						}
#endif
#if 0
						else if (!strncmp(md.BaseDllName, "ntdll.dll",
							sizeof md.BaseDllName))
						{
							UCHAR tmp[8192] = { 0 };
							SSIZE_T siz = KMemoryBuf::Rpm<UCHAR, sizeof tmp>(targetPID, (PVOID)((ULONGLONG)md.DllBase), tmp);
							/*
													if (siz > 0)
														printBuf(tmp, sizeof tmp, 64);

													UCHAR scan[] = { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };
													SSIZE_T found = KScan::ScanSimple<UCHAR, sizeof scan>(targetPID,
														(PVOID)((ULONGLONG)md.DllBase), sizeof tmp, scan);
													std::wcout << "FOUND: 0x" << std::hex << (found - (ULONGLONG)md.DllBase) << std::endl;
							*/
							static Diff<10> diff = { 0 };
							siz = KScan::BinDiffSimple(targetPID, (PVOID)((ULONGLONG)md.DllBase), &diff);
							if (siz > 0) {
								std::wcout << L"Got " << std::dec << diff.diffs.size() << L" diffs" << std::endl;
								for (auto& e : diff.diffs) {
									printBuf((UCHAR *)((ULONGLONG)md.DllBase + e.first), e.second, e.second);
									/*
									std::wcout << std::dec << L"  addr: " << e.first
										<< std::endl << L"  size: " << e.second
										<< std::endl;
									*/
								}
							}
						}
#endif
				}
			}
		}
		catch (std::runtime_error& err) {
			std::wcout << err.what() << std::endl;
		}
	} while (running);

	std::wcout << L"Driver shutdown .." << std::endl;
	ki.Exit();

	return 0;
}