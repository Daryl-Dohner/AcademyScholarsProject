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
#include <sstream>
#include <memory> // std::unique_ptr

// project headers
#include "Dashboard.hpp"
#include "misc.hpp"
#include "ImageLoader.hpp"
#include "GUI.hpp"

// windows headers
#define UNICODE
#include <windows.h>

namespace ASP
{
	void Dashboard::createContainerWindow(WindowData* data, const COLORREF BkColor, const size_t countLangs, const unsigned int DashboardNumDrops, const unsigned int HeaderHeight, const unsigned int MenuHeight, const unsigned int maxViewWidth) noexcept
	{
		// metrics
		const int ContHeight = HeaderHeight + MenuHeight;
		const int ContX = (data->width - maxViewWidth) / 2;
		const int baseY = static_cast<int>(data->height * 0.4);
		const int bkBoxY = baseY - HeaderHeight;

		HWND hCont = CreateWindowEx(0, L"Container", L"", WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_CLIPCHILDREN, ContX, bkBoxY, maxViewWidth, ContHeight, data->handle, nullptr, nullptr, nullptr);
		std::unique_ptr<WindowData> wd_ptr_cont = std::make_unique<WindowData>(hCont, ChildList(), BkColor, ContHeight, maxViewWidth, ContX, bkBoxY, data, true);
		CheckEmplace(data->children.emplace(L"container", std::move(wd_ptr_cont)), L"dashboard container");

		// scrollbar
		SCROLLINFO contScroll = {};
		contScroll.cbSize = sizeof(SCROLLINFO);
		contScroll.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
		contScroll.nMin = 0; //the range start.  It's zero-based, so start at 0
		contScroll.nMax = static_cast<int>((countLangs * 10)); //the last position in the range.  
		contScroll.nPos = 0;
		contScroll.nPage = (DashboardNumDrops * 10);
		SetScrollInfo(hCont, SB_HORZ, &contScroll, false);
	}

	void Dashboard::createDropDowns(WindowData* data, const LangList& LanguageMap, const Palette& ColorPalette, HFONT itemFont, const unsigned int singleWidth, const unsigned int spacer, const unsigned int HeaderHeight, const unsigned int MenuHeight, const unsigned int itemHeight) noexcept
	{
		// Metrics
		const int DropDownY = 0;

		int counter = 0;
		for (auto const & lang : LanguageMap)
		{
			// move over the X position for each successive drop down
			unsigned int DropDownX = (counter * singleWidth);
			if (counter != 0) DropDownX += counter * spacer;

			// Step 2.1: Header
			Dashboard::createDropDownHeader(data, lang.first, lang.second->name, ColorPalette.BoxColor, DropDownX, DropDownY, singleWidth, HeaderHeight);

			// Step 2.2: drop down 
			Dashboard::createDropDownArrow(data, lang.first, singleWidth, HeaderHeight);

			// Step 2.3: drop down menu
			Dashboard::createDropDownMenu(data, lang.first, ColorPalette.LightGray, singleWidth, HeaderHeight, MenuHeight, itemHeight, lang.second->lessonGroups.size());

			// Step 2.4: propogate with lesson groups
			Dashboard::fillDropDownMenu(data, LanguageMap, lang.first, itemFont, itemHeight, singleWidth - GetSystemMetrics(SM_CXVSCROLL));

			counter++;
		}
	}

	void Dashboard::createDropDownHeader(WindowData* data, const std::wstring& lang_key, const std::wstring& lang_name, const COLORREF BkColor, const unsigned int DropDownX, const unsigned int DropDownY, const unsigned int singleWidth, const unsigned int HeaderHeight) noexcept
	{
		HWND hHeader = CreateWindowEx(0, L"Groutfit", lang_name.c_str(), WS_VISIBLE | WS_CHILD, DropDownX, DropDownY, singleWidth, HeaderHeight, data->children.at(L"container")->handle, nullptr, nullptr, nullptr);
		std::unique_ptr<WindowData> wd_ptr_header = std::make_unique<WindowData>(hHeader, ChildList(), BkColor, HeaderHeight, singleWidth, DropDownX, DropDownY, &(*data->children.at(L"container")), true);
		std::wstring key = L"header_" + lang_key;
		CheckEmplace(data->children.at(L"container")->children.emplace(key, std::move(wd_ptr_header)), L"dashboard menu headers");
	}

