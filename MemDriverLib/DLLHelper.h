#pragma once

#include <string>

class DLLHelper
{
public:
	DLLHelper();
	~DLLHelper();

	bool Init(HANDLE targetPID, std::string& fullDllPath);
	bool VerifyHeader();
	bool InitTargetMemory();

private:
	HANDLE m_TargetPID = 0;
	std::string m_DLLPath;
	DWORD m_DLLSize = 0;
	UINT8 *m_DLLPtr = nullptr;
	IMAGE_DOS_HEADER *m_DOSHeader = nullptr;
	IMAGE_NT_HEADERS *m_NTHeader = nullptr;
	PVOID m_TargetBaseAddress = nullptr;
};

