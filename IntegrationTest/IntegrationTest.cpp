// IntegrationTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "KInterface.h"

#include <iostream>

#define PRINT_CHECK_MSG(message) std::wcout << L ## message
#define PRINT_FAIL_MSG() std::wcout << L" [FAIL]" << std::endl
#define PRINT_OK_MSG() std::wcout << L" [ OK ]" << std::endl
#define KM_ASSERT_EQUAL(equal, condition, message) \
	do { PRINT_CHECK_MSG(message); if ((condition) != (equal)) { \
		PRINT_FAIL_MSG(); goto error; } else { PRINT_OK_MSG(); } \
	} while (0);

int main()
{
	HANDLE this_pid = (HANDLE)((ULONG_PTR)GetCurrentProcessId());

	KInterface& ki = KInterface::getInstance();
	KM_ASSERT_EQUAL(true, true, "Integration Test Init");

	try {
		KM_ASSERT_EQUAL(true, ki.Init(), "Kernel Interface Init");
		KM_ASSERT_EQUAL(true, ki.Handshake(), "Kernel Interface Handshake");
		KM_ASSERT_EQUAL(true, ki.getBuffer() != NULL, "Kernel Interface Buffer != NULL");
		KM_ASSERT_EQUAL(true, ki.getKHandle() != ki.getUHandle() && ki.getKHandle() != NULL && ki.getUHandle() != NULL, "Kernel Interface Handles");
		KM_ASSERT_EQUAL(SRR_TIMEOUT, ki.RecvWait(), "Kernel Interface Receive Wait");
		KM_ASSERT_EQUAL(true, ki.Ping(), "Kernel Interface PING - PONG #1");
		KM_ASSERT_EQUAL(true, ki.Ping(), "Kernel Interface PING - PONG #2");
		KM_ASSERT_EQUAL(true, ki.Ping(), "Kernel Interface PING - PONG #3");

		{
			SIZE_T required_processes_found = 0;
			std::vector<PROCESS_DATA> processes;
			KM_ASSERT_EQUAL(true, ki.Processes(processes), "Kernel Interface Processes");
			KM_ASSERT_EQUAL(0, ki.getLastNtStatus(), "Last NtStatus");

			for (auto& process : processes)
			{
				//std::cout << "Process Name: " << process.ImageName << std::endl;
				if (strcmp(process.ImageName, "IntegrationTest-kmem.exe") == 0 && strlen(process.ImageName) == strlen("IntegrationTest-kmem.exe"))
				{
					required_processes_found++;
				}
				if (strcmp(process.ImageName, "System") == 0 && strlen(process.ImageName) == strlen("System"))
				{
					required_processes_found++;
				}
				if (strcmp(process.ImageName, "Registry") == 0 && strlen(process.ImageName) == strlen("Registry"))
				{
					required_processes_found++;
				}
				if (strcmp(process.ImageName, "wininit.exe") == 0 && strlen(process.ImageName) == strlen("wininit.exe"))
				{
					required_processes_found++;
				}
				if (strcmp(process.ImageName, "winlogon.exe") == 0 && strlen(process.ImageName) == strlen("winlogon.exe"))
				{
					required_processes_found++;
				}
				if (strcmp(process.ImageName, "lsass.exe") == 0 && strlen(process.ImageName) == strlen("lsass.exe"))
				{
					required_processes_found++;
				}
			}
			KM_ASSERT_EQUAL(6, required_processes_found, "Kernel Interface Modules (6 required found)");
		}

		{
			SIZE_T required_modules_found = 0;
			std::vector<MODULE_DATA> modules;
			KM_ASSERT_EQUAL(true, ki.Modules(this_pid, modules), "Kernel Interface Modules");
			KM_ASSERT_EQUAL(0, ki.getLastNtStatus(), "Last NtStatus");
			for (auto& module : modules) {
				//std::cout << "DLL Name: " << module.BaseDllName << std::endl;
				if (strcmp(module.BaseDllName, "IntegrationTest-kmem.exe") == 0 && strlen(module.BaseDllName) == strlen("IntegrationTest-kmem.exe"))
				{
					required_modules_found++;
				}
				if (strcmp(module.BaseDllName, "ntdll.dll") == 0 && strlen(module.BaseDllName) == strlen("ntdll.dll"))
				{
					required_modules_found++;
				}
				if (strcmp(module.BaseDllName, "KERNEL32.DLL") == 0 && strlen(module.BaseDllName) == strlen("KERNEL32.DLL"))
				{
					required_modules_found++;
				}
			}
			KM_ASSERT_EQUAL(3, required_modules_found, "Kernel Interface Modules (3 required found)");
		}

		{
			SIZE_T found_shmaddr = 0;
			std::vector<MEMORY_BASIC_INFORMATION> pages;
			KM_ASSERT_EQUAL(true, ki.Pages(this_pid, pages), "Kernel Interface Pages");
			KM_ASSERT_EQUAL(0, ki.getLastNtStatus(), "Last NtStatus");
			for (auto& page : pages) {
				if (page.BaseAddress == (PVOID)SHMEM_ADDR && page.RegionSize == SHMEM_SIZE) {
					found_shmaddr++;
				}
			}
			KM_ASSERT_EQUAL(1, found_shmaddr, "Kernel Interface Pages (1 required found)");
		}

		{
			PVOID addr = (PVOID)SHMEM_ADDR;
			SIZE_T size = 0x100;

			KM_ASSERT_EQUAL(true,
				ki.VAlloc(this_pid, &addr, &size, PAGE_READWRITE), "Kernel Interface VirtualAlloc SHMEM_ADDR");

			addr = NULL;
			size = 0x100;

			KM_ASSERT_EQUAL(true,
				ki.VAlloc(this_pid, &addr, &size, PAGE_READWRITE), "Kernel Interface VirtualAlloc");
			KM_ASSERT_EQUAL(true,
				ki.VFree(this_pid, addr, size), "Kernel Interface VirtualFree");
		}

		KM_ASSERT_EQUAL(true, ki.Ping(), "Kernel Interface PING - PONG #4");
		KM_ASSERT_EQUAL(true, ki.Exit(), "Kernel Interface Driver Shutdown");
	}
	catch (std::runtime_error& err) {
		std::wcout << err.what() << std::endl;
	}

	std::wcout << "Done." << std::endl;
error:
	ki.Exit();
	std::wcout << std::endl << "KMemDriver Shutdown [PRESS RETURN KEY TO EXIT]" << std::endl;
	getchar();
}