	void Dashboard::createDropDownArrow(WindowData* data, const std::wstring& curDropDownLang, const unsigned int singleWidth, const unsigned int HeaderHeight) noexcept
	{
		const unsigned int ArrowHeight = static_cast<int>(HeaderHeight * 0.5);
		const unsigned int ArrowWidth = ArrowHeight;
		const unsigned int ArrowX = static_cast<int>(singleWidth * 0.95) - ArrowWidth;
		const unsigned int ArrowY = (HeaderHeight - ArrowHeight) / 2;

		HWND h = data->children.at(L"container")->children.at(L"header_" + curDropDownLang)->handle;
		HWND hArrow = CreateWindowEx(0, L"Image", L"", WS_VISIBLE | WS_CHILD, ArrowX, ArrowY, ArrowWidth, ArrowHeight, data->children.at(L"container")->children.at(L"header_" + curDropDownLang)->handle, nullptr, nullptr, nullptr);
		std::unique_ptr<WindowData> wd_ptr_arrow = std::make_unique<WindowData>(hArrow, ChildList(), 0, ArrowHeight, ArrowWidth, ArrowX, ArrowY, &(*data->children.at(L"container")->children.at(L"header_" + curDropDownLang)), true);
		CheckEmplace(data->children.at(L"container")->children.at(L"header_" + curDropDownLang)->children.emplace(L"arrow", std::move(wd_ptr_arrow)), L"dashboard header drop down arrows");
	}

	void Dashboard::createDropDownMenu(WindowData* data, const std::wstring& curDropDownLang, const COLORREF BkColor, const unsigned int singleWidth, const unsigned int HeaderHeight, const unsigned int MenuHeight, const unsigned int itemHeight, const size_t numLessonGroups) noexcept
	{
		const unsigned int MenuX = data->children.at(L"container")->children.at(L"header_" + curDropDownLang)->x;
		const unsigned int MenuY = HeaderHeight;
		const unsigned int MenuWidth = singleWidth;

		HWND hMenu = CreateWindowEx(0, L"Light Groutfit", L"", WS_CHILD | WS_VSCROLL, MenuX, MenuY, MenuWidth, MenuHeight, data->children.at(L"container")->handle, nullptr, nullptr, nullptr);
		std::unique_ptr<WindowData> wd_ptr_menu = std::make_unique<WindowData>(hMenu, ChildList(), BkColor, MenuHeight, MenuWidth, MenuX, MenuY, &(*data->children.at(L"container")), false);
		CheckEmplace(data->children.at(L"container")->children.emplace(L"menu_" + curDropDownLang, std::move(wd_ptr_menu)), L"dashboard drop down menus");

		const unsigned int maxVisibleLines = MenuHeight / itemHeight;

		SCROLLINFO menuScroll = {};
		menuScroll.cbSize = sizeof(SCROLLINFO);
		menuScroll.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
		menuScroll.nMin = 0; //the range start.  It's zero-based, so start at 0
		menuScroll.nMax = static_cast<UINT>(numLessonGroups * 1) - 1; //the last position in the range.  
		menuScroll.nPos = 0;
		menuScroll.nPage = (maxVisibleLines * 1);
		SetScrollInfo(hMenu, SB_VERT, &menuScroll, false);
		GetScrollInfo(hMenu, SB_VERT, &menuScroll);
	}

	void Dashboard::fillDropDownMenu(WindowData* data, const LangList& langMap, const std::wstring& curDropDownLang, HFONT itemFont, const unsigned int itemHeight, const unsigned int singleWidth) noexcept
	{
		unsigned int counter = 0;
		for (auto const & lessonGroup : langMap.at(curDropDownLang)->lessonGroups)
		{
			// Step 0: Common Metrics
			const unsigned int arrowWidth = static_cast<unsigned int>(singleWidth * 0.1);
			const unsigned int itemWidthLessonGroup = singleWidth - arrowWidth;
			//const unsigned int itemWidthLesson = itemWidthLessonGroup - arrowWidth;

			// Step 1: Create The Lesson Groups
			Dashboard::createLessonGroupMenuItem(data, curDropDownLang, lessonGroup.first, lessonGroup.second.name, itemFont, arrowWidth, counter * itemHeight, itemWidthLessonGroup, itemHeight);

			// Step 2: Create The Lesson Group Drop Down Arrows
			Dashboard::createLessonGroupMenuItemArrow(data, curDropDownLang, lessonGroup.first, counter * itemHeight, arrowWidth, itemHeight);

			counter++;
		}
	}

