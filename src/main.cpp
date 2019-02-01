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

// msvc
#pragma comment(lib, "comctl32.lib") // subclassing
#pragma comment(lib, "windowscodecs.lib") // Windows Imagaging Component
#pragma comment(lib, "Msimg32.lib") // AlphaBlend function

// the below pragmas dont work so they are done in the compilier parameters
// #pragma warning(disable: 26400) // Do not assign the result of an allocation or a function call with an owner<T> return value
// #pragma warning(disable: 26401) // Do not delete a raw pointer that is not an owner<T>
// #pragma warning(disable: 26409) // Avoid calling new and delete explicitly, use std::make_unique<T> instead
// #pragma warning(disable: 26426) // Global initializer calls a non-constexpr function
// #pragma warning(disable: 26451) // Aritmetic overflow
// #pragma warning(disable: 26462) // The value pointed to is assigned only once, mark it as a pointer to const
// #pragma warning(disable: 26472) // Don't use a static_cast for arithmetic conversions.  Use brace initialization, gsl::narrow_cast, or gsl::narrow
// #pragma warning(disable: 26481) // Don't use pointer arithmetic. Use span instead
// #pragma warning(disable: 26485) // No array to pointer decay
// #pragma warning(disable: 26490) // Don't use reinterpret_cast

// STL headers
#include <string>
#include <vector>
#include <unordered_map>
#include <memory> // std::unique_ptr
#include <iostream>

// Windows headers
#define UNICODE
#include <windows.h>

// Program headers
#include "misc.hpp"
#include "WProc.hpp"
#include "WClass.hpp"
#include "GUI.hpp"

namespace ASP
{
	// window classes
	extern WNDCLASSEX MainWC; // Defined in WClass.cpp
	extern WNDCLASSEX GroutfitWC; // Defined in WClass.cpp
	extern WNDCLASSEX LightGroutfitWC; // Defined in WClass.cpp
	extern WNDCLASSEX ContainerWC; // Defined in Wclass.cpp
	extern WNDCLASSEX ImageWC; // Defined in Wclass.cpp

	// WindowData structures
	WindowData MainWData;
	WindowData AboutWData;
	WindowData StartWData;
	WindowData NewProfileWData;
	WindowData DashboardWData;
	WindowData LessonPageWData;

	// other
	Palette ColorPalette;
	LangList LanguageMap; // ALL the languages and ALL the lessons
	GUI gui; // Main window GUI instance
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) // Main Window Function
{
	HWND hwnd = nullptr;
	MSG Msg = {};
	ASP::gui = ASP::GUI(ASP::GUI::Pages::HOME);
	/***/
	ASP::SetPalette(&ASP::ColorPalette);
	ASP::MainWData.color		= ASP::ColorPalette.BackgroundColor;
	ASP::AboutWData.color		= ASP::ColorPalette.BackgroundColor;
	ASP::StartWData.color		= ASP::ColorPalette.BackgroundColor;
	ASP::NewProfileWData.color	= ASP::ColorPalette.BackgroundColor;
	ASP::DashboardWData.color	= ASP::ColorPalette.BackgroundColor;
	ASP::LessonPageWData.color	= ASP::ColorPalette.BackgroundColor;
	/***/
	std::vector<ASP::WindowData*> data;
	data.push_back(&ASP::MainWData);
	data.push_back(&ASP::AboutWData);
	data.push_back(&ASP::StartWData);
	data.push_back(&ASP::NewProfileWData);
	data.push_back(&ASP::DashboardWData);
	data.push_back(&ASP::LessonPageWData);
	MakeWClasses(hInstance, data);
	/***/
	if(!RegisterClassEx(&ASP::MainWC)
		|| !RegisterClassEx(&ASP::GroutfitWC)
		|| !RegisterClassEx(&ASP::LightGroutfitWC)
		|| !RegisterClassEx(&ASP::ContainerWC)
		|| !RegisterClassEx(&ASP::ImageWC))
	{
		MessageBox(nullptr, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
    
    //get height of taskbar so that it doesn't overlap the window
	RECT workArea = {};
    //GetWindowRect(GetDesktopWindow(), &Desk);
    //GetWindowRect(FindWindow("Shell_traywnd", nullptr), &Task);
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    //const int taskH = Task.bottom-Task.top;
	int MainWinX = 0;
	const int MainWinY = 0;
	ASP::MainWData.width = workArea.right;
	ASP::MainWData.height = workArea.bottom;
	if (ASP::GetWindowsMajorVersion() == 10)
	{
		const int frame_width = 9;
		MainWinX -= frame_width;
		ASP::MainWData.width += 2 * frame_width;
		ASP::MainWData.height += frame_width;
	}
	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
		L"MainWindow",
		L"LearnCS++ Coding Tutorials",
		WS_VISIBLE | ((WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME) ^ WS_MAXIMIZEBOX),
		MainWinX,
		MainWinY,
		ASP::MainWData.width,
		ASP::MainWData.height,
		nullptr,nullptr,hInstance,nullptr);
	if(hwnd == nullptr)
	{
		MessageBox(nullptr, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	ASP::MainWData.visible = true;
	while(GetMessage(&Msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	
	return static_cast<int>(Msg.wParam);
}
