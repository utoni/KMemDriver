#pragma once

#include <string>

class DLLHelper
{
public:
	DLLHelper();
	~DLLHelper();

	bool Init(std::string& fullDllPath);
	bool VerifyHeader();

private:
	std::string m_DLLPath;
	DWORD m_DLLSize;
	UINT8 *m_DLLPtr = nullptr;
	IMAGE_DOS_HEADER *m_DOSHeader;
	IMAGE_NT_HEADERS *m_NTHeader;
};

