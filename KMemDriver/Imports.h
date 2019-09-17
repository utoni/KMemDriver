#pragma once

#include <ntddk.h>
#include <wdm.h>


extern POBJECT_TYPE* IoDriverObjectType;

NTKERNELAPI
NTSTATUS
NTAPI
MmCopyVirtualMemory
(
	PEPROCESS SourceProcess,
	PVOID SourceAddress,
	PEPROCESS TargetProcess,
	PVOID TargetAddress,
	SIZE_T BufferSize,
	KPROCESSOR_MODE PreviousMode,
	PSIZE_T ReturnSize
);

NTKERNELAPI
NTSTATUS
NTAPI
PsLookupProcessByProcessId(
	_In_ HANDLE ProcessId,
	_Outptr_ PEPROCESS *Process
);

typedef struct _KAPC_STATE
{
	LIST_ENTRY ApcListHead[2];
	PKPROCESS Process;
	UCHAR KernelApcInProgress;
	UCHAR KernelApcPending;
	UCHAR UserApcPending;
} KAPC_STATE, *PKAPC_STATE, *PRKAPC_STATE;

NTKERNELAPI
VOID
NTAPI
KeStackAttachProcess(
	PRKPROCESS   PROCESS,
	PRKAPC_STATE ApcState
);

NTKERNELAPI
VOID
NTAPI
KeUnstackDetachProcess(
	PRKAPC_STATE ApcState
);

NTKERNELAPI
PPEB
NTAPI
PsGetProcessPeb(PEPROCESS Process);

NTKERNELAPI
NTSTATUS
NTAPI
ObOpenObjectByPointer(
	PVOID           Object,
	ULONG           HandleAttributes,
	PACCESS_STATE   PassedAccessState,
	ACCESS_MASK     DesiredAccess,
	POBJECT_TYPE    ObjectType,
	KPROCESSOR_MODE AccessMode,
	PHANDLE         Handle
);

typedef enum _MEMORY_INFORMATION_CLASS {
	MemoryBasicInformation
} MEMORY_INFORMATION_CLASS;

NTKERNELAPI
NTSTATUS
NTAPI
ZwQueryVirtualMemory(
	_In_      HANDLE                   ProcessHandle,
	_In_opt_  PVOID                    BaseAddress,
	_In_      MEMORY_INFORMATION_CLASS MemoryInformationClass,
	_Out_     PVOID                    MemoryInformation,
	_In_      SIZE_T                   MemoryInformationLength,
	_Out_opt_ PSIZE_T                  ReturnLength
);

NTKERNELAPI
NTSTATUS
NTAPI
ZwProtectVirtualMemory(
	IN HANDLE ProcessHandle,
	IN PVOID* BaseAddress, /* THIS IS ACTUALLY AN IN_OUT */
	IN SIZE_T* NumberOfBytesToProtect,
	IN ULONG NewAccessProtection,
	OUT PULONG OldAccessProtection
);

NTKERNELAPI
NTSTATUS
NTAPI
ObReferenceObjectByName(
	PUNICODE_STRING ObjectName,
	ULONG Attributes,
	PACCESS_STATE Passed,
	ACCESS_MASK DesiredAccess,
	POBJECT_TYPE ObjectType,
	KPROCESSOR_MODE Access,
	PVOID ParseContext,
	PVOID* ObjectPtr
);

NTSTATUS ZwAllocateVirtualMemory(
	_In_    HANDLE    ProcessHandle,
	_Inout_ PVOID     *BaseAddress,
	_In_    ULONG_PTR ZeroBits,
	_Inout_ PSIZE_T   RegionSize,
	_In_    ULONG     AllocationType,
	_In_    ULONG     Protect
);

NTSTATUS ZwFreeVirtualMemory(
	_In_    HANDLE  ProcessHandle,
	_Inout_ PVOID   *BaseAddress,
	_Inout_ PSIZE_T RegionSize,
	_In_    ULONG   FreeType
);

NTKERNELAPI
PVOID
NTAPI
PsGetProcessWow64Process(IN PEPROCESS Process);

NTSYSAPI
PVOID
NTAPI
RtlAvlRemoveNode(
	IN PRTL_AVL_TREE pTree,
	IN PMMADDRESS_NODE pNode
);