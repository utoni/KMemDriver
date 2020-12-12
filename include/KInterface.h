#pragma once

#include "KMemDriver.h"

#include <stdexcept>
#include <vector>
#include <Windows.h>

#define DEFAULT_TIMEOUT 2500
#define INVALID_NTSTATUS (UINT32)-1

typedef enum SendRecvReturn {
	SRR_INVALID = 0, SRR_SIGNALED, SRR_TIMEOUT, SRR_ERR_UEVENT, SRR_ERR_KEVENT, SRR_ERR_HEADER
} SendRecvReturn;

class KInterface
{
public:
	static KInterface& getInstance()
	{
		static KInterface instance;
		return instance;
	}
	KInterface();
	KInterface(KInterface const&) = delete;
	void operator=(KInterface const&) = delete;

	bool Init();
	bool Handshake();
	bool Ping();
	bool Processes(std::vector<PROCESS_DATA>& dest);
	bool Pages(HANDLE targetPID,
		std::vector<MEMORY_BASIC_INFORMATION>& dest,
		PVOID start_address = NULL);
	bool Modules(HANDLE targetPID,
		std::vector<MODULE_DATA>& dest);
	bool Exit();
	bool RPM(HANDLE targetPID, PVOID address, BYTE* buf, SIZE_T size,
		PKERNEL_READ_REQUEST result);
	bool WPM(HANDLE targetPID, PVOID address, BYTE* buf, SIZE_T size,
		PKERNEL_WRITE_REQUEST result);
	bool VAlloc(HANDLE targetPID, PVOID* address, SIZE_T* size, ULONG protection);
	bool VFree(HANDLE targetPID, PVOID address, SIZE_T size);
	bool VUnlink(HANDLE targetPID, PVOID address);

	PVOID getBuffer();
	HANDLE getKHandle();
	HANDLE getUHandle();
	UINT32 getLastPingValue();
	UINT32 getLastNtStatus();
	SendRecvReturn RecvWait(DWORD timeout = DEFAULT_TIMEOUT);

private:
	SendRecvReturn SendRecvWait(UINT32 type, DWORD timeout = DEFAULT_TIMEOUT);

	PVOID m_shmem = NULL;
	HANDLE m_kevent = NULL, m_uevent = NULL;

	UINT32 m_last_ping_value = 0;
	UINT32 m_last_ntstatus = INVALID_NTSTATUS;
};

class KMemory
{
public:
	template <class T>
	static T Rpm(HANDLE targetPID, PVOID address) {
		T buf;
		if (!KInterface::getInstance().RPM(targetPID, address, (BYTE*)&buf, sizeof buf, NULL))
			throw std::runtime_error("KMemory RPM failed");
		return buf;
	}
	template <class T>
	static void Wpm(HANDLE targetPID, PVOID address, T* buf) {
		if (!KInterface::getInstance().WPM(targetPID, address, (BYTE*)buf, sizeof * buf, NULL))
			throw std::runtime_error("KMemory WPM failed");
	}
};

class KMemoryBuf
{
public:
	template <size_t SIZE>
	static SSIZE_T Rpm(HANDLE targetPID, PVOID address, BYTE* dest) {
		KERNEL_READ_REQUEST rr = { 0 };
		if (!KInterface::getInstance().RPM(targetPID, address, &dest[0], SIZE, &rr))
			return -1;
		return rr.SizeRes;
	}
	template <size_t SIZE>
	static SSIZE_T Wpm(HANDLE targetPID, PVOID address, BYTE* dest) {
		KERNEL_WRITE_REQUEST wr = { 0 };
		if (!KInterface::getInstance().WPM(targetPID, address, &dest[0], SIZE, &wr))
			return -1;
		return wr.SizeRes;
	}
};