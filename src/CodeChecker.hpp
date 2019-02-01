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

#pragma once

#ifndef CODECHECKER_HPP
#define CODECHECKER_HPP

// STL headers
#include <string>

// Windows headers
#define UNICODE
#include <windows.h>

namespace ASP
{
	class CodeChecker
	{
		public:
			enum class Error : unsigned int
			{
				ALLGOOD = 0,
				LOADLIBRARY = 1,
				GETRESPBUFSIZE = 2,
				GETIDENTIFY = 3,
				GETCHECKSYNTAX = 4,
				GETQUERY = 5,
				STRINGERROR = 6,
				IDENTIFY = 7,
				CHECKSYNTAX = 8,
				QUERY = 9,
			};
		private:
			std::wstring language = L"";
			std::wstring langID = L"";
			std::wstring code = L"";
			HINSTANCE hDLL = nullptr;
			size_t respBufSize = 0;
			CodeChecker::Error errorState = CodeChecker::Error::ALLGOOD;
			const std::wstring errStr = L"!ERROR!";
			using LibFunctionIdentify = void(*)(wchar_t*, const size_t);
			using LibFunctionCheckSyntax = void (*)(const wchar_t*, wchar_t*, const size_t);
			using LibFunctionQuery = void(*)(const wchar_t*, const wchar_t*, wchar_t*, const size_t);
			LibFunctionIdentify IdentifyPtr = nullptr;
			LibFunctionCheckSyntax CheckSyntaxPtr = nullptr;
			LibFunctionQuery QueryPtr = nullptr;
		public:
			CodeChecker(const std::wstring& _language, const std::wstring& _langID, const std::wstring& _code) noexcept;
			void Init(void);
			size_t getRespBufSize(void) const noexcept;
			CodeChecker::Error getErrorState(void) const noexcept;
			std::wstring Identify(void);
			std::wstring CheckSyntax(void);
			std::wstring Query(const std::wstring& command);
			~CodeChecker(void) noexcept;
	};
}

#endif