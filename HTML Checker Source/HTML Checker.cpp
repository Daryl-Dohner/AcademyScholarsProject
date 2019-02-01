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

// HTML Checker.cpp : Defines the exported functions for the DLL application.

// Library headers
#define TIDY_STATIC
#include "tidy.h"
#include "tidybuffio.h"

// Windows headers
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include "targetver.h"
#include <windows.h>

// Project headers
#include "HTML Checker.hpp"

// STL headers
#include <string>
#include <sstream>
#include <vector>
#include <algorithm> // std::remove_if


// ----- "public" required functions -----

DLL void Identify(wchar_t* respBuf, const size_t bufSize)
{
	if (!checkRespBufSize(respBuf, bufSize)) return;
	const wchar_t* respStr = L"HTML Checker yaaaaay\0";
	copyToRespBuf(respBuf, bufSize, respStr, wcslen(respStr));
}

DLL void CheckSyntax(const wchar_t* code, wchar_t* respBuf, const size_t bufSize)
{
	if (!checkRespBufSize(respBuf, bufSize)) return;

	// convert from UTF-16 wchar_t to UTF-8 multibyte char
	const int wc2mb_size_needed = WideCharToMultiByte(CP_UTF8, 0, code, static_cast<int>(wcslen(code)), nullptr, 0, nullptr, nullptr);
	char* mbCodeBuf = new char[wc2mb_size_needed + 1];
	WideCharToMultiByte(CP_UTF8, 0, code, static_cast<int>(wcslen(code)), mbCodeBuf, wc2mb_size_needed, nullptr, nullptr);
	mbCodeBuf[wc2mb_size_needed] = '\0';

	// Initialize tidy
	TidyBuffer errorMessageBuffer = {};
	TidyDoc TD = tidyCreate();
	const bool err = tidyOptSetValue(TD, TidyCharEncoding, "utf8");
	if (!err)
	{
		const wchar_t * respStr = L"LibTidy Error 1\0";
		copyToRespBuf(respBuf, bufSize, respStr, wcslen(respStr));
		delete[] mbCodeBuf;
		tidyRelease(TD);
		return;
	}
	const int tseb_errorCode = tidySetErrorBuffer(TD, &errorMessageBuffer);
	if (tseb_errorCode != 0)
	{
		const wchar_t * respStr = L"LibTidy Error 2\0";
		copyToRespBuf(respBuf, bufSize, respStr, wcslen(respStr));
		delete[] mbCodeBuf;
		tidyRelease(TD);
		return;
	}

	// Check Syntax
	const int tps_errorCode = tidyParseString(TD, mbCodeBuf);
	if (tps_errorCode == 0) // no errors or warnings
	{
		const wchar_t * respStr = L"SUCCESS\0";
		const size_t respStrLen = wcslen(respStr);
		copyToRespBuf(respBuf, bufSize, respStr, respStrLen);
		delete[] mbCodeBuf;
		if(errorMessageBuffer.bp) tidyBufFree(&errorMessageBuffer);
		tidyRelease(TD);
		return;
	}
	else // there are errors and/or warnings
	{
		// convert back from UTF-8 multibyte char to UTF-16 wchar_t
		const int mb2wc_size_needed = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(errorMessageBuffer.bp), static_cast<int>(errorMessageBuffer.size), nullptr, 0);
		wchar_t * wErrMsgBuf = new wchar_t[mb2wc_size_needed];
		MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(errorMessageBuffer.bp), static_cast<int>(errorMessageBuffer.size), wErrMsgBuf, mb2wc_size_needed);

		// clean up memory and return
		copyToRespBuf(respBuf, bufSize, wErrMsgBuf, mb2wc_size_needed);
		delete[] wErrMsgBuf;
		delete[] mbCodeBuf;
		if(errorMessageBuffer.bp) tidyBufFree(&errorMessageBuffer);
		tidyRelease(TD);
		return;
	}
}

DLL void Query(const wchar_t* code, const wchar_t* command, wchar_t* respBuf, const size_t bufSize)
{
	if (!checkRespBufSize(respBuf, bufSize)) return;
	if (wcscmp(command, L"StyleCheck") == 0) CheckStyle(code, respBuf, bufSize);
	else if (wcsstr(command, L"contains") == command) Contains(code, command, respBuf, bufSize);
}

