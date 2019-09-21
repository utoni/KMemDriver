#include "KMemDriver.h"
#include "Imports.h"
#include "Native.h"

#include <ntddk.h>
#include <Ntstrsafe.h>

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
			for (i = 0; i < mbiLength; ++i) {
				//KDBG("Page #%03u: base -> 0x%p, prot -> 0x%02X, size -> 0x%X\n",
				//     (*mbiUsed) + i, (*(mbiArr + i)).BaseAddress, (*(mbiArr + i)).Protect,
				//     (*(mbiArr + i)).RegionSize);
			}
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
			tmpUnicodeStr.Buffer = (PWCH)ldrEntry32->FullDllName.Buffer;
			tmpUnicodeStr.Length = ldrEntry32->FullDllName.Length;
			tmpUnicodeStr.MaximumLength = ldrEntry32->FullDllName.MaximumLength;
			if (NT_SUCCESS(RtlUnicodeStringToAnsiString(&name, &tmpUnicodeStr, TRUE))) {
				RtlCopyMemory(pmod->FullDllPath, name.Buffer,
					(name.Length > sizeof pmod->FullDllPath ?
						sizeof pmod->FullDllPath : name.Length)
				);
				RtlFreeAnsiString(&name);
			}
			pmod->DllBase = (PVOID)ldrEntry32->DllBase;
			pmod->SizeOfImage = ldrEntry32->SizeOfImage;
			//KDBG("DLL32 #%02lu: base -> 0x%p, size -> 0x%06X, name -> '%s'\n", used,
			//     pmod->DllBase, pmod->SizeOfImage, pmod->BaseDllName);
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
			if (NT_SUCCESS(RtlUnicodeStringToAnsiString(&name, &ldrEntry->FullDllName, TRUE))) {
				RtlCopyMemory(pmod->FullDllPath, name.Buffer,
					(name.Length > sizeof pmod->FullDllPath ?
						sizeof pmod->FullDllPath : name.Length)
				);
				RtlFreeAnsiString(&name);
			}
			pmod->DllBase = ldrEntry->DllBase;
			pmod->SizeOfImage = ldrEntry->SizeOfImage;
			//KDBG("DLL #%02lu: base -> 0x%p, size -> 0x%06X, name -> '%s'\n", used,
			//     pmod->DllBase, pmod->SizeOfImage, pmod->BaseDllName);
		}
	}

	*psiz = used;
	return STATUS_SUCCESS;
}

NTSTATUS KeReadVirtualMemory(
	IN PEPROCESS Process, IN PVOID SourceAddress,
	IN PVOID TargetAddress, IN PSIZE_T Size
)
{
	NTSTATUS status;
	SIZE_T Bytes = 0;

	try {
		status = MmCopyVirtualMemory(Process, SourceAddress, PsGetCurrentProcess(),
			TargetAddress, *Size, KernelMode, &Bytes);
	} except(STATUS_ACCESS_VIOLATION) {
		return STATUS_ACCESS_VIOLATION;
	}
	if (NT_SUCCESS(status))
	{
		*Size = Bytes;
		return STATUS_SUCCESS;
	}
	else {
		return status;
	}
}

NTSTATUS KeWriteVirtualMemory(
	IN PEPROCESS Process, IN PVOID SourceAddress,
	IN PVOID TargetAddress, IN PSIZE_T Size
)
{
	NTSTATUS status;
	SIZE_T Bytes = 0;

	try {
		status = MmCopyVirtualMemory(PsGetCurrentProcess(), SourceAddress, Process,
			TargetAddress, *Size, KernelMode, &Bytes);
	} except(STATUS_ACCESS_VIOLATION) {
		return STATUS_ACCESS_VIOLATION;
	}
	if (NT_SUCCESS(status))
	{
		*Size = Bytes;
		return STATUS_SUCCESS;
	}
	else {
		return status;
	}
}

NTSTATUS KeProtectVirtualMemory(
	IN HANDLE hProcess, IN PVOID addr,
	IN SIZE_T siz, IN ULONG new_prot,
	OUT ULONG *old_prot
)
{
	NTSTATUS status;
	PVOID prot_addr = addr;
	SIZE_T prot_size = siz;
	ULONG prot = 0;

	try {
		status = ZwProtectVirtualMemory(hProcess, &prot_addr,
			&prot_size, new_prot, &prot);
		if (NT_SUCCESS(status)) {
			*old_prot = prot;
		}
	} except(STATUS_ACCESS_VIOLATION) {
		return STATUS_ACCESS_VIOLATION;
	}
	return status;
}

NTSTATUS KeRestoreProtectVirtualMemory(IN HANDLE hProcess,
	IN PVOID addr, IN SIZE_T siz,
	IN ULONG old_prot)
{
	NTSTATUS status;
	PVOID prot_addr = addr;
	SIZE_T prot_size = siz;
	ULONG prot = 0;

	try {
		status = ZwProtectVirtualMemory(hProcess, &prot_addr,
			&prot_size, old_prot, &prot);
	} except(STATUS_ACCESS_VIOLATION) {
		return STATUS_ACCESS_VIOLATION;
	}
	return status;
}

NTSTATUS AllocMemoryToProcess(IN PEPROCESS pep, IN OUT PVOID *baseAddr, IN OUT SIZE_T *outSize, IN ULONG protect)
{
	NTSTATUS status;
	PKAPC_STATE apc;

	if (!baseAddr || !outSize) {
		return STATUS_UNSUCCESSFUL;
	}
	apc = MmAllocateNonCachedMemory(sizeof(*apc));
	if (!apc) {
		return STATUS_UNSUCCESSFUL;
	}
	KeStackAttachProcess((PRKPROCESS)pep, apc);
	status = ZwAllocateVirtualMemory(ZwCurrentProcess(), baseAddr, 0, outSize, MEM_COMMIT, protect);
	if (!NT_SUCCESS(status)) {
		KDBG("ZwAllocateVirtualMemory failed with 0x%X\n", status);
	}
	KeUnstackDetachProcess(apc);
	MmFreeNonCachedMemory(apc, sizeof(*apc));

	return status;
}

NTSTATUS FreeMemoryFromProcess(IN PEPROCESS pep, IN PVOID baseAddr, IN SIZE_T size)
{
	NTSTATUS status;
	PKAPC_STATE apc;

	apc = MmAllocateNonCachedMemory(sizeof(*apc));
	if (!apc) {
		return STATUS_UNSUCCESSFUL;
	}
	KeStackAttachProcess((PRKPROCESS)pep, apc);
	status = ZwFreeVirtualMemory(ZwCurrentProcess(), &baseAddr, &size, MEM_RELEASE);
	KeUnstackDetachProcess(apc);
	MmFreeNonCachedMemory(apc, sizeof(*apc));

	return status;
}