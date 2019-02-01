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
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <sstream>
#include <regex>

//Windows headers
#define UNICODE
#include <Windows.h>

//Program headers
#include "Profiles.hpp"
#include "GUI.hpp"
#include "AppData.hpp"

namespace ASP
{
	//init static const member vars
	const wchar_t * Profiles::ProfileFileName = L"profiles.bin";
	bool NewProfileAllowed = true;

	bool Profiles::LoadProfiles(std::vector<Profiles::Profile> *vec) //Load profile blocks from file and push to vector
	{
		if (vec == nullptr) return false;
		HANDLE proFILE = CreateFile(ProfileFileName, GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (proFILE == INVALID_HANDLE_VALUE)
		{
			Error(L"Load Profile (R)");
			return false;
		}
		std::vector<Profile> blocks;
		const unsigned int FileSize = GetFileSize(proFILE, nullptr);
		for (unsigned int i = 0; i < (FileSize / sizeof(Profile)); i++)
		{
			Profile temp;
			SecureZeroMemory(&temp, sizeof(temp));
			DWORD BytesRead = 0;
			if (!ReadFile(proFILE, &temp, sizeof(temp), &BytesRead, nullptr))
			{
				Error(L"Read Profile (R)");
				CloseHandle(proFILE);
				return false;
			}
			blocks.push_back(temp);
		}
		*vec = blocks;
		CloseHandle(proFILE);
		return true;
	}

	bool Profiles::CreateNewProfile(wchar_t Name[Profiles::MaxProfileNameLength + 1])
	{
		// get profile counter
		AppData ADO;
		AppData::Data data;
		ADO.ReadData(&data);
		if (data.NumProfiles == Profiles::MaxProfiles)
		{
			Error(L"Max Profile Limit Reached (W)");
			return false;
		}
		data.NumProfiles++;
		data.ProfileCounter++;
		// make the profile
		Profiles::Profile NewProfile = {};
		NewProfile.cur_lang_id = 0;
		NewProfile.cur_lesson_id = 0;
		NewProfile.profile_id = data.ProfileCounter;
		wcscpy_s(NewProfile.profile_name, Profiles::MaxProfileNameLength + 1, Name);
		// do the writing
		HANDLE proFILE = CreateFile(this->ProfileFileName, GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (proFILE == INVALID_HANDLE_VALUE)
		{
			Error(L"Load Profile (W)");
			return false;
		}
		DWORD BytesWritten = 0;
		// append mode
		if (SetFilePointer(proFILE, 0, nullptr, FILE_END) == INVALID_SET_FILE_POINTER)
		{
			Error(L"Write Profile - SFP (W)");
			return false;
		}
		if (!WriteFile(proFILE, &NewProfile, sizeof(Profile), &BytesWritten, nullptr))
		{
			Error(L"Write Profile (W)");
			return false;
		}
		// Create unique profile file
		const std::wstring FileName = L"profile_" + std::to_wstring(static_cast<int>(data.ProfileCounter)) + L".bin";
		HANDLE NewProFILE = CreateFile(FileName.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (NewProFILE == INVALID_HANDLE_VALUE)
		{
			Error(L"Create File (W)");
			return false;
		}
		ADO.WriteData(data);
		CloseHandle(proFILE);
		CloseHandle(NewProFILE);
		return true;
	}

	bool Profiles::CheckName(std::wstring name)
	{
		if (name.empty())
		{
			Error(L"You must provide a profile name.");
			return false; // no empty strings
		}
		std::wregex RegEx(L"([a-zA-Z0-9])+"); // allow only alphanumeric chars
		if (!std::regex_match(name, RegEx))
		{
			Error(L"Profile names can only contain alphanumeric characters (letters and numbers).");
			return false;
		}
		std::vector<Profiles::Profile> ProVec;
		Profiles::LoadProfiles(&ProVec);
		for (auto const & prof : ProVec)
		{
			if (name == std::wstring(prof.profile_name))
			{
				Error(L"A profile exists with the specified name.  Please choose another.");
				return false;
			}
		}
		return true;
	}
}