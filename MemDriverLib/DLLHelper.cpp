#include "stdafx.h"
#include "DLLHelper.h"

#include <sstream>
#include <Windows.h>


#define MakePtr(cast, ptr, addValue) (cast)((DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))
#define MakeDelta(cast, x, y) (cast) ((DWORD_PTR)(x) - (DWORD_PTR)(y))


DLLHelper::DLLHelper()
{
}


DLLHelper::~DLLHelper()
{
	if (m_DLLPtr) {
		delete m_DLLPtr;
	}
}

bool DLLHelper::Init(std::string& fullDllPath) {
	m_DLLPath = fullDllPath;

	HANDLE hFile = CreateFileA(m_DLLPath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		std::stringstream err_str;
		err_str << "Open file '" << m_DLLPath << "': " << GetLastError() << std::endl;
		throw std::runtime_error(err_str.str());
		return false;
	}

	if (GetFileAttributesA(m_DLLPath.c_str()) & FILE_ATTRIBUTE_COMPRESSED) {
		m_DLLSize = GetCompressedFileSizeA(m_DLLPath.c_str(), NULL);
	}
	else {
		m_DLLSize = GetFileSize(hFile, NULL);
	}

	m_DLLPtr = new UINT8[m_DLLSize];

	DWORD nBytes = 0;
	if (!ReadFile(hFile, m_DLLPtr, m_DLLSize, &nBytes, FALSE)) {
		std::stringstream err_str;
		err_str << "Read file '" << m_DLLPath << "': " << GetLastError() << std::endl;
		throw std::runtime_error(err_str.str());
		return false;
	}
	if (m_DLLSize != nBytes) {
		std::stringstream err_str;
		err_str << "Read file '" << m_DLLPath << "': returned "
			<< nBytes << " != " << m_DLLSize << std::endl;
		throw std::runtime_error(err_str.str());
		return false;
	}

	CloseHandle(hFile);
	return true;
}

bool DLLHelper::VerifyHeader()
{
	if (!m_DLLPtr) {
		return false;
	}
	m_DOSHeader = MakePtr(IMAGE_DOS_HEADER *, m_DLLPtr, 0);

	if (m_DOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		delete m_DLLPtr;
		return false;
	}
	m_NTHeader = MakePtr(IMAGE_NT_HEADERS *, m_DLLPtr, m_DOSHeader->e_lfanew);
	if (m_NTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		delete m_DLLPtr;
		return false;
	}

	return true;
}