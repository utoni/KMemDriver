#include "KMemDriver.h"
#include "Imports.h"
#include "Native.h"

#include <ntddk.h>
#include <Ntstrsafe.h>

#define CHEAT_EXE L"kmem"

#ifdef _DEBUG_
#define KDBG(fmt, ...) DbgPrint("KMemDriver[%01d]: " fmt, KeGetCurrentIrql(), __VA_ARGS__)
#else
#define KDBG(fmt, ...)
#endif

#ifndef _DEBUG_
#define FNZERO_MARKER() \
	do { \
		volatile UINT32 marker = 0xDEADC0DE;\
		UNREFERENCED_PARAMETER(marker); \
	} while (0)
#define FNZERO_FN(fn_start) \
	do { fn_zero_text((PVOID)fn_start); } while (0)
#define FNZERO(fn_start) \
	FNZERO_MARKER(); \
	FNZERO_FN(fn_start)
#else
#define FNZERO_MARKER()
#define FNZERO_FN(fn_start)
#define FNZERO(fn_start)
#endif

#define WAIT_OBJECT_0 ((STATUS_WAIT_0 ) + 0 )

DRIVER_INITIALIZE DriverEntry;
#pragma alloc_text(INIT, DriverEntry)
void OnImageLoad(
	PUNICODE_STRING FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO ImageInfo
);
#pragma alloc_text(PAGE, OnImageLoad)

NTSTATUS WaitForControlProcess(OUT PEPROCESS *ppEProcess);
NTSTATUS VerifyControlProcess(IN PEPROCESS pEProcess);
NTSTATUS InitSharedMemory(IN PEPROCESS pEProcess);
NTSTATUS WaitForHandshake(
	IN PEPROCESS pEProcess,
	OUT HANDLE *pKEvent, OUT HANDLE *pUEvent
);
NTSTATUS OpenEventReference(
	IN PEPROCESS pEProcess,
	IN KAPC_STATE *pKAPCState, IN HANDLE hEvent,
	OUT PKEVENT *pPKEvent
);
NTSTATUS UpdatePPEPIfRequired(
	IN HANDLE wantedPID,
	IN HANDLE lastPID, OUT HANDLE *lastPROC,
	OUT PEPROCESS *lastPEP
);
NTSTATUS GetPages(
	IN PEPROCESS Process,
	OUT MEMORY_BASIC_INFORMATION *mbiArr,
	IN SIZE_T mbiArrLen, OUT SIZE_T *mbiUsed,
	IN PVOID start_addr
);
NTSTATUS GetModules(
	IN PEPROCESS pEProcess,
	OUT PMODULE_DATA pmod, IN OUT SIZE_T *psiz,
	IN SIZE_T start_index,
	IN BOOLEAN isWow64
);
NTSTATUS KeReadVirtualMemory(
	IN PEPROCESS pEProcess,
	IN PVOID SourceAddress,
	IN PVOID TargetAddress, IN PSIZE_T Size
);
NTSTATUS KeWriteVirtualMemory(
	IN PEPROCESS pEProcess,
	IN PVOID SourceAddress,
	IN PVOID TargetAddress,
	IN PSIZE_T Size
);
NTSTATUS KeProtectVirtualMemory(
	IN HANDLE hProcess,
	IN PVOID addr, IN SIZE_T siz,
	IN ULONG new_prot, OUT ULONG *old_prot
);
NTSTATUS KeRestoreProtectVirtualMemory(
	IN HANDLE hProcess,
	IN PVOID addr, IN SIZE_T siz,
	IN ULONG old_prot
);
NTSTATUS GetDriverObject(
	IN OUT PDRIVER_OBJECT *lpObj,
	IN WCHAR* DriverDirName
);
NTSTATUS KRThread(IN PVOID pArg);
TABLE_SEARCH_RESULT VADFindNodeOrParent(
	IN PMM_AVL_TABLE Table,
	IN ULONG_PTR StartingVpn,
	OUT PMMADDRESS_NODE *NodeOrParent
);
NTSTATUS VADFind(
	IN PEPROCESS pProcess,
	IN ULONG_PTR address,
	OUT PMMVAD_SHORT* pResult
);
NTSTATUS VADProtect(
	IN PEPROCESS pProcess,
	IN ULONG_PTR address,
	IN ULONG prot
);
PHANDLE_TABLE_ENTRY ExpLookupHandleTableEntry(
	PVOID pHandleTable,
	HANDLE handle
);

#pragma alloc_text(PAGE, WaitForControlProcess)
#pragma alloc_text(PAGE, VerifyControlProcess)
#pragma alloc_text(PAGE, InitSharedMemory)
#pragma alloc_text(PAGE, WaitForHandshake)
#pragma alloc_text(PAGE, OpenEventReference)
#pragma alloc_text(PAGE, UpdatePPEPIfRequired)
#pragma alloc_text(PAGE, GetPages)
#pragma alloc_text(PAGE, GetModules)
#pragma alloc_text(PAGE, KeReadVirtualMemory)
#pragma alloc_text(PAGE, KeWriteVirtualMemory)
#pragma alloc_text(PAGE, KeProtectVirtualMemory)
#pragma alloc_text(PAGE, KeRestoreProtectVirtualMemory)
#pragma alloc_text(PAGE, GetDriverObject)
#pragma alloc_text(PAGE, KRThread)
#pragma alloc_text(PAGE, VADFindNodeOrParent)
#pragma alloc_text(PAGE, VADFind)
#pragma alloc_text(PAGE, VADProtect)
#pragma alloc_text(PAGE, ExpLookupHandleTableEntry)

