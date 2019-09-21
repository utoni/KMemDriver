#pragma once

#include <string>
#include <Windows.h>

class DLLHelper
{
public:
	DLLHelper();
	~DLLHelper();

	bool Init(HANDLE targetPID, const char * fullDllPath);
	bool VerifyHeader();
	bool InitTargetMemory();
	bool FixImports();

private:
	HANDLE m_TargetPID = 0;
	std::string m_DLLPath;
	DWORD m_DLLSize = 0;
	UINT8 *m_DLLPtr = nullptr;
	IMAGE_DOS_HEADER *m_DOSHeader = nullptr;
	IMAGE_NT_HEADERS *m_NTHeader = nullptr;
	PVOID m_TargetBaseAddress = nullptr;
};

