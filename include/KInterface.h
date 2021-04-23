#pragma once

#include "KMemDriver.h"

#include <Windows.h>

#include <mutex>
#include <stdexcept>
#include <vector>

#define DEFAULT_TIMEOUT_MS ((KRNL_WAIT_TIME_US / 1000) * (KRNL_MAX_WAITS - 1))
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

	bool MtInit() {
		std::unique_lock<std::mutex> lck(m_jobLock);
		return Init();
	}
	bool MtHandshake() {
		std::unique_lock<std::mutex> lck(m_jobLock);
		return Handshake();
	}
	bool MtPing() {
		std::unique_lock<std::mutex> lck(m_jobLock);
		return Ping();
	}
	bool MtProcesses(std::vector<PROCESS_DATA>& dest) {
		std::unique_lock<std::mutex> lck(m_jobLock);
		return Processes(dest);
	}
	bool MtPages(HANDLE targetPID, std::vector<MEMORY_BASIC_INFORMATION>& dest, PVOID start_address = NULL) {
		std::unique_lock<std::mutex> lck(m_jobLock);
		return Pages(targetPID, dest, start_address);
	}
	bool MtModules(HANDLE targetPID, std::vector<MODULE_DATA>& dest) {
		std::unique_lock<std::mutex> lck(m_jobLock);
		return Modules(targetPID, dest);
	}
	bool MtExit() {
		std::unique_lock<std::mutex> lck(m_jobLock);
		return Exit();
	}
	bool MtRPM(HANDLE targetPID, PVOID address, BYTE* buf, SIZE_T size, PKERNEL_READ_REQUEST result) {
		std::unique_lock<std::mutex> lck(m_jobLock);
		return RPM(targetPID, address, buf, size, result);
	}
	bool MtWPM(HANDLE targetPID, PVOID address, BYTE* buf, SIZE_T size, PKERNEL_WRITE_REQUEST result) {
		std::unique_lock<std::mutex> lck(m_jobLock);
		return WPM(targetPID, address, buf, size, result);
	}
	bool MtVAlloc(HANDLE targetPID, PVOID* address, SIZE_T* size, ULONG protection) {
		std::unique_lock<std::mutex> lck(m_jobLock);
		return VAlloc(targetPID, address, size, protection);
	}
	bool MtVFree(HANDLE targetPID, PVOID address, SIZE_T size) {
		std::unique_lock<std::mutex> lck(m_jobLock);
		return VFree(targetPID, address, size);
	}

	PVOID getBuffer();
	HANDLE getKHandle();
	HANDLE getUHandle();
	UINT32 getLastPingValue();
	UINT32 getLastNtStatus();

	static bool PageIsFreed(MEMORY_BASIC_INFORMATION& page) {
		return (page.Protect & PAGE_NOACCESS) != 0;
	}
	static bool PageIsPrivateReserved(MEMORY_BASIC_INFORMATION& page) {
		return page.Protect == 0;
	}
	SendRecvReturn RecvWait(DWORD timeout = DEFAULT_TIMEOUT_MS);
	void StartPingThread(void(__cdecl* onTimeout)(void));

private:
	SendRecvReturn SendRecvWait(UINT32 type, DWORD timeout = DEFAULT_TIMEOUT_MS);
	void PingThread(void(__cdecl* onTimeout)(void));

	PVOID m_shmem = NULL;
	HANDLE m_kevent = NULL, m_uevent = NULL;

	UINT32 m_last_ping_value = 0;
	UINT32 m_last_ntstatus = INVALID_NTSTATUS;

	bool m_pingThreadStarted = false;
	std::thread m_pingThread;
	std::mutex m_jobLock;
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