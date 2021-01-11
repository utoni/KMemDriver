#include "pch.h"
#include "KInterface.h"

#include <iostream>
#include <map>

#define PRINT_CHECK_MSG(message) std::wcout << L ## message ## " "
#define PRINT_FAIL_MSG() std::wcout << L" [FAIL]" << std::endl
#define PRINT_OK_MSG() std::wcout << L" [ OK ]" << std::endl
#define KM_ASSERT_EQUAL(equal, condition, message) \
	do { PRINT_CHECK_MSG(message); if ((condition) != (equal)) { \
		PRINT_FAIL_MSG(); goto error; } else { PRINT_OK_MSG(); } \
	} while (0)
#define KM_ASSERT_EQUAL_NO_MSG(equal, condition) \
	do { if ((condition) != (equal)) { goto error; } } while (0)
#define KM_TEST_SUITE(condition, message) \
	do { \
		PRINT_CHECK_MSG("--- TestSuite " ## message ## " ---\n"); \
		if ((condition) != true) { goto error; } \
	} while(0)

static bool test_Processes(KInterface& ki)
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

	return true;
error:
	return false;
}

static bool test_Modules(KInterface& ki, HANDLE pid)
{
	SIZE_T required_modules_found = 0;
	std::vector<MODULE_DATA> modules;
	KM_ASSERT_EQUAL(true, ki.Modules(pid, modules), "Kernel Interface Modules");
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

	return true;
error:
	return false;
}

static bool test_Pages(KInterface& ki, HANDLE pid)
{
	SIZE_T found_shmaddr = 0;
	std::vector<MEMORY_BASIC_INFORMATION> pages;
	KM_ASSERT_EQUAL(true, ki.Pages(pid, pages), "Kernel Interface Pages");
	KM_ASSERT_EQUAL(0, ki.getLastNtStatus(), "Last NtStatus");
	for (auto& page : pages) {
		if (page.BaseAddress == (PVOID)SHMEM_ADDR && page.RegionSize == SHMEM_SIZE) {
			found_shmaddr++;
		}
	}
	KM_ASSERT_EQUAL(1, found_shmaddr, "Kernel Interface Pages (1 required found)");

	return true;
error:
	return false;
}

static bool test_VirtualMemory(KInterface& ki, HANDLE pid)
{
	PVOID addr = (PVOID)SHMEM_ADDR;
	SIZE_T size = 0x100;

	KM_ASSERT_EQUAL(true,
		ki.VAlloc(pid, &addr, &size, PAGE_READWRITE), "Kernel Interface VirtualAlloc SHMEM_ADDR");
	KM_ASSERT_EQUAL(0, ki.getLastNtStatus(), "Last NtStatus");

	addr = NULL;
	size = 0x100;

	KM_ASSERT_EQUAL(true,
		ki.VAlloc(pid, &addr, &size, PAGE_READWRITE), "Kernel Interface VirtualAlloc");
	KM_ASSERT_EQUAL(0, ki.getLastNtStatus(), "Last NtStatus");
	KM_ASSERT_EQUAL(true,
		ki.VFree(pid, addr, size), "Kernel Interface VirtualFree");
	KM_ASSERT_EQUAL(0, ki.getLastNtStatus(), "Last NtStatus");

	return true;
error:
	return false;
}

static bool test_MemoryReadWrite(KInterface& ki, HANDLE pid)
{
	KERNEL_READ_REQUEST krr;
	KERNEL_WRITE_REQUEST kwr;

	uint8_t redzone0[16];
	uint8_t buffer[128];
	uint8_t redzone1[16];
	uint8_t test_buffer[sizeof(buffer)];
	uint8_t redzone2[16];

	memset(&krr, 0, sizeof(krr));
	memset(&kwr, 0, sizeof(kwr));

	memset(redzone0, 0xFF, sizeof(redzone0));
	memset(redzone1, 0xFF, sizeof(redzone1));
	memset(redzone2, 0xFF, sizeof(redzone2));

	memset(buffer, 0x41, sizeof(buffer));
	memset(test_buffer, 0, sizeof(test_buffer));
	KM_ASSERT_EQUAL(true, ki.RPM(pid, buffer, test_buffer, sizeof(buffer), &krr) &&
		krr.SizeReq == krr.SizeRes && krr.StatusRes == 0, "Kernel RPM stack memory");
	KM_ASSERT_EQUAL(0, memcmp(buffer, test_buffer, sizeof(buffer)), "Kernel RPM stack memory equal");
	KM_ASSERT_EQUAL(true, memcmp(redzone0, redzone1, sizeof(redzone0)) == 0 &&
		memcmp(redzone0, redzone2, sizeof(redzone0)) == 0, "Kernel RPM redzones check");

	memset(buffer, 0x42, sizeof(buffer));
	memset(test_buffer, 0, sizeof(test_buffer));
	KM_ASSERT_EQUAL(true, ki.WPM(pid, test_buffer, buffer, sizeof(buffer), &kwr) &&
		kwr.SizeReq == kwr.SizeRes && kwr.StatusRes == 0, "Kernel WPM stack memory");
	KM_ASSERT_EQUAL(0, memcmp(buffer, test_buffer, sizeof(buffer)), "Kernel WPM stack memory equal");
	KM_ASSERT_EQUAL(true, memcmp(redzone0, redzone1, sizeof(redzone0)) == 0 &&
		memcmp(redzone0, redzone2, sizeof(redzone0)) == 0, "Kernel WPM redzones check");

	return true;
error:
	return false;
}

