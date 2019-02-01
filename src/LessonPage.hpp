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

#ifndef LESSONPAGE_HPP
#define LESSONPAGE_HPP

// STL Headers
#include <string>
#include <memory> // std::unique_ptr
#include <thread>

// Program Headers
#include "misc.hpp"
#include "LessonParser.hpp"
#include "SyntaxHighlighter.hpp"
#include "SCEdit.hpp"
#include "GUI.hpp"

// Windows Headers
#define UNICODE
#include <windows.h>

namespace ASP
{
	class LessonPage
	{
		private:
			// member vars
			LangList* langs = nullptr;
			std::wstring curLangID;
			std::wstring curLangName;
			std::wstring curLGID;
			std::wstring curLGName;
			std::wstring curLessonID;
			std::wstring curLessonName;
			LessonParser::LessonData lessonData;
			std::unique_ptr<SyntaxHighlighter> syntaxHighlighter;
			std::unique_ptr<SCEdit> SCEditBox;
			std::thread* syntaxHighlighterThread;
			const COLORREF defTextColor = RGB(0xFF, 0xFF, 0xFF);
			unsigned int LBoxContentHeight = 0;
			unsigned int BigBoxContentHeight = 0;
			unsigned int SCBoxContentHeight = 0;
			// member funcs
			void createLBox(WindowData& wData, const COLORREF BkColor, Metric bodyX, Metric bodyY, Metric bodyWidth, Metric bodyHeight, Metric spacerX, Metric columnWidth) const;
			void createSCBox(WindowData& wData, const COLORREF BkColor, const HFONT font, Metric bodyX, Metric bodyY, Metric bodyWidth, Metric bodyHeight, Metric spacerX, Metric columnWidth);
			void createBigBox(WindowData& wData, const COLORREF BkColor, Metric bodyX, Metric bodyY, Metric bodyWidth, Metric bodyHeight) const;
			void createCCButton(WindowData& wData, const COLORREF BkColor, Metric bodyX, Metric bodyY, Metric bodyWidth, Metric bodyHeight, Metric spacerX, Metric columnWidth) const;
			std::wstring CheckCode(WindowData& data) const;
			void CCButtonMessage(WindowData& data, const GUI& gui, const Palette& ColorPalette, const std::wstring& message, const bool correct) const;
		public:
			static const unsigned int MaxElemTextLength = 16384; // seems like more than enough chars.
			LessonPage(void) noexcept : LessonPage(nullptr, L"", L"", L"", L"", L"", L"") {};
			LessonPage(LangList* _langs, const std::wstring& curLangID_, const std::wstring& curLangName_, const std::wstring& curLGID_, const std::wstring& curLGName_, const std::wstring& curLessonID_, const std::wstring& curLessonName_) noexcept : langs(_langs), curLangID(curLangID_), curLangName(curLangName_), curLGID(curLGID_), curLGName(curLGName_), curLessonID(curLessonID_), curLessonName(curLessonName_), lessonData{} {};
			LessonPage(const LessonPage&) = delete;
			LessonPage& operator=(const LessonPage&) = delete;
			~LessonPage();
			std::wstring getTitleStr(void) const noexcept;
			bool getSCBoxReadOnly(void) const noexcept;
			void SCEditSHUpdate();
			void loadLesson(void);
			void createLessonPageBody(WindowData& wData, const Palette& ColorPalette, const HFONT font);
			void LBoxWM_Paint(const WindowData& data, const HWND hwnd, const HDC hdc, const COLORREF TextColor, const COLORREF BkColor, const COLORREF BorderColor, const HFONT font, const double xMarginDivisor = 3.0, const double yMarginDivisor = 3.0, const unsigned int textLength = 64, const unsigned int textAlignX = DT_CENTER, const unsigned int textAlignY = DT_VCENTER, const bool multiLine = false);
			void LBoxWM_VScroll(const HWND hwnd, const WPARAM wParam) const;
			void BigBoxWM_Paint(const WindowData& data, const HWND hwnd, const HDC hdc, const COLORREF TextColor, const COLORREF BkColor, const COLORREF BorderColor, const HFONT font, const double xMarginDivisor = 3.0, const double yMarginDivisor = 3.0, const unsigned int textLength = 64, const unsigned int textAlignX = DT_CENTER, const unsigned int textAlignY = DT_VCENTER, const bool multiLine = false);
			void BigBoxWM_VScroll(const HWND hwnd, const WPARAM wParam) const;
			void StaticSCBoxWM_Paint(const HWND hwnd, const HDC hdc, const COLORREF BkColor, const COLORREF BorderColor, const HFONT font);
			void StaticSCBoxWM_VScroll(const HWND hwnd, const WPARAM wParam) const;
			void CCButtonWM_LButtonUp(WindowData& data, const GUI& gui, const Palette& ColorPalette) const;
			//static void WM_Paint(const HWND hwnd, const HDC hdc, const COLORREF TextColor, const COLORREF BkColor, HFONT font);
			void setSyntaxHighlighter(void);
			void Uninit(void);
	};
}

#endif