static void fn_zero_text(PVOID fn_start);
static HANDLE ctrlPID;
static PVOID imageBase;

static PVOID mmapedBase = NULL;
static INT hijacked = 0;
static PDRIVER_OBJECT hijackedDriver = NULL;
static DRIVER_OBJECT hijackedDriverOriginal;


NTSTATUS DriverEntry(
	_In_  DRIVER_OBJECT *DriverObject,
	_In_  PUNICODE_STRING RegistryPath
)
{
	NTSTATUS status;
	HANDLE hThread = NULL;
	CLIENT_ID clientID = { 0 };
	OBJECT_ATTRIBUTES obAttr = { 0 };

	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	KDBG("Driver Loaded\n");
	if (!DriverObject && RegistryPath) {
		/* assume that we are manual mapped by PastDSE */
		mmapedBase = RegistryPath;
		KDBG("Manual mapped image base: 0x%p\n", mmapedBase);
	}
	InitializeObjectAttributes(&obAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
	status = PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, &obAttr, NULL, &clientID, &KRThread, NULL);
	if (!NT_SUCCESS(status))
	{
		KDBG("Failed to create worker thread. Status: 0x%X\n", status);
		return status;
	}

	FNZERO(DriverEntry);
	return status;
}

void OnImageLoad(
	PUNICODE_STRING FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO ImageInfo
)
{
	UNREFERENCED_PARAMETER(ImageInfo);
#if 0
	KDBG("ProcessID: 0x%X\n", ProcessId);
	KDBG("FullImage: %wZ\n", FullImageName);
#endif
	if (wcsstr(FullImageName->Buffer, CHEAT_EXE)) {
		ctrlPID = ProcessId;
		imageBase = ImageInfo->ImageBase;
		KDBG("Found Target !!!\n");
	}
}

NTSTATUS WaitForControlProcess(OUT PEPROCESS *ppEProcess)
{
	NTSTATUS status;

	if (!ppEProcess)
		return STATUS_INVALID_ADDRESS;

	imageBase = NULL;
	ctrlPID = NULL;

	status = PsSetLoadImageNotifyRoutine(OnImageLoad);
	if (!NT_SUCCESS(status)) {
		KDBG("PsSetLoadImageNotifyRoutine failed with 0x%X\n", status);
		return status;
	}

	while (!ctrlPID) {
		LARGE_INTEGER wait = { .QuadPart = -1000000 };
		KeDelayExecutionThread(KernelMode, TRUE, &wait);
	}

	status = PsRemoveLoadImageNotifyRoutine(OnImageLoad);
	if (!NT_SUCCESS(status)) {
		KDBG("PsRemoveLoadImageNotifyRoutine failed with 0x%X\n", status);
		return status;
	}

	status = PsLookupProcessByProcessId(ctrlPID, ppEProcess);
	if (!NT_SUCCESS(status)) {
		KDBG("PsLookupProcessByProcessId failed with 0x%X\n", status);
		return status;
	}

	KDBG("Got Ctrl Process PID: 0x%X (%d)\n",
		ctrlPID, ctrlPID);

	return STATUS_SUCCESS;
}

NTSTATUS VerifyControlProcess(IN PEPROCESS pEProcess)
{
	NTSTATUS status;
	UCHAR pefile[4] = { 0 };
	SIZE_T pesiz = sizeof pefile;

	status = KeReadVirtualMemory(pEProcess,
		imageBase, pefile, &pesiz);
	if (!NT_SUCCESS(status) || pesiz != sizeof pefile) {
		KDBG("KeReadVirtualMemory failed with 0x%X\n", status);
		return status;
	}
	if (pefile[0] != 0x4D || pefile[1] != 0x5A ||
		pefile[2] != 0x90 || pefile[3] != 0x00)
	{
		KDBG("Invalid PE file\n");
		return status;
	}

	return status;
}

NTSTATUS InitSharedMemory(IN PEPROCESS pEProcess)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	SIZE_T maxWaits = 20;

	KDBG("Init Shmem ..\n");
	while (--maxWaits) {
		UCHAR buf[4] = { 0 };
		SIZE_T bufsiz = sizeof buf;
		LARGE_INTEGER wait = { .QuadPart = -5000000 };
		status = KeReadVirtualMemory(pEProcess,
			(PVOID)SHMEM_ADDR, buf, &bufsiz);
		if (NT_SUCCESS(status) && bufsiz == sizeof buf)
			break;
		KDBG("Waiting until 0x%X alloc'd by user space app ..\n", SHMEM_ADDR);
		KeDelayExecutionThread(KernelMode, TRUE, &wait);
	}

	return status;
}

