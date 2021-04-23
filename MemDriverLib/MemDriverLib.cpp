// MemDriverLib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "KInterface.h"
#include "KMemDriver.h"

#include <sstream>
#include <ctime>
#include <stdexcept>

#pragma warning(push)
#pragma warning(disable : 26812)

KInterface::KInterface()
{
}

bool KInterface::Init()
{
	std::srand((unsigned int)std::time(nullptr));
	m_shmem = VirtualAlloc((PVOID)SHMEM_ADDR, SHMEM_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	m_kevent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_uevent = CreateEvent(NULL, FALSE, FALSE, NULL);
	return m_shmem && m_kevent != INVALID_HANDLE_VALUE && m_uevent != INVALID_HANDLE_VALUE;
}

bool KInterface::Handshake()
{
	PKERNEL_HANDSHAKE hnds = (PKERNEL_HANDSHAKE)getBuffer();
	hnds->kevent = m_kevent;
	hnds->uevent = m_uevent;
	m_last_ntstatus = INVALID_NTSTATUS;
	return SendRecvWait(MEM_HANDSHAKE) == SRR_SIGNALED;
}

bool KInterface::Ping()
{
	SendRecvReturn srr;
	PKERNEL_PING ping = (PKERNEL_PING)getBuffer();
	m_last_ping_value = ping->rnd_user = (std::rand() << 16) | std::rand();
	std::srand(m_last_ping_value);
	m_last_ntstatus = INVALID_NTSTATUS;
	srr = SendRecvWait(MEM_PING);
	if (ping->rnd_kern != getLastPingValue())
		return false;
	return srr == SRR_SIGNALED;
}

bool KInterface::Processes(std::vector<PROCESS_DATA>& dest)
{
	SendRecvReturn srr;
	PKERNEL_PROCESSES_REQUEST processes = (PKERNEL_PROCESSES_REQUEST)getBuffer();
	PPROCESS_DATA data = (PPROCESS_DATA)(processes + 1);

	m_last_ntstatus = INVALID_NTSTATUS;
	srr = SendRecvWait(MEM_PROCESSES);
	if (srr == SRR_SIGNALED) {
		m_last_ntstatus = processes->StatusRes;
		if (validateResponeEx(processes, processes->StatusRes, processes->ProcessCount * sizeof(PROCESS_DATA)) == MEM_PROCESSES)
		{
			for (SIZE_T i = 0; i < processes->ProcessCount; ++i, ++data)
			{
				dest.push_back(*data);
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}

	return true;
}

bool KInterface::Pages(HANDLE targetPID,
	std::vector<MEMORY_BASIC_INFORMATION>& dest,
	PVOID start_address)
{
	PKERNEL_PAGE pages = (PKERNEL_PAGE)getBuffer();
	const ULONGLONG max_pages = (SHMEM_SIZE - sizeof * pages +
		sizeof pages->pages_start) / sizeof pages->pages_start;
	SendRecvReturn srr;
	bool success = false;

	do {
		m_last_ntstatus = INVALID_NTSTATUS;
		pages->pages = 0;
		pages->ProcessId = targetPID;
		pages->StartAddress = start_address;
		srr = SendRecvWait(MEM_PAGES);
		if (srr == SRR_SIGNALED) {
			m_last_ntstatus = pages->StatusRes;
			if (validateRespone(getBuffer()) == MEM_PAGES &&
				!pages->StatusRes &&
				pages->pages * sizeof(pages->pages_start) <= SHMEM_SIZE)
			{
				for (SIZE_T i = 0; i < pages->pages; ++i) {
					dest.push_back((&pages->pages_start)[i]);
					start_address = (PVOID)
						((ULONG_PTR)((&pages->pages_start)[i].BaseAddress)
							+ (&pages->pages_start)[i].RegionSize);
				}
				success = true;
			}
			else {
				success = false;
				break;
			}
		}
	} while (srr == SRR_SIGNALED && pages->pages == max_pages && pages->pages);
	return success && srr == SRR_SIGNALED;;
}

bool KInterface::Modules(HANDLE targetPID,
	std::vector<MODULE_DATA>& dest)
{
	PKERNEL_MODULES mods = (PKERNEL_MODULES)getBuffer();
	SIZE_T start_index = 0;
	const ULONGLONG max_mods = (SHMEM_SIZE - sizeof * mods +
		sizeof mods->modules_start) / sizeof mods->modules_start;
	SendRecvReturn srr;
	bool success = false;

	do {
		m_last_ntstatus = INVALID_NTSTATUS;
		mods->modules = 0;
		mods->ProcessId = targetPID;
		mods->StartIndex = start_index;
		srr = SendRecvWait(MEM_MODULES);
		if (srr == SRR_SIGNALED) {
			m_last_ntstatus = mods->StatusRes;
			if (validateRespone(getBuffer()) == MEM_MODULES &&
				!mods->StatusRes &&
				mods->modules * sizeof(mods->modules_start) <= SHMEM_SIZE)
			{
				for (SIZE_T i = 0; i < mods->modules; ++i) {
					dest.push_back((&mods->modules_start)[i]);
					start_index++;
				}
				success = true;
			}
			else {
				success = false;
				break;
			}
		}
	} while (srr == SRR_SIGNALED && mods->modules == max_mods && mods->modules);
	return success && srr == SRR_SIGNALED;
}

bool KInterface::Exit()
{
	if (m_pingThreadStarted == true) {
		m_pingThreadStarted = false;
		m_pingThread.join();
	}
	m_last_ntstatus = INVALID_NTSTATUS;
	return SendRecvWait(MEM_EXIT, INFINITE) == SRR_SIGNALED;
}

bool KInterface::RPM(HANDLE targetPID, PVOID address, BYTE* buf, SIZE_T size,
	PKERNEL_READ_REQUEST result)
{
	PKERNEL_READ_REQUEST rr = (PKERNEL_READ_REQUEST)getBuffer();
	m_last_ntstatus = INVALID_NTSTATUS;
	if (size > SHMEM_SIZE - sizeof * rr)
		return false;
	rr->ProcessId = targetPID;
	rr->Address = address;
	rr->SizeReq = size;
	rr->SizeRes = (SIZE_T)-1;
	rr->StatusRes = (NTSTATUS)-1;
	if (SendRecvWait(MEM_RPM) == SRR_SIGNALED) {
		m_last_ntstatus = rr->StatusRes;
		if (rr->StatusRes ||
			rr->SizeRes != size)
		{
			std::stringstream err_str;
			err_str << "Call RPM(0x" << std::hex << address
				<< "," << std::dec << size
				<< ") failed with 0x"
				<< std::hex << rr->StatusRes
				<< " (Size Req/Res: "
				<< std::dec << rr->SizeReq << "/" << (SSIZE_T)rr->SizeRes
				<< ")";
			throw std::runtime_error(err_str.str());
		}
		memcpy(buf, (BYTE*)rr + sizeof * rr, size);
		if (result)
			*result = *rr;
		return true;
	}
	return false;
}

bool KInterface::WPM(HANDLE targetPID, PVOID address, BYTE* buf, SIZE_T size,
	PKERNEL_WRITE_REQUEST result)
{
	PKERNEL_WRITE_REQUEST wr = (PKERNEL_WRITE_REQUEST)getBuffer();
	m_last_ntstatus = INVALID_NTSTATUS;
	if (size > SHMEM_SIZE - sizeof * wr)
		return false;
	wr->ProcessId = targetPID;
	wr->Address = address;
	wr->SizeReq = size;
	wr->SizeRes = (SIZE_T)-1;
	wr->StatusRes = (NTSTATUS)-1;
	memcpy((BYTE*)wr + sizeof * wr, buf, size);
	if (SendRecvWait(MEM_WPM) == SRR_SIGNALED) {
		m_last_ntstatus = wr->StatusRes;
		if (wr->StatusRes ||
			wr->SizeRes != size)
		{
			std::stringstream err_str;
			err_str << "Call WPM(0x" << std::hex << address
				<< "," << std::dec << size
				<< ") failed with 0x"
				<< std::hex << wr->StatusRes
				<< " (Size Req/Res: "
				<< std::dec << wr->SizeReq << "/" << (SSIZE_T)wr->SizeRes
				<< ")";
			throw std::runtime_error(err_str.str());
		}
		if (result)
			*result = *wr;
		return true;
	}
	return false;
}

bool KInterface::VAlloc(HANDLE targetPID, PVOID* address, SIZE_T* size, ULONG protection)
{
	PKERNEL_VALLOC_REQUEST vr = (PKERNEL_VALLOC_REQUEST)getBuffer();
	m_last_ntstatus = INVALID_NTSTATUS;
	vr->ProcessId = targetPID;
	vr->AddressReq = *address;
	vr->SizeReq = *size;
	vr->Protection = protection;
	vr->AddressRes = NULL;
	vr->SizeRes = (SIZE_T)-1;
	vr->StatusRes = (NTSTATUS)-1;
	if (SendRecvWait(MEM_VALLOC) == SRR_SIGNALED) {
		m_last_ntstatus = vr->StatusRes;
		if (vr->StatusRes ||
			vr->SizeRes < *size)
		{
			std::stringstream err_str;
			err_str << "Call VAlloc(0x" << std::hex << *address
				<< "," << std::dec << *size
				<< ") failed with 0x"
				<< std::hex << vr->StatusRes
				<< " (Size Req/Res: "
				<< std::dec << vr->SizeReq << "/" << (SSIZE_T)vr->SizeRes
				<< ")";
			throw std::runtime_error(err_str.str());
		}
		*address = vr->AddressRes;
		*size = vr->SizeRes;
		return true;
	}
	return false;
}

bool KInterface::VFree(HANDLE targetPID, PVOID address, SIZE_T size)
{
	PKERNEL_VFREE_REQUEST vr = (PKERNEL_VFREE_REQUEST)getBuffer();
	m_last_ntstatus = INVALID_NTSTATUS;
	vr->ProcessId = targetPID;
	vr->Address = address;
	vr->Size = size;
	vr->StatusRes = (NTSTATUS)-1;
	if (SendRecvWait(MEM_VFREE) == SRR_SIGNALED) {
		m_last_ntstatus = vr->StatusRes;
		if (vr->StatusRes)
		{
			std::stringstream err_str;
			err_str << "Call VFree(0x" << std::hex << address
				<< "," << std::dec << size
				<< ") failed with 0x"
				<< std::hex << vr->StatusRes
				<< " with size " << std::dec << vr->Size;
			throw std::runtime_error(err_str.str());
		}
		return true;
	}
	return false;
}

PVOID KInterface::getBuffer() {
	if (!m_shmem)
		throw std::runtime_error("Call Init() before..");
	return m_shmem;
}

HANDLE KInterface::getKHandle() {
	if (!m_kevent)
		throw std::runtime_error("Call Init() before..");
	return m_kevent;
}

HANDLE KInterface::getUHandle() {
	if (!m_uevent)
		throw std::runtime_error("Call Init() before..");
	return m_uevent;
}

UINT32 KInterface::getLastPingValue() {
	return m_last_ping_value;
}

UINT32 KInterface::getLastNtStatus() {
	return m_last_ntstatus;
}

SendRecvReturn KInterface::SendRecvWait(UINT32 type, DWORD timeout)
{
	prepareRequest(getBuffer(), type);
	if (!SetEvent(m_kevent))
		return SRR_ERR_KEVENT;
	return RecvWait(timeout);
}

SendRecvReturn KInterface::RecvWait(DWORD timeout)
{
	switch (WaitForSingleObject(m_uevent, timeout)) {
	case WAIT_OBJECT_0:
		return validateRespone(getBuffer()) != INVALID_REQUEST ? SRR_SIGNALED : SRR_ERR_HEADER;
	case WAIT_TIMEOUT:
		return SRR_TIMEOUT;
	}
	return SRR_ERR_UEVENT;
}

void KInterface::PingThread(void(__cdecl* onTimeout)(void))
{
	while (m_pingThreadStarted == true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_TIMEOUT_MS));
		if (MtPing() != true) {
			m_pingThreadStarted = false;
			break;
		}
	}
	onTimeout();
}

void KInterface::StartPingThread(void(__cdecl* onTimeout)(void))
{
	m_pingThreadStarted = true;
	m_pingThread = std::move(std::thread(&KInterface::PingThread, this, onTimeout));
}

#pragma warning(pop)