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

#ifndef SCEDIT_HPP
#define SCEDIT_HPP

// STL Headers
#include <string>

// Windows Headers
#define UNICODE
#include <windows.h>

// Program heaers
#include "misc.hpp"
#include "SyntaxHighlighter.hpp"

namespace ASP
{
	class SCEdit
	{
		private:
			bool readOnly = false;
			HWND handle = nullptr;
			HMODULE hRichEditLibrary = nullptr;
			std::wstring language = L"";
			std::wstring langID = L"";
			std::wstring defaultText = L"";
			COLORREF bkColor = 0;
			COLORREF defaultTextColor = 0;
			HFONT font = nullptr;
			void applyDefaultStyles(void);
			void updateSyntaxHighlighting(const int lineNo = -1);
		public:
			SCEdit() noexcept = default;
			SCEdit(WindowData& wData, const std::wstring& lang, const std::wstring& _langID, const std::wstring& defText, Metric x, Metric y, Metric width, Metric height, const HWND parent, const COLORREF bkColor, const COLORREF textColor, const HFONT font, const bool _readOnly = false) noexcept;
			HWND getHandle(void) const noexcept;
			//HMODULE getLibHandle(void) const noexcept;
			void Uninit(void) noexcept;
			void SHUpdate(void);
			
	};
}

#endif