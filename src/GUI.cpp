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

// STL headers
#include <string>
#include <unordered_map>
#include <vector>
#include <memory> // std::unique_ptr
#include <utility> // std::move
#include <sstream>
#include <type_traits> // std::remove_reference
#include <tuple> // for C++17 structured bindings
#include <algorithm> // std::find

// Windows headers
#define UNICODE
#include <windows.h>
#include <commctrl.h> // subclassing
#include <strsafe.h> // StringCchCopy

// Program headers
#include "misc.hpp"
#include "GUI.hpp"
#include "WProc.hpp"
#include "Profiles.hpp"
#include "AppData.hpp"
#include "Dashboard.hpp"
#include "LessonPage.hpp"
#include "StartPage.hpp"
#include "NewProfilePage.hpp"
#include "AboutPage.hpp"
#include "HomePage.hpp"

namespace ASP
{
	extern LessonPage* LP; // defined in WProc.cpp   look, it was either this or have a non-generalized function in this class, ok

	const std::vector<std::wstring> GUI::ReservedLessonFileNames{ L"SYNTAX.txt", L"CHECKER.dll" };

	GUI::GUI() noexcept : page(GUI::Pages::HOME) //constructor (default)
	{
		InitFonts();
	}

	GUI::GUI(GUI::Pages p) noexcept : page(p) //constructor (actual)
	{
		InitFonts();
	}

	GUI::GUI(GUI&& OtherGUI) noexcept //move ctor
	{
		page = OtherGUI.page;
		InitFonts();
	}

	GUI& GUI::operator=(GUI&& OtherGUI) noexcept //move assignment operator
	{
		page = OtherGUI.page;
		return *this;
	}