// ----- obscured functions -----
DLL void CheckStyle(const wchar_t* code, wchar_t* respBuf, const size_t bufSize)
{
	if (!checkRespBufSize(respBuf, bufSize)) return;
	
	// convert from UTF-16 wchar_t to UTF-8 multibyte char
	const int wc2mb_size_needed = WideCharToMultiByte(CP_UTF8, 0, code, static_cast<int>(wcslen(code)), nullptr, 0, nullptr, nullptr);
	char* mbCodeBuf = new char[wc2mb_size_needed + 1];
	WideCharToMultiByte(CP_UTF8, 0, code, static_cast<int>(wcslen(code)), mbCodeBuf, wc2mb_size_needed, nullptr, nullptr);
	mbCodeBuf[wc2mb_size_needed] = '\0';

	// Initialize tidy
	TidyBuffer errorMessageBuffer = {};
	TidyBuffer outputBuffer = {};
	TidyDoc TD = tidyCreate();
	const bool err = tidyOptSetValue(TD, TidyCharEncoding, "utf8");
	const bool err2 = tidyOptSetInt(TD, TidyIndentContent, TidyTriState::TidyAutoState);
	const bool err3 = tidyOptSetBool(TD, TidyForceOutput, Bool::yes);
	const bool err4 = tidyOptSetBool(TD, TidyDropEmptyElems, Bool::no);
	const bool err5 = tidyOptSetBool(TD, TidyDropEmptyParas, Bool::no);
	//const bool err6 = tidyOptSetInt(TD, TidyVertSpace, TidyTriState::TidyAutoState);
	if (!err || !err2 || !err3 || !err4 || !err5)
	{
		const wchar_t * respStr = L"LibTidy Error 1\0";
		copyToRespBuf(respBuf, bufSize, respStr, wcslen(respStr));
		delete[] mbCodeBuf;
		tidyRelease(TD);
		return;
	}
	const int tseb_errorCode = tidySetErrorBuffer(TD, &errorMessageBuffer);
	if (tseb_errorCode != 0)
	{
		const wchar_t * respStr = L"LibTidy Error 2\0";
		copyToRespBuf(respBuf, bufSize, respStr, wcslen(respStr));
		delete[] mbCodeBuf;
		tidyRelease(TD);
		return;
	}

	// Load Code
	const int tps_errorCode = tidyParseString(TD, mbCodeBuf);
	if(tps_errorCode != 0) // Welp, can't process style with syntax errors
	{
		const wchar_t * respStr = L"Error:  cannot check style if there are syntax errors!\0";
		const size_t respStrLen = wcslen(respStr);
		copyToRespBuf(respBuf, bufSize, respStr, respStrLen);
		delete[] mbCodeBuf;
		if (errorMessageBuffer.bp) tidyBufFree(&errorMessageBuffer);
		tidyRelease(TD);
		return;
	}

	// Check formatting
	const int tsb_errorCode = tidySaveBuffer(TD, &outputBuffer);
	char* outBuf = new char[outputBuffer.size + 1];
	strncpy(outBuf, reinterpret_cast<const char*>(outputBuffer.bp), outputBuffer.size);
	if (outBuf[outputBuffer.size - 2] == '\r') // remove trailing newline
	{
		outBuf[outputBuffer.size - 2] = '\0';
		outBuf[outputBuffer.size - 1] = '\0';
	}
	if (tsb_errorCode == 0)
	{
		if (strcmp(mbCodeBuf, outBuf) == 0)
		{
			const wchar_t * respStr = L"SUCCESS\0";
			const size_t respStrLen = wcslen(respStr);
			copyToRespBuf(respBuf, bufSize, respStr, respStrLen);
		}
		else
		{
			const wchar_t * respStr = L"Style Error: remember to indent and put new tags on new lines!\0";
			const size_t respStrLen = wcslen(respStr);
			copyToRespBuf(respBuf, bufSize, respStr, respStrLen);
		}
	}
	else
	{
		const wchar_t * respStr = L"LibTidy Error 3\0";
		copyToRespBuf(respBuf, bufSize, respStr, wcslen(respStr));
	}
	delete[] mbCodeBuf;
	if (errorMessageBuffer.bp) tidyBufFree(&errorMessageBuffer);
	if (outputBuffer.bp) tidyBufFree(&outputBuffer);
	tidyRelease(TD);
	return;
}

