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

//msvc
#pragma comment(lib, "netapi32.lib") //version info

//STL headers
#include <vector>
#include <iostream>
#include <string>
#include <cstring> // strlen

//Windows headers
#define UNICODE
#include <windows.h>
#include <strsafe.h> // for code in ErrorExit
#include <Lm.h> // for code in GetWindowsMajorVersion
#include <windowsx.h> // for Edit_ functions

//Program headers
#include "misc.hpp"

namespace ASP
{
	void SetPalette(Palette* CP) noexcept // Define Color Scheme
	{
		CP->BackgroundColor = RGB(207, 207, 250);
		CP->TextColor = RGB(127, 0, 0);
		CP->BoxColor = RGB(160, 160, 160);
		CP->BorderColor = RGB(127, 0, 0);
		CP->LightGray = RGB(190, 190, 190);
		CP->BoxColorDisabled = RGB(190, 190, 190);
		//CP->BorderColorDisabled = RGB(128, 64, 64);
		CP->BorderColorDisabled = RGB(130, 130, 130);
		CP->BoxColorSelected = RGB(140, 160, 200);
		CP->BorderColorSelected = RGB(79, 119, 198);
		CP->BlueButtonColor = RGB(59, 89, 149);
		CP->Black = RGB(34, 34, 34),
		CP->Green = RGB(0, 255, 0);
		CP->Red = RGB(127, 0, 0);
	}

	void ErrorExit(const wchar_t * lpszFunction) //Windows Error function - modified
	{
		void * lpMsgBuf = nullptr;
		const int dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<wchar_t*>(&lpMsgBuf),
			0, nullptr);

		void * lpDisplayBuf = LocalAlloc(LMEM_ZEROINIT,
			(lstrlen(static_cast<const wchar_t*>(lpMsgBuf)) + lstrlen(lpszFunction) + 40) * sizeof(TCHAR));

		if (lpDisplayBuf != nullptr)
			StringCchPrintf(static_cast<wchar_t*>(lpDisplayBuf),
				LocalSize(lpDisplayBuf) / sizeof(TCHAR),
				TEXT("%s failed with error %d: %s"),
				lpszFunction, dw, lpMsgBuf);

		MessageBox(nullptr, static_cast<wchar_t*>(lpDisplayBuf), TEXT("Error"), MB_OK);

		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);
		//ExitProcess(1);
	}

	void Error(const wchar_t * ErrDesc) noexcept
	{
		MessageBox(nullptr, ErrDesc, L"Error", MB_OK);
	}

	void Console() noexcept // ready a debug console
	{
		AllocConsole();
		FILE * stream = nullptr;
		if(freopen_s(&stream, "CONOUT$", "w", stdout) != 0) Error(L"Cannot Open Console");
		std::ios_base::sync_with_stdio(false);
		// do not close stream
	}

	void DebugRect(const RECT& rect, const std::wstring name) // pretty, isn't it?
	{
		std::wcout << L'\n';
		if (name != L"") std::wcout << name << L":\n";
		std::wcout << L'\t' << rect.top << L'\n';
		std::wcout << rect.left << L"\t\t" << rect.right << L'\n';
		std::wcout << L'\t' << rect.bottom << L'\n';
	}

	bool RectScreenToClient(const HWND hwnd, RECT* rect) noexcept
	{
		POINT TopLeft;
		TopLeft.x = rect->left;
		TopLeft.y = rect->top;
		const bool Call_1 = ScreenToClient(hwnd, &TopLeft);

		POINT BottomRight;
		BottomRight.x = rect->right;
		BottomRight.y = rect->bottom;
		const bool Call_2 = ScreenToClient(hwnd, &BottomRight);

		rect->left = TopLeft.x;
		rect->top = TopLeft.y;
		rect->right = BottomRight.x;
		rect->bottom = BottomRight.y;

		return (Call_1 && Call_2);
	}

	bool RectClientToScreen(const HWND hwnd, RECT* rect) noexcept
	{
		POINT TopLeft;
		TopLeft.x = rect->left;
		TopLeft.y = rect->top;
		const bool Call_1 = ClientToScreen(hwnd, &TopLeft);

		POINT BottomRight;
		BottomRight.x = rect->right;
		BottomRight.y = rect->bottom;
		const bool Call_2 = ClientToScreen(hwnd, &BottomRight);

		rect->left = TopLeft.x;
		rect->top = TopLeft.y;
		rect->right = BottomRight.x;
		rect->bottom = BottomRight.y;

		return (Call_1 && Call_2);
	}

	int GetWindowsMajorVersion() noexcept
	{
		SERVER_INFO_101* ServerInfo = nullptr;
		if (NetServerGetInfo(nullptr, 101, reinterpret_cast<LPBYTE*>(&ServerInfo)) != NERR_Success) return -1;
		return ServerInfo->sv101_version_major;
	}

	void DebugLangList(LangList& langs)
	{
		for (auto const & lang : langs)
		{
			std::wcout << lang.second->name << L" (" << lang.second->langID << L")\n";
			for (auto lessonGroup : lang.second->lessonGroups)
			{
				std::wcout << L'\t' << lessonGroup.second.name << L" (" << lessonGroup.first << L")\n";
				for (auto lesson : lessonGroup.second.lessons)
				{
					std::wcout << L"\t\t" << lesson.second << L" (" << lesson.first << L")\n";
				}
			}
			std::wcout << L'\n';
		}
	}

	// following two functions taken from the StackExchange Network https://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte
	// Question Attribution: Obediah Stane (https://stackoverflow.com/users/23120/obediah-stane), Edited by Antti Haapala (https://stackoverflow.com/users/918959/antti-haapala)
	// Answer Attribution: tfinniga (https://stackoverflow.com/users/9042/tfinniga)
	// Modified by me
	std::string utf8_encode(const std::wstring &wstr) // Convert a wide Unicode string to an UTF8 string
	{
		if (wstr.empty()) return std::string();
		const int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), &strTo[0], size_needed, nullptr, nullptr);
		return strTo;
	}

	std::wstring utf8_decode(const std::string &str) // Convert an UTF8 string to a wide Unicode String
	{
		if (str.empty()) return std::wstring();
		const int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), nullptr, 0);
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &wstrTo[0], size_needed);
		return wstrTo;
	}
	//---
}