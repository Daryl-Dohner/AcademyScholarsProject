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

#ifndef NEWPROFILEPAGE_HPP
#define NEWPROFILEPAGE_HPP

#include "misc.hpp"
#include "GUI.hpp"

namespace ASP
{
	class NewProfilePage
	{
		private:
			static void CreateLabel(WindowData*, const HFONT, const COLORREF, const int, const int, const int, const int);
			static void CreateTextBox(WindowData*, const HFONT, const COLORREF, const int, int*, const int, const int);
			static void CreateButton(WindowData*, const HFONT, const COLORREF, const int, const int, const int, const int);
		public:
			NewProfilePage() noexcept = default;
			static void CreateNewProfileBody(WindowData*, const GUI::Pages, const COLORREF, const HFONT);
			static INT_PTR WM_CTLColorEdit(const Palette, HWND, HDC, HFONT, const bool&, bool*, bool = false);
			static INT_PTR WM_CTLColorStatic(const Palette, HWND, HDC, HFONT, const bool&, bool*);
			static bool TextBoxWM_KeyDown(WindowData*, const HWND&, const UINT&, const WPARAM&, const LPARAM&);
	};
}

#endif