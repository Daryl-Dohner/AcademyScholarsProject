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

//Windows headers
#define UNICODE
#include <windows.h>

//Program headers
#include "WClass.hpp"
#include "WProc.hpp"

namespace ASP
{
	extern Palette ColorPalette; // Defined in main.cpp
	WNDCLASSEX MainWC;
	WNDCLASSEX GroutfitWC;
	WNDCLASSEX LightGroutfitWC;
	WNDCLASSEX ContainerWC;
	WNDCLASSEX ImageWC;

	void MakeWClasses(HINSTANCE hInstance, std::vector<WindowData*> data) // Initialize all the window classes
	{
		// Main window class
		memset(&MainWC, 0, sizeof(MainWC));
		MainWC.cbSize = sizeof(WNDCLASSEX);
		MainWC.lpfnWndProc = MainWndProc;
		MainWC.hInstance = hInstance;
		MainWC.hCursor = LoadCursor(nullptr, IDC_ARROW);
		MainWC.hbrBackground = CreateSolidBrush(data[0]->color);
		MainWC.lpszClassName = L"MainWindow";
		MainWC.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		MainWC.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

		// gray bordered box class
		memset(&GroutfitWC, 0, sizeof(GroutfitWC));
		GroutfitWC.cbSize = sizeof(WNDCLASSEX);
		GroutfitWC.lpfnWndProc = GroutfitWndProc;
		GroutfitWC.hInstance = hInstance;
		GroutfitWC.hCursor = LoadCursor(nullptr, IDC_ARROW);
		GroutfitWC.hbrBackground = CreateSolidBrush(ColorPalette.BoxColor);
		GroutfitWC.lpszClassName = L"Groutfit";
		GroutfitWC.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		GroutfitWC.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

		// light gray bordered box class
		memset(&LightGroutfitWC, 0, sizeof(LightGroutfitWC));
		LightGroutfitWC.cbSize = sizeof(WNDCLASSEX);
		LightGroutfitWC.lpfnWndProc = LightGroutfitWndProc;
		LightGroutfitWC.hInstance = hInstance;
		LightGroutfitWC.hCursor = LoadCursor(nullptr, IDC_ARROW);
		LightGroutfitWC.hbrBackground = CreateSolidBrush(ColorPalette.LightGray);
		LightGroutfitWC.lpszClassName = L"Light Groutfit";
		LightGroutfitWC.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		LightGroutfitWC.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

		//transparent container window class
		memset(&ContainerWC, 0, sizeof(ContainerWC));
		ContainerWC.cbSize = sizeof(WNDCLASSEX);
		ContainerWC.lpfnWndProc = ContainerWndProc;
		ContainerWC.hInstance = hInstance;
		ContainerWC.hCursor = LoadCursor(nullptr, IDC_ARROW);
		ContainerWC.hbrBackground = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
		ContainerWC.lpszClassName = L"Container";
		ContainerWC.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		ContainerWC.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

		// image window class
		memset(&ImageWC, 0, sizeof(ImageWC));
		ImageWC.cbSize = sizeof(WNDCLASSEX);
		ImageWC.lpfnWndProc = ImageWndProc;
		ImageWC.hInstance = hInstance;
		ImageWC.hCursor = LoadCursor(nullptr, IDC_ARROW);
		ImageWC.hbrBackground = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
		ImageWC.lpszClassName = L"Image";
		ImageWC.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		ImageWC.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	}
}