static std::map<HANDLE, bool> GetCurrentHeaps()
{
	std::map<HANDLE, bool> retval;
	SIZE_T const max_heaps = 64;
	HANDLE heaps[max_heaps];
	DWORD count = GetProcessHeaps(max_heaps, heaps);

	if (count > 0) {
		for (SIZE_T j = 0; j < count; ++j) {
			retval[heaps[j]] = true;
		}
	}
	return retval;
}

static bool test_PagesRPM(KInterface& ki, HANDLE pid)
{
	{
		std::vector<MEMORY_BASIC_INFORMATION> pages;
		KM_ASSERT_EQUAL(true, ki.Pages(pid, pages), "Kernel Interface Pages");
		KM_ASSERT_EQUAL(0, ki.getLastNtStatus(), "Last NtStatus");
		for (auto& page : pages) {
			std::map<HANDLE, bool> heaps = GetCurrentHeaps();
			if (heaps.find(page.BaseAddress) != heaps.end()) {
				continue;
			}
			if (KInterface::PageIsFreed(page) == true || KInterface::PageIsPrivateReserved(page) == true) {
				continue;
			}
			BYTE buf[4096];
			SIZE_T siz = (page.RegionSize > sizeof(buf) ? sizeof(buf) : page.RegionSize);
			KM_ASSERT_EQUAL_NO_MSG(true, ki.RPM(pid, page.BaseAddress, buf, siz, NULL));
			KM_ASSERT_EQUAL_NO_MSG(0, ki.getLastNtStatus());
		}

		std::vector<MEMORY_BASIC_INFORMATION> vq_pages;
		MEMORY_BASIC_INFORMATION page;
		SIZE_T base = NULL;
		while (VirtualQuery((LPVOID)base, &page, sizeof(page)) > 0) {
			std::map<HANDLE, bool> vq_heaps = GetCurrentHeaps();
			base += page.RegionSize;
			if (vq_heaps.find(page.BaseAddress) != vq_heaps.end()) {
				continue;
			}
			vq_pages.push_back(page);
		}

		for (SIZE_T i = 0; i < vq_pages.size(); ++i) {
			if (vq_pages[i].BaseAddress != pages[i].BaseAddress) {
				/* not optimal as we do test all pages */
				break;
			}
			KM_ASSERT_EQUAL_NO_MSG(vq_pages[i].Protect, pages[i].Protect);
			KM_ASSERT_EQUAL_NO_MSG(vq_pages[i].RegionSize, pages[i].RegionSize);
			KM_ASSERT_EQUAL_NO_MSG(vq_pages[i].State, pages[i].State);
			KM_ASSERT_EQUAL_NO_MSG(vq_pages[i].Type, pages[i].Type);
		}

		KM_ASSERT_EQUAL_NO_MSG(pages.size(), pages.size());
	}

	return true;
error:
	return false;
}

int main()
{
	HANDLE this_pid = (HANDLE)((ULONG_PTR)GetCurrentProcessId());

	KInterface& ki = KInterface::getInstance();
	KM_ASSERT_EQUAL(true, true, "Integration Test Init");

	try {
		KM_ASSERT_EQUAL(true, ki.Init(), "Kernel Interface Init");
		KM_ASSERT_EQUAL(true, ki.Handshake(), "Kernel Interface Handshake");
		KM_ASSERT_EQUAL(true, ki.getKHandle() != ki.getUHandle() && ki.getKHandle() != NULL && ki.getUHandle() != NULL, "Kernel Interface Handles");
		KM_ASSERT_EQUAL(true, ki.getBuffer() != NULL, "Kernel Interface Buffer != NULL");
		KM_ASSERT_EQUAL(SRR_TIMEOUT, ki.RecvWait(1000), "Kernel Interface Receive Wait");
		KM_ASSERT_EQUAL(true, ki.Ping(), "Kernel Interface PING - PONG #1");
		KM_ASSERT_EQUAL(true, ki.Ping(), "Kernel Interface PING - PONG #2");
		KM_ASSERT_EQUAL(true, ki.Ping(), "Kernel Interface PING - PONG #3");
		KM_TEST_SUITE(test_Processes(ki), "Processes");
		KM_TEST_SUITE(test_Modules(ki, this_pid), "Modules");
		KM_TEST_SUITE(test_Pages(ki, this_pid), "Pages");
		KM_TEST_SUITE(test_VirtualMemory(ki, this_pid), "VirtualMemory");
		KM_TEST_SUITE(test_MemoryReadWrite(ki, this_pid), "MemoryReadWrite");
		KM_TEST_SUITE(test_PagesRPM(ki, this_pid), "ModulesPagesRPM");
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
	std::getchar();
}