#include "KMemDriver.h"
#include "Imports.h"
#include "Native.h"

#include <ntddk.h>
#include <Ntstrsafe.h>

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