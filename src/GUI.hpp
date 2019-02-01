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

#ifndef GUI_HPP
#define GUI_HPP

//STL headers
#include <string>
#include <vector>
#include <memory> // std::unique_ptr
#include <utility> // std::move
#include <type_traits> // std::remove_pointer
#include <functional> // std::reference_wrapper

//Windows headers
#define UNICODE
#include <windows.h>
#include <commctrl.h> //subclassing

//Program headers
#include "misc.hpp"

namespace ASP
{
	class GUI // Static GUI class aka for direct rendering of hardcoded GUI elements
	{
		public:
			struct HFONT_deleter
			{
				void operator()(HFONT h) noexcept
				{
					DeleteObject(h);
				}
			};

			using HFONT_ptr = std::unique_ptr<std::remove_pointer<HFONT>::type, HFONT_deleter>; // because that whole HFONT unique_ptr kerfuffle

			enum class Pages
			{
				NONE,
				HOME,
				ABOUT,
				START,
				NEW_PROFILE,
				DASHBOARD,
				LESSON
			};

		private:
			void InitFonts(void) noexcept; // Initialize Fonts and Such
			Pages page = Pages::NONE; // page - which page/layout is selected
			bool GUI::LoadLanguagesLessonGroups(const std::wstring& dirName, Language& NewLang) const;
			bool GUI::LoadLanguagesLessons(const std::wstring& LessonGroupDirName, const std::wstring& dirName, const std::wstring& lg_filename, LessonGroup& NewLessonGroup) const; // LoadLanguages() Helper Function
			bool CheckIfReservedLessonFilename(const std::wstring& filename) const;

		public:
			// Object functions
			GUI() noexcept; // default constructor
			explicit GUI(GUI::Pages) noexcept; // actual constructor
			GUI(GUI&&) noexcept; // move constructor
			GUI(const GUI&) = delete; // copy ctor - copying forbidden
			GUI& operator=(GUI&&) noexcept; // move assignment operator
			GUI& operator=(const GUI&) = delete; // copy assignment operator - copy forbidden
			~GUI() noexcept = default; // destructor

			// Meta functions
			void setPage(GUI::Pages); // change/set the page
			GUI::Pages getPage(void) const noexcept; // returns the currently selected page
			void ClearWindows(WindowData&, bool = false) const; // hide all windows
			void RestoreWindows(WindowData&, bool = false) const; // restore all hidden windows
			void DestroyWindows(WindowData&, bool = false) const; // delete all windows
			void GoBack(WindowData& data, std::vector<std::reference_wrapper<WindowData>>& history, bool destroy = false) const; // when you press the back button, go back
			static const std::vector<std::wstring> ReservedLessonFileNames;
			void LoadLanguages(LangList*) const;

			// Common GUI Functions
			void CreateTitle(WindowData*, const std::wstring = L"<no title provided>") const; // Make the home/about/start/new_profile page title
			void CreateCopyright(WindowData*) const; // make the copyright statement at the bottonm of all pages
			void CreateBackButton(WindowData*, const Palette&, const std::wstring = L"") const; // back button for all pages

			// Common Window Procedures
			static INT_PTR WM_CTLColorStatic(const HDC, const COLORREF, const COLORREF); // standard WM_CTLCOLORSTATIC procedure
			static void WM_Paint(const HWND hwnd, const HDC hdc, const COLORREF TextColor, const COLORREF BkColor, const COLORREF BorderColor, const HFONT font, const double xMarginDivisor = 3.0, const double yMarginDivisor = 3.0, const unsigned int textLength = 64, const unsigned int textAlignX = DT_CENTER, const unsigned int textAlignY = DT_VCENTER, const bool multiLine = false, const bool calculateHeight = false, unsigned int* textTotalHeight = nullptr); // standard WM_PAINT procedure
			static void WM_VScroll(const HWND, const WPARAM, const unsigned int);
			static void WM_HScroll(const HWND, const WPARAM, const unsigned int);

			// Home Page
			void LoadHomePage(WindowData*, const Palette&) const;

			// About Page
			void LoadAboutPage(WindowData*, const Palette&, bool*, bool*, bool*, bool*, bool*) const;

			// Start Page
			void LoadStartPage(WindowData*, const Palette&, bool*, bool*, bool*, bool*, bool*) const;

			// New Profiles Page
			void LoadNewProfilePage(WindowData*, const Palette&, bool*, bool*, bool*, bool*) const;

			// Dashboard Page
			void LoadDashboardPage(WindowData*, const Palette&, LangList*, const unsigned int, bool*, bool*, bool*, bool*) const;

			// Lesson Page
			void GUI::LoadLessonPage(WindowData* data, LangList* langs, const Palette& ColorPalette, const std::wstring& curLangID, const std::wstring& curLangName, const std::wstring& curLGID, const std::wstring& curLGName, const std::wstring& curLessonID, const std::wstring& curLessonName, bool* readyLessonPage, bool* readyLessonPageBody, bool* readyLessonPageCopyright, bool* LessonPageCreated) const; // Lesson Page Body

			// Fonts!
			HFONT_ptr corbelTitle; // Title font
			HFONT_ptr corbelButton; // Button text font
			HFONT_ptr verdanaSmall; // copyright text font
			HFONT_ptr verdanaNormal; // regular text font
			HFONT_ptr monoCode; // code editor font (monospace)
	};
}

#endif
