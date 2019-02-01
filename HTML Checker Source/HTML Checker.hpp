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

#ifndef HTMLCHECKER_HPP
#define HTMLCHECKER_CPP

// STL headers
#include <string>
#include <map>

// types
#define DLL extern "C" __declspec(dllexport)
struct Elem
{
	std::string name = "";
	std::map<std::string, std::string> attributes; // (attr_name,attr_val)
};

// variables
DLL const size_t respBufSize = 64;

// "public" required functions
DLL void Identify(wchar_t* respBuf, const size_t bufSize);
DLL void CheckSyntax(const wchar_t* code, wchar_t* respBuf, const size_t bufSize);
DLL void Query(const wchar_t* code, const wchar_t* command, wchar_t* respBuf, const size_t bufSize);

// obscured functions
DLL void CheckStyle(const wchar_t* code, wchar_t* respBuf, const size_t bufSize);
DLL void Contains(const wchar_t* code, const wchar_t* command, wchar_t* respBuf, const size_t bufSize);

// helper functions
DLL bool checkRespBufSize(wchar_t* respBuf, const size_t bufSize);
DLL void copyToRespBuf(wchar_t* respBuf, const size_t respBufSize, const wchar_t* sourceBuf, const size_t sourceBufSize);
DLL bool elemContainsExactAttributes(const Elem& elem, TidyNode& childNode);

#endif