NTSTATUS WaitForHandshake(
	IN PEPROCESS pEProcess,
	OUT HANDLE *pKEvent, OUT HANDLE *pUEvent
)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	SIZE_T maxWaits = 20;

	KDBG("Wait for Handshake ..\n");
	while (--maxWaits) {
		KERNEL_HANDSHAKE hnds;
		SIZE_T siz = sizeof hnds;
		LARGE_INTEGER wait = { .QuadPart = -5000000 };
		status = KeReadVirtualMemory(pEProcess, (PVOID)SHMEM_ADDR, &hnds, &siz);
		if (NT_SUCCESS(status) && siz == sizeof hnds &&
			hnds.kevent)
		{
			if (validateRequest(&hnds) != MEM_HANDSHAKE) {
				KDBG("Invalid Handshake received: 0x%X\n", hnds.hdr.magic);
				return STATUS_INVALID_CONNECTION;
			}
			*pKEvent = hnds.kevent;
			*pUEvent = hnds.uevent;
			KDBG("Got Event Handle 0x%X (Kernel) and 0x%X (User)\n",
				*pKEvent, *pUEvent);
			break;
		}
		KDBG("Waiting for handshake at 0x%X ..\n", SHMEM_ADDR);
		KeDelayExecutionThread(KernelMode, TRUE, &wait);
	}

	return status;
}

NTSTATUS OpenEventReference(
	IN PEPROCESS pEProcess,
	IN KAPC_STATE *pKAPCState, IN HANDLE hEvent,
	OUT PKEVENT *pPKEvent
)
{
	NTSTATUS status;

	KeStackAttachProcess((PRKPROCESS)pEProcess, pKAPCState);
	status = ObReferenceObjectByHandle(
		hEvent, SYNCHRONIZE | EVENT_MODIFY_STATE,
		*ExEventObjectType, UserMode, pPKEvent, NULL
	);
	KeUnstackDetachProcess(pKAPCState);
	if (!NT_SUCCESS(status))
		KDBG("ObReferenceObjectByHandle for Handle 0x%X failed with 0x%X\n",
			hEvent, status);

	return status;
}

