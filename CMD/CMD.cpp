#include "pch.h"
#include "KMemDriver.h"
#include "KInterface.h"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <windows.h>

static BOOL running = false;
static const wchar_t *wName = L"desk"; /* name of the CMD windows */


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

	MODULE_DATA *dll = NULL;
	for (MODULE_DATA& md : modules) {
		if (strncmp(md.BaseDllName, "msvcrt.dll", sizeof md.BaseDllName) == 0) {
			std::wcout << L"FOUND ENGINE DLL at " << std::hex << md.DllBase << "!!!" << std::endl;
			dll = &md;
		}
	}

	running = TRUE;
	do {
		if (dll) {
			DWORD dwRData = 0x76000;
			//DWORD dwRData = 0x8f000;
			PVOID rdata = (PVOID)((ULONG_PTR)dll->DllBase + dwRData);
			DWORD value = 0xDEADC0DE;
			KMemory::Wpm<DWORD>(targetPID, rdata, &value);
			value = 0x0;
			value = KMemory::Rpm<DWORD>(targetPID, rdata);
			std::cout << "Value: " << std::hex << value << std::endl;

			PVOID targetAddr = (PVOID)((UINT64)NULL);
			SIZE_T targetSize = 4096;
			try {
				if (!ki.VAlloc(targetPID, &targetAddr, &targetSize, PAGE_EXECUTE_READWRITE)) {
					std::wcout << L"VAlloc failed" << std::endl;
				}
#if 0
				if (!ki.VUnlink(targetPID, targetAddr)) {
					std::wcout << L"VUnlink failed" << std::endl;
				}
#endif
				if (!ki.VFree(targetPID, targetAddr, targetSize)) {
					std::wcout << L"VFree failed" << std::endl;
				}
			}
			catch (std::runtime_error& err) {
				std::wcout << err.what() << std::endl;
			}

			std::this_thread::sleep_for(std::chrono::microseconds(2500000));
		}
		else

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