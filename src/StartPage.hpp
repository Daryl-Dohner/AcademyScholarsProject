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

#ifndef STARTPAGE_HPP
#define STARTPAGE_HPP

// Windows Headers
#define UNICODE
#include <windows.h>

// Program Headers
#include "misc.hpp"
#include "GUI.hpp"
#include "Profiles.hpp"

namespace ASP
{
	class StartPage
	{
		public:
			StartPage() noexcept = default;
			static bool TableVectorSorter(HWND, HWND) noexcept; // for sorting a table like start body

			// gui functions
			static void CreateStartBody(WindowData*, const Palette&, const GUI::Pages, bool*); // Start page body
			static void CreateNewProfileButton(WindowData*, const Palette&, GUI::Pages); // Start page Create New Profile button
			static void UpdateStartBody(WindowData*, GUI::Pages, bool*); // Update Start page body
			static void UpdateSelectedProfileButtons(WindowData*, const Palette&, std::wstring&); // add the buttons to the selected profile row
			
			// Blue Buttons message procedures
			static void DeleteButtonWM_LButtonUp(WindowData*, const Palette&, const HWND, const HWND, const GUI::Pages, bool*); // Delete Button WM_LBUTTONUP procedure
			static void GoButtonWM_LButtonUp(const HWND, const HWND); // Go Button WM_LBUTTONUP procedure

			// Rows message procedures
			static void RowsWM_Paint(Profiles*, const Palette&, const HWND, const HDC, const COLORREF, COLORREF, COLORREF, const HFONT, const double = 3, const double = 3, const unsigned int = 64); // Rows WM_PAINT procedure
			static void RowsWM_LButtonUp(WindowData*, Profiles*, const Palette&, const HWND); // Rows WM_LBUTTONUP procedure
	};
}
#endif