NTSTATUS KRThread(IN PVOID pArg)
{
	NTSTATUS status;
	INT reinit;
	HANDLE kevent, uevent;
	PEPROCESS ctrlPEP;
	KAPC_STATE apcstate;
	PKEVENT pk_kevent, pk_uevent;

	UNREFERENCED_PARAMETER(pArg);

	do {
		reinit = 0;
		ctrlPEP = NULL;
		pk_kevent = pk_uevent = NULL;

		KeLowerIrql(0);
		KDBG("Init ..\n");
		{
			ULONG_PTR low, high;
			IoGetStackLimits(&low, &high);
			KDBG("Stack limits (high/low/total/remaining): 0x%p/0x%p/0x%X/0x%X\n",
				low, high, high - low, IoGetRemainingStackSize());
		}

		if (mmapedBase && !hijackedDriver &&
			NT_SUCCESS(GetDriverObject(&hijackedDriver, L"\\Driver\\ahcache")))
		{
			if (hijackedDriver) {
#ifdef _DEBUG_
				KDBG("Got DriverObject at 0x%p\n", hijackedDriver);
				PKLDR_DATA_TABLE_ENTRY drv_section = hijackedDriver->DriverSection;
				KDBG("PDrvObj: base -> 0x%p , name -> '%wZ' , flags -> 0x%X\n",
					drv_section->DllBase, drv_section->BaseDllName, drv_section->Flags);
#endif
				/* !!! EXPERIMENTAL !!! */
#if 0
				hijacked = 1;
				/* the following lines are known to cause a bugcheck */
				hijackedDriverOriginal = *hijackedDriver;
				hijackedDriver->DriverStart = mmapedBase;
				//hijackedDriver->DriverSection = (PVOID)((ULONG_PTR)mmapedBase + 100);
#endif
#if 0
				/* the following lines are known to not work with ahcache driver */
				hijackedDriver->DriverInit = (PDRIVER_INITIALIZE)DriverEntry;
				hijackedDriver->DriverStartIo = NULL;
				hijackedDriver->DriverUnload = NULL;
				SIZE_T funcs = sizeof hijackedDriver->MajorFunction / sizeof hijackedDriver->MajorFunction[0];
				for (SIZE_T i = 0; i < funcs; ++i) {
					hijackedDriver->MajorFunction[i] = NULL;
				}
#endif
			}
		}

		status = WaitForControlProcess(&ctrlPEP);
		if (!NT_SUCCESS(status))
			goto finish;

		if (hijackedDriver && hijacked) {
			*hijackedDriver = hijackedDriverOriginal;
		}

		status = VerifyControlProcess(ctrlPEP);
		if (!NT_SUCCESS(status))
			goto finish_ctrlpep;

		status = InitSharedMemory(ctrlPEP);
		if (!NT_SUCCESS(status))
			goto finish_ctrlpep;

		status = WaitForHandshake(ctrlPEP,
			&kevent, &uevent);
		if (!NT_SUCCESS(status))
			goto finish_ctrlpep;

		status = OpenEventReference(ctrlPEP,
			&apcstate, kevent, &pk_kevent);
		if (!NT_SUCCESS(status))
			goto finish_ctrlpep;

		status = OpenEventReference(ctrlPEP,
			&apcstate, uevent, &pk_uevent);
		if (!NT_SUCCESS(status))
			goto finish_kevent;

		KeClearEvent(pk_kevent);
		KeClearEvent(pk_uevent);

		PVOID shm_buf = MmAllocateNonCachedMemory(SHMEM_SIZE);
		if (!shm_buf) {
			KDBG("MmAllocateNonCachedMemory with size 0x%X failed\n", SHMEM_SIZE);
			goto nomem;
		}

		reinit = 1;
		KeSetEvent(pk_uevent, FILE_DEVICE_MOUSE, TRUE);
		HANDLE lastPID = NULL, lastPROC = NULL;
		PEPROCESS lastPEP = NULL;
		INT running = 1;
		SIZE_T maxWaits = 20;
		do {
			LARGE_INTEGER wait = { .QuadPart = -10000000 };
			status = KeWaitForSingleObject(pk_kevent, Executive, UserMode, FALSE, &wait);
			if (NT_SUCCESS(status) && status == WAIT_OBJECT_0) {
				maxWaits = 20;
				/* parse memory request */
				SIZE_T siz = SHMEM_SIZE;
				status = KeReadVirtualMemory(ctrlPEP, (PVOID)SHMEM_ADDR, shm_buf, &siz);

				if (NT_SUCCESS(status) && siz == SHMEM_SIZE) {
					switch (validateRequest(shm_buf))
					{
					case MEM_HANDSHAKE:
						KDBG("Invalid Request MEM_HANDSHAKE\n");
						break;
					case MEM_PING: {
						PKERNEL_PING ping = (PKERNEL_PING)shm_buf;
						KDBG("Got a PING with rng 0x%X, sending PONG !\n",
							ping->rnd_user);
						ping->rnd_kern = ping->rnd_user;
						siz = sizeof *ping;
						KeWriteVirtualMemory(ctrlPEP, ping, (PVOID)SHMEM_ADDR, &siz);
						break;
					}
					case MEM_PAGES: {
						PKERNEL_PAGE pages = (PKERNEL_PAGE)shm_buf;
						KDBG("Got a PAGES request for process 0x%X start at address 0x%p\n",
							pages->ProcessId, pages->StartAddress);
						if (!NT_SUCCESS(UpdatePPEPIfRequired(pages->ProcessId,
							lastPID, &lastPROC, &lastPEP)))
						{
							running = 0;
							break;
						}
						siz = (SHMEM_SIZE - sizeof *pages + sizeof pages->pages_start)
							/ sizeof pages->pages_start;
						pages->StatusRes = GetPages(lastPEP, &pages->pages_start, siz,
							&pages->pages, pages->StartAddress);
						siz = (sizeof *pages - sizeof pages->pages_start) +
							sizeof pages->pages_start * pages->pages;
						KeWriteVirtualMemory(ctrlPEP, pages, (PVOID)SHMEM_ADDR, &siz);
						break;
					}
					case MEM_MODULES: {
						PKERNEL_MODULES mods = (PKERNEL_MODULES)shm_buf;
						KDBG("Got a MODULES request for process 0x%X start at index 0x%X\n",
							mods->ProcessId, mods->StartIndex);
						if (!NT_SUCCESS(UpdatePPEPIfRequired(mods->ProcessId,
							lastPID, &lastPROC, &lastPEP)))
						{
							running = 0;
							break;
						}
						siz = (SHMEM_SIZE - sizeof *mods + sizeof mods->modules_start)
							/ sizeof mods->modules_start;
						PMODULE_DATA entries = &mods->modules_start;
						KDBG("GetModules max entries: %u\n", siz);
						KeStackAttachProcess((PRKPROCESS)lastPEP, &apcstate);
						mods->StatusRes = GetModules(lastPEP, entries, &siz, mods->StartIndex,
							PsGetProcessWow64Process(lastPEP) != NULL);
						KeUnstackDetachProcess(&apcstate);
						mods->modules = siz;
						siz = (sizeof *mods - sizeof mods->modules_start) +
							sizeof mods->modules_start * mods->modules;
						KeWriteVirtualMemory(ctrlPEP, mods, (PVOID)SHMEM_ADDR, &siz);
						break;
					}
					case MEM_RPM: {
						PKERNEL_READ_REQUEST rr = (PKERNEL_READ_REQUEST)shm_buf;
						KDBG("Got a RPM to process 0x%X, address 0x%p with size 0x%lX\n",
							rr->ProcessId, rr->Address, rr->SizeReq);
						if (!NT_SUCCESS(UpdatePPEPIfRequired(rr->ProcessId,
							lastPID, &lastPROC, &lastPEP)))
						{
							running = 0;
							break;
						}
						if (rr->SizeReq > SHMEM_SIZE - sizeof *rr) {
							siz = SHMEM_SIZE - sizeof *rr;
						}
						else {
							siz = rr->SizeReq;
						}
						ULONG new_prot = PAGE_EXECUTE_READWRITE, old_prot = 0;
						KeProtectVirtualMemory(lastPROC, rr->Address, rr->SizeReq, new_prot, &old_prot);
						KDBG("RPM to 0x%p size 0x%X bytes (protection before/after: 0x%X/0x%X)\n",
							rr->Address, rr->SizeReq, old_prot, new_prot);
						rr->StatusRes = KeReadVirtualMemory(lastPEP, (PVOID)rr->Address,
							(PVOID)((ULONG_PTR)shm_buf + sizeof *rr), &siz);
						KeRestoreProtectVirtualMemory(lastPROC, rr->Address, rr->SizeReq, old_prot);
						if (NT_SUCCESS(rr->StatusRes)) {
							rr->SizeRes = siz;
							siz += sizeof *rr;
						}
						else {
							rr->SizeRes = 0;
							siz = sizeof *rr;
						}
						KeWriteVirtualMemory(ctrlPEP, rr, (PVOID)SHMEM_ADDR, &siz);
						break;
					}
					case MEM_WPM: {
						PKERNEL_WRITE_REQUEST wr = (PKERNEL_WRITE_REQUEST)shm_buf;
						KDBG("Got a WPM to process 0x%X, address 0x%p with size 0x%lX\n",
							wr->ProcessId, wr->Address, wr->SizeReq);
						if (!NT_SUCCESS(UpdatePPEPIfRequired(wr->ProcessId,
							lastPID, &lastPROC, &lastPEP)))
						{
							running = 0;
							break;
						}
						if (wr->SizeReq > SHMEM_SIZE - sizeof *wr) {
							siz = SHMEM_SIZE - sizeof *wr;
						}
						else {
							siz = wr->SizeReq;
						}
						ULONG new_prot = PAGE_EXECUTE_READWRITE, old_prot = 0;
						KeProtectVirtualMemory(lastPEP, wr->Address, wr->SizeReq, new_prot, &old_prot);
						KDBG("WPM to 0x%p size 0x%X bytes (protection before/after: 0x%X/0x%X)\n",
							wr->Address, wr->SizeReq, old_prot, new_prot);
						wr->StatusRes = KeWriteVirtualMemory(lastPEP, (PVOID)((ULONG_PTR)shm_buf + sizeof *wr),
							(PVOID)wr->Address, &siz);
						KeRestoreProtectVirtualMemory(lastPROC, wr->Address, wr->SizeReq, old_prot);
						if (NT_SUCCESS(wr->StatusRes)) {
							wr->SizeRes = siz;
							siz += sizeof *wr;
						}
						else {
							wr->SizeRes = 0;
							siz = sizeof *wr;
						}
						KeWriteVirtualMemory(ctrlPEP, wr, (PVOID)SHMEM_ADDR, &siz);
						break;
					}
					case MEM_EXIT:
						KDBG("Gracefully exiting ..\n");
						KeClearEvent(pk_kevent);
						KeClearEvent(pk_uevent);
						running = 0;
						reinit = 0;
						break;
					default:
						KDBG("Invalid Request\n");
						running = 0;
						reinit = 0;
						break;
					}
				}

				if (KeSetEvent(pk_uevent, FILE_DEVICE_MOUSE, TRUE)) {
					KDBG("Previous signal state wasn't consumed!?\n");
				}
			}
			else {
				if (!maxWaits--) {
					KDBG("No activity, abort ..\n");
					running = 0;
				}
			}
		} while (running);
		KeSetEvent(pk_uevent, FILE_DEVICE_MOUSE, TRUE);

		if (lastPEP)
			ObDereferenceObject(lastPEP);
		if (lastPROC)
			ZwClose(lastPROC);
		MmFreeNonCachedMemory(shm_buf, SHMEM_SIZE);
	nomem:
		ObDereferenceObject(pk_uevent);
	finish_kevent:
		ObDereferenceObject(pk_kevent);
	finish_ctrlpep:
		ObDereferenceObject(ctrlPEP);
	finish:
		if (reinit) {
			LARGE_INTEGER wait = { .QuadPart = -50000000 };
			KeDelayExecutionThread(KernelMode, TRUE, &wait);
		}
	} while (reinit);

	KDBG("Terminating ..\n");
	PsTerminateSystemThread(status);
	return status;
}