DLL void Contains(const wchar_t* code, const wchar_t* command, wchar_t* respBuf, const size_t bufSize)
{
	// Step 0: convert command from UTF-16 wchar_t to UTF-8 multibyte char
	const int wc2mb_size_needed_cmd = WideCharToMultiByte(CP_UTF8, 0, command, static_cast<int>(wcslen(command)), nullptr, 0, nullptr, nullptr);
	std::string mbCmdStr(wc2mb_size_needed_cmd, 0);
	WideCharToMultiByte(CP_UTF8, 0, command, static_cast<int>(wcslen(command)), &mbCmdStr[0], wc2mb_size_needed_cmd, nullptr, nullptr);

	// First thing on the agenda is parsing the command.  The parameter defines tags, nesting, and attributes.  All that info needs to be extracted
	std::string paramsStr = mbCmdStr.substr(8); // exclude the command name (contains) so we only have to parse the parameter
	paramsStr.erase(std::remove_if(paramsStr.begin(), paramsStr.end(), isspace), paramsStr.end()); // remove whitespace
	std::vector <Elem> elems = {};
	const char withinDelim = '>';
	size_t offset = paramsStr.length();
	while (offset != std::string::npos) // back-to-front extract elems (right-most elem is lowest-level nesting)
	{
		const size_t withinFoundPos = paramsStr.rfind(withinDelim, offset);
		std::string elemAndAttr = "";
		if (withinFoundPos == std::string::npos) elemAndAttr = paramsStr.substr(0, offset + 1);
		else elemAndAttr = paramsStr.substr(withinFoundPos + 1, offset - withinFoundPos);
		const size_t openParenFoundPos = elemAndAttr.find('(');
		const size_t closeParenFoundPos = elemAndAttr.find(')');
		if ((openParenFoundPos == std::string::npos) || (closeParenFoundPos == std::string::npos)) break;
		Elem elem = {};
		elem.name = elemAndAttr.substr(0, openParenFoundPos);
		if (closeParenFoundPos - openParenFoundPos != 1) // extract the attribute data
		{
			const std::string attributes = elemAndAttr.substr(openParenFoundPos + 1, closeParenFoundPos - openParenFoundPos - 1);
			std::istringstream stream(attributes);
			std::string attrKVPair = "";
			while (std::getline(stream, attrKVPair, '|'))
			{
				const size_t equalsFoundPos = attrKVPair.find('=');
				if (equalsFoundPos == std::string::npos) break;
				const std::string key = attrKVPair.substr(0, equalsFoundPos);
				const size_t firstQuotePos = attrKVPair.find('\"');
				const size_t lastQuotePos = attrKVPair.rfind('\"');
				if ((firstQuotePos == std::string::npos) || (lastQuotePos == std::string::npos) || (firstQuotePos == lastQuotePos)) break;
				const std::string value = attrKVPair.substr(firstQuotePos + 1, lastQuotePos - firstQuotePos - 1);
				elem.attributes.emplace(key, value);
			}
			if (elem.attributes.empty()) break;
		}
		elems.push_back(elem);
		if(withinFoundPos != std::string::npos) offset = withinFoundPos - 1;
		else break;
	}

	// Alright, time to process the code and check for our contains statement
	if (!checkRespBufSize(respBuf, bufSize)) return;

	// convert code from UTF-16 wchar_t to UTF-8 multibyte char
	const int wc2mb_size_needed = WideCharToMultiByte(CP_UTF8, 0, code, static_cast<int>(wcslen(code)), nullptr, 0, nullptr, nullptr);
	char* mbCodeBuf = new char[wc2mb_size_needed + 1];
	WideCharToMultiByte(CP_UTF8, 0, code, static_cast<int>(wcslen(code)), mbCodeBuf, wc2mb_size_needed, nullptr, nullptr);
	mbCodeBuf[wc2mb_size_needed] = '\0';

	// Initialize tidy
	TidyBuffer errorMessageBuffer = {};
	TidyBuffer outputBuffer = {};
	TidyDoc TD = tidyCreate();
	const bool err = tidyOptSetValue(TD, TidyCharEncoding, "utf8");
	if (!err)
	{
		const wchar_t * respStr = L"LibTidy Error 1\0";
		copyToRespBuf(respBuf, bufSize, respStr, wcslen(respStr));
		delete[] mbCodeBuf;
		tidyRelease(TD);
		return;
	}
	const int tseb_errorCode = tidySetErrorBuffer(TD, &errorMessageBuffer);
	if (tseb_errorCode != 0)
	{
		const wchar_t * respStr = L"LibTidy Error 2\0";
		copyToRespBuf(respBuf, bufSize, respStr, wcslen(respStr));
		delete[] mbCodeBuf;
		tidyRelease(TD);
		return;
	}

	// Load Code
	const int tps_errorCode = tidyParseString(TD, mbCodeBuf);
	if (tps_errorCode != 0) // Welp, can't process style with syntax errors
	{
		const wchar_t * respStr = L"Error:  cannot check style if there are syntax errors!\0";
		const size_t respStrLen = wcslen(respStr);
		copyToRespBuf(respBuf, bufSize, respStr, respStrLen);
		delete[] mbCodeBuf;
		if (errorMessageBuffer.bp) tidyBufFree(&errorMessageBuffer);
		tidyRelease(TD);
		return;
	}

	// Check against code
	TidyNode curNode = {};
	if (elems[0].name == "head")
	{
		curNode = tidyGetHead(TD);
	}
	else
	{
		curNode = tidyGetBody(TD);
	}
	int numSuccess = 1;
	for (int i = 1; i < elems.size(); i++)
	{
		// find the element type
		TidyNode lastChildNode = {};
		TidyNode curChildNode = tidyGetChild(curNode);
		while (curChildNode && (curChildNode != lastChildNode) )
		{
			const std::string name = tidyNodeGetName(curChildNode);
			if (name == elems[i].name)
			{
				if (elemContainsExactAttributes(elems[i], curChildNode))
				{
					numSuccess++;
					curNode = curChildNode;
					break;
				}
				else
				{
					lastChildNode = curChildNode;
					curChildNode = tidyGetNext(curChildNode);
				}
			}
			else
			{
				lastChildNode = curChildNode;
				curChildNode = tidyGetNext(curChildNode);
			}
		}
	}
	if (numSuccess == elems.size())
	{
		const wchar_t * respStr = L"SUCCESS";
		copyToRespBuf(respBuf, respBufSize, respStr, wcslen(respStr));
	}
	else
	{
		const int mb2wc_size_needed = MultiByteToWideChar(CP_UTF8, 0, &elems[numSuccess].name[0], static_cast<int>(elems[numSuccess].name.length()), nullptr, 0);
		std::wstring name(mb2wc_size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &elems[numSuccess].name[0], static_cast<int>(elems[numSuccess].name.length()), &name[0], mb2wc_size_needed);
		std::wstring respStr = L"The required \'" + name + L"\' element is missing or incorrect";
		copyToRespBuf(respBuf, respBufSize, respStr.c_str(), respStr.length());
	}
}

