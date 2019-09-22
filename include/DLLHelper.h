#pragma once

#include <string>
#include <Windows.h>


typedef void(*LibEntry_FN)(void);

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

private:
	HANDLE m_TargetPID = 0;
	std::string m_DLLPath;
	DWORD m_DLLSize = 0;
	UINT8 *m_DLLPtr = nullptr;
	IMAGE_DOS_HEADER *m_DOSHeader = nullptr;
	IMAGE_NT_HEADERS *m_NTHeader = nullptr;
	PVOID m_TargetBaseAddress = nullptr;
};

