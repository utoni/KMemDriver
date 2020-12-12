#pragma once

#include <ntddk.h>

void CryptoInit(PVOID fn, ...);
void CryptoDo(PVOID fn);

#define CRYPTO_FNPTR(fn) ((PVOID)fn)

#define CRYPT_PROLOGUE() \
	do { \
		volatile UINT64 index_and_marker = { 0x11111111C0DEC0DE }; \
		UNREFERENCED_PARAMETER(index_and_marker); \
	} while (0)
#define CRYPT_EPILOGUE() \
	do { \
		volatile UINT32 marker = 0xDEADDEAD;\
		UNREFERENCED_PARAMETER(marker); \
	} while (0)