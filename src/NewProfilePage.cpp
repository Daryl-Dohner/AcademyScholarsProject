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

// Program Headers
#include "NewProfilePage.hpp"
#include "misc.hpp"
#include "GUI.hpp"
#include "WProc.hpp"
#include "Profiles.hpp"

namespace ASP
{
	void NewProfilePage::CreateNewProfileBody(WindowData *data, const GUI::Pages page, const COLORREF color, const HFONT font)
	{
		if (page == GUI::Pages::NEW_PROFILE)
		{
			//Step 0: Common Metrics
			const int NwidthLabel = static_cast<int>(data->width * 0.067);
			const int NwidthTextBox = static_cast<int>(data->width * 0.4);
			const int NwidthButton = static_cast<int>(data->width * 0.067);
			const int Nheight = static_cast<int>(data->height * 0.05);
			const int TotalWidth = NwidthLabel + NwidthTextBox + NwidthButton + static_cast<int>(data->width * 0.01);

			//Step 1: label
			const int xLabel = ((data->width - TotalWidth) / 2);
			const int yLabel = data->children.at(L"title")->y + (data->children.at(L"title")->height) + static_cast<int>(data->height * 0.1);
			CreateLabel(data, font, color, NwidthLabel, Nheight, xLabel, yLabel);

			//Step 2: Text Field
			const int xTextBox = xLabel + NwidthLabel + static_cast<int>(data->width * 0.005);
			int yTextBox = yLabel;
			CreateTextBox(data, font, color, xTextBox, &yTextBox, NwidthTextBox, Nheight);

			//Step 3: Button
			const int xButton = xTextBox + NwidthTextBox + static_cast<int>(data->width * 0.005);
			const int yButton = data->children.at(L"title")->y + (data->children.at(L"title")->height) + static_cast<int>(data->height * 0.1);
			CreateButton(data, font, color, xButton, yButton, NwidthButton, Nheight);
		}
	}