NTSTATUS UpdatePPEPIfRequired(
	IN HANDLE wantedPID,
	IN HANDLE lastPID, OUT HANDLE *lastPROC,
	OUT PEPROCESS *lastPEP
)
{
	NTSTATUS status = STATUS_SUCCESS;

	if (wantedPID != lastPID) {
		if (lastPID) {
			ObDereferenceObject(*lastPEP);
			*lastPEP = NULL;
			ZwClose(*lastPROC);
			*lastPROC = NULL;
		}
		status = PsLookupProcessByProcessId(wantedPID, lastPEP);
		if (!NT_SUCCESS(status)) {
			KDBG("PsLookupProcessByProcessId failed with 0x%X\n", status);
		}
		else {
			status = ObOpenObjectByPointer(*lastPEP,
				OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, GENERIC_ALL,
				*PsProcessType, KernelMode, lastPROC
			);
			if (!NT_SUCCESS(status)) {
				KDBG("ObOpenObjectByPointer failed with 0x%X\n", status);
			}
			else {
				PEPROCESS pep = *lastPEP;
				PVOID base = NULL;
				SIZE_T size = ADDRESS_AND_SIZE_TO_SPAN_PAGES(base, 4096);
				PKAPC_STATE apc = MmAllocateNonCachedMemory(sizeof(*apc));
				KeStackAttachProcess((PRKPROCESS)pep, apc);
				status = ZwAllocateVirtualMemory(ZwCurrentProcess(), &base, 0, &size, MEM_COMMIT, PAGE_READWRITE);
				if (!NT_SUCCESS(status)) {
					KDBG("ZwAllocateVirtualMemory failed with 0x%X\n", status);
				}
				else {
					*(UINT64 *)base = 0x4141414142424242;
				}
				KeUnstackDetachProcess(apc);
				KDBG("VAD Test Alloc.: 0x%p (status: 0x%X)\n", base, status);
				PMMVAD_SHORT mmvad;
				status = VADFind(pep, (ULONG_PTR)base, &mmvad);
				KDBG("VAD Test.......: 0x%p (status: 0x%X)\n", mmvad->StartingVpn, status);
				KeStackAttachProcess((PRKPROCESS)pep, apc);
				if (*(UINT64 *)base != 0x4141414142424242) {
					KDBG("VAD Test failed: 0x%p != 0x%p\n", 0x4141414142424242, base);
				}
				ZwFreeVirtualMemory(ZwCurrentProcess(), &base, &size, MEM_RELEASE);
				KeUnstackDetachProcess(apc);
				MmFreeNonCachedMemory(apc, sizeof(*apc));
#if 0
				PMM_AVL_TABLE avltable = (PMM_AVL_TABLE)((ULONG_PTR *)pep + VAD_TREE_1803);
				KDBG("VAD-ROOT.....: 0x%p\n", GET_VAD_ROOT(avltable));
				KDBG("NODE-HINT....: 0x%p\n", avltable->NodeHint);
				KDBG("NMBR-OF-ELEMs: %d\n", avltable->NumberGenericTableElements);
				KDBG("FLAGS........: 0x%p\n", *((UINT32 *)pep + 0x304));
				KDBG("VSIZE........: %d\n", *((UINT64 *)pep + 0x338));
				KDBG("IMAGEFILENAME: %.*s\n", 15, ((const char *)pep + 0x450));
#endif
#if 0
				PVOID handleTable = (PVOID)((ULONG_PTR)pep + 0x418);
				KDBG("lastPROC HandleTableEntry: %p\n", ExpLookupHandleTableEntry(handleTable, *lastPROC));
#endif
			}
		}
	}
	return status;
}

