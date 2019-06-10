#pragma once

#include <ntddk.h>

typedef struct _PEB_LDR_DATA
{
	ULONG Length;
	UCHAR Initialized;
	PVOID SsHandle;
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB
{
	UCHAR InheritedAddressSpace;
	UCHAR ReadImageFileExecOptions;
	UCHAR BeingDebugged;
	UCHAR BitField;
	PVOID Mutant;
	PVOID ImageBaseAddress;
	PPEB_LDR_DATA Ldr;
	PVOID ProcessParameters;
	PVOID SubSystemData;
	PVOID ProcessHeap;
	PVOID FastPebLock;
	PVOID AtlThunkSListPtr;
	PVOID IFEOKey;
	PVOID CrossProcessFlags;
	PVOID KernelCallbackTable;
	ULONG SystemReserved;
	ULONG AtlThunkSListPtr32;
	PVOID ApiSetMap;
} PEB, *PPEB;

typedef struct _MEMORY_BASIC_INFORMATION {
	PVOID  BaseAddress;
	PVOID  AllocationBase;
	ULONG  AllocationProtect;
	SIZE_T RegionSize;
	ULONG  State;
	ULONG  Protect;
	ULONG  Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

typedef struct _KLDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;
	PVOID ExceptionTable;
	ULONG ExceptionTableSize;
	PVOID GpValue;
	PVOID NonPagedDebugInfo;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT __Unused;
	PVOID SectionPointer;
	ULONG CheckSum;
	PVOID LoadedImports;
	PVOID PatchInformation;
} KLDR_DATA_TABLE_ENTRY, *PKLDR_DATA_TABLE_ENTRY;

typedef struct _MM_AVL_NODE // Size=24
{
	struct _MM_AVL_NODE * LeftChild; // Size=8 Offset=0
	struct _MM_AVL_NODE * RightChild; // Size=8 Offset=8

	union // Size=8
	{
		struct
		{
			INT Red : 1; // Size=8 Offset=0 BitOffset=0 BitCount=1
		} s1;
		struct
		{
			INT Balance : 2; // Size=8 Offset=0 BitOffset=0 BitCount=2
		} s2;
		struct _MM_AVL_NODE * Parent; // Size=8 Offset=0
	} u1;
} MM_AVL_NODE, *PMM_AVL_NODE, *PMMADDRESS_NODE;

typedef struct _RTL_AVL_TREE // Size=8
{
	PMM_AVL_NODE BalancedRoot;
	void * NodeHint;
	UINT64 NumberGenericTableElements;
} RTL_AVL_TREE, *PRTL_AVL_TREE, MM_AVL_TABLE, *PMM_AVL_TABLE;