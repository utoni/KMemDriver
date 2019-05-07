#include "pch.h"
#include "KInterface.h"
#include "Driver.h"

#include <sstream>
#include <ctime>
#include <stdexcept>


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

bool KInterface::Pages(HANDLE targetPID,
	std::vector<MEMORY_BASIC_INFORMATION>& dest,
	PVOID start_address)
{
	PKERNEL_PAGE pages = (PKERNEL_PAGE)getBuffer();
	const ULONGLONG max_pages = (SHMEM_SIZE - sizeof *pages +
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
	const ULONGLONG max_mods = (SHMEM_SIZE - sizeof *mods +
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
	m_last_ntstatus = INVALID_NTSTATUS;
	return SendRecvWait(MEM_EXIT, INFINITE) == SRR_SIGNALED;
}

bool KInterface::RPM(HANDLE targetPID, PVOID address, BYTE *buf, SIZE_T size,
	PKERNEL_READ_REQUEST result)
{
	PKERNEL_READ_REQUEST rr = (PKERNEL_READ_REQUEST)getBuffer();
	m_last_ntstatus = INVALID_NTSTATUS;
	if (size > SHMEM_SIZE - sizeof *rr)
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
		memcpy(buf, (BYTE *)rr + sizeof *rr, size);
		if (result)
			*result = *rr;
		return true;
	}
	return false;
}

bool KInterface::WPM(HANDLE targetPID, PVOID address, BYTE *buf, SIZE_T size,
	PKERNEL_WRITE_REQUEST result)
{
	PKERNEL_WRITE_REQUEST wr = (PKERNEL_WRITE_REQUEST)getBuffer();
	m_last_ntstatus = INVALID_NTSTATUS;
	if (size > SHMEM_SIZE - sizeof *wr)
		return false;
	wr->ProcessId = targetPID;
	wr->Address = address;
	wr->SizeReq = size;
	wr->SizeRes = (SIZE_T)-1;
	wr->StatusRes = (NTSTATUS)-1;
	memcpy((BYTE *)wr + sizeof *wr, buf, size);
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

SSIZE_T KScan::KScanSimple(HANDLE targetPID, PVOID start_address, SIZE_T max_scansize,
	PVOID scanbuf, SIZE_T scanbuf_size)
{
	ULONG_PTR max_addr;
	ULONG_PTR cur_addr = (ULONG_PTR)start_address;
	BYTE tmp_rpmbuf[SHMEM_SIZE];
	SIZE_T scan_index, processed, real_size, diff_size;
	std::vector<MEMORY_BASIC_INFORMATION> mbis;
	KERNEL_READ_REQUEST rr = { 0 };

	if (max_scansize < scanbuf_size)
		return -1;
	if (!KInterface::getInstance().Pages(targetPID, mbis, start_address))
		return -1;

	diff_size = (ULONG_PTR)start_address - (ULONG_PTR)mbis.at(0).BaseAddress;
	real_size = (mbis.at(0).RegionSize - diff_size > max_scansize ?
		max_scansize : (ULONG_PTR)mbis.at(0).RegionSize - diff_size);
	max_addr = (ULONG_PTR)start_address + real_size;

	while (cur_addr < max_addr) {
		if (!KInterface::getInstance().RPM(targetPID, (PVOID)cur_addr,
			tmp_rpmbuf, (sizeof tmp_rpmbuf > real_size ? real_size : sizeof tmp_rpmbuf), &rr))
		{
			break;
		}

		if (rr.StatusRes || rr.SizeRes < scanbuf_size)
			break;

		for (processed = 0, scan_index = 0; processed < rr.SizeRes; ++processed) {
			if (tmp_rpmbuf[processed] != *((BYTE*)scanbuf + scan_index)) {
				scan_index = 0;
			}
			else {
				scan_index++;
				if (scan_index == scanbuf_size) {
					return cur_addr + processed - scanbuf_size + 1;
				}
			}
		}
		cur_addr += processed;
		real_size -= processed;
	}
	return -1;
}

SSIZE_T KScan::KBinDiffSimple(HANDLE targetPID, PVOID start_address,
	BYTE *curbuf, BYTE *oldbuf, SIZE_T siz, std::vector<std::pair<SIZE_T, SIZE_T>> *diffs)
{
	SSIZE_T scanned, diff_start;
	SIZE_T diff_size;
	KERNEL_READ_REQUEST rr = { 0 };

	if (!KInterface::getInstance().RPM(targetPID, start_address,
		curbuf, siz, &rr))
	{
		scanned = -1;
	}
	else scanned = rr.SizeRes;

	if (scanned > 0) {
		diffs->clear();
		diff_start = -1;
		diff_size = 0;
		for (SIZE_T i = 0; i < (SIZE_T)scanned; ++i) {
			if (curbuf[i] != oldbuf[i]) {
				if (diff_start < 0)
					diff_start = i;
				diff_size++;
			}
			else if (diff_start >= 0) {
				diffs->push_back(std::pair<SIZE_T, SIZE_T>
					(diff_start, diff_size));
				diff_start = -1;
				diff_size = 0;
			}
		}
		memcpy(oldbuf, curbuf, scanned);
		if ((SIZE_T)scanned < siz)
			memset(oldbuf + scanned, 0, siz - scanned);
	}

	return scanned;
}