NTSTATUS GetPages(
	IN PEPROCESS Process,
	OUT MEMORY_BASIC_INFORMATION *mbiArr,
	IN SIZE_T mbiArrLen, OUT SIZE_T *mbiUsed,
	IN PVOID start_addr
)
{
	NTSTATUS status;
	HANDLE procHandle;
	SIZE_T i, mbiLength, mbiReturn;
	ULONG_PTR baseAddr = (ULONG_PTR)start_addr;

	status = ObOpenObjectByPointer(Process,
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, GENERIC_ALL,
		*PsProcessType, KernelMode, &procHandle
	);
	if (!NT_SUCCESS(status)) {
		KDBG("ObOpenObjectByPointer failed with 0x%X\n", status);
		return status;
	}

	KDBG("ZwQueryVirtualMemory max entries: %u\n", mbiArrLen);
	*mbiUsed = 0;
	do {
		mbiReturn = 0;
		status = ZwQueryVirtualMemory(procHandle, (PVOID)baseAddr,
			MemoryBasicInformation, mbiArr, sizeof *mbiArr * mbiArrLen, &mbiReturn);
		mbiLength = mbiReturn / sizeof *mbiArr;
		if (!NT_SUCCESS(status)) {
			if (status == STATUS_INVALID_PARAMETER)
				status = STATUS_SUCCESS;
			else
				KDBG("ZwQueryVirtualMemory failed with 0x%X\n", status);
			break;
		}
		else {
			for (i = 0; i < mbiLength; ++i)
				KDBG("Page #%03u: base -> 0x%p, prot -> 0x%02X, size -> 0x%X\n",
				(*mbiUsed) + i, (*(mbiArr + i)).BaseAddress, (*(mbiArr + i)).Protect,
					(*(mbiArr + i)).RegionSize);
		}
		baseAddr += (SIZE_T)(mbiArr + mbiLength - 1)->RegionSize;
		*mbiUsed += mbiLength;
		mbiArr += mbiLength;
	} while (*mbiUsed < mbiArrLen && mbiReturn > 0);

	ZwClose(procHandle);
	return status;
}

