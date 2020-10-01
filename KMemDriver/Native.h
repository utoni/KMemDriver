#pragma once

#include <ntddk.h>

#define GET_VAD_ROOT(Table) Table->BalancedRoot

typedef enum native_offsets {
	VAD_TREE_1803 = 0x628
} native_offsets;

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

typedef struct _PEB_LDR_DATA32
{
	ULONG Length;
	UCHAR Initialized;
	ULONG SsHandle;
	LIST_ENTRY32 InLoadOrderModuleList;
	LIST_ENTRY32 InMemoryOrderModuleList;
	LIST_ENTRY32 InInitializationOrderModuleList;
} PEB_LDR_DATA32, *PPEB_LDR_DATA32;

typedef struct _LDR_DATA_TABLE_ENTRY32
{
	LIST_ENTRY32 InLoadOrderLinks;
	LIST_ENTRY32 InMemoryOrderLinks;
	LIST_ENTRY32 InInitializationOrderLinks;
	ULONG DllBase;
	ULONG EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING32 FullDllName;
	UNICODE_STRING32 BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY32 HashLinks;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY32, *PLDR_DATA_TABLE_ENTRY32;

typedef struct _PEB32
{
	UCHAR InheritedAddressSpace;
	UCHAR ReadImageFileExecOptions;
	UCHAR BeingDebugged;
	UCHAR BitField;
	ULONG Mutant;
	ULONG ImageBaseAddress;
	ULONG Ldr;
	ULONG ProcessParameters;
	ULONG SubSystemData;
	ULONG ProcessHeap;
	ULONG FastPebLock;
	ULONG AtlThunkSListPtr;
	ULONG IFEOKey;
	ULONG CrossProcessFlags;
	ULONG UserSharedInfoPtr;
	ULONG SystemReserved;
	ULONG AtlThunkSListPtr32;
	ULONG ApiSetMap;
} PEB32, *PPEB32;

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

#pragma warning(disable : 4214 4201)
#pragma pack(push, 1)

typedef struct _MM_AVL_NODE // Size=24
{
	struct _MM_AVL_NODE * LeftChild; // Size=8 Offset=0
	struct _MM_AVL_NODE * RightChild; // Size=8 Offset=8

	union // Size=8
	{
		struct
		{
			__int64  Red : 1; // Size=8 Offset=0 BitOffset=0 BitCount=1
		};
		struct
		{
			__int64  Balance : 2; // Size=8 Offset=0 BitOffset=0 BitCount=2
		};
		struct _MM_AVL_NODE * Parent; // Size=8 Offset=0
	};
} MM_AVL_NODE, *PMM_AVL_NODE, *PMMADDRESS_NODE;

union _EX_PUSH_LOCK // Size=8
{
	struct
	{
		unsigned __int64 Locked : 1; // Size=8 Offset=0 BitOffset=0 BitCount=1
		unsigned __int64 Waiting : 1; // Size=8 Offset=0 BitOffset=1 BitCount=1
		unsigned __int64 Waking : 1; // Size=8 Offset=0 BitOffset=2 BitCount=1
		unsigned __int64 MultipleShared : 1; // Size=8 Offset=0 BitOffset=3 BitCount=1
		unsigned __int64 Shared : 60; // Size=8 Offset=0 BitOffset=4 BitCount=60
	};
	unsigned __int64 Value; // Size=8 Offset=0
	void * Ptr; // Size=8 Offset=0
};

struct _MMVAD_FLAGS // Size=4
{
	unsigned long VadType : 3; // Size=4 Offset=0 BitOffset=0 BitCount=3
	unsigned long Protection : 5; // Size=4 Offset=0 BitOffset=3 BitCount=5
	unsigned long PreferredNode : 6; // Size=4 Offset=0 BitOffset=8 BitCount=6
	unsigned long NoChange : 1; // Size=4 Offset=0 BitOffset=14 BitCount=1
	unsigned long PrivateMemory : 1; // Size=4 Offset=0 BitOffset=15 BitCount=1
	unsigned long Teb : 1; // Size=4 Offset=0 BitOffset=16 BitCount=1
	unsigned long PrivateFixup : 1; // Size=4 Offset=0 BitOffset=17 BitCount=1
	unsigned long ManySubsections : 1; // Size=4 Offset=0 BitOffset=18 BitCount=1
	unsigned long Spare : 12; // Size=4 Offset=0 BitOffset=19 BitCount=12
	unsigned long DeleteInProgress : 1; // Size=4 Offset=0 BitOffset=31 BitCount=1
};

struct _MMVAD_FLAGS1 // Size=4
{
	unsigned long CommitCharge : 31; // Size=4 Offset=0 BitOffset=0 BitCount=31
	unsigned long MemCommit : 1; // Size=4 Offset=0 BitOffset=31 BitCount=1
};

union MMVAD_SHORT_u1 // Size=4
{
	unsigned long LongFlags; // Size=4 Offset=0
	struct _MMVAD_FLAGS VadFlags; // Size=4 Offset=0
};

union MMVAD_SHORT_u2 // Size=4
{
	unsigned long LongFlags1; // Size=4 Offset=0
	struct _MMVAD_FLAGS1 VadFlags1; // Size=4 Offset=0
};

typedef struct _MMVAD_SHORT // Size=64
{
	union
	{
		struct _RTL_BALANCED_NODE VadNode; // Size=24 Offset=0
		struct _MMVAD_SHORT * NextVad; // Size=8 Offset=0
	};
	unsigned long StartingVpn; // Size=4 Offset=24
	unsigned long EndingVpn; // Size=4 Offset=28
	unsigned char StartingVpnHigh; // Size=1 Offset=32
	unsigned char EndingVpnHigh; // Size=1 Offset=33
	unsigned char CommitChargeHigh; // Size=1 Offset=34
	unsigned char SpareNT64VadUChar; // Size=1 Offset=35
	long ReferenceCount; // Size=4 Offset=36
	union _EX_PUSH_LOCK PushLock; // Size=8 Offset=40
	union MMVAD_SHORT_u1 u; // Size=4 Offset=48
	union MMVAD_SHORT_u2 u1; // Size=4 Offset=52
	struct _MI_VAD_EVENT_BLOCK * EventList; // Size=8 Offset=56
} MMVAD_SHORT, *PMMVAD_SHORT;

struct _MMVAD_FLAGS2 // Size=4
{
	unsigned long FileOffset : 24; // Size=4 Offset=0 BitOffset=0 BitCount=24
	unsigned long Large : 1; // Size=4 Offset=0 BitOffset=24 BitCount=1
	unsigned long TrimBehind : 1; // Size=4 Offset=0 BitOffset=25 BitCount=1
	unsigned long Inherit : 1; // Size=4 Offset=0 BitOffset=26 BitCount=1
	unsigned long CopyOnWrite : 1; // Size=4 Offset=0 BitOffset=27 BitCount=1
	unsigned long NoValidationNeeded : 1; // Size=4 Offset=0 BitOffset=28 BitCount=1
	unsigned long Spare : 3; // Size=4 Offset=0 BitOffset=29 BitCount=3
};

struct _MI_VAD_SEQUENTIAL_INFO // Size=8
{
	unsigned __int64 Length : 12; // Size=8 Offset=0 BitOffset=0 BitCount=12
	unsigned __int64 Vpn : 52; // Size=8 Offset=0 BitOffset=12 BitCount=52
};

union ___unnamed2047 // Size=4
{
	unsigned long LongFlags2; // Size=4 Offset=0
	struct _MMVAD_FLAGS2 VadFlags2; // Size=4 Offset=0
};

union ___unnamed2048 // Size=8
{
	struct _MI_VAD_SEQUENTIAL_INFO SequentialVa; // Size=8 Offset=0
	struct _MMEXTEND_INFO * ExtendedInfo; // Size=8 Offset=0
};

typedef union _EX_FAST_REF // Size=8
{
	void * Object;
	struct
	{
		unsigned __int64 RefCnt : 4;
	};
	unsigned __int64 Value;
} EX_FAST_REF, *PEX_FAST_REF;

typedef struct _CONTROL_AREA // Size=120
{
	struct _SEGMENT * Segment;
	struct _LIST_ENTRY ListHead;
	unsigned __int64 NumberOfSectionReferences;
	unsigned __int64 NumberOfPfnReferences;
	unsigned __int64 NumberOfMappedViews;
	unsigned __int64 NumberOfUserReferences;
	unsigned long f1;
	unsigned long f2;
	EX_FAST_REF FilePointer;
	// Other fields
} CONTROL_AREA, *PCONTROL_AREA;

typedef struct _SUBSECTION // Size=56
{
	PCONTROL_AREA ControlArea;
	// Other fields
} SUBSECTION, *PSUBSECTION;

typedef struct _MMVAD // Size=128
{
	struct _MMVAD_SHORT Core; // Size=64 Offset=0
	union ___unnamed2047 u2; // Size=4 Offset=64
	unsigned long pad0;  // Size=4 Offset=68
	struct _SUBSECTION * Subsection; // Size=8 Offset=72
	struct _MMPTE * FirstPrototypePte; // Size=8 Offset=80
	struct _MMPTE * LastContiguousPte; // Size=8 Offset=88
	struct _LIST_ENTRY ViewLinks; // Size=16 Offset=96
	struct _EPROCESS * VadsProcess; // Size=8 Offset=112
	union ___unnamed2048 u4; // Size=8 Offset=120
	struct _FILE_OBJECT * FileObject; // Size=8 Offset=128
} MMVAD, *PMMVAD;

typedef enum _MI_VAD_TYPE
{
	VadNone,
	VadDevicePhysicalMemory,
	VadImageMap,
	VadAwe,
	VadWriteWatch,
	VadLargePages,
	VadRotatePhysical,
	VadLargePageSection
} MI_VAD_TYPE, *PMI_VAD_TYPE;

typedef struct _RTL_AVL_TREE // Size=8
{
	PMM_AVL_NODE BalancedRoot;
	void * NodeHint;
	UINT64 NumberGenericTableElements;
} RTL_AVL_TREE, *PRTL_AVL_TREE, MM_AVL_TABLE, *PMM_AVL_TABLE;

typedef struct _HANDLE_TABLE_ENTRY_INFO {
	UINT32 AuditMask;
	UINT32 MaxRelativeAccessMask;
} HANDLE_TABLE_ENTRY_INFO, *PHANDLE_TABLE_ENTRY_INFO;

typedef struct _HANDLE_TABLE_ENTRY
{
	union
	{
		PVOID Object;
		ULONG ObAttributes;
		PHANDLE_TABLE_ENTRY_INFO InfoTable;
		ULONG Value;
	};
	union
	{
		ULONG GrantedAccess;
		struct
		{
			SHORT GrantedAccessIndex;
			SHORT CreatorBackTraceIndex;
		};
		LONG NextFreeTableEntry;
	};
} HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;

typedef struct _HANDLE_TABLE_FREE_LIST
{
	EX_PUSH_LOCK FreeListLock;
	PHANDLE_TABLE_ENTRY FirstFreeHandleEntry;
	PHANDLE_TABLE_ENTRY LastFreeHandleEntry;
	UINT32 HandleCount;
	UINT32 HighWaterMark;
} HANDLE_TABLE_FREE_LIST, *PHANDLE_TABLE_FREE_LIST;

typedef struct _HANDLE_TABLE
{
	UINT32 NextHandleNeedingPool;
	UINT32 ExtraInfoPages;
	UINT32 TableCode;
	PEPROCESS QuotaProcess;
	LIST_ENTRY HandleTableList;
	UINT32 UniqueProcessId;
	union {
		UINT32 Flags;
		struct {
			UINT32 StrictFIFO : 1;
			UINT32 EnableHandleExceptions : 1;
			UINT32 Rundown : 1;
			UINT32 Duplicated : 1;
			UINT32 RaiseUMExceptionOnInvalidHandleClose : 1;
		};
	};
	EX_PUSH_LOCK HandleContentionEvent;
	EX_PUSH_LOCK HandleTableLock;
	HANDLE_TABLE_FREE_LIST FreeLists;
	UCHAR ActualEntry[32];
	PVOID DebugInfo;
} PHANDLE_TABLE;

typedef struct _SYSTEM_PROCESS_INFORMATION {
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	UINT8 Reserved1[48];
	UNICODE_STRING ImageName;
	KPRIORITY BasePriority;
	ULONG Reserved2;
	HANDLE UniqueProcessId;
	PVOID Reserved3;
	ULONG HandleCount;
	ULONG SessionId;
	PVOID Reserved4;
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize;
	ULONG Reserved5;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	PVOID Reserved6;
	SIZE_T QuotaPagedPoolUsage;
	PVOID Reserved7;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
	LARGE_INTEGER Reserved8[6];
} SYSTEM_PROCESS_INFORMATION;

#pragma pack(pop)
#pragma warning(default : 4214 4201)