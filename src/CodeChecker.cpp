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

// STL headers
#include <vector>

// program headers
#include "CodeChecker.hpp"
#include "misc.hpp"
#include "gui.hpp"

namespace ASP
{
	CodeChecker::CodeChecker(const std::wstring& _language, const std::wstring& _langID, const std::wstring& _code) noexcept : language(_language), langID(_langID), code(_code)
	{
		// load the CHECKER.dll
		const std::wstring dllPath = L"C:\\Users\\Daryl_2017\\Desktop\\Dev\\ACA\\src\\Languages\\" + _langID + L"_" + _language + L"\\" + GUI::ReservedLessonFileNames[1];
		this->hDLL = LoadLibrary(dllPath.c_str());
		if (!this->hDLL)
		{
			this->errorState = CodeChecker::Error::LOADLIBRARY;
			ErrorExit();
		}
	}

	CodeChecker::~CodeChecker() noexcept
	{
		FreeLibrary(this->hDLL);
	}

	void CodeChecker::Init()
	{
		if (this->errorState != CodeChecker::Error::ALLGOOD) return;
		size_t* respBufSizePtr = reinterpret_cast<size_t*>(GetProcAddress(this->hDLL, "respBufSize"));
		if (!respBufSizePtr)
		{
			this->errorState = CodeChecker::Error::GETRESPBUFSIZE;
			return;
		}
		this->respBufSize = *respBufSizePtr;
		this->IdentifyPtr = reinterpret_cast<CodeChecker::LibFunctionIdentify>(GetProcAddress(this->hDLL, "Identify"));
		if (!this->IdentifyPtr)
		{
			this->errorState = CodeChecker::Error::GETIDENTIFY;
			return;
		}
		this->CheckSyntaxPtr = reinterpret_cast<CodeChecker::LibFunctionCheckSyntax>(GetProcAddress(this->hDLL, "CheckSyntax"));
		if (!this->CheckSyntaxPtr)
		{
			this->errorState = CodeChecker::Error::GETCHECKSYNTAX;
			return;
		}
		this->QueryPtr = reinterpret_cast<CodeChecker::LibFunctionQuery>(GetProcAddress(this->hDLL, "Query"));
		if (!this->QueryPtr)
		{
			this->errorState = CodeChecker::Error::GETQUERY;
			return;
		}
	}

	size_t CodeChecker::getRespBufSize() const noexcept
	{
		if (this->errorState != CodeChecker::Error::GETRESPBUFSIZE) return this->respBufSize;
		else return 0;
	}

	CodeChecker::Error CodeChecker::getErrorState() const noexcept
	{
		return this->errorState;
	}

	std::wstring CodeChecker::Identify()
	{
		if (this->errorState != CodeChecker::Error::ALLGOOD) return this->errStr;
		std::vector<wchar_t> buffer(this->respBufSize + 1, 0);
		this->IdentifyPtr(&buffer[0], buffer.size());
		if (buffer[0] == L'\0')
		{
			if(buffer[1] == EINVAL || buffer[1] == ERANGE || buffer[1] == STRUNCATE) this->errorState = CodeChecker::Error::STRINGERROR;
			else this->errorState = CodeChecker::Error::IDENTIFY;
			return this->errStr + L" - !" + std::to_wstring(buffer[1]) + L"!";
		}
		return std::wstring(&buffer[0]);
	}

	std::wstring CodeChecker::CheckSyntax()
	{
		if (errorState != CodeChecker::Error::ALLGOOD) return this->errStr;
		std::vector<wchar_t> buffer(this->respBufSize + 1, 0);
		this->CheckSyntaxPtr(this->code.c_str(), &buffer[0], buffer.size() - 1);
		if (buffer.at(0) == L'\0')
		{
			if (buffer[1] == EINVAL || buffer[1] == ERANGE || buffer[1] == STRUNCATE) this->errorState = CodeChecker::Error::STRINGERROR;
			else this->errorState = CodeChecker::Error::CHECKSYNTAX;
			return this->errStr + L" - !" + std::to_wstring(buffer[1]) + L"!";
		}
		return std::wstring(&buffer[0]);
	}

	std::wstring CodeChecker::Query(const std::wstring& command)
	{
		if (errorState == CodeChecker::Error::GETQUERY) return this->errStr;
		std::vector<wchar_t> buffer(this->respBufSize + 1, 0);
		this->QueryPtr(this->code.c_str(), command.c_str(), &buffer[0], buffer.size());
		if (buffer[0] == L'\0')
		{
			if (buffer[1] == EINVAL || buffer[1] == ERANGE || buffer[1] == STRUNCATE) this->errorState = CodeChecker::Error::STRINGERROR;
			else this->errorState = CodeChecker::Error::QUERY;
			return this->errStr + L" - !" + std::to_wstring(buffer[1]) + L"!";
		}
		return std::wstring(&buffer[0]);
	}
}