NTSTATUS GetModules(
	IN PEPROCESS Process,
	OUT PMODULE_DATA pmod, IN OUT SIZE_T *psiz,
	IN SIZE_T start_index,
	IN BOOLEAN isWow64
)
{
	SIZE_T used = 0, index = 0;
	INT waitCount = 0;

	if (isWow64) {
		PPEB32 peb32 = (PPEB32)PsGetProcessWow64Process(Process);
		if (!peb32) {
			KDBG("PsGetProcessWow64Process failed");
			return STATUS_UNSUCCESSFUL;
		}

		PPEB_LDR_DATA32 ldr32 = (PPEB_LDR_DATA32)peb32->Ldr;

		if (!ldr32) {
			KDBG("peb32->Ldr is invalid");
			return STATUS_UNSUCCESSFUL;
		}

		if (!ldr32->Initialized) {
			while (!ldr32->Initialized && waitCount++ < 4) {
				LARGE_INTEGER wait = { .QuadPart = -2500 };
				KeDelayExecutionThread(KernelMode, TRUE, &wait);
			}

			if (!ldr32->Initialized) {
				KDBG("ldr32->Initialized is 0");
				return STATUS_UNSUCCESSFUL;
			}
		}

		for (PLIST_ENTRY32 listEntry = (PLIST_ENTRY32)ldr32->InLoadOrderModuleList.Flink;
			listEntry != &ldr32->InLoadOrderModuleList && used < *psiz;
			listEntry = (PLIST_ENTRY32)listEntry->Flink, ++pmod, ++index) {
			if (index < start_index)
				continue;
			used++;

			PLDR_DATA_TABLE_ENTRY32 ldrEntry32 = CONTAINING_RECORD(listEntry, LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);
			ANSI_STRING name;
			UNICODE_STRING tmpUnicodeStr;
			tmpUnicodeStr.Buffer = (PWCH)ldrEntry32->BaseDllName.Buffer;
			tmpUnicodeStr.Length = ldrEntry32->BaseDllName.Length;
			tmpUnicodeStr.MaximumLength = ldrEntry32->BaseDllName.MaximumLength;
			if (NT_SUCCESS(RtlUnicodeStringToAnsiString(&name, &tmpUnicodeStr, TRUE))) {
				RtlCopyMemory(pmod->BaseDllName, name.Buffer,
					(name.Length > sizeof pmod->BaseDllName ?
						sizeof pmod->BaseDllName : name.Length)
				);
				RtlFreeAnsiString(&name);
			}
			pmod->DllBase = (PVOID)ldrEntry32->DllBase;
			pmod->SizeOfImage = ldrEntry32->SizeOfImage;
			KDBG("DLL32 #%02lu: base -> 0x%p, size -> 0x%06X, name -> '%s'\n", used,
				pmod->DllBase, pmod->SizeOfImage, pmod->BaseDllName);
		}
	}
	else {
		PPEB peb = PsGetProcessPeb(Process);
		if (!peb) {
			KDBG("PsGetProcessPeb failed");
			return STATUS_UNSUCCESSFUL;
		}

		PPEB_LDR_DATA ldr = peb->Ldr;

		if (!ldr) {
			KDBG("peb->Ldr is invalid");
			return STATUS_UNSUCCESSFUL;
		}

		if (!ldr->Initialized) {
			while (!ldr->Initialized && waitCount++ < 4) {
				LARGE_INTEGER wait = { .QuadPart = -2500 };
				KeDelayExecutionThread(KernelMode, TRUE, &wait);
			}

			if (!ldr->Initialized) {
				KDBG("ldr->Initialized is 0");
				return STATUS_UNSUCCESSFUL;
			}
		}

		for (PLIST_ENTRY listEntry = (PLIST_ENTRY)ldr->InLoadOrderModuleList.Flink;
			listEntry != &ldr->InLoadOrderModuleList && used < *psiz;
			listEntry = (PLIST_ENTRY)listEntry->Flink, ++pmod, ++index) {
			if (index < start_index)
				continue;
			used++;

			PLDR_DATA_TABLE_ENTRY ldrEntry = CONTAINING_RECORD(listEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
			ANSI_STRING name;
			if (NT_SUCCESS(RtlUnicodeStringToAnsiString(&name, &ldrEntry->BaseDllName, TRUE))) {
				RtlCopyMemory(pmod->BaseDllName, name.Buffer,
					(name.Length > sizeof pmod->BaseDllName ?
						sizeof pmod->BaseDllName : name.Length)
				);
				RtlFreeAnsiString(&name);
			}
			pmod->DllBase = ldrEntry->DllBase;
			pmod->SizeOfImage = ldrEntry->SizeOfImage;
			KDBG("DLL #%02lu: base -> 0x%p, size -> 0x%06X, name -> '%s'\n", used,
				pmod->DllBase, pmod->SizeOfImage, pmod->BaseDllName);
		}
	}

	*psiz = used;
	return STATUS_SUCCESS;
}

static void fn_zero_text(PVOID fn_start)
{
	SIZE_T i;
	UINT32 marker = 0xDEADC0DE;
	PUCHAR fnbuf = (PUCHAR)fn_start;

	KDBG("Fn: %p\n", fn_start);
	for (i = 0; i < 0x1000; ++i && fnbuf++) {
		if (*(UINT32 *)fnbuf == marker) {
			KDBG("Marker: 0x%X\n", i);
			RtlSecureZeroMemory(fn_start, i + 4);
		}
	}
}

NTSTATUS GetDriverObject(
	IN OUT PDRIVER_OBJECT *lpObj,
	IN WCHAR* DriverDirName
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDRIVER_OBJECT pBeepObj = NULL;
	UNICODE_STRING DevName = { 0 };

	if (!MmIsAddressValid(lpObj))
		return STATUS_INVALID_ADDRESS;

	RtlInitUnicodeString(&DevName, DriverDirName);

	status = ObReferenceObjectByName(&DevName, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, &pBeepObj);

	if (NT_SUCCESS(status))
		*lpObj = pBeepObj;
	else
	{
		*lpObj = NULL;
	}

	return status;
}

TABLE_SEARCH_RESULT
VADFindNodeOrParent(
	IN PMM_AVL_TABLE Table,
	IN ULONG_PTR StartingVpn,
	OUT PMMADDRESS_NODE *NodeOrParent
)
{
	PMMADDRESS_NODE Child;
	PMMADDRESS_NODE NodeToExamine;
	PMMVAD_SHORT    VpnCompare;
	ULONG_PTR       startVpn;
	ULONG_PTR       endVpn;

	if (Table->NumberGenericTableElements == 0) {
		return TableEmptyTree;
	}

	NodeToExamine = (PMMADDRESS_NODE)GET_VAD_ROOT(Table);

	for (;;) {

		VpnCompare = (PMMVAD_SHORT)NodeToExamine;
		startVpn = VpnCompare->StartingVpn;
		endVpn = VpnCompare->EndingVpn;

		startVpn |= (ULONG_PTR)VpnCompare->StartingVpnHigh << 32;
		endVpn |= (ULONG_PTR)VpnCompare->EndingVpnHigh << 32;

		KDBG("Examining Node 0x%p with start VA 0x%p and end VA 0x%p\n", VpnCompare, startVpn, endVpn);

		//
		// Compare the buffer with the key in the tree element.
		//

		if (StartingVpn < startVpn) {

			Child = NodeToExamine->LeftChild;

			if (Child != NULL) {
				NodeToExamine = Child;
			}
			else {

				//
				// Node is not in the tree.  Set the output
				// parameter to point to what would be its
				// parent and return which child it would be.
				//

				*NodeOrParent = NodeToExamine;
				return TableInsertAsLeft;
			}
		}
		else if (StartingVpn <= endVpn) {

			//
			// This is the node.
			//

			*NodeOrParent = NodeToExamine;
			return TableFoundNode;
		}
		else {

			Child = NodeToExamine->RightChild;

			if (Child != NULL) {
				NodeToExamine = Child;
			}
			else {

				//
				// Node is not in the tree.  Set the output
				// parameter to point to what would be its
				// parent and return which child it would be.
				//

				*NodeOrParent = NodeToExamine;
				return TableInsertAsRight;
			}
		}
	}
}

NTSTATUS VADFind(
	IN PEPROCESS pProcess,
	IN ULONG_PTR address,
	OUT PMMVAD_SHORT* pResult
)
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG_PTR vpnStart = address >> PAGE_SHIFT;
	PMM_AVL_TABLE pTable = (PMM_AVL_TABLE)((PUCHAR)pProcess + VAD_TREE_1803);
	PMM_AVL_NODE pNode = GET_VAD_ROOT(pTable);

	if (pProcess == NULL || pResult == NULL)
		return STATUS_INVALID_PARAMETER;

	// Search VAD
	if (VADFindNodeOrParent(pTable, vpnStart, &pNode) == TableFoundNode)
	{
		*pResult = (PMMVAD_SHORT)pNode;
	}
	else
	{
		KDBG("%s: VAD entry for address 0x%p not found\n", __FUNCTION__, address);
		status = STATUS_NOT_FOUND;
	}

	return status;
}

