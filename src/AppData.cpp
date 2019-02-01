//	Copyright (c) 2016-2019 Daryl Dohner
//	
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//	
//	The above copyright notice and this permission notice shall be included in all
//	copies or substantial portions of the Software.
//	
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//	SOFTWARE.

//STL headers
#include <iostream>
#include <string>

//Windows headers
#define UNICODE
#include <Windows.h>

//Program headers
#include "misc.hpp"
#include "AppData.hpp"

namespace ASP
{
	//init static const member vars
	const wchar_t * AppData::AppDataFileName = L"data.bin";


	bool AppData::ReadData(AppData::Data *data) noexcept //read data from file to struct
	{
		HANDLE DataFile = CreateFile(AppDataFileName, GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (DataFile == INVALID_HANDLE_VALUE)
		{
			Error(L"Load App Data (R)");
			return false;
		}
		AppData::Data temp = {};
		DWORD BytesRead = 0;
		if (!ReadFile(DataFile, &temp, sizeof(temp), &BytesRead, nullptr))
		{
			Error(L"Read App Data (R)");
			return false;
		}
		CloseHandle(DataFile);
		*data = temp;
		return true;
	}

	bool AppData::WriteData(const AppData::Data& data) noexcept // write data from struct to file
	{
		HANDLE DataFile = CreateFile(AppDataFileName, GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS | TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (DataFile == INVALID_HANDLE_VALUE)
		{
			Error(L"Load App Data (W)");
			return false;
		}
		DWORD BytesWritten = 0;
		if (!WriteFile(DataFile, &data, sizeof(data), &BytesWritten, nullptr))
		{
			Error(L"Write App Data (W)");
			return false;
		}
		CloseHandle(DataFile);
		return true;
	}

	bool AppData::DebugData(bool NewConsole) // read data from file to console for debugging
	{
		if (NewConsole) Console();
		AppData::Data data = {};
		if (!AppData::ReadData(&data))
		{
			std::wcout << L"DebugData Error" << std::endl;
			return false;
		}
		std::wcout << L"DebugData:" << std::endl;
		std::wcout << L"\t| " << (int)data.ProfileCounter << std::endl;
		std::wcout << L"\t| " << (int)data.NumProfiles << std::endl;
		return true;
	}

	void AppData::ResetFile() noexcept//reset the data file
	{
		AppData::Data ds = {};
		AppData::WriteData(ds);
	}
}