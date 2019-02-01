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

#ifndef MISC_HPP
#define MISC_HPP

//STL headers
#include <string>
#include <memory> //std::unique_ptr
#include <unordered_map>
#include <map>
#include <iostream>

//Windows headers
#define UNICODE
#include <windows.h>

namespace ASP
{
	struct WindowData;

	using Metric = const unsigned int;

	using ChildList = std::unordered_map<std::wstring, std::unique_ptr<WindowData>>;

	using ChildPair = std::pair<const std::wstring, std::unique_ptr<WindowData>>;

	struct WindowData // the infamous Window Data struct
	{
		WindowData() noexcept : handle(nullptr), children(ChildList()), color(0), height(0), width(0), x(0), y(0), parent(nullptr), visible(true) {};
		WindowData(const HWND handle_, ChildList& children_, const COLORREF color_, const int height_, const int width_, const int x_, const int y_, WindowData * parent_, const bool visible_) noexcept : handle(handle_), children(std::move(children_)), color(color_), height(height_), width(width_), x(x_), y(y_), parent(parent_), visible(visible_) {};
		HWND handle;
		ChildList children;
		COLORREF color;
		int height;
		int width;
		int x;
		int y;
		WindowData *parent;
		bool visible;
	};

	struct Palette // the colors for the different GUI elements
	{
		COLORREF BackgroundColor;
		COLORREF TextColor;
		COLORREF BoxColor;
		COLORREF BorderColor;
		COLORREF LightGray;
		COLORREF BoxColorDisabled;
		COLORREF BorderColorDisabled;
		COLORREF BoxColorSelected;
		COLORREF BorderColorSelected;
		COLORREF BlueButtonColor;
		COLORREF Black;
		COLORREF Green;
		COLORREF Red;
	};

	void SetPalette(Palette*) noexcept; // Set the Color Scheme

	void ErrorExit(const wchar_t * = L""); // debug error

	void Error(const wchar_t * = L"") noexcept; // user error

	void Console(void) noexcept; // allocate a console for debug io

	void DebugRect(const RECT&, const std::wstring = L""); // fancy output for a RECT

	bool RectScreenToClient(const HWND, RECT*) noexcept;  // Because M$ is annoying

	bool RectClientToScreen(const HWND, RECT*) noexcept;

	template<class t>
	void CheckEmplace(t pair, const std::wstring& name = L"<no name provided>") noexcept // check if an emplace worked.
	{
		if (!pair.second)
		{
			const std::wstring errmsg = L"CheckEmplace: " + name;
			Error(errmsg.c_str());
		}
	}

	using Lesson = std::pair<std::wstring, std::wstring>; // (name, ID)

	struct LessonGroup
	{
		std::wstring name;
		std::map<std::wstring, std::wstring> lessons; // (ID, name)
	};

	struct Language
	{
		Language() noexcept : name(L""), langID(L""), lessonGroups(std::map<std::wstring, LessonGroup>()) {};
		Language(const std::wstring& name_, const std::wstring& langID_, std::map<std::wstring, LessonGroup>& lessonGroups_) noexcept : name(name_), langID(langID_), lessonGroups(lessonGroups_) {};
		std::wstring name;
		std::wstring langID;
		std::map<std::wstring, LessonGroup> lessonGroups; // (ID, LG)
	};

	using LangList = std::unordered_map<std::wstring, std::unique_ptr<Language>>;

	int GetWindowsMajorVersion(void) noexcept;

	void DebugLangList(LangList&);

	std::string utf8_encode(const std::wstring&);

	std::wstring utf8_decode(const std::string&);
}

#endif