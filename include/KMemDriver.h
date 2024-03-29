#pragma once

#ifdef KERNEL_MODULE
#include "Native.h"
#ifdef _DEBUG_
#define KDBG(fmt, ...) DbgPrint("KMemDriver[%01d]: " fmt, KeGetCurrentIrql(), __VA_ARGS__)
#else
#define KDBG(fmt, ...)
#endif
#else
#include <windows.h>
#ifndef NTSTATUS
typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
#endif
#endif

#define HDR_MAGIC				0xDEADC0DE
#define SHMEM_ADDR				0x60000000
#define SHMEM_SIZE				8192*8*2
#define INVALID_REQUEST			(UINT32)-1
#define KRNL_MAX_WAITS			20
#define KRNL_WAIT_TIME_US		1000000

#define MEM_HANDSHAKE			0x800
#define MEM_PING				0x801
#define MEM_PROCESSES			0x802
#define MEM_MODULES				0x803
#define MEM_PAGES				0x804
#define MEM_RPM					0x805
#define MEM_WPM					0x806
#define MEM_VALLOC				0x807
#define MEM_VFREE				0x808
#define MEM_EXIT				0x809

typedef struct _KERNEL_HEADER
{
	UINT32 magic;
	UINT32 type;
} KERNEL_HEADER, * PKERNEL_HEADER;

typedef struct _KERNEL_HANDSHAKE
{
	KERNEL_HEADER hdr;
	HANDLE kevent;
	HANDLE uevent;
} KERNEL_HANDSHAKE, * PKERNEL_HANDSHAKE;

typedef struct _KERNEL_PING
{
	KERNEL_HEADER hdr;
	UINT32 rnd_user;
	UINT32 rnd_kern;
} KERNEL_PING, * PKERNEL_PING;

typedef struct _KERNEL_PAGE
{
	KERNEL_HEADER hdr;
	HANDLE ProcessId;
	PVOID StartAddress;

	NTSTATUS StatusRes;
	SIZE_T pages;
	MEMORY_BASIC_INFORMATION pages_start;
} KERNEL_PAGE, * PKERNEL_PAGE;

typedef struct _MODULE_DATA
{
	PVOID DllBase;
	ULONG SizeOfImage;
	CHAR BaseDllName[64];
	CHAR FullDllPath[256];
} MODULE_DATA, * PMODULE_DATA;

typedef struct _KERNEL_MODULES
{
	KERNEL_HEADER hdr;
	HANDLE ProcessId;
	SIZE_T StartIndex;

	NTSTATUS StatusRes;
	SIZE_T modules;
	MODULE_DATA modules_start;
} KERNEL_MODULES, * PKERNEL_MODULES;

typedef struct _KERNEL_EXIT
{
	KERNEL_HEADER hdr;
} KERNEL_EXIT, * PKERNEL_EXIT;

typedef struct _KERNEL_READ_REQUEST
{
	KERNEL_HEADER hdr;
	HANDLE ProcessId;
	PVOID Address;
	SIZE_T SizeReq;

	NTSTATUS StatusRes;
	SIZE_T SizeRes;
} KERNEL_READ_REQUEST, * PKERNEL_READ_REQUEST;

typedef struct _KERNEL_WRITE_REQUEST
{
	KERNEL_HEADER hdr;
	HANDLE ProcessId;
	PVOID Address;
	SIZE_T SizeReq;

	NTSTATUS StatusRes;
	SIZE_T SizeRes;
} KERNEL_WRITE_REQUEST, * PKERNEL_WRITE_REQUEST;

typedef struct _KERNEL_VALLOC_REQUEST
{
	KERNEL_HEADER hdr;
	HANDLE ProcessId;
	PVOID AddressReq;
	SIZE_T SizeReq;
	ULONG Protection;

	NTSTATUS StatusRes;
	PVOID AddressRes;
	SIZE_T SizeRes;
} KERNEL_VALLOC_REQUEST, * PKERNEL_VALLOC_REQUEST;

typedef struct _KERNEL_VFREE_REQUEST
{
	KERNEL_HEADER hdr;
	HANDLE ProcessId;
	PVOID Address;
	SIZE_T Size;

	NTSTATUS StatusRes;
} KERNEL_VFREE_REQUEST, * PKERNEL_VFREE_REQUEST;

typedef struct _PROCESS_DATA
{
	ULONG NumberOfThreads;
	CHAR ImageName[80];
	HANDLE UniqueProcessId;
	ULONG HandleCount;
} PROCESS_DATA, * PPROCESS_DATA;

typedef struct _KERNEL_PROCESSES_REQUEST
{
	KERNEL_HEADER hdr;

	NTSTATUS StatusRes;
	SIZE_T ProcessCount;
	//PROCESS_DATA processes[ProcessCount];
} KERNEL_PROCESSES_REQUEST, * PKERNEL_PROCESSES_REQUEST;


#ifndef KERNEL_MODULE
static inline VOID prepareRequest(PVOID buf, UINT32 type)
{
	PKERNEL_HEADER hdr = (PKERNEL_HEADER)buf;
	hdr->magic = HDR_MAGIC;
	hdr->type = type;
}
#endif

#ifndef KERNEL_MODULE
#define validateResponeEx(buf, status, count) (!status && count <= SHMEM_SIZE) && validateRespone(buf)
#endif

static inline UINT32
#ifndef KERNEL_MODULE
validateRespone
#else
validateRequest
#endif
(PVOID buf)
{
	PKERNEL_HEADER hdr = (PKERNEL_HEADER)buf;
	if (hdr->magic != HDR_MAGIC)
		return INVALID_REQUEST;
	switch (hdr->type) {
	case MEM_HANDSHAKE:
	case MEM_PING:
	case MEM_PROCESSES:
	case MEM_PAGES:
	case MEM_MODULES:
	case MEM_RPM:
	case MEM_WPM:
	case MEM_VALLOC:
	case MEM_VFREE:
	case MEM_EXIT:
		return hdr->type;
	default:
		return INVALID_REQUEST;
	}
}