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

// Windows Headers
#define UNICODE
#include <windows.h>

// App Headers
#include "AboutPage.hpp"
#include "misc.hpp"
#include "GUI.hpp"

namespace ASP
{
	void AboutPage::CreateAboutBody(WindowData *data, const GUI::Pages page, Palette ColorPalette) // About Page Body Text
	{
		if (page == GUI::Pages::ABOUT)
		{
			const int Nwidth = static_cast<int>(data->width * 0.8);
			const int Nheight = static_cast<int>(data->height * 0.4);
			const int x = (data->width - Nwidth) / 2;
			const int y = data->children.at(L"title")->y + data->children.at(L"title")->height;
			HWND h = CreateWindowEx(0, L"Groutfit", L"", WS_CHILD | WS_VISIBLE, x, y, Nwidth, Nheight, data->handle, nullptr, nullptr, nullptr);
			std::unique_ptr<WindowData> wd_ptr = std::make_unique<WindowData>(h, ChildList(), ColorPalette.BoxColor, Nheight, Nwidth, x, y, data, true);
			CheckEmplace(data->children.emplace(L"about_body", std::move(wd_ptr)), L"about body");
		}
	}

	void AboutPage::CreateAboutBodyText(WindowData *data, const GUI::Pages page, const HFONT font, bool *readyAboutTextColor) // About Page Body Text Actual Text (lol)
	{
		if (page == GUI::Pages::ABOUT)
		{
			const wchar_t * AboutText = L"Let there be text!";  // maybe load in from a text file?
			const int Nwidth = static_cast<int>(data->children.at(L"about_body")->width * 0.95);
			const int Nheight = static_cast<int>(data->children.at(L"about_body")->height * 0.9);
			const int x = static_cast<int>(data->children.at(L"about_body")->width * 0.025);
			const int y = static_cast<int>(data->children.at(L"about_body")->height * 0.05);
			HWND h = CreateWindowEx(0, L"STATIC", AboutText, WS_VISIBLE | WS_CHILD | SS_LEFT, x, y, Nwidth, Nheight, data->children.at(L"about_body")->handle, nullptr, nullptr, nullptr);
			std::unique_ptr<WindowData> wd_ptr = std::make_unique<WindowData>(h, ChildList(), data->color, Nheight, Nwidth, x, y, data, true);
			CheckEmplace(data->children.at(L"about_body")->children.emplace(L"about_body_text", std::move(wd_ptr)), L"about body text");
			*readyAboutTextColor = true;
			InvalidateRect(h, nullptr, false);
			SendMessage(h, WM_SETFONT, reinterpret_cast<WPARAM>(font), true);
		}
	}
}