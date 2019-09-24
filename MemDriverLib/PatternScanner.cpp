#include "stdafx.h"

#include "DLLHelper.h"
#include "PatternScanner.h"

#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
#include <Windows.h>


const struct map_file_data loadlib_data = {
	map_file_loadlib, map_file_loadlib_cleanup, true
};

const struct map_file_data kmem_data = {
	map_file_kmem, map_file_kmem_cleanup, false
};

bool map_file_loadlib(MODULE_DATA& module, PVOID * const buffer,
	SIZE_T * const size, PVOID const user_ptr)
{
	HMODULE hMod;
	struct loadlib_user_data * const user_data = (struct loadlib_user_data * const) user_ptr;
	std::vector<DLL_DIRECTORY_COOKIE> dir_cookies;

	if (user_data) {
		if (user_data->additionalDllSearchDirectories.size() == 1) {
			SetDllDirectoryA(user_data->additionalDllSearchDirectories[0].c_str());
		}
		else {
			for (auto& searchDir : user_data->additionalDllSearchDirectories) {
				dir_cookies.push_back(AddDllDirectory(std::wstring(searchDir.begin(),
					searchDir.end()).c_str()));
			}
			if (!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_USER_DIRS)) {
				return false;
			}
		}
	}

	hMod = LoadLibraryA(module.FullDllPath);

	if (user_data) {
		if (dir_cookies.size() > 1) {
			SetDllDirectoryA("");
		} else
		for (auto& searchDir : dir_cookies) {
			RemoveDllDirectory(searchDir);
		}
	}

	if (!hMod) {
		*buffer = NULL;
		*size = 0;
		return false;
	}
	else {
		*buffer = hMod;
		*size = module.SizeOfImage;
		return true;
	}
}

bool map_file_loadlib_cleanup(MODULE_DATA& module, PVOID buffer, PVOID const user_ptr)
{
	return FreeLibrary((HMODULE)buffer);
}

bool map_file_kmem(MODULE_DATA& module, PVOID * const buffer,
	SIZE_T * const size, PVOID const user_ptr)
{
	return false;
}

bool map_file_kmem_cleanup(MODULE_DATA& module, PVOID buffer, PVOID const user_ptr)
{
	return false;
}

PatternScanner::PatternScanner(struct map_file_data const * const mfd, PVOID map_file_user_data)
	: mfd(mfd), map_file_user_data(map_file_user_data)
{
	if (!mfd) {
		throw std::runtime_error("MapFileData was NULL");
	}
}

PatternScanner::~PatternScanner()
{
}

static void findAndReplaceAll(std::string& data, const char * const search, const char * const replace)
{
	std::string toSearch(search);
	std::string replaceStr(replace);

	size_t pos = data.find(toSearch);

	while (pos != std::string::npos)
	{
		data.replace(pos, toSearch.size(), replaceStr);
		pos = data.find(toSearch, pos + replaceStr.size());
	}
}

bool PatternScanner::checkPattern(MODULE_DATA& module, const char * const pattern, std::string& result)
{
	const char * const hexalnum = "0123456789abcdefABCDEF ?";
	std::string str_pattern(pattern);

	std::size_t found = str_pattern.find_first_not_of(hexalnum);
	if (found != std::string::npos) {
		std::stringstream err_str;
		err_str << "Found an invalid character at " << found
			<< " (allowed characters: \"" << hexalnum << "\")";
		throw std::runtime_error(err_str.str());
		return false;
	}

	findAndReplaceAll(str_pattern, " ", "");
	if (str_pattern.length() % 2 != 0) {
		std::stringstream err_str;
		err_str << "Pattern length is not a multiple of 2";
		throw std::runtime_error(err_str.str());
		return false;
	}

	result = str_pattern;
	return true;
}

bool PatternScanner::doScan(UINT8 *buf, SIZE_T size, std::vector<UINT64>& foundOffsets)
{
	return false;
}

#include <iostream>
bool PatternScanner::Scan(MODULE_DATA& module, const char * const pattern)
{
	std::string validPattern;
	IMAGE_NT_HEADERS *ntHeader;
	IMAGE_SECTION_HEADER *secHeader;
	UINT8 *mappedBuffer = NULL;
	SIZE_T mappedSize = 0;
	std::vector<UINT64> foundOffsets;

	if (!checkPattern(module, pattern, validPattern)) {
		return false;
	}

	if (!mfd->map_file(module, (PVOID *)&mappedBuffer, &mappedSize, map_file_user_data))
	{
		return false;
	}

	if (mfd->in_memory_module) {
		if (!VerifyPeHeader(mappedBuffer, mappedSize, &ntHeader) || !ntHeader) {
			return false;
		}

		DWORD nBytes = 0, virtualSize;
		secHeader = IMAGE_FIRST_SECTION(ntHeader);
		for (SIZE_T i = 0; ntHeader->FileHeader.NumberOfSections; i++)
		{
			if (nBytes >= ntHeader->OptionalHeader.SizeOfImage)
				break;

			std::cout << "Sec: " << secHeader->Name << std::endl;

			virtualSize = secHeader->VirtualAddress;
			secHeader++;
			virtualSize = secHeader->VirtualAddress - virtualSize;
			nBytes += virtualSize;
		}
	}
	else {
		doScan(mappedBuffer, mappedSize, foundOffsets);
	}

	if (!mfd->map_file_cleanup(module, mappedBuffer, map_file_user_data))
	{
		return false;
	}

	//std::wcout << "BLAAAAAAAAAAAAA" << std::endl;
	//std::wstring bla(str_pattern.begin(), str_pattern.end());
	//std::wcout << bla << std::endl;
	std::cout << validPattern << std::endl;

	return true;
}