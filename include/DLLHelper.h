#pragma once

#include <string>
#include <Windows.h>


typedef void(*LibEntry_FN)(void);

static inline bool LoadAndTestLibraryEntry(const char * const fullDllPath)
{
	HMODULE TestDLLModule = LoadLibraryA(fullDllPath);
	LibEntry_FN LibEntryProc = (LibEntry_FN)GetProcAddress(TestDLLModule, "LibEntry");
	if (LibEntryProc) {
		LibEntryProc();
		return true;
	}
	else {
		return false;
	}
}

class DLLHelper
{
public:
	DLLHelper();
	~DLLHelper();

	bool Init(HANDLE targetPID, const char * fullDllPath);
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

private:
	HANDLE m_TargetPID = 0;
	std::string m_DLLPath;
	DWORD m_DLLSize = 0;
	UINT8 *m_DLLPtr = nullptr;
	IMAGE_DOS_HEADER *m_DOSHeader = nullptr;
	IMAGE_NT_HEADERS *m_NTHeader = nullptr;
	PVOID m_TargetBaseAddress = nullptr;
};