// ----- helper functions -----

DLL bool checkRespBufSize(wchar_t* respBuf, const size_t bufSize)
{
	if (bufSize < 2)  // forbidden; at least 2 chars needed for error reporting
	{
		if (bufSize == 1) respBuf[0] = L'0';
		return false;
	}
	else return true;
}

DLL void copyToRespBuf(wchar_t* respBuf, const size_t respBufSize, const wchar_t* sourceBuf, const size_t sourceBufSize)
{
	try
	{
		const size_t charsToCopy = (respBufSize < sourceBufSize) ? respBufSize : sourceBufSize;
		wcsncpy(respBuf, sourceBuf, respBufSize);
		//const errno_t err = wcsncpy_s(respBuf, respBufSize, sourceBuf, sourceBufSize);
		//if ( (err != 0) && (respBuf[0] == L'\0') ) respBuf[1] = static_cast<wchar_t>(err);
	}
	catch (...){}
}

DLL bool elemContainsExactAttributes(const Elem& elem, TidyNode& childNode)
{
	int numSuccess = 0;
	for (auto const & attrPair : elem.attributes)
	{
		TidyAttr lastAttr = {};
		TidyAttr curAttr = tidyAttrFirst(childNode);
		while (curAttr && (curAttr != lastAttr) )
		{
			const std::string attrName = tidyAttrName(curAttr);
			const std::string attrValue = tidyAttrValue(curAttr);
			if ( (attrName == attrPair.first) && (attrValue == attrPair.second))
			{
				numSuccess++;
				break;
			}
			else
			{
				lastAttr = curAttr;
				curAttr = tidyAttrNext(curAttr);
			}
		}
	}
	return numSuccess == elem.attributes.size();
}