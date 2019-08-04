#include "pch.h"
#include "KMemDriver.h"
#include "KInterface.h"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <windows.h>

static BOOL running = false;
static const wchar_t *wName = L"Counter-Strike: Global Offensive";

typedef struct player_info_s
{
	__int64         unknown;            //0x0000 
	union
	{
		__int64       steamID64;          //0x0008 - SteamID64
		struct
		{
			__int32     xuid_low;
			__int32     xuid_high;
		};
	};
	char            szName[128];        //0x0010 - Player Name
	int             userId;             //0x0090 - Unique Server Identifier
	char            szSteamID[20];      //0x0094 - STEAM_X:Y:Z
	char            pad_0x00A8[0x10];   //0x00A8
	unsigned long   iSteamID;           //0x00B8 - SteamID 
	char            szFriendsName[128];
	bool            fakeplayer;
	bool            ishltv;
	unsigned int    customfiles[4];
	unsigned char   filesdownloaded;
} player_info_t;


static bool consoleHandler(int signal) {
	if (signal == CTRL_C_EVENT) {
		if (!running)
			exit(EXIT_FAILURE);
		running = false;
		std::wcout << L"Waiting for graceful shutdown .." << std::endl;
	}
	return true;
}

static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam)
{
	int length = GetWindowTextLength(hWnd);
	TCHAR* buffer;
	buffer = new TCHAR[length + 1];
	memset(buffer, 0, (length + 1) * sizeof(TCHAR));
	GetWindowText(hWnd, buffer, length + 1);
	//wprintf(L"Window: '%ls'\n", buffer);
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

	if (!ki.Modules(targetPID, modules))
		std::wcout << L"Kernel Interface Modules() failed with 0x"
		<< std::hex << ki.getLastNtStatus() << std::endl;
	else std::wcout << L"Got " << std::dec << modules.size() << L" modules for pid 0x"
		<< std::hex << targetPID << std::endl;
#if 0
	if (!ki.Pages(targetPID, pages))
		std::wcout << L"Kernel Interface Pages() failed with 0x"
		<< std::hex << ki.getLastNtStatus() << std::endl;
	else std::wcout << L"Got " << std::dec << pages.size() << L" mapped pages for pid 0x"
		<< std::hex << targetPID << std::endl;
#endif

	MODULE_DATA *engineDLL = NULL;
	MODULE_DATA *clientDLL = NULL;
	for (MODULE_DATA& md : modules) {
		if (strncmp(md.BaseDllName, "engine.dll", sizeof md.BaseDllName) == 0) {
			std::wcout << L"FOUND ENGINE DLL at " << std::hex << md.DllBase << "!!!" << std::endl;
			engineDLL = &md;
		}
		if (strncmp(md.BaseDllName, "client_panorama.dll", sizeof md.BaseDllName) == 0) {
			std::wcout << L"FOUND CLIENT DLL at " << std::hex << md.DllBase << "!!!" << std::endl;
			clientDLL = &md;
		}
	}

	running = TRUE;
	do {
		if (engineDLL) {
			/* unused */
		}

		if (clientDLL) {
			DWORD dwLocalPlayer = 13580876;
			PVOID localPlayerPtr = (PVOID)((ULONG_PTR)clientDLL->DllBase + dwLocalPlayer);
			localPlayerPtr = (PVOID)((ULONG_PTR)KMemory::Rpm<DWORD>(targetPID, localPlayerPtr));
			std::wcout << L"localPlayerPtr..................: " << std::hex << localPlayerPtr << std::endl;

			DWORD dwEntityList = 80763620;
			PVOID entityListPtr = (PVOID)((ULONG_PTR)clientDLL->DllBase + dwEntityList);
			std::wcout << L"client_panorama.dll+dwEntityList: " << std::hex << entityListPtr << std::endl;

			for (size_t i = 0; i < 32; ++i) {
				PVOID entityPtr = (PVOID)((ULONG_PTR)entityListPtr + (i * 0x10));
				try {
					entityPtr = (PVOID)((ULONG_PTR)KMemory::Rpm<DWORD>(targetPID, entityPtr));
					if (!entityPtr) {
						continue;
					}
				}
				catch (std::runtime_error &) {
					continue;
				}

				DWORD dwHealth = 256;
				PVOID healthPtr = (PVOID)((ULONG_PTR)entityPtr + dwHealth);
				DWORD health;
				try {
					health = KMemory::Rpm<DWORD>(targetPID, healthPtr);
				}
				catch (std::runtime_error &) {
					continue;
				}

				std::wcout << L"entityPtr.......................: " << std::hex << entityPtr << " -> " << std::dec << health << std::endl;

				DWORD dwSpotted = 2365;
				PVOID spottedPtr = (PVOID)((ULONG_PTR)entityPtr + dwSpotted);
				DWORD spotted = KMemory::Rpm<DWORD>(targetPID, spottedPtr);
				DWORD dwSpottedBy = 2432;
				PVOID spottedByPtr = (PVOID)((ULONG_PTR)entityPtr + dwSpottedBy);
				DWORD spottedBy = KMemory::Rpm<DWORD>(targetPID, spottedByPtr);
				if (spotted) {
					spotted = 0;
				}
				else {
					spotted = 1;
					spottedBy |= 0xFF;
					KMemory::Wpm<DWORD>(targetPID, spottedByPtr, &spottedBy);
				}
				KMemory::Wpm<DWORD>(targetPID, spottedPtr, &spotted);
				//std::wcout << L"Sp: " << spotted << std::endl;
			}

			std::this_thread::sleep_for(std::chrono::microseconds(250000));
		} else

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
			}
		}
		catch (std::runtime_error& err) {
			std::wcout << err.what() << std::endl;
		}
	} while (running);

	std::wcout << L"Driver shutdown .." << std::endl;
	ki.Exit();
}