	void GUI::InitFonts() noexcept // initialize fonts and such.
	{
		// corbelTitle
		HFONT corbelTitlePre = CreateFont(22, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Corbel");
		HFONT_ptr corbelTitlePrePtr(corbelTitlePre, HFONT_deleter());
		corbelTitle = std::move(corbelTitlePrePtr);

		// corbelButton
		HFONT corbelButtonPre = CreateFont(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Corbel");
		HFONT_ptr corbelButtonPrePtr(corbelButtonPre, HFONT_deleter());
		corbelButton = std::move(corbelButtonPrePtr);

		// verdanaSmall
		HFONT verdanaSmallPre = CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Verdana");
		HFONT_ptr verdanaSmallPrePtr(verdanaSmallPre, HFONT_deleter());
		verdanaSmall = std::move(verdanaSmallPrePtr);

		// verdanaNormal
		HFONT verdanaNormalPre = CreateFont(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Verdana");
		HFONT_ptr verdanaNormalPrePtr(verdanaNormalPre, HFONT_deleter());
		verdanaNormal = std::move(verdanaNormalPrePtr);

		// monoCode
		HFONT monoCodePre = CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Courier New");
		HFONT_ptr monoCodePrePtr(monoCodePre, HFONT_deleter());
		monoCode = std::move(monoCodePrePtr);
	}

	void GUI::setPage(GUI::Pages Page)
	{
		page = Page;
	}

	GUI::Pages GUI::getPage() const noexcept
	{
		return page;
	}

	void GUI::ClearWindows(WindowData& data, bool clearTopLevel) const // remove all the above windows
	{
		for (auto const & child_pair : data.children)  // yay for C++11 ranged loops!  const to ensure read-only, and pair because of std::pair<> from std::unordered_list<>
		{
			ShowWindow(child_pair.second->handle, SW_HIDE);
			child_pair.second->visible = false;
		}
		if (clearTopLevel)
		{
			ShowWindow(data.handle, SW_HIDE);
			data.visible = false;
		}
	}

	void GUI::RestoreWindows(WindowData& data, bool restoreTopLevel) const // restore all the hidden windows
	{
		for (auto const & child_pair : data.children)
		{
			ShowWindow(child_pair.second->handle, SW_SHOWNORMAL);
			child_pair.second->visible = true;
		}
		if (restoreTopLevel)
		{
			ShowWindow(data.handle, SW_SHOWNORMAL);
			data.visible = true;
		}
	}

	void GUI::DestroyWindows(WindowData& data, bool destroyTopLevel) const // delete all windows
	{
		if (destroyTopLevel) DestroyWindow(data.handle);
		else
		{
			for (auto const & child_pair : data.children)
			{
				DestroyWindow(child_pair.second->handle);
			}
		}
		data.children.clear();
	}

	void GUI::GoBack(WindowData& data, std::vector<std::reference_wrapper<WindowData>>& history, bool destroy) const
	{
		if (destroy) this->DestroyWindows(data);
		else this->ClearWindows(data);
		this->RestoreWindows(history.back());
		history.pop_back();
	}

	INT_PTR GUI::WM_CTLColorStatic(const HDC hdc, const COLORREF TextColor, const COLORREF BkColor)
	{
		SetTextColor(hdc, TextColor);
		SetBkMode(hdc, TRANSPARENT);
		return reinterpret_cast<INT_PTR>(CreateSolidBrush(BkColor));
	}

	void GUI::WM_Paint(const HWND hwnd, const HDC hdc, const COLORREF TextColor, const COLORREF BkColor, const COLORREF BorderColor, const HFONT font, const double xMarginDivisor, const double yMarginDivisor, const unsigned int textLength, const unsigned int textAlignX, const unsigned int textAlignY, const bool multiLine, const bool calculateHeight, unsigned int* textTotalHeight)
	{
		wchar_t* WindowText = new wchar_t[textLength];
		GetWindowText(hwnd, WindowText, textLength);
		
		// redraw client rect
		SetBkColor(hdc, BkColor);
		RECT rect;
		GetClientRect(hwnd, &rect);
		HBRUSH hBrush = CreateSolidBrush(BkColor);
		HPEN x = CreatePen(BS_SOLID, 1, BorderColor);
		SelectObject(hdc, hBrush);
		SelectObject(hdc, x);
		RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 9, 9);
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
		SelectObject(hdc, GetStockObject(NULL_PEN));
		DeleteObject(hBrush);
		DeleteObject(x);
		
		// redraw text
		const int rWidth = rect.right - rect.left;
		const int rHeight = rect.bottom - rect.top;
		RECT nRect = { rect.left + static_cast<LONG>(rWidth / xMarginDivisor), rect.top + static_cast<LONG>(rHeight / yMarginDivisor), rect.right - static_cast<LONG>(rWidth / xMarginDivisor), rect.bottom - static_cast<LONG>(rHeight / yMarginDivisor) };
		LOGFONT lf;
		GetObject(font, sizeof(LOGFONT), &lf);
		HFONT hf = CreateFontIndirect(&lf);
		HFONT oldf = static_cast<HFONT>(SelectObject(hdc, hf));
		SetTextColor(hdc, TextColor);
		unsigned int multiLineFlag = multiLine ? DT_WORDBREAK : DT_SINGLELINE;
		if (calculateHeight) *textTotalHeight = DrawTextEx(hdc, WindowText, -1, &nRect, textAlignX | textAlignY | multiLineFlag | DT_CALCRECT, nullptr);;
		DrawTextEx(hdc, WindowText, -1, &nRect, textAlignX | textAlignY | multiLineFlag, nullptr);
		SelectObject(hdc, oldf);
		DeleteObject(hf);
		delete[] WindowText;
	}

	void GUI::WM_VScroll(const HWND hwnd, const WPARAM wParam, const unsigned int totalHeight)
	{
		SCROLLINFO si = {};
		// Get all the scroll bar information.
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;

		// Save the position for comparison later on.
		GetScrollInfo(hwnd, SB_VERT, &si);
		const int yPos = si.nPos;
		switch (LOWORD(wParam))
		{
			// User clicked the left arrow.
		case SB_LINEUP:
			si.nPos -= 1;
			break;

			// User clicked the right arrow.
		case SB_LINEDOWN:
			si.nPos += 1;
			break;

			// User clicked the scroll bar shaft left of the scroll box.
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;

			// User clicked the scroll bar shaft right of the scroll box.
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;

			// User dragged the scroll box.
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;

		default:
			break;
		}

		// Set the position and then retrieve it.  Due to adjustments
		// by Windows it may not be the same as the value set.
		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hwnd, SB_VERT, &si);

		// If the position has changed, scroll the window.
		if (si.nPos != yPos)
		{
			const double pxPerScrollPos = static_cast<double>(totalHeight) / si.nMax;
			const unsigned int flags = SW_ERASE | SW_INVALIDATE | SW_SCROLLCHILDREN;
			ScrollWindowEx(hwnd, 0, static_cast<int>((yPos - si.nPos) * pxPerScrollPos), nullptr, nullptr, nullptr, nullptr, flags);
			InvalidateRect(hwnd, nullptr, true);
		}
	}

	void GUI::WM_HScroll(const HWND hwnd, const WPARAM wParam, const unsigned int totalWidth)
	{
		SCROLLINFO si = {};
		// Get all the scroll bar information.
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;

		// Save the position for comparison later on.
		GetScrollInfo(hwnd, SB_HORZ, &si);
		const int xPos = si.nPos;
		switch (LOWORD(wParam))
		{
			// User clicked the left arrow.
		case SB_LINELEFT:
			si.nPos -= 1;
			break;

			// User clicked the right arrow.
		case SB_LINERIGHT:
			si.nPos += 1;
			break;

			// User clicked the scroll bar shaft left of the scroll box.
		case SB_PAGELEFT:
			si.nPos -= si.nPage;
			break;

			// User clicked the scroll bar shaft right of the scroll box.
		case SB_PAGERIGHT:
			si.nPos += si.nPage;
			break;

			// User dragged the scroll box.
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;

		default:
			break;
		}

		// Set the position and then retrieve it.  Due to adjustments
		// by Windows it may not be the same as the value set.
		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		GetScrollInfo(hwnd, SB_HORZ, &si);

		// If the position has changed, scroll the window.
		if (si.nPos != xPos)
		{

			double pxPerScrollPos2 = static_cast<double>(totalWidth) / si.nMax;
			if (si.nPos == si.nMax - si.nPage) pxPerScrollPos2 -= totalWidth - (static_cast<int>(pxPerScrollPos2) * si.nMax);
			ScrollWindowEx(hwnd, static_cast<int>((xPos - si.nPos) * pxPerScrollPos2), 0, nullptr, nullptr, nullptr, nullptr, SW_ERASE | SW_INVALIDATE | SW_SCROLLCHILDREN);
			InvalidateRect(hwnd, nullptr, true);
		}
	}

	void GUI::CreateTitle(WindowData *data, const std::wstring PageTitle) const
	{
		const int Nwidth = data->width;
		const int Nheight = (page == GUI::Pages::HOME) ? static_cast<int>(data->height * 0.3) : ((page == GUI::Pages::LESSON) ? static_cast<int>(data->height * 0.15) : static_cast<int>(data->height * 0.2));
		HFONT TitleFont = CreateFont(static_cast<int>(Nheight * 0.8), 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Corbel");
		const int x = (data->width / 2) - ((Nwidth) / 2);
		const int y = (page == GUI::Pages::HOME || page == GUI::Pages::LESSON) ? static_cast<int>(data->height * 0.04) : static_cast<int>(data->height * 0.08);
		HWND handle = CreateWindowEx(0, L"STATIC", PageTitle.c_str(), SS_CENTER | WS_VISIBLE | WS_CHILD, x, y, Nwidth, Nheight, data->handle, nullptr, nullptr, nullptr);
		SendMessage(handle, WM_SETFONT, reinterpret_cast<WPARAM>(TitleFont), TRUE);
		std::unique_ptr<WindowData> wd_ptr = std::make_unique<WindowData>(handle, ChildList(), data->color, Nheight, Nwidth, x, y, data, true);
		CheckEmplace(data->children.emplace(L"title", std::move(wd_ptr)), L"Title: " + PageTitle);
	}

	void GUI::CreateCopyright(WindowData *data) const
	{
		const wchar_t * copyright_text = L"(c) Copyright 2016-2019 Daryl Dohner - See license.txt for details.";
		const int Nwidth = static_cast<int>(data->width * 0.66);
		const int Nheight = 16;
		const int x = data->width / 6;
		const int y = 10 * (data->height / 11);
		HWND h = CreateWindowEx(0, L"STATIC", copyright_text, SS_CENTER | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD, x, y, Nwidth, Nheight, data->handle, nullptr, nullptr, nullptr);
		SendMessage(h, WM_SETFONT, reinterpret_cast<WPARAM>(&(*verdanaSmall)), TRUE);
		std::unique_ptr<WindowData> wd_ptr = std::make_unique<WindowData>(h, ChildList(), data->color, Nheight, Nwidth, x, y, data, true);
		CheckEmplace(data->children.emplace(L"copyright", std::move(wd_ptr)), L"copyright");
	}

	void GUI::CreateBackButton(WindowData *data, const Palette& ColorPalette, const std::wstring AltText) const
	{
		if (page != GUI::Pages::HOME)
		{
			const int Nwidth = static_cast<int>(data->width * 0.09956);
			const int Nheight = static_cast<int>(Nwidth * 0.33333);
			const int x = static_cast<int>(data->width*0.02489);
			const int y = x;
			std::wstring label = L"Back";
			if (!AltText.empty()) label = AltText;
			HWND h = CreateWindowEx(0, L"STATIC", label.c_str(), SS_CENTER | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD | SS_NOTIFY, x, y, Nwidth, Nheight, data->handle, nullptr, nullptr, nullptr);
			std::unique_ptr<WindowData> wd_ptr = std::make_unique<WindowData>(h, ChildList(), ColorPalette.BoxColor, Nheight, Nwidth, x, y, data, true);
			CheckEmplace(data->children.emplace(L"back_button", std::move(wd_ptr)), L"back_button");
			SetWindowSubclass(h, ButtonSubclass, 0, 0);	
		}
	}

	void GUI::LoadHomePage(WindowData* data, const Palette& ColorPalette) const
	{
		if (page == GUI::Pages::HOME)
		{
			CreateTitle(data, L"LearnCS++");
			HomePage::CreateAboutButton(data, page, ColorPalette);
			HomePage::CreateStartButton(data, page, ColorPalette);
			CreateCopyright(data);
		}
	}

	void GUI::LoadAboutPage(WindowData *data, const Palette& ColorPalette, bool *readyAbout, bool *readyAboutGroutfit, bool *readyAboutCopyright, bool *AboutCreated, bool *readyAboutTextColor) const
	{
		if (page == GUI::Pages::ABOUT)
		{
			CreateTitle(data, L"About");
			*readyAbout = true;
			InvalidateRect(data->children.at(L"title")->handle, nullptr, false);
			AboutPage::CreateAboutBody(data, page, ColorPalette);
			*readyAboutGroutfit = true;
			AboutPage::CreateAboutBodyText(data, page, &(*corbelButton), readyAboutTextColor);
			CreateBackButton(data, ColorPalette);
			CreateCopyright(data);
			*readyAboutCopyright = true;
			InvalidateRect(data->children.at(L"copyright")->handle, nullptr, false);
			*AboutCreated = true;
		}
	}

	void GUI::LoadStartPage(WindowData *data, const Palette& ColorPalette, bool *NewProfileAllowed, bool *readyStart, bool *readyStartBody, bool *readyStartCopyright, bool *StartCreated) const
	{
		if (page == GUI::Pages::START)
		{
			CreateTitle(data, L"Profiles");
			*readyStart = true;
			InvalidateRect(data->children.at(L"title")->handle, nullptr, false);
			StartPage::CreateStartBody(data, ColorPalette, page, NewProfileAllowed);
			*readyStartBody = true;
			StartPage::CreateNewProfileButton(data, ColorPalette, page);
			CreateBackButton(data, ColorPalette);
			CreateCopyright(data);
			*readyStartCopyright = true;
			InvalidateRect(data->children.at(L"copyright")->handle, nullptr, false);
			*StartCreated = true;
		}
	}

	void GUI::LoadNewProfilePage(WindowData *data, const Palette& ColorPalette, bool *readyNewProfile, bool *readyNewProfileBody, bool *readyNewProfileCopyright, bool *NewProfileCreated) const
	{
		if (page == GUI::Pages::NEW_PROFILE)
		{
			// Create Title
			CreateTitle(data, L"New Profile");
			*readyNewProfile = true;
			InvalidateRect(data->children.at(L"title")->handle, nullptr, false);

			// Create Body
			NewProfilePage::CreateNewProfileBody(data, page, ColorPalette.BoxColor, &(*verdanaNormal));
			*readyNewProfileBody = true;
			InvalidateRect(data->children.at(L"Label")->handle, nullptr, false);
			InvalidateRect(data->children.at(L"CreateButton")->handle, nullptr, false);

			// Back Button / Copyright
			CreateBackButton(data, ColorPalette);
			CreateCopyright(data);
			*readyNewProfileCopyright = true;
			InvalidateRect(data->children.at(L"copyright")->handle, nullptr, false);
			SetFocus(data->children.at(L"TextBox")->handle);
			*NewProfileCreated = true;
		}
	}

	bool GUI::CheckIfReservedLessonFilename(const std::wstring& filename) const
	{
		bool isReserved = std::find(this->ReservedLessonFileNames.begin(), this->ReservedLessonFileNames.end(), filename) != this->ReservedLessonFileNames.end();
		bool isDLL = false;
		if (filename.length() >= 4)
		{
			std::wstring ext = filename.substr(filename.length() - 4);
			isDLL = ext == L".dll" || ext == L".DLL";
		}
		return isReserved || isDLL;
	}

	void GUI::LoadLanguages(LangList* langs) const
	{
		if (langs == nullptr || langs->size() != 0) return;
		
		// get languages
		WIN32_FIND_DATA searchData = {};
		HANDLE hFind = INVALID_HANDLE_VALUE;
		hFind = FindFirstFile(L"Languages\\*", &searchData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			Error(L"Loading Languages (D1)");
			return;
		}

		do // languages
		{
			Language NewLang = {};
			const std::wstring dirName = searchData.cFileName;
			if ((dirName == L".") || (dirName == L"..")) continue;
			const size_t pos = dirName.find_first_of(L"_");
			NewLang.langID = dirName.substr(0, pos);
			NewLang.name = dirName.substr(pos + 1);

			// Get the lesson groups
			if (!this->LoadLanguagesLessonGroups(dirName, NewLang)) continue;
			
			// Add Language to LangList
			std::unique_ptr<Language> NewLangPtr = std::make_unique<Language>(NewLang.name, NewLang.langID, NewLang.lessonGroups);
			CheckEmplace(langs->emplace(NewLang.langID, std::move(NewLangPtr)), L"LoadLanguages: Lang");
		} while (FindNextFile(hFind, &searchData) != 0);
		FindClose(hFind);
	}

	bool GUI::LoadLanguagesLessonGroups(const std::wstring& dirName, Language& NewLang) const
	{
		std::wstringstream LangDirStream;
		LangDirStream << L"Languages\\" << dirName << L"\\*";
		const std::wstring LangDirName = LangDirStream.str();
		WIN32_FIND_DATA searchDataLessonGroups = {};
		HANDLE hFindLessonGroups = INVALID_HANDLE_VALUE;
		hFindLessonGroups = FindFirstFile(LangDirName.c_str(), &searchDataLessonGroups);
		if (hFindLessonGroups == INVALID_HANDLE_VALUE)
		{
			Error(L"Loading Language Groups (D1.33)");
			return false;
		}
		do // lesson groups
		{
			LessonGroup NewLessonGroup = {};
			const std::wstring lg_filename = searchDataLessonGroups.cFileName;
			const size_t spaceFoundPos = lg_filename.find(L' ');
			if ((lg_filename == L".") || (lg_filename == L"..") || (lg_filename.length() <= 4) || (spaceFoundPos == std::wstring::npos) || this->CheckIfReservedLessonFilename(lg_filename)) continue; // 4 is the length of ###_
			const std::wstring lgID = lg_filename.substr(0, spaceFoundPos);
			NewLessonGroup.name = lg_filename.substr(spaceFoundPos + 1);
			const std::wstring LessonGroupDirName = L"Languages\\" + dirName + L"\\" + lg_filename + L"\\*";

			// Get the lessons
			if(!this->LoadLanguagesLessons(LessonGroupDirName, dirName, lg_filename, NewLessonGroup)) continue;

			// add lesson group to NewLand.lessonGroups
			CheckEmplace(NewLang.lessonGroups.emplace(lgID, NewLessonGroup), L"LoadLanguages: LG");
		} while (FindNextFile(hFindLessonGroups, &searchDataLessonGroups) != 0);
		FindClose(hFindLessonGroups);
		return true;
	}

	bool GUI::LoadLanguagesLessons(const std::wstring& LessonGroupDirName, const std::wstring& dirName, const std::wstring& lg_filename, LessonGroup& NewLessonGroup) const
	{
		WIN32_FIND_DATA searchDataLessons = {};
		HANDLE hFindLessons = INVALID_HANDLE_VALUE;
		hFindLessons = FindFirstFile(LessonGroupDirName.c_str(), &searchDataLessons);
		if (hFindLessons == INVALID_HANDLE_VALUE)
		{
			Error(L"Loading Lessons (D1.67)");
			return false;
		}
		do // lessons
		{
			const std::wstring filename = searchDataLessons.cFileName;
			const size_t underscoreFoundPos = filename.find(L'_');
			if ((filename == L".") || (filename == L"..") || (filename.length() <= 4) || (underscoreFoundPos == std::wstring::npos) || this->CheckIfReservedLessonFilename(filename)) continue; // 4 is the length of _###
			const std::wstring filePath = L"Languages\\" + dirName + L"\\" + lg_filename + L"\\" + filename;;
			const std::wstring lessonID = filename.substr(underscoreFoundPos + 1, 3);
			HANDLE hLessonFile = CreateFile(filePath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (hLessonFile == INVALID_HANDLE_VALUE)
			{
				Error(L"Load Lesson File (D2)");
				continue;
			}
			DWORD BytesRead = 0;
			DWORD FileSize = searchDataLessons.nFileSizeLow;
			wchar_t * FileBuffer = new wchar_t[FileSize / sizeof(wchar_t) + 1];
			SecureZeroMemory(FileBuffer, FileSize + 1);
			if (!ReadFile(hLessonFile, FileBuffer, FileSize, &BytesRead, nullptr))
			{
				Error(L"Read Lesson File (D3)");
				delete[] FileBuffer;
				continue;
			}
			const std::wstring FileText = std::wstring(FileBuffer);
			const size_t startPos = FileText.find(L"TITLE ") + 6;
			const size_t endPos = FileText.find(L"\r\n", startPos);
			const size_t strWidth = (endPos == std::wstring::npos) ? endPos : (endPos - startPos);
			std::wstring lessonName = FileText.substr(startPos, strWidth);
			CheckEmplace(NewLessonGroup.lessons.emplace(lessonID, lessonName), L"LoadLanguages: Lesson");
			delete[] FileBuffer;
			CloseHandle(hLessonFile);
		} while (FindNextFile(hFindLessons, &searchDataLessons) != 0);
		FindClose(hFindLessons);
		return true;
	}

	void GUI::LoadDashboardPage(WindowData* data, const Palette& ColorPalette, LangList* LanguageMap, const unsigned int DashboardNumDrops, bool *readyDashboard, bool *readyDashboardBody, bool *readyDashboardCopyright, bool *DashboardCreated) const
	{
		if (page == Pages::DASHBOARD)
		{
			// Step -1: Load In Language Modules
			if (LanguageMap == nullptr) return;
			GUI::LoadLanguages(LanguageMap);
			DebugLangList(*LanguageMap);

			// Step -0.5: Title
			CreateTitle(data, L"Dashboard");
			*readyDashboard = true;
			InvalidateRect(data->children.at(L"title")->handle, nullptr, false);

			// Steps 0 - 2: Body
			Dashboard::createDashboardBody(data, page, LanguageMap, &(*this->corbelButton), ColorPalette, DashboardNumDrops);
			*readyDashboardBody = true;

			// Step 3: Back Button & Copyright
			CreateBackButton(data, ColorPalette);
			CreateCopyright(data);
			*readyDashboardCopyright = true;
			InvalidateRect(data->children.at(L"back_button")->handle, nullptr, false);
			InvalidateRect(data->children.at(L"copyright")->handle, nullptr, false);
			*DashboardCreated = true;
		}
	}

	void GUI::LoadLessonPage(WindowData* data, LangList* langs, const Palette& ColorPalette, const std::wstring& curLangID, const std::wstring& curLangName, const std::wstring& curLGID, const std::wstring& curLGName, const std::wstring& curLessonID, const std::wstring& curLessonName, bool* readyLessonPage, bool* readyLessonPageBody, bool* readyLessonPageCopyright, bool* LessonPageCreated) const
	{
		if (page == Pages::LESSON)
		{
			LP = new LessonPage(langs, curLangID, curLangName, curLGID, curLGName, curLessonID, curLessonName);
			const std::wstring titleStr = LP->getTitleStr();
			CreateTitle(data, titleStr);
			*readyLessonPage = true;
			InvalidateRect(data->children.at(L"title")->handle, nullptr, false);
			CreateCopyright(data);
			*readyLessonPageCopyright = true;
			InvalidateRect(data->children.at(L"copyright")->handle, nullptr, false);
			CreateBackButton(data, ColorPalette, L"Dashboard");
			LP->loadLesson();
			LP->createLessonPageBody(*data, ColorPalette, &(*this->monoCode));
			*readyLessonPageBody = true;
			*LessonPageCreated = true;
		}
	}
}