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

#ifndef DASHBOARD_HPP
#define DASHBOARD_HPP

// STL headers
#include <memory> // std::unique_ptr
#include <string>

// project headers
#include "misc.hpp"
#include "GUI.hpp"

// windows headers
#define UNICODE
#include <windows.h>

namespace ASP
{
	class Dashboard
	{
		private:
			static void createDropDownHeader(WindowData*, const std::wstring&, const std::wstring&, const COLORREF, const unsigned int, const unsigned int, const unsigned int, const unsigned int) noexcept;
			static void createDropDownArrow(WindowData*, const std::wstring&, const unsigned int, const unsigned int) noexcept;
			static void createDropDownMenu(WindowData*, const std::wstring&, const COLORREF, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const size_t) noexcept;
			static void fillDropDownMenu(WindowData*, const LangList&, const std::wstring&, HFONT, const unsigned int, const unsigned int) noexcept;
			static void createLessonGroupMenuItem(WindowData*, const std::wstring&, const std::wstring&, const std::wstring&, HFONT, const unsigned int, const unsigned int, const unsigned int, const unsigned int) noexcept;
			static void createLessonGroupMenuItemArrow(WindowData*, const std::wstring&, const std::wstring&, const unsigned int, const unsigned int, const unsigned int) noexcept;
			static void bumpBelowMenuItems(WindowData*, const std::wstring&, const int, const int);
			static void createContainerWindow(WindowData*, const COLORREF, const size_t, const unsigned int, const unsigned int, const unsigned int, const unsigned int) noexcept;
			static void createDropDowns(WindowData*, const LangList&, const Palette&, HFONT, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int) noexcept;
		public:
			Dashboard() noexcept = default;
			static void createDashboardBody(WindowData*, const GUI::Pages, LangList*, HFONT, const Palette&, const unsigned int);
			static void insertLessonMenuItems(WindowData*, const LangList&, const std::wstring&, const std::wstring&, HFONT) noexcept;
			static void removeLessonMenuItems(WindowData*, const std::wstring&, const std::wstring&);
			static void WM_Paint(const HWND, const HDC, const COLORREF, const COLORREF, HFONT);
			static void MenuWM_VScroll(WindowData*, const HWND, const WPARAM);
			static void ContainerWM_HScroll(WindowData*, const size_t, const HWND, const WPARAM);
			static void HeaderArrowWM_Paint(WindowData*, const std::wstring&, const HDC, const unsigned int, const unsigned int);
			static void HeaderArrowWM_LButtonUp(WindowData&, GUI&, const ChildPair&, const ChildPair&, const HWND, const COLORREF);
			static void LessonGroupArrowWM_Paint(const ChildPair&, const ChildPair&, const HDC);
			static void LessonGroupWM_LButtonUp(WindowData&, const LangList&, const ChildPair&, const ChildPair&, const HWND, const HFONT);
	};
}

#endif