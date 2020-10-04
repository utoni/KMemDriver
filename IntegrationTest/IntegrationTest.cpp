// IntegrationTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "KInterface.h"

#include <iostream>

#define PRINT_CHECK_MSG(message) std::wcout << L ## message
#define PRINT_FAIL_MSG() std::wcout << L" [FAIL]" << std::endl
#define PRINT_OK_MSG() std::wcout << L" [ OK ]" << std::endl
#define KM_ASSERT_EQUAL(equal, condition, message) \
	do { PRINT_CHECK_MSG(message); if (condition != equal) { \
		PRINT_FAIL_MSG(); goto error; } else { PRINT_OK_MSG(); } \
	} while (0);

int main()
{
	ULONG_PTR this_pid = GetCurrentProcessId();

	KM_ASSERT_EQUAL(true, true, "Integration Test Init");

	try {
		KInterface &ki = KInterface::getInstance();
		KM_ASSERT_EQUAL(true, ki.Init(), "Kernel Interface Init");
		KM_ASSERT_EQUAL(true, ki.Handshake(), "Kernel Interface Handshake");
		KM_ASSERT_EQUAL(SRR_TIMEOUT, ki.RecvWait(), "Kernel Interface Receive Wait");
		KM_ASSERT_EQUAL(true, ki.Ping(), "Kernel Interface PING - PONG #1");
		KM_ASSERT_EQUAL(true, ki.Ping(), "Kernel Interface PING - PONG #2");
		KM_ASSERT_EQUAL(true, ki.Ping(), "Kernel Interface PING - PONG #3");

		{
			PVOID addr = (PVOID)SHMEM_ADDR;
			SIZE_T size = 0x100;

			KM_ASSERT_EQUAL(true,
				ki.VAlloc((HANDLE)this_pid, &addr, &size, PAGE_READWRITE), "Kernel Interface VirtualAlloc SHMEM_ADDR");

			addr = NULL;
			size = 0x100;

			KM_ASSERT_EQUAL(true,
				ki.VAlloc((HANDLE)this_pid, &addr, &size, PAGE_READWRITE), "Kernel Interface VirtualAlloc");
			KM_ASSERT_EQUAL(true,
				ki.VFree((HANDLE)this_pid, addr, size), "Kernel Interface VirtualFree");
		}

		KM_ASSERT_EQUAL(true, ki.Ping(), "Kernel Interface PING - PONG #4");
		KM_ASSERT_EQUAL(true, ki.Exit(), "Kernel Interface Driver Shutdown");
	}
	catch (std::runtime_error& err) {
		std::wcout << err.what() << std::endl;
	}

	std::wcout << "Done." << std::endl;
error:
	std::wcout << std::endl << "[PRESS RETURN KEY TO EXIT]" << std::endl;
	getchar();
}