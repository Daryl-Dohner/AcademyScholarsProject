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

// STL Headers
#include <tuple>

// Windows Headers
#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <Richedit.h>
#pragma comment(lib, "comctl32.lib")
#include <windowsx.h> // Edit_ functions

// Program Headers
#include "SCEdit.hpp"
#include "WProc.hpp"

namespace ASP
{
	SCEdit::SCEdit(WindowData& wData, const std::wstring& lang, const std::wstring& _langID, const std::wstring& defText, Metric x, Metric y, Metric width, Metric height, const HWND parent, const COLORREF _bkColor, const COLORREF textColor, const HFONT _font, const bool _readOnly) noexcept : language(lang), langID(_langID), defaultText(defText), bkColor(_bkColor), defaultTextColor(textColor), font(_font), readOnly(_readOnly)
	{
		this->hRichEditLibrary = LoadLibrary(L"Msftedit.dll");
		this->handle = CreateWindowEx(0, MSFTEDIT_CLASS, defText.c_str(), ES_LEFT | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL | ES_AUTOVSCROLL | WS_HSCROLL | ES_AUTOHSCROLL | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP, x, y, width, height, parent, nullptr, nullptr, nullptr);
		std::unique_ptr<WindowData> wd_ptr_SCBox = std::make_unique<WindowData>(this->handle, ChildList(), _bkColor, height, width, x, y, &wData, true);
		CheckEmplace(wData.children.emplace(L"SCBOX", std::move(wd_ptr_SCBox)), L"SCBOX");
		SetWindowSubclass(this->handle, SCEditSubclass, 0, 0);
		SetWindowLongPtr(this->handle, GWLP_USERDATA, (LONG_PTR)this);
		this->applyDefaultStyles();
	}

	void SCEdit::applyDefaultStyles()
	{
		SendMessage(this->handle, EM_SETBKGNDCOLOR, 0, this->bkColor);
		this->updateSyntaxHighlighting();
		if(this->readOnly) Edit_SetReadOnly(this->handle, true);
	}

	HWND SCEdit::getHandle() const noexcept
	{
		return this->handle;
	}

	void SCEdit::Uninit() noexcept
	{
		this->handle = nullptr;
		FreeLibrary(this->hRichEditLibrary);
		this->hRichEditLibrary = nullptr;
	}

	void SCEdit::updateSyntaxHighlighting(const int lineNo)
	{
		std::wstring textToProcess = L"";
		int startPos = 0;
		if (lineNo < 0) // process the default text, and set the default styles
		{
			textToProcess = defaultText; // process the default text

			LOGFONT lf = {};
			GetObject(font, sizeof(LOGFONT), &lf);
			CHARFORMAT format = {};
			format.cbSize = sizeof(CHARFORMAT);
			format.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE;
			format.crTextColor = this->defaultTextColor;
			wcscpy_s(format.szFaceName, LF_FACESIZE, lf.lfFaceName);
			format.yHeight = ((lf.lfHeight * 72) / GetDeviceCaps(GetDC(nullptr), LOGPIXELSY)) * 20; // THIS FORMULA IS RIDICULOUS, @M$: convert logical units to points by multiplying by 72 then dividing by GetDeviceCaps(), convert points to twips by multiplying by 20
			SendMessage(this->handle, EM_SETCHARFORMAT, SCF_DEFAULT, reinterpret_cast<LPARAM>(&format));
		}
		else // retrieve and update the specified line
		{
			startPos = Edit_LineIndex(this->handle, lineNo);
			const int bufSize = Edit_LineLength(this->handle, startPos); // Documentation Error!  second param is the index of any character on the line, not the line number
			wchar_t* buffer = new wchar_t[bufSize + 1]; // magic number
			SecureZeroMemory(buffer, bufSize + 1);
			const int copied = Edit_GetLine(this->handle, lineNo, reinterpret_cast<LPARAM>(buffer), bufSize); // Documentation error!  last param is number of CHARs, not buffer size
			textToProcess = std::wstring(buffer);
			std::cout << std::endl << '\t' << bufSize << '\t' << copied;
			delete[] buffer;
		}
		SyntaxHighlighter SH(this->language, this->langID, textToProcess);
		std::vector<SyntaxHighlighter::Instruction> instructions = SH.getInstructions();
		size_t newLineCorrection = 0;
		for (auto instr : instructions)
		{
			if (instr == SH.newLineCommand) // no need to process new line commands because we aren't writing the text
			{
				newLineCorrection += 1; // richedit gets confused and adds an extra character for newlines, so let's fix things
				continue; 
			}
			const size_t selStartPos = startPos + std::get<0>(instr) - newLineCorrection;
			const size_t selEndPos = startPos + std::get<1>(instr) - newLineCorrection;
			Edit_SetSel(this->handle, selStartPos, selEndPos);
			CHARFORMAT format = {};
			format.cbSize = sizeof(CHARFORMAT);
			format.dwMask = CFM_COLOR;
			format.crTextColor = std::get<2>(instr);
			SendMessage(this->handle, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&format));
		}
		Edit_SetSel(this->handle, -1, 0); // deselect
	}

	void SCEdit::SHUpdate()
	{
		DWORD hi = 0, lo = 0;
		SendMessage(this->handle, EM_GETSEL, (WPARAM)&hi, (LPARAM)&lo);
		const int lineNo = Edit_LineFromChar(this->handle, -1); // retrieves the current line number; Documentation Error!  Last param must be -1, not 1
		this->updateSyntaxHighlighting(lineNo - 1);  // since enter brings us to a new line, update the last line

		// now return to default
		const int currentCharIndex = Edit_LineIndex(this->handle, lineNo);
		std::cout << std::endl << lineNo << std::endl << currentCharIndex << std::endl << hi << " " << lo;
		Edit_SetSel(this->handle, currentCharIndex-1, currentCharIndex);
		CHARFORMAT format = {};
		format.cbSize = sizeof(CHARFORMAT);
		format.dwMask = CFM_COLOR;
		format.crTextColor = this->defaultTextColor;
		SendMessage(this->handle, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&format));
		
		Edit_SetSel(this->handle, -1, 0); // deselect
	}
}