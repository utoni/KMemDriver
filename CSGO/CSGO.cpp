#include "pch.h"
#include "KMemDriver.h"
#include "KInterface.h"

#include <iostream>
#include <iomanip>
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

	for (MODULE_DATA& md : modules) {
		std::wcout << md.BaseDllName << std::endl;
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
			}
		}
		catch (std::runtime_error& err) {
			std::wcout << err.what() << std::endl;
		}
	} while (running);

	std::wcout << L"Driver shutdown .." << std::endl;
	ki.Exit();
}