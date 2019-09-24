#pragma once

#include <string>
#include <Windows.h>


typedef void(*LibEntry_FN)(void);

static inline bool LoadAndTestLibraryEntry(const char * const fullDllPath);
bool VerifyPeHeader(UINT8 const * const buf, SIZE_T siz, IMAGE_NT_HEADERS ** const return_NTHeader);

class DLLHelper
{
public:
	DLLHelper();
	~DLLHelper();

	bool Init(HANDLE targetPID, const char * const fullDllPath);
	bool VerifyHeader();
	bool InitTargetMemory();
	bool HasImports() {
		return m_NTHeader &&
			m_NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
	}
	bool FixImports();
	bool HasRelocs() {
		return m_NTHeader &&
			m_NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
	}
	bool FixRelocs();
	bool CopyHeaderAndSections();
	UINT64 GetEntryPoint() {
		if (!m_NTHeader) {
			return 0;
		}
		return (UINT64)m_TargetBaseAddress + m_NTHeader->OptionalHeader.AddressOfEntryPoint;
	}
	UINT64 GetBaseAddress() {
		return (UINT64)m_TargetBaseAddress;
	}
	UINT64 GetDllProcAddress(const char * const proc_name) {
		HMODULE hBase = LoadLibraryA(m_DLLPath.c_str());
		FARPROC hEntry = GetProcAddress(hBase, proc_name);
		UINT64 result = ((UINT64)hEntry - (UINT64)hBase) + (UINT64)m_TargetBaseAddress;
		FreeLibrary(hBase);
		return result;
	}

private:
	HANDLE m_TargetPID = 0;
	std::string m_DLLPath;
	DWORD m_DLLSize = 0;
	UINT8 *m_DLLPtr = nullptr;
	IMAGE_NT_HEADERS *m_NTHeader = nullptr;
	PVOID m_TargetBaseAddress = nullptr;
};