	void NewProfilePage::CreateLabel(WindowData* data, const HFONT font, const COLORREF color, const int NwidthLabel, const int Nheight, const int xLabel, const int yLabel)
	{
		HWND hLabel = CreateWindowEx(0, L"STATIC", L"Name: ", SS_CENTER | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD | SS_NOTIFY, xLabel, yLabel, NwidthLabel, Nheight, data->handle, nullptr, nullptr, nullptr);
		SendMessage(hLabel, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
		std::unique_ptr<WindowData> wd_ptr_label = std::make_unique<WindowData>(hLabel, ChildList(), color, Nheight, NwidthLabel, xLabel, yLabel, data, true);
		CheckEmplace(data->children.emplace(L"Label", std::move(wd_ptr_label)), L"new profile label");
	}

	void NewProfilePage::CreateTextBox(WindowData* data, const HFONT font, const COLORREF color, const int xTextBox, int* yTextBox, const int NwidthTextBox, const int Nheight)
	{
		//attempt number 0xFFFFFFFF for v-centering this text...Part 1
		LOGFONT LF_verdanaNormal = {};
		GetObject(font, sizeof(LF_verdanaNormal), &LF_verdanaNormal);
		const int FontHeight = LF_verdanaNormal.lfHeight;
		const int OffsetY = (Nheight - FontHeight) / 2;
		*yTextBox += OffsetY;

		HWND hTextBox = CreateWindowEx(0, L"EDIT", nullptr, ES_LEFT | WS_VISIBLE | WS_CHILD, xTextBox, *yTextBox, NwidthTextBox, Nheight, data->handle, nullptr, nullptr, nullptr);
		SendMessage(hTextBox, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
		SendMessage(hTextBox, EM_LIMITTEXT, static_cast<WPARAM>(Profiles::MaxProfileNameLength), 0);
		std::unique_ptr<WindowData> wd_ptr_textbox = std::make_unique<WindowData>(hTextBox, ChildList(), color, Nheight, NwidthTextBox, xTextBox, *yTextBox, data, true);
		CheckEmplace(data->children.emplace(L"TextBox", std::move(wd_ptr_textbox)), L"new profile textbox");
		SetWindowSubclass(hTextBox, TextBoxSubclass, 0, 0);
	}

	void NewProfilePage::CreateButton(WindowData *data, const HFONT font, const COLORREF color, const int xButton, const int yButton, const int NwidthButton, const int Nheight)
	{
		HWND hButton = CreateWindowEx(0, L"STATIC", L"Create", SS_CENTER | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD | SS_NOTIFY, xButton, yButton, NwidthButton, Nheight, data->handle, nullptr, nullptr, nullptr);
		SendMessage(hButton, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
		std::unique_ptr<WindowData> wd_ptr_button = std::make_unique<WindowData>(hButton, ChildList(), color, Nheight, NwidthButton, xButton, yButton, data, true);
		CheckEmplace(data->children.emplace(L"CreateButton", std::move(wd_ptr_button)), L"new profile create button");
		SetWindowSubclass(hButton, ButtonSubclass, 0, 0);
		InvalidateRect(hButton, nullptr, false);
	}

	INT_PTR NewProfilePage::WM_CTLColorEdit(const Palette ColorPalette, HWND subjectHWND, HDC hdC, HFONT font, const bool& NewProfileAllowed, bool* NewProfilesColorEditHandled, bool disable)
	{
		if (!NewProfileAllowed && !disable)
		{
			EnableWindow(subjectHWND, FALSE);
			return reinterpret_cast<LRESULT>(GetStockObject(NULL_BRUSH));
		}
		SetTextColor(hdC, ColorPalette.TextColor);
		SetBkMode(hdC, TRANSPARENT);
		RECT ClientRect = {};
		GetClientRect(subjectHWND, &ClientRect);
		//attempt number 0xFFFFFFFF for v-centering this text...Part 2
		LOGFONT verdanaNormal_lf = {};
		GetObject(font, sizeof(LOGFONT), &verdanaNormal_lf);
		const int FontHeight = verdanaNormal_lf.lfHeight;
		const int ClientHeight = (ClientRect.bottom - ClientRect.top);
		const int OffsetY = (ClientHeight - FontHeight) / 2;
		ClientRect.top -= OffsetY;
		ClientRect.bottom -= OffsetY;
		HBRUSH hBrush = 0;
		HPEN hPen = 0;
		if (!disable)
		{
			hBrush = CreateSolidBrush(ColorPalette.BoxColor);
			hPen = CreatePen(BS_SOLID, 1, ColorPalette.BorderColor);
		}
		else
		{
			hBrush = CreateSolidBrush(ColorPalette.BoxColorDisabled);
			hPen = CreatePen(BS_SOLID, 1, ColorPalette.BorderColorDisabled);
		}
		HGDIOBJ hOldBrush = SelectObject(hdC, hBrush);
		HGDIOBJ hOldPen = SelectObject(hdC, hPen);
		if (*NewProfilesColorEditHandled) RoundRect(hdC, ClientRect.left, ClientRect.top, ClientRect.right, ClientRect.bottom, 7, 7);
		SelectObject(hdC, hOldBrush);
		SelectObject(hdC, hOldPen);
		DeleteObject(hBrush);
		DeleteObject(hPen);
		*NewProfilesColorEditHandled = true;
		return reinterpret_cast<LRESULT>(GetStockObject(NULL_BRUSH));
	}

	INT_PTR NewProfilePage::WM_CTLColorStatic(const Palette ColorPalette, HWND subjectHWND, HDC hdC, HFONT font, const bool& NewProfileAllowed, bool* NewProfilesColorEditHandled) // sent instead of WM_CTLCOLOREDIT when the text box is disabled
	{
		return NewProfilePage::WM_CTLColorEdit(ColorPalette, subjectHWND, hdC, font, NewProfileAllowed, NewProfilesColorEditHandled, true);
	}

	bool NewProfilePage::TextBoxWM_KeyDown(WindowData* data, const HWND& hwnd, const UINT& uMsg, const WPARAM& wParam, const LPARAM& lParam)
	{
		if (wParam == VK_RETURN)
		{
			SendMessage(data->handle, WM_COMMAND, STN_CLICKED, reinterpret_cast<LPARAM>(data->children.at(L"CreateButton")->handle));
			return false;
		}
		else return DefSubclassProc(hwnd, uMsg, wParam, lParam);
	}
}