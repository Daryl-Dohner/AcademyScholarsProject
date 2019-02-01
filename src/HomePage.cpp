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


// Project Headers
#include "HomePage.hpp"
#include "misc.hpp"
#include "GUI.hpp"
#include "WProc.hpp"

// Windows Headers
#define UNICODE
#include <windows.h>

namespace ASP
{
	void HomePage::CreateAboutButton(WindowData *data, const GUI::Pages page, const Palette ColorPalette) // Home Page About Button
	{
		if (page == GUI::Pages::HOME)
		{
			const int Nwidth = static_cast<int>(data->width * 0.15);
			const int Nheight = Nwidth;
			const int x = (data->width / 2) - Nwidth - 15;
			const int y = data->children.at(L"title")->y + data->children.at(L"title")->height + 100;
			HWND h = CreateWindowEx(0, L"STATIC", L"About", SS_CENTER | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD | SS_NOTIFY, x, y, Nwidth, Nheight, data->handle, nullptr, nullptr, nullptr);
			std::unique_ptr<WindowData> wd_ptr = std::make_unique<WindowData>(h, ChildList(), ColorPalette.BoxColor, Nheight, Nwidth, x, y, data, true);
			CheckEmplace(data->children.emplace(L"about_button", std::move(wd_ptr)), L"about button");
			SetWindowSubclass(h, ButtonSubclass, 0, 0);
		}
	}

	void HomePage::CreateStartButton(WindowData *data, const GUI::Pages page, const Palette ColorPalette) // Home Page Start Button
	{
		if (page == GUI::Pages::HOME)
		{
			const int Nwidth = static_cast<int>(data->width * 0.15);
			const int Nheight = Nwidth;
			const int x = (data->width / 2) + 15;
			const int y = data->children.at(L"title")->y + data->children.at(L"title")->height + 100;
			HWND h = CreateWindowEx(0, L"STATIC", L"Start", SS_CENTER | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD | SS_NOTIFY, x, y, Nwidth, Nheight, data->handle, nullptr, nullptr, nullptr);
			std::unique_ptr<WindowData> wd_ptr = std::make_unique<WindowData>(h, ChildList(), ColorPalette.BoxColor, Nheight, Nwidth, x, y, data, true);
			CheckEmplace(data->children.emplace(L"start_button", std::move(wd_ptr)), L"start button");
			SetWindowSubclass(h, ButtonSubclass, 0, 0);
		}
	}
}