	void Dashboard::createLessonGroupMenuItem(WindowData* data, const std::wstring& curDropDownLang, const std::wstring& curLessonGroup, const std::wstring& lessonGroupName, HFONT itemFont, const unsigned int LGMI_x, const unsigned int LGMI_y, const unsigned int LGMI_width, const unsigned int LGMI_height) noexcept
	{
		HWND LGMI_handle = CreateWindowEx(0, L"STATIC", lessonGroupName.c_str(), WS_CHILD | SS_CENTERIMAGE | SS_NOTIFY, LGMI_x, LGMI_y, LGMI_width, LGMI_height, data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->handle, nullptr, nullptr, nullptr);
		std::unique_ptr<WindowData> wd_ptr_LGMI = std::make_unique<WindowData>(LGMI_handle, ChildList(), data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->color, LGMI_height, LGMI_width, LGMI_x, LGMI_y, &(*data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)), false);
		CheckEmplace(data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children.emplace(L"lessongroup_" + curLessonGroup, std::move(wd_ptr_LGMI)), L"dashboard lesson group menu item");
		SendMessage(LGMI_handle, WM_SETFONT, reinterpret_cast<WPARAM>(itemFont), true);
	}

	void Dashboard::createLessonGroupMenuItemArrow(WindowData* data, const std::wstring& curDropDownLang, const std::wstring& curLessonGroup, const unsigned int LGMIA_y, const unsigned int LGMIA_width, const unsigned int LGMIA_height) noexcept
	{
		HWND LGMIA_handle = CreateWindowEx(0, L"Image", L"", WS_CHILD | WS_CLIPSIBLINGS, 0, LGMIA_y, LGMIA_width, LGMIA_height, data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->handle, nullptr, nullptr, nullptr);
		std::unique_ptr<WindowData> wd_ptr_LGMIA = std::make_unique<WindowData>(LGMIA_handle, ChildList(), data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->color, LGMIA_height, LGMIA_width, 0, LGMIA_y, &(*data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)), false);
		CheckEmplace(data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children.emplace(L"lessongrouparrow_" + curLessonGroup, std::move(wd_ptr_LGMIA)), L"dashboard lesson group arrows");
	}

	void Dashboard::insertLessonMenuItems(WindowData* data, const LangList& langs, const std::wstring& curDropDownLang, const std::wstring& curLessonGroup, HFONT itemFont) noexcept
	{
		LessonGroup lg = langs.at(curDropDownLang)->lessonGroups.at(curLessonGroup);
		// metrics
		const unsigned int numLessons = static_cast<unsigned int>(lg.lessons.size());
		const int xDiff = data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children.at(L"lessongrouparrow_" + curLessonGroup)->width;
		const int x = data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children.at(L"lessongroup_" + curLessonGroup)->x + xDiff;
		const int height = data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children.at(L"lessongroup_" + curLessonGroup)->height;
		const int width = data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children.at(L"lessongroup_" + curLessonGroup)->width - xDiff;
		int y = data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children.at(L"lessongroup_" + curLessonGroup)->y + height;
		const int maxY = y + (numLessons * height);

		// bump the other LessonGroups down
		Dashboard::bumpBelowMenuItems(data, curDropDownLang, y, maxY);

		// make the lesson menu item windows
		for (auto const & lesson : lg.lessons)
		{
			const std::wstring LessonName = lesson.second;
			const std::wstring LessonID = lesson.first;
			HWND LMI_handle = CreateWindowEx(0, L"STATIC", LessonName.c_str(), WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE | WS_CLIPSIBLINGS | SS_NOTIFY, x, y, width, height, data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->handle, nullptr, nullptr, nullptr);
			std::unique_ptr<WindowData> wd_ptr_LMI = std::make_unique<WindowData>(LMI_handle, ChildList(), data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->color, height, width, x, y, &(*data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)), true);
			CheckEmplace(data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children.emplace(L"lesson_" + curLessonGroup + L"_" + LessonID, std::move(wd_ptr_LMI)), L"dashboard lesson menu items");
			SendMessage(LMI_handle, WM_SETFONT, reinterpret_cast<WPARAM>(itemFont), true);
			y += height;
		}

		// update scrollbar
		SCROLLINFO si = {};
		GetScrollInfo(data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->handle, SB_VERT, &si);
		int nMax = 0;
		for (auto const & child_pair : data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children)
		{
			if (child_pair.first.find(L"lessongroup_") != std::wstring::npos || child_pair.first.find(L"lesson_") != std::wstring::npos)
				nMax++;
		}
		si.nMax = nMax;
		si.nPage = data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->height / height + 1;
		si.fMask = SIF_RANGE | SIF_PAGE;
		SetScrollInfo(data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->handle, SB_VERT, &si, true);
		GetScrollInfo(data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->handle, SB_VERT, &si);
	}

	void Dashboard::removeLessonMenuItems(WindowData* data, const std::wstring& curDropDownLang, const std::wstring& curLessonGroup)
	{
		std::vector<std::wstring> itemsToRemove = {};
		int bottomY = 0;
		for (auto const & item : data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children) // step 1: destroy the windows
		{
			if (item.first.find(L"lesson_" + curLessonGroup) != std::wstring::npos)
			{
				if (item.second->y > bottomY) bottomY = item.second->y;
				DestroyWindow(item.second->handle);
				itemsToRemove.push_back(item.first);
			}
		}
		for (auto const & itemToRemove : itemsToRemove) // step 2: remove them from the WindowData
		{
			data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children.erase(itemToRemove);
		}
		// step 3: bump the below windows back up
		const int itemHeight = data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children.at(L"lessongroup_" + curLessonGroup)->height;
		bottomY += itemHeight;
		const int newTopY = data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children.at(L"lessongroup_" + curLessonGroup)->y + itemHeight;
		Dashboard::bumpBelowMenuItems(data, curDropDownLang, bottomY, newTopY);
	}

	void Dashboard::bumpBelowMenuItems(WindowData* data, const std::wstring& curDropDownLang, const int oldTopY, const int newTopY)
	{
		const int delta = newTopY - oldTopY;
		for (auto const & item : data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children)
		{
			if (item.second->y >= oldTopY)
			{
				wchar_t* buf = new wchar_t[60];
				GetWindowText(item.second->handle, buf, 60);
				if (MoveWindow(item.second->handle, item.second->x, item.second->y + delta, item.second->width, item.second->height, true) != 0)
				{
					data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children.at(item.first)->y += delta;
					GetWindowText(item.second->handle, buf, 60);
					SetWindowText(item.second->handle, buf);
					UpdateWindow(item.second->parent->handle);
				}
			}
		}
		for (auto const & item : data->children.at(L"container")->children.at(L"menu_" + curDropDownLang)->children)
		{
			UpdateWindow(item.second->handle);
		}
	}

	void Dashboard::createDashboardBody(WindowData *data, const GUI::Pages page, LangList* LanguageMap, HFONT corbelButton, const Palette& ColorPalette, const unsigned int DashboardNumDrops)
	{
		if (page == GUI::Pages::DASHBOARD)
		{
			// Step 0: Common Metrics
			const unsigned int HeaderHeight = static_cast<int>(data->height * 0.05);
			unsigned int MenuHeight = static_cast<int>(data->height * 0.5);
			const unsigned int itemHeight = static_cast<int>(MenuHeight * 0.05);
			const unsigned int singleWidth = data->width / 5;
			const unsigned int spacer = static_cast<int>(data->width * 0.00366); // roughly 5px
			const unsigned int maxViewWidth = (singleWidth * DashboardNumDrops) + (spacer * (DashboardNumDrops - 1));

			// Step 1: Container Window
			Dashboard::createContainerWindow(data, ColorPalette.BackgroundColor, LanguageMap->size(), DashboardNumDrops, HeaderHeight, MenuHeight, maxViewWidth);
			MenuHeight -= GetSystemMetrics(SM_CYHSCROLL) + 1;

			// Step 2: Make Language Drop-Downs
			Dashboard::createDropDowns(data, *LanguageMap, ColorPalette, corbelButton, singleWidth, spacer, HeaderHeight, MenuHeight, itemHeight);
		}
	}

	void Dashboard::WM_Paint(const HWND hwnd, const HDC hdc, const COLORREF TextColor, const COLORREF BkColor, HFONT font)
	{
		RECT rect;
		GetClientRect(hwnd, &rect);
		SendMessage(hwnd, WM_ERASEBKGND, reinterpret_cast<WPARAM>(hdc), reinterpret_cast<LPARAM>(nullptr));
		HBRUSH hBrush = CreateSolidBrush(BkColor);
		SelectObject(hdc, hBrush);
		SelectObject(hdc, GetStockObject(NULL_PEN));
		Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
		DeleteObject(hBrush);
		wchar_t * WindowText = new wchar_t[30];
		GetWindowText(hwnd, WindowText, 30);
		LOGFONT lf;
		GetObject(font, sizeof(LOGFONT), &lf);
		HFONT hf = CreateFontIndirect(&lf);
		HFONT oldf = static_cast<HFONT>(SelectObject(hdc, hf));
		SetTextColor(hdc, TextColor);
		SetBkMode(hdc, TRANSPARENT);
		RECT nRect = {static_cast<int>(rect.left + ( (rect.right - rect.left) * 0.05 )),
		rect.top,
		static_cast<int>(rect.left + (rect.right - rect.left) * 0.5),
		rect.bottom};
		DrawTextEx(hdc, WindowText, -1, &nRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE, nullptr);
		delete[] WindowText;
	}

	void Dashboard::MenuWM_VScroll(WindowData* data, const HWND hwnd, const WPARAM wParam)
	{
		unsigned int totalHeight = 0;
		for (auto const & child_pair : data->children.at(L"container")->children)
		{
			if (hwnd == child_pair.second->handle)
			{
				for (auto const & child_pair2 : child_pair.second->children)
				{
					if (child_pair2.first.find(L"lessongroup_") != std::wstring::npos || child_pair2.first.find(L"lesson_")
						!= std::wstring::npos)
					{
						totalHeight += child_pair2.second->height;
					}
				}
				break;
			}
		}
		GUI::WM_VScroll(hwnd, wParam, totalHeight);
	}

	void Dashboard::ContainerWM_HScroll(WindowData* data, const size_t numLangs, const HWND hwnd, const WPARAM wParam)
	{
		int singleWidth = 0;
		int spacer = 0;
		for (auto const & child_pair : data->children.at(L"container")->children)
		{
			singleWidth = child_pair.second->width;
			const int x = child_pair.second->x;
			if ((x > 0) && (x < (singleWidth * 2))) // the second drop-down
			{
				spacer = x - singleWidth;
				break;
			}
		}
		const size_t totalWidth = (singleWidth * numLangs) - (spacer * (numLangs - 1));
		GUI::WM_HScroll(hwnd, wParam, static_cast<unsigned int>(totalWidth)); // narrowing cast
	}

	void Dashboard::HeaderArrowWM_Paint(WindowData* data, const std::wstring& curLangDropDown, const HDC hdc, const unsigned int width, const unsigned int height)
	{
		ImageLoader il(L"gui/elems/dropdownarrow_dark.png");
		try
		{
			il.loadImage();
			if (il.getIsLoaded())
			{
				if (data->children.at(L"container")->children.at(L"menu_" + curLangDropDown)->visible)
				{
					il.rotateImage(WICBitmapTransformFlipVertical);
					if (il.getIsRotated()) il.renderImage(hdc, 0, 0, width, height);
				}
				else il.renderImage(hdc, 0, 0, width, height);
			}
			if (!il.getIsRendered()) throw;
		}
		catch (int ex_num)
		{
			std::wcout << L"IL Exception: " << ex_num << std::endl;
		}
	}

	void Dashboard::HeaderArrowWM_LButtonUp(WindowData& data, GUI& gui, const ChildPair& child_pair, const ChildPair& child_pair2, const HWND hwnd, const COLORREF BkColor)
	{
		ImageLoader il(L"gui/elems/dropdownarrow_dark.png");
		try
		{
			HDC hdc = GetDC(hwnd);
			il.loadImage();
			const unsigned int width = child_pair2.second->width;
			const unsigned int height = child_pair2.second->height;
			if (il.getIsLoaded())
			{
				const std::wstring curLangDropDown = child_pair.first.substr(child_pair.first.find(L"_") + 1);
				if (data.children.at(L"container")->children.at(L"menu_" + curLangDropDown)->visible) // drop down menu is currently visible
				{
					il.clearImage(hdc, child_pair.second->color, width, height);
					il.renderImage(hdc, 0, 0, width, height);
					if (!il.getIsRendered()) throw;
					gui.ClearWindows(*data.children.at(L"container")->children.at(L"menu_" + curLangDropDown), true); // hide the drop down menu
				}
				else // drop down menu is currently hidden
				{
					il.rotateImage(WICBitmapTransformFlipVertical);
					if (il.getIsRotated())
					{
						il.clearImage(hdc, BkColor, width, height);
						il.renderImage(hdc, 0, 0, width, height);
						if (!il.getIsRendered()) throw;
						gui.RestoreWindows(*data.children.at(L"container")->children.at(L"menu_" + curLangDropDown), true); // make the drop down menu visible
					}
				}
			}
			ReleaseDC(hwnd, hdc);
		}
		catch (int ex_num)
		{
			std::wcout << L"IL Exception: " << ex_num << std::endl;
		}
	}

	void Dashboard::LessonGroupArrowWM_Paint(const ChildPair& child_pair, const ChildPair& child_pair2, const HDC hdc)
	{
		const int width = child_pair2.second->width;
		const int height = static_cast<int>(child_pair2.second->height * 0.75);
		const int x = (width - height) / 2;
		const int y = (child_pair2.second->height - height) / 2;
		bool dir270 = true;
		const std::wstring lgid = child_pair2.first.substr(child_pair2.first.find(L"_") + 1);
		for (auto const & child_pair3 : child_pair.second->children)
		{
			if (child_pair3.first.find(L"lesson_" + lgid) != std::wstring::npos)
			{
				dir270 = false;
				break;
			}
		}
		ImageLoader il(L"gui/elems/dropdownarrow_dark.png");
		try
		{
			il.loadImage();
			if (dir270 && il.getIsLoaded()) il.rotateImage(WICBitmapTransformRotate270);
			else if (!dir270 && il.getIsLoaded()) il.rotateImage(WICBitmapTransformFlipVertical);
			if (il.getIsRotated()) il.renderImage(hdc, x, y, height, height);
			if (!il.getIsRendered()) throw;
		}
		catch (int ex_num)
		{
			std::wcout << L"IL Exception: " << ex_num << std::endl;
		}
	}

	void Dashboard::LessonGroupWM_LButtonUp(WindowData& data, const LangList& LanguageMap, const ChildPair& child_pair, const ChildPair& child_pair2, const HWND hwnd, const HFONT font)
	{
		const std::wstring curLangDropDown = child_pair.first.substr(child_pair.first.find(L"_") + 1);
		const std::wstring curLessonGroup = child_pair2.first.substr(child_pair2.first.find(L"_") + 1);
		if (LanguageMap.at(curLangDropDown)->lessonGroups.at(curLessonGroup).lessons.size() == 0) // make sure there are lessons to display.  If not, don't waste my valuable time rendering and looping and calculating
		{
			const std::wstring errorMsg = L"Lesson Group " + curLessonGroup + L" has no installed lessons.";
			Error(errorMsg.c_str());
			return;
		}
		const unsigned int width = child_pair2.second->width;
		const unsigned int height = static_cast<unsigned int>(child_pair2.second->height * 0.75);
		const int x = (width - height) / 2;
		const int y = (child_pair2.second->height - height) / 2;
		/***/
		ImageLoader il(L"gui/elems/dropdownarrow_dark.png");
		try
		{
			HDC hdc = GetDC(hwnd);
			il.loadImage();
			if (il.getIsLoaded())
			{
				if (GetPixel(hdc, width / 2 + 1, child_pair2.second->height / 2) == child_pair2.second->color) // this Lesson Group's lessons are currently visible.  True if the pixel is the background color of the arrow (ie true if the pixel is not red)
				{
					il.rotateImage(WICBitmapTransformRotate270);
					if (il.getIsRotated())
					{
						il.clearImage(hdc, child_pair2.second->color, child_pair2.second->width, child_pair2.second->height);
						il.renderImage(hdc, x, y, height, height);
						if (!il.getIsRendered()) throw;
						// hide this Lesson Group's lessons & bump up all the windows below
						Dashboard::removeLessonMenuItems(&data, curLangDropDown, curLessonGroup);
					}
				}
				else // this Lesson Group's lessons are currently hidden
				{
					il.rotateImage(WICBitmapTransformFlipVertical);
					if (il.getIsRotated())
					{
						il.clearImage(hdc, child_pair2.second->color, child_pair2.second->width, child_pair2.second->height);
						il.renderImage(hdc, x, y, height, height);
						if (!il.getIsRendered()) throw;
						// bump down all the windows below & make this Lesson Group's lessons visible
						Dashboard::insertLessonMenuItems(&data, LanguageMap, curLangDropDown, curLessonGroup, font);
					}
				}
			}
			ReleaseDC(hwnd, hdc);
		}
		catch (int ex_num)
		{
			std::wcout << "IL Exception: " << ex_num << std::endl;
		}
	}
}