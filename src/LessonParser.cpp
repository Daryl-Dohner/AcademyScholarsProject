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

// Project Headers
#include "LessonParser.hpp"

// Windows Headers
#define UNICODE
#include <Windows.h>

// STL Headers
#include <iostream>
#include <sstream>
#include <iterator>

namespace ASP
{
	LessonParser::LessonParser(const std::wstring& _fileName, LangList* _langs) noexcept : filePath(_fileName), langs(_langs)
	{
		try
		{
			this->LoadFile();
		}
		catch (int err)
		{
			const std::wstring errMsg = L"LessonParser Error: " + std::to_wstring(err);
			Error(errMsg.c_str());
		}
		using namespace std::string_literals;
		commandMap = { { L"TITLE"s, &LessonParser::TITLE },
					   { L"LBOX"s, &LessonParser::LBOX },
					   { L"SCBOX"s, &LessonParser::SCBOX },
					   { L"BIGBOX"s, &LessonParser::BIGBOX },
					   { L"CCBUTTON"s, &LessonParser::CCBUTTON } };
	}

	void LessonParser::LoadFile()
	{
		HANDLE hFile = CreateFile(this->filePath.c_str(), GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE) throw 0;
		const unsigned int fileSize = GetFileSize(hFile, nullptr);
		wchar_t* buffer = new wchar_t[fileSize + 1];
		DWORD BytesRead = 0;
		if (!ReadFile(hFile, buffer, fileSize, &BytesRead, nullptr))
		{
			CloseHandle(hFile);
			throw 1;
		}
		CloseHandle(hFile);
		std::wstring rawData = std::wstring(buffer);
		delete[] buffer;
		if (rawData.size() == 0) throw 2;
		const std::wstring searchString = L"\r\n";
		size_t offset = 0;
		while(offset < rawData.size())
		{
			size_t foundIndex = 0;
			foundIndex = rawData.find(searchString, offset);
			std::wstring line = L"";
			if (foundIndex == std::wstring::npos)
			{
				line = rawData.substr(offset);
				this->fileLines.push_back(line);
				break;
			}
			else line = rawData.substr(offset, foundIndex - offset);
			this->fileLines.push_back(line);
			offset = foundIndex + searchString.size();
		}
		std::wcout << L'\n';
		for (const std::wstring& ln : this->fileLines) std::wcout << ln << L'\n';
	}

	void LessonParser::parse(const size_t line)
	{
		size_t nextLine = line + 1;
		if (this->fileLines.empty()) throw 3;
		for (auto const & commandPair : commandMap)
		{
			const size_t foundIndex = fileLines[line].find(commandPair.first);
			if (foundIndex == std::wstring::npos) continue;
			nextLine = (this->*commandPair.second)(line);
			break;
		}
		if(nextLine < this->fileLines.size()) this->parse(nextLine);
	}

	LessonParser::LessonData LessonParser::getLessonData() const noexcept
	{
		return this->lessonData;
	}

	void LessonParser::debugLessonData(const LessonData& ld)
	{
		std::wcout << L"\n_____LessonData_____"
				   << L"\nTitle: " << ld.Title
				   << L"\nLBox: " << ld.LBox
				   << L"\nSCBox: " << ld.SCBox
				   << L"\nSCLangID" << ld.SCLangID
				   << L"\nSCLang: " << ld.SCLang
				   << L"\nSCLock: " << ld.SCReadOnly
				   << L"\nBigBox: " << ld.BigBox
				   << L"\nCCButton: " << ld.CCButton
				   << L"\nCCButtonData: ";
		for (auto const & line : ld.CCButtonData) std::wcout << L"\n" << line;
		std::wcout << L"\nLBoxData:\n---\n" << ld.LBoxData
				   << L"\n---\nSCBoxData:\n---\n" << ld.SCBoxData
				   << L"\n---\nBigBoxData:\n---\n" << ld.BigBoxData
				   << L"\n____________________\n";
	}

	size_t LessonParser::findEND(const size_t curLine)
	{
		for (size_t line = curLine + 1; line < this->fileLines.size(); line++) // find the END command, signifying the end of the text for LBOX or SCBOX
		{
			if (fileLines[line].find(L"END") == 0) return line;
		}
		throw 5;
	}

	size_t LessonParser::TITLE(const size_t curLine) // Lesson Title
	{
		this->lessonData.Title = this->fileLines[curLine].substr(6); // 6 == length of "TITLE "
		return curLine + 1;
	}

	size_t LessonParser::LBOX(const size_t curLine) // Left Info Box
	{
		const size_t endLine = this->findEND(curLine);
		std::wstring textContent = this->fileLines[curLine].substr(5); // 5 == length of "LBOX "
		for (size_t lineNo = curLine + 1; lineNo < endLine; lineNo++) textContent += L"\r\n" + this->fileLines[lineNo];
		if (!this->lessonData.BigBox)
		{
			this->lessonData.LBox = true;
			this->lessonData.LBoxData = textContent;
		}
		return endLine + 1;
	}

	size_t LessonParser::SCBOX(const size_t curLine) // Source Code Box
	{
		const size_t endLine = this->findEND(curLine);
		const std::wstring rawParams = this->fileLines[curLine].substr(6); // 6 == length of "SCBOX "
		std::wistringstream iss(rawParams); // this line and next splits the rawParams delimited by a space
		const std::vector<std::wstring> params{std::istream_iterator<std::wstring, wchar_t>(iss), std::istream_iterator<std::wstring, wchar_t>()};
		std::wstring textContent = L"";
		for (size_t lineNo = curLine + 1; lineNo < endLine; lineNo++)
		{
			if (lineNo > curLine + 1) textContent += L"\r\n";
			textContent += this->fileLines[lineNo];
		}
		if (!this->lessonData.BigBox)
		{
			this->lessonData.SCBox = true;
			this->lessonData.SCBoxData = textContent;
			this->lessonData.SCLang = params[0];
			if (this->langs != nullptr)
			{
				for(auto const & lang : *langs)
				{
					if (lang.second->name == this->lessonData.SCLang)
					{
						this->lessonData.SCLangID = lang.first;
						break;
					}
				}
			}
			this->lessonData.SCReadOnly = (params.size() > 1 && params[1] == L"READONLY");
		}
		return endLine + 1;
	}

	size_t LessonParser::BIGBOX(const size_t curLine) // Full Page Info Box
	{
		const size_t endLine = this->findEND(curLine);
		std::wstring textContent = this->fileLines[curLine].substr(7); // 7 == length of "BIGBOX "
		for (size_t lineNo = curLine + 1; lineNo < endLine; lineNo++) textContent += L"\r\n" + this->fileLines[lineNo];
		if (!this->lessonData.LBox && !this->lessonData.SCBox)
		{
			this->lessonData.BigBox = true;
			this->lessonData.BigBoxData = textContent;
		}
		return endLine + 1;
	}

	size_t LessonParser::CCBUTTON(const size_t curLine) // Code Check Button
	{
		if (this->lessonData.SCBox && !this->lessonData.SCReadOnly)
		{
			this->lessonData.CCButton = true;
			const size_t endLine = this->findEND(curLine);
			for (size_t lineNo = curLine + 1; lineNo < endLine; lineNo++) this->lessonData.CCButtonData.push_back(this->fileLines[lineNo]);
			return endLine + 1;
		}
		return curLine + 1;
	}
}