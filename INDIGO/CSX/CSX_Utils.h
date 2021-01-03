#pragma once
#include "CSX.h"

namespace CSX {
	namespace Utils {
//[swap_lines]
		/* Wait dwMsec Load Module */
		bool IsModuleLoad(PCHAR szModule, DWORD dwMsec = 5000);

		/* Get Current Process Path */
		string GetCurrentProcessPath();

		/* Get Current Process Name */
		string GetCurrentProcessName();

		/* Get Module File Path */
		string GetModuleFilePath(HMODULE hModule);

		/* Get Module Dir */
		string GetModuleBaseDir(HMODULE hModule);

		/* Random Int Range */
		int RandomIntRange(int min, int max);

		/* Get hwProfile GUID */
		string GetHwProfileGUID();

		/* VMT Return False If Read Ptr Error */
		bool VMT_IsBadReadPtr(PVOID pPointer);

		/* New Return False If Read Ptr Error */
		bool New_IsBadReadPtr(std::uintptr_t* pPointer);

		/* Unicode To Utf8 Convert */
		BSTR ConvertStringToBSTR_UTF8(const char* szBuff);

		std::string WstringToString(std::wstring wstr);
		std::wstring StringToWstring(std::string str);

		/* GetHackWorkingDirectory */
		std::string GetHackWorkingDirectory();
//[/swap_lines]
	}
}