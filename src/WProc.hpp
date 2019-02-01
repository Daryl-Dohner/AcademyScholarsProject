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

#ifndef WPROC_HPP
#define WPROC_HPP

//Windows headers
#define UNICODE
#include <windows.h>

//Program headers
#include "GUI.hpp"

namespace ASP
{
	// Main Window Procedure
	LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM); // Main Window Procedure

	// Subclass Procedures
	LRESULT CALLBACK BlueButtonSubclass(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR); // Blue Button Subclass Procedure
	LRESULT CALLBACK ButtonSubclass(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR); // Button Subclass Procedure
	LRESULT CALLBACK TextBoxSubclass(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR); // Text Box Subclass Procedure
	LRESULT CALLBACK SCEditSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	// Me-Defined Window Classes
	LRESULT CALLBACK GroutfitWndProc(HWND, UINT, WPARAM, LPARAM); // gray bordered box window procedure
	LRESULT CALLBACK LightGroutfitWndProc(HWND, UINT, WPARAM, LPARAM); // light gray bordered box window procedure
	LRESULT CALLBACK ContainerWndProc(HWND, UINT, WPARAM, LPARAM); // container window procedure
	LRESULT CALLBACK ImageWndProc(HWND, UINT, WPARAM, LPARAM); // image window procedure
}

#endif