NTSTATUS VADProtect(
	IN PEPROCESS pProcess,
	IN ULONG_PTR address, IN ULONG prot
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PMMVAD_SHORT pVadShort = NULL;

	status = VADFind(pProcess, address, &pVadShort);
	if (NT_SUCCESS(status))
		pVadShort->u.VadFlags.Protection = prot;

	return status;
}

PHANDLE_TABLE_ENTRY ExpLookupHandleTableEntry(PVOID pHandleTable, HANDLE handle)
{
	unsigned __int64 v2; // rdx
	__int64 v3; // r8
	signed __int64 v4; // rax
	__int64 v5; // rax

	v2 = (__int64)handle & 0xFFFFFFFFFFFFFFFCui64;
	if (v2 >= *(DWORD*)pHandleTable)
		return 0i64;
	v3 = *((uintptr_t*)pHandleTable + 1);
	v4 = *((uintptr_t *)pHandleTable + 1) & 3i64;
	if ((UINT32)v4 == 1)
	{
		v5 = *(uintptr_t*)(v3 + 8 * (v2 >> 10) - 1);
		return (PHANDLE_TABLE_ENTRY)(v5 + 4 * (v2 & 0x3FF));
	}
	if ((UINT32)v4)
	{
		v5 = *(uintptr_t*)(*(uintptr_t *)(v3 + 8 * (v2 >> 19) - 2) + 8 * ((v2 >> 10) & 0x1FF));
		return (PHANDLE_TABLE_ENTRY)(v5 + 4 * (v2 & 0x3FF));
	}
	return (PHANDLE_TABLE_ENTRY)(v3 + 4 * v2);
}