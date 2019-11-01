#include "pch.h"
#include "KMemDriver.h"
#include "KInterface.h"
#include "DLLHelper.h"
#include "PatternScanner.h"

#include <array>
#include <iostream>
#include <iomanip>
#include <windows.h>
#include <Shlwapi.h>

#define WHEXOUT std::setfill(L'0') << std::setw(16) << std::hex

static BOOL running = false;
static const wchar_t *wName = L"HUNT";


static bool consoleHandler(int signal) {
	if (signal == CTRL_C_EVENT) {
		if (!running)
			exit(EXIT_FAILURE);
		running = false;
		std::wcout << L"Waiting for graceful shutdown .." << std::endl;
	}
	return true;
}

static void printBuf(UCHAR *buf, SIZE_T siz, SIZE_T bytesBeforeNewline) {
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

static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam)
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
					if (!strncmp(md.BaseDllName, "CryEntitySystem.dll",
						sizeof md.BaseDllName))
					{
						/* "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.16.27023\bin\Hostx64\x64\cl.exe" /Zp2 /c /d1reportSingleClassLayoutCEntitySystem C:\Users\segfault\Source\Repos\CRYENGINE\Code\CryEngine\CryEntitySystem\EntitySystem.cpp /I C:\Users\segfault\Source\Repos\CRYENGINE\Code\CryEngine\CryCommon /I "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.16.27023\include" /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\ucrt" /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\shared" /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\um" */

						static bool first = true;
						if (first) {
							first = false;

							SymbolResolver sresolv;
							DLLHelper dll(sresolv);
							if (!dll.Init(targetPID, "./TestDLL.dll")) {
								std::wcout << L"DLL Init failed" << std::endl;
							}
							if (!dll.VerifyHeader()) {
								std::wcout << L"DLL VerifyHeader failed" << std::endl;
							}
							if (!dll.InitTargetMemory(/* 0x7ffe00000000 */)) {
								std::wcout << L"DLL InitTargetMemory failed" << std::endl;
							}
							if (!dll.HasImports())
							{
								std::wcout << L"DLL has no ImportTable" << std::endl;
							}
							else if (!dll.FixImports()) {
								std::wcout << L"DLL FixImports failed" << std::endl;
							}
							if (!dll.HasRelocs()) {
								std::wcout << L"DLL has no RelocTable" << std::endl;
							}
							else if (!dll.FixRelocs()) {
								std::wcout << L"DLL FixRelocs failed" << std::endl;
							}
							if (!dll.CopyHeaderAndSections()) {
								std::wcout << L"DLL CopyHeaderAndSections failed" << std::endl;
							}
							std::wcout << L"DLL mapping succesful, "
								<< "BaseAddress: " << WHEXOUT << dll.GetBaseAddress()
								<< ", EntryPoint: " << WHEXOUT << dll.GetEntryPoint() << std::endl;

							PVOID targetAddr = (PVOID)(dll.GetBaseAddress());
							std::wcout << "ADDRESS -> " << WHEXOUT << targetAddr << std::endl;

							UINT64 g_pEnvSys = 0;
							g_pEnvSys = (UINT64)md.DllBase + 0x28E3F8;

							for (MODULE_DATA& md : modules) {
								if (!strncmp(md.BaseDllName, "CryAction.dll",
									sizeof md.BaseDllName)) {

									struct loadlib_user_data llua;
									char * cryDllDir = new char[sizeof md.FullDllPath];
									std::memcpy(cryDllDir, md.FullDllPath, sizeof md.FullDllPath);
									PathRemoveFileSpecA(cryDllDir);
									llua.additionalDllSearchDirectories.push_back(std::string(cryDllDir));
									delete cryDllDir;
									for (auto& dir : llua.additionalDllSearchDirectories) {
										std::wcout << L"AdditionalDLLDir: "
											<< std::wstring(dir.begin(), dir.end()) << std::endl;
									}

									PatternScanner pscan(sresolv, &map_loadlib, &llua);
									std::vector<SIZE_T> foundAddresses;
									pscan.Scan(md, "48 8B 48 20 48 8B 01 FF 90 20 01 00 00", foundAddresses);
									for (auto& addr : foundAddresses) {
										std::wcout << "Addr: " << addr << ", Content: ";
										BYTE content[32];
										KMemoryBuf::Rpm<sizeof content>(targetPID, (PVOID)addr, &content[0]);
										printBuf(content, sizeof content, 32);
									}

									// pEnv: 48 8B 48 20 48 8B 01 FF 90 20 01 00 00
									//globalEnvAddr = (UINT64)md.DllBase + 0x70E848;
									break;
								}
							}

							BYTE cc[] = { /* push rax; push rbx; push rcx; push rdx; push rsi;
											 push rdi; push rsp; push rbp; push r8; push r9;
											 push r10; push r11; push r12; push r13; push r14;
											 push r15 */
										  0x50, 0x53, 0x51, 0x52, 0x56, 0x57,
										  0x54, 0x55, 0x41, 0x50, 0x41, 0x51,
										  0x41, 0x52, 0x41, 0x53, 0x41, 0x54,
										  0x41, 0x55, 0x41, 0x56, 0x41, 0x57,
										  /* nops */
										  0x90, 0x90, 0x90, 0x90, 0x90,
										  /* mov rcx, 0x0000000000000000 */
										  0x48, 0xB9,
										  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										  /* mov rax, 0x0000000000000000 */
										  0x48, 0xB8,
										  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										  /* call rax */
										  0xFF, 0xD0,
										  /* nops */
										  0x90, 0x90,
										  /* pop r15; pop r14; pop r13; pop r12; pop r11;
											 pop r10; pop r9; pop r8; pop rbp; pop rsp;
											 pop rdi; pop rsi; pop rdx; pop rcx; pop rbx;
											 pop rax */
										  0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D,
										  0x41, 0x5C, 0x41, 0x5B, 0x41, 0x5A,
										  0x41, 0x59, 0x41, 0x58, 0x5D, 0x5C,
										  0x5F, 0x5E, 0x5A, 0x59, 0x5B, 0x58,
										  /* nops */
										  0x90, 0x90,
										  /* mov rax, 0x0000000000000000 */
										  0x48, 0xB8,
										  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										  /* jmp rax */
										  0xFF, 0xE0 };
							*(UINT64 *)((BYTE *)cc + 31) = g_pEnvSys;
							*(UINT64 *)((BYTE *)cc + 41) = dll.GetEntryPoint();
							/* PATTERN: 48 89 4C 24 08 48 83 EC 48 +0x275 */
							UINT64 jumpBackAddr = (UINT64)md.DllBase + 0x70875;
							*(UINT64 *)((BYTE *)cc + 81) = jumpBackAddr;
							printBuf(cc, sizeof cc, 32);
							KMemoryBuf::Wpm<sizeof cc>(targetPID, (PVOID)targetAddr, &cc[0]);

							BYTE dd[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };
							*(UINT64 *)((BYTE *)dd + 2) = (UINT64)targetAddr;
							printBuf(dd, sizeof dd, 32);
							/* PATTERN: 48 89 4C 24 08 48 83 EC 48 +0x9 */
							KMemoryBuf::Wpm<sizeof dd>(targetPID, (PVOID)((UINT64)md.DllBase + 0x70609), &dd[0]);
#if 0
							Sleep(1000);
							if (!ki.VUnlink(targetPID, targetAddr)) {
								std::wcout << L"VUnlink failed" << std::endl;
							}
#endif
						}
					}
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