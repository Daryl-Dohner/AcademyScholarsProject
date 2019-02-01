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
#include <iostream>
#include <sstream>
#include <functional> // std::reference_wrapper

// Windows headers
#define UNICODE
#include <windows.h>
#include <Richedit.h>

// Program headers
#include "WProc.hpp"
#include "GUI.hpp"
#include "Profiles.hpp"
#include "AppData.hpp"
#include "ImageLoader.hpp"
#include "Dashboard.hpp"
#include "LessonPage.hpp"
#include "StartPage.hpp"
#include "NewProfilePage.hpp"
#include "LessonPage.hpp"

namespace ASP
{
	extern WindowData MainWData; // defined in main.cpp
	extern WindowData AboutWData; // defined in main.cpp
	extern WindowData StartWData; // defined in main.cpp
	extern WindowData NewProfileWData; // defined in main.cpp
	extern WindowData DashboardWData; // defined in main.cpp
	extern WindowData LessonPageWData; // defined in main.cpp

	extern GUI gui; // defined in main.cpp
	extern Palette ColorPalette; // defined in main.cpp
	extern bool NewProfileAllowed; // defined in Profiles.cpp
	extern LangList LanguageMap; // Defined in main.cpp

	std::vector<std::reference_wrapper<WindowData>> History;
	Profiles CurrentProfile;
	LessonPage* LP;
	bool HandlesSet = false;

	bool readyAbout						= false; // About Page
	bool readyAboutGroutfit				= false;
	bool readyAboutTextColor			= false;
	bool readyAboutCopyright			= false;
	bool AboutCreated					= false;

	bool readyStart						= false; // Start Page aka Profiles Page
	bool StartCreated					= false;
	bool readyStartCopyright			= false;
	bool readyStartBody					= false;

	bool NewProfileCreated				= false; // New Profile Page
	bool readyNewProfile				= false;
	bool readyNewProfileBody			= false;
	bool readyNewProfileCopyright		= false;
	bool NewProfilesColorEditHandled	= false;

	bool DashboardCreated				= false; // Dashboard Page
	bool readyDashboard					= false;
	bool readyDashboardBody				= false;
	bool readyDashboardCopyright		= false;
	const unsigned int DashboardNumDrops= 4;

	bool LessonPageCreated				= false; // Lesson Page
	bool readyLessonPage				= false;
	bool readyLessonPageBody			= false;
	bool readyLessonPageCopyright		= false;

	LRESULT CALLBACK MainWndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) //Main window procedure
	{
		if (!HandlesSet)
		{
			MainWData.handle = hwnd;
			AboutWData.handle = hwnd;
			StartWData.handle = hwnd;
			NewProfileWData.handle = hwnd;
			DashboardWData.handle = hwnd;
			LessonPageWData.handle = hwnd;
			HandlesSet = true;
		}
		switch(Message)
		{
			case WM_CREATE:
			{
				Console();
				gui.LoadHomePage(&MainWData, ColorPalette);
				break;
			}
			case WM_CTLCOLORSTATIC:  // remember: these comparisons are here so that things only update when they're ready!
			{
				HDC hdc = reinterpret_cast<HDC>(wParam);
				HWND subjectHWND = reinterpret_cast<HWND>(lParam);
				if(subjectHWND == MainWData.children.at(L"title")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, MainWData.color);
				else if(subjectHWND == MainWData.children.at(L"about_button")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, MainWData.color);
				else if(subjectHWND == MainWData.children.at(L"start_button")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, MainWData.color);
				else if (subjectHWND == MainWData.children.at(L"copyright")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, MainWData.color);
				if(readyAbout)
				{
					if(subjectHWND == AboutWData.children.at(L"title")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, AboutWData.color);
					if (readyAboutCopyright)
					{
						if (subjectHWND == AboutWData.children.at(L"copyright")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, AboutWData.color);
					}
				}
				if(readyStart)
				{
					if(subjectHWND == StartWData.children.at(L"title")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, StartWData.color);
					else if(readyStartCopyright)
					{
						if(subjectHWND == StartWData.children.at(L"copyright")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, StartWData.color);
						if(readyNewProfile)
						{
							if(subjectHWND == NewProfileWData.children.at(L"title")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, NewProfileWData.color);
							else if(readyNewProfileBody)
							{
								if(subjectHWND == NewProfileWData.children.at(L"Label")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, NewProfileWData.color);
								else if (subjectHWND == NewProfileWData.children.at(L"TextBox")->handle) return NewProfilePage::WM_CTLColorStatic(ColorPalette, subjectHWND, hdc, &(*gui.verdanaNormal), NewProfileAllowed, &NewProfilesColorEditHandled);
								else if (readyNewProfileCopyright)
								{
									if (subjectHWND == NewProfileWData.children.at(L"copyright")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, NewProfileWData.color);
								}
							}
						}
						if (readyDashboard)
						{
							if (subjectHWND == DashboardWData.children.at(L"title")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, DashboardWData.color);
							else if (readyDashboardCopyright)
							{
								if (subjectHWND == DashboardWData.children.at(L"copyright")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, DashboardWData.color);
								else if (readyLessonPage)
								{
									if (subjectHWND == LessonPageWData.children.at(L"title")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, NewProfileWData.color);
									else if (readyLessonPageCopyright)
									{
										if (subjectHWND == LessonPageWData.children.at(L"copyright")->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, LessonPageWData.color);
									}
								}
							}
						}
					}
				}
				break;
			}
			case WM_CTLCOLOREDIT:
			{
				HDC hdC = reinterpret_cast<HDC>(wParam);
				HWND subjectHWND = reinterpret_cast<HWND>(lParam);
				if(readyStart)
				{
					if(readyNewProfile)
					{
						if(readyNewProfileBody)
						{
							if(subjectHWND == NewProfileWData.children.at(L"TextBox")->handle) return NewProfilePage::WM_CTLColorEdit(ColorPalette, subjectHWND, hdC, &(*gui.verdanaNormal), NewProfileAllowed, &NewProfilesColorEditHandled);
						}
					}
				}
    			break;
			}
			case WM_COMMAND:
			{
				const HWND subjectHWND = reinterpret_cast<HWND>(lParam);
				const UINT_PTR state = wParam;
				if(state == STN_CLICKED) // if a child STATIC window was clicked....
				{
					if(subjectHWND == MainWData.children.at(L"about_button")->handle) // go to about page (about button)
					{
						History.push_back(MainWData);
						gui.ClearWindows(MainWData);
						if(!AboutCreated)
						{
							AboutWData.width  = MainWData.width;
							AboutWData.height = MainWData.height;
							AboutWData.handle = MainWData.handle;
							gui.setPage(GUI::Pages::ABOUT);
							gui.LoadAboutPage(&AboutWData, ColorPalette, &readyAbout, &readyAboutGroutfit, &readyAboutCopyright, &AboutCreated, &readyAboutTextColor);
						}
						else gui.RestoreWindows(AboutWData);
					}
					else if(subjectHWND == MainWData.children.at(L"start_button")->handle) // go to start page (start button)
					{
						History.push_back(MainWData);
						gui.ClearWindows(MainWData);
						if(!StartCreated)
						{
							StartWData.width = MainWData.width;
							StartWData.height = MainWData.height;
							StartWData.handle = MainWData.handle;
							gui.setPage(GUI::Pages::START);
							gui.LoadStartPage(&StartWData, ColorPalette, &NewProfileAllowed, &readyStart, &readyStartBody, &readyStartCopyright, &StartCreated);
						}
						else gui.RestoreWindows(StartWData);
					}
					if(AboutCreated)
					{
						if(subjectHWND == AboutWData.children.at(L"back_button")->handle) // reset to home page (back button)
						{
							gui.GoBack(AboutWData, History);
						}
					}
					if(StartCreated)
					{
						// so, stupid table, need to find the handle for the GO button
						bool IsThisTheGoButton = false;
						for (auto const & child_pair : StartWData.children)
						{
							if (child_pair.first.find(L"Groutfit") != std::wstring::npos)
							{
								if (subjectHWND == child_pair.second->children.at(L"GoButton")->handle) IsThisTheGoButton = true;
							}
						}
						if(subjectHWND == StartWData.children.at(L"back_button")->handle) // reset to home page (back button)
						{
							gui.GoBack(StartWData, History);
						}
						else if(subjectHWND == StartWData.children.at(L"NewProfileButton")->handle) // Go to new profiles page
						{
							if (!NewProfileAllowed)
							{
								Error(L"Max Profile Limit Reached");
								break;
							}
							History.push_back(StartWData);
							gui.ClearWindows(StartWData);
							if(!NewProfileCreated)
							{
								NewProfileWData.width	= MainWData.width;
								NewProfileWData.height	= MainWData.height;
								NewProfileWData.handle	= MainWData.handle;
								gui.setPage(GUI::Pages::NEW_PROFILE);
								gui.LoadNewProfilePage(&NewProfileWData, ColorPalette, &readyNewProfile, &readyNewProfileBody, &readyNewProfileCopyright, &NewProfileCreated);
							}
							else gui.RestoreWindows(NewProfileWData);
						}
						else if ( IsThisTheGoButton ) // GO GO GO
						{
							History.push_back(StartWData);
							gui.ClearWindows(StartWData);
							if (!DashboardCreated)
							{
								DashboardWData.width	= MainWData.width;
								DashboardWData.height	= MainWData.height;
								DashboardWData.handle	= MainWData.handle;
								gui.setPage(GUI::Pages::DASHBOARD);
								gui.LoadDashboardPage(&DashboardWData, ColorPalette, &LanguageMap, DashboardNumDrops, &readyDashboard, &readyDashboardBody, &readyDashboardCopyright, &DashboardCreated);
							}
							else gui.RestoreWindows(DashboardWData);
						}
						else
						{
							if (NewProfileCreated)
							{
								if (subjectHWND == NewProfileWData.children.at(L"CreateButton")->handle) // New Profile Edit Box Submitted
								{
									wchar_t* name = new wchar_t[Profiles::MaxProfileNameLength + 1];
									GetWindowText(NewProfileWData.children.at(L"TextBox")->handle, name, Profiles::MaxProfileNameLength + 1);
									Profiles Pro;
									if (!Pro.CheckName(std::wstring(name))) break;
									if (Pro.CreateNewProfile(name))
									{
										SendMessage(NewProfileWData.handle, WM_COMMAND, 0, reinterpret_cast<LPARAM>(NewProfileWData.children.at(L"back_button")->handle));
										StartPage::UpdateStartBody(&StartWData, gui.getPage(), &NewProfileAllowed);
									}
									delete[] name;
								}
								else if (subjectHWND == NewProfileWData.children.at(L"back_button")->handle) // reset to home page (back button)
								{
									gui.GoBack(NewProfileWData, History);
								}
							}
							if (DashboardCreated)
							{
								if (subjectHWND == DashboardWData.children.at(L"back_button")->handle)
								{
									DashboardCreated = false; // flags need to be reset to false in case the user loads a different profile
									readyDashboard = false;
									readyDashboardBody = false;
									readyDashboardCopyright = false;
									gui.GoBack(DashboardWData, History, true);
								}
								else
								{
									if (LessonPageCreated)
									{
										if (subjectHWND == LessonPageWData.children.at(L"back_button")->handle)
										{
											LessonPageCreated = false; // flags need to be reset to false in case the user loads a different lesson
											readyLessonPage = false;
											readyLessonPageBody = false;
											readyLessonPageCopyright = false;
											gui.GoBack(LessonPageWData, History, true);
											LP->Uninit();
										}
										else if (subjectHWND == LessonPageWData.children.at(L"CCBUTTON")->handle)
										{
											LP->CCButtonWM_LButtonUp(LessonPageWData, gui, ColorPalette);
										}
									}
									else
									{
										for (auto const & possible_menu : DashboardWData.children.at(L"container")->children)
										{
											if (possible_menu.first.find(L"menu_") != std::wstring::npos) // drop down menus only
											{
												for (auto const & possible_LMI : possible_menu.second->children) // LMI = Lesson Menu Item
												{
													if (possible_LMI.first.find(L"lesson_") != std::wstring::npos) // LMIs only
													{
														if (subjectHWND == possible_LMI.second->handle)
														{
															History.push_back(DashboardWData);
															gui.ClearWindows(DashboardWData);

															LessonPageWData.width = MainWData.width;
															LessonPageWData.height = MainWData.height;
															LessonPageWData.handle = MainWData.handle;
															gui.setPage(GUI::Pages::LESSON);

															const size_t delimPosLang = possible_menu.first.find(L'_');
															const size_t delimPos1 = possible_LMI.first.find(L'_');
															const size_t delimPos2 = possible_LMI.first.find(L'_', delimPos1 + 1);
															const size_t delimDelta = delimPos2 - delimPos1 - 1;

															const std::wstring curLangID = possible_menu.first.substr(delimPosLang + 1);
															const std::wstring curLangName = LanguageMap.at(curLangID)->name;
															const std::wstring curLessonGroupID = possible_LMI.first.substr(delimPos1 + 1, delimDelta);
															const std::wstring curLessonGroupName = LanguageMap.at(curLangID)->lessonGroups.at(curLessonGroupID).name;
															const std::wstring curLessonID = possible_LMI.first.substr(delimPos2 + 1);
															const std::wstring curLessonName = LanguageMap.at(curLangID)->lessonGroups.at(curLessonGroupID).lessons.at(curLessonID);

															std::wcout << L"LMI Clicked: " << curLangID << L"::" << curLessonGroupID << L"::" << curLessonID << L'\n';
															gui.LoadLessonPage(&LessonPageWData, &LanguageMap, ColorPalette, curLangID, curLangName, curLessonGroupID, curLessonGroupName, curLessonID, curLessonName, &readyLessonPage, &readyLessonPageBody, &readyLessonPageCopyright, &LessonPageCreated);
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
				break;
			}
			case WM_DESTROY:
			{
				PostQuitMessage(0);
				break;
			}
			default:
				return DefWindowProc(hwnd, Message, wParam, lParam);
		}
		return 0;
	}

	LRESULT CALLBACK ButtonSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		switch(uMsg)
		{
			case WM_PAINT:
			{
				PAINTSTRUCT ps = {0};
				HDC hdc = BeginPaint(hwnd, &ps);
				if(hwnd == MainWData.children.at(L"about_button")->handle) GUI::WM_Paint(hwnd, hdc, ColorPalette.TextColor, MainWData.children.at(L"about_button")->color, ColorPalette.BorderColor, &(*gui.corbelButton));
				else if(hwnd == MainWData.children.at(L"start_button")->handle) GUI::WM_Paint(hwnd, hdc, ColorPalette.TextColor, MainWData.children.at(L"start_button")->color, ColorPalette.BorderColor, &(*gui.corbelButton));
				if(AboutCreated)
				{
					if(hwnd == AboutWData.children.at(L"back_button")->handle) GUI::WM_Paint(hwnd, hdc, ColorPalette.TextColor, AboutWData.children.at(L"back_button")->color, ColorPalette.BorderColor, &(*gui.corbelButton));
        		}
        		if(StartCreated)
        		{
        			if(hwnd == StartWData.children.at(L"NewProfileButton")->handle)
        			{
						COLORREF BkColor = 0, BorderColor = 0, TextColor = 0;
						if (NewProfileAllowed)
						{
							BkColor = StartWData.children.at(L"NewProfileButton")->color;
							BorderColor = ColorPalette.BorderColor;
							TextColor = ColorPalette.TextColor;
						}
						else
						{
							BkColor = ColorPalette.BoxColorDisabled;
							BorderColor = ColorPalette.BorderColorDisabled;
							TextColor = ColorPalette.BorderColorDisabled;
						}
						GUI::WM_Paint(hwnd, hdc, TextColor, BkColor, BorderColor, &(*gui.corbelButton), 4.1);
        			}
					else if(hwnd == StartWData.children.at(L"back_button")->handle)
						GUI::WM_Paint(hwnd, hdc, ColorPalette.TextColor, StartWData.children.at(L"back_button")->color, ColorPalette.BorderColor, &(*gui.corbelButton));
					else
					{
						if (NewProfileCreated)
						{
							if (hwnd == NewProfileWData.children.at(L"CreateButton")->handle)
								GUI::WM_Paint(hwnd, hdc, ColorPalette.TextColor, NewProfileWData.children.at(L"CreateButton")->color, ColorPalette.BorderColor, &(*gui.corbelButton), 4.0);
							else if (hwnd == NewProfileWData.children.at(L"back_button")->handle)
								GUI::WM_Paint(hwnd, hdc, ColorPalette.TextColor, NewProfileWData.children.at(L"back_button")->color, ColorPalette.BorderColor, &(*gui.corbelButton));
						}
						if (DashboardCreated)
						{
							if (hwnd == DashboardWData.children.at(L"back_button")->handle)
							{
								GUI::WM_Paint(hwnd, hdc, ColorPalette.TextColor, DashboardWData.children.at(L"back_button")->color, ColorPalette.BorderColor, &(*gui.corbelButton));
							}
							else if (LessonPageCreated)
							{
								if ( (LessonPageWData.children.find(L"back_button") != LessonPageWData.children.end()) && (hwnd == LessonPageWData.children.at(L"back_button")->handle) )
									GUI::WM_Paint(hwnd, hdc, ColorPalette.TextColor, LessonPageWData.children.at(L"back_button")->color, ColorPalette.BorderColor, &(*gui.corbelButton), 4.0);
								else if ( (LessonPageWData.children.find(L"CCBUTTON") != LessonPageWData.children.end()) && (hwnd == LessonPageWData.children.at(L"CCBUTTON")->handle) )
									GUI::WM_Paint(hwnd, hdc, ColorPalette.TextColor, LessonPageWData.children.at(L"CCBUTTON")->color, ColorPalette.BorderColor, &(*gui.corbelButton), 4.0);
							}
						}
					}
				}
				EndPaint(hwnd, &ps);
				break;
			}
			default:
				return DefSubclassProc(hwnd, uMsg, wParam, lParam);
		}
		return 0;
	}

	LRESULT CALLBACK BlueButtonSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		switch (uMsg)
		{
			case WM_PAINT:
			{
				PAINTSTRUCT ps = {0};
				HDC hdc = BeginPaint(hwnd, &ps);
				if (readyStart)
				{
					for (auto const & child_pair : StartWData.children)
					{
						if (child_pair.first.find(L"Groutfit") != std::wstring::npos)
						{
							if (hwnd == child_pair.second->children.at(L"DeleteButton")->handle)
							{
								const COLORREF DeleteButtonColor = child_pair.second->children.at(L"DeleteButton")->color;
								GUI::WM_Paint(hwnd, hdc, ColorPalette.BoxColorSelected, DeleteButtonColor, DeleteButtonColor, &(*gui.corbelButton), 4.0, 4.0);
							}
							else if (hwnd == child_pair.second->children.at(L"GoButton")->handle)
							{
								const COLORREF GoButtonColor = child_pair.second->children.at(L"GoButton")->color;
								GUI::WM_Paint(hwnd, hdc, ColorPalette.BoxColorSelected, GoButtonColor, GoButtonColor, &(*gui.corbelButton), 4.0, 4.0);
							}
						}
					}
				}
				break;
			}
			case WM_LBUTTONUP: //button clicked
			{
				if (readyStart)
				{
					for (auto const & child_pair : StartWData.children)
					{
						if (child_pair.first.find(L"Groutfit") != std::wstring::npos)
						{
							if (child_pair.second->children.at(L"DeleteButton")->visible && hwnd == child_pair.second->children.at(L"DeleteButton")->handle)
								StartPage::DeleteButtonWM_LButtonUp(&StartWData, ColorPalette, hwnd, child_pair.second->handle, gui.getPage(), &NewProfileAllowed);
							else if (child_pair.second->children.at(L"GoButton")->visible && hwnd == child_pair.second->children.at(L"GoButton")->handle)
								StartPage::GoButtonWM_LButtonUp(hwnd, StartWData.handle);
						}
					}
				}
				break;
			}
			default:
				return DefSubclassProc(hwnd, uMsg, wParam, lParam);
		}
		return 0;
	}

	LRESULT CALLBACK TextBoxSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		switch(uMsg)
		{
    		case WM_KEYDOWN:
    		{
    			if(readyStart)
				{
					if(readyNewProfile)
					{
						if(readyNewProfileBody)
						{
							if(hwnd == NewProfileWData.children.at(L"TextBox")->handle) return NewProfilePage::TextBoxWM_KeyDown(&NewProfileWData, hwnd, uMsg, wParam, lParam);
						}
					}
				}
    			break;
    		}
    		case WM_PASTE:
    		{
    			// disable copypasta by overriding default behaviour and doing absolutely nothing
    			break;
    		}
			case WM_SHOWWINDOW:
			{
				if (lParam == 0) // ShowWindow was called
				{
					if (static_cast<bool>(wParam) == false) // SW_HIDE
					{
						InvalidateRect(NewProfileWData.handle, nullptr, false);
					}
				}
				break;
			}
			default:
				return DefSubclassProc(hwnd, uMsg, wParam, lParam);
		}
		return 0;
	}

	LRESULT CALLBACK GroutfitWndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) //gray bordered box procedure
	{
		switch(Message)
		{
			case WM_PAINT:
			{
				PAINTSTRUCT ps = {0};
				HDC hdc = BeginPaint(hwnd, &ps);
				if(readyAboutGroutfit)
				{
					if(hwnd == AboutWData.children.at(L"about_body")->handle) GUI::WM_Paint(hwnd, hdc, 0, AboutWData.children.at(L"about_body")->color, ColorPalette.BorderColor, nullptr, 1.0, 1.0);
				}
				if(readyStart)
				{
            		for(auto const & child_pair : StartWData.children)
            		{
            			if(hwnd == child_pair.second->handle) StartPage::RowsWM_Paint(&CurrentProfile, ColorPalette, hwnd, hdc, ColorPalette.TextColor, child_pair.second->color, ColorPalette.BorderColor, &(*gui.corbelButton), 20.0, 20.0, Profiles::MaxProfileNameLength + 1);
            		}
					if (readyDashboard)
					{
						if (readyDashboardBody)
						{
							if (readyLessonPage)
							{
								if ((LessonPageWData.children.find(L"SCBOX") != LessonPageWData.children.end()) && (hwnd == LessonPageWData.children.at(L"SCBOX")->handle))
									LP->StaticSCBoxWM_Paint(hwnd, hdc, LessonPageWData.children.at(L"SCBOX")->color, ColorPalette.Green, &(*gui.monoCode));
							}
							for (auto const & child_pair : DashboardWData.children.at(L"container")->children)
							{
								if (hwnd == child_pair.second->handle)
									Dashboard::WM_Paint(hwnd, hdc, ColorPalette.TextColor, ColorPalette.BoxColor, &(*gui.corbelTitle));
							}
						}
					}
				}
				EndPaint(hwnd, &ps);
				break;
			}
			case WM_ERASEBKGND:  // becase roundrect corners
			{
				break;
			}
			case WM_CTLCOLORSTATIC:
			{
				HWND subjectHWND = reinterpret_cast<HWND>(lParam);
				HDC hdc = reinterpret_cast<HDC>(wParam);
				if(readyAboutTextColor == true)
				{
					if(subjectHWND == AboutWData.children.at(L"about_body")->children.at(L"about_body_text")->handle)
						return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, AboutWData.children.at(L"about_body")->color);
				}
				break;
			}
			case WM_LBUTTONUP:
			{
				if (readyStart)
				{
					for (auto const & child_pair : StartWData.children)
					{
						if (child_pair.second->handle == hwnd) StartPage::RowsWM_LButtonUp(&StartWData, &CurrentProfile, ColorPalette, hwnd);
					}
				}
				break;
			}
			case WM_VSCROLL:
			{
				if (readyStart)
				{
					if (readyDashboard)
					{
						if (readyLessonPageBody)
						{
							if ((LessonPageWData.children.find(L"SCBOX") != LessonPageWData.children.end()) && (hwnd == LessonPageWData.children.at(L"SCBOX")->handle))
							{
								LP->StaticSCBoxWM_VScroll(hwnd, wParam);
								return 0;
							}
						}
					}
				}
				break;
			}
			default:
				return DefWindowProc(hwnd, Message, wParam, lParam);
		}
		return 0;
	}

	LRESULT CALLBACK LightGroutfitWndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) // light gray bordered box procedure
	{
		switch(Message)
		{
			case WM_PAINT:
			{
				PAINTSTRUCT ps = {0};
				HDC hdc = BeginPaint(hwnd, &ps);
				if(readyStart)
				{
            		for(auto const & child_pair : StartWData.children)
            		{
            			if(hwnd == child_pair.second->handle) StartPage::RowsWM_Paint(&CurrentProfile, ColorPalette, hwnd, hdc, ColorPalette.TextColor, child_pair.second->color, ColorPalette.BorderColor, &(*gui.corbelButton), 20.0, 20.0, Profiles::MaxProfileNameLength + 1);
            		}
					if (readyDashboard)
					{
						if (readyDashboardBody)
						{
							if (readyLessonPage)
							{
								if ((LessonPageWData.children.find(L"LBOX") != LessonPageWData.children.end()) && (hwnd == LessonPageWData.children.at(L"LBOX")->handle))
									LP->LBoxWM_Paint(LessonPageWData, hwnd, hdc, ColorPalette.TextColor, LessonPageWData.children.at(L"LBOX")->color, ColorPalette.BorderColor, &(*gui.corbelButton), 20.0, 20.0, LessonPage::MaxElemTextLength, DT_LEFT, DT_TOP, true);
								else if ((LessonPageWData.children.find(L"BIGBOX") != LessonPageWData.children.end()) && (hwnd == LessonPageWData.children.at(L"BIGBOX")->handle))
									LP->BigBoxWM_Paint(LessonPageWData, hwnd, hdc, ColorPalette.TextColor, LessonPageWData.children.at(L"BIGBOX")->color, ColorPalette.BorderColor, &(*gui.corbelButton), 20.0, 20.0, LessonPage::MaxElemTextLength, DT_LEFT, DT_TOP, true);

							}
							for (auto const & child_pair : DashboardWData.children.at(L"container")->children)
							{
								if (child_pair.first.find(L"menu_") != std::wstring::npos && hwnd == child_pair.second->handle)
									Dashboard::WM_Paint(hwnd, hdc, 0, child_pair.second->color, &(*gui.corbelButton));
							}
						}
					}
				}
				EndPaint(hwnd, &ps);
				break;
			}
			case WM_CTLCOLORSTATIC:
			{
				HDC hdc = reinterpret_cast<HDC>(wParam);
				HWND subjectHWND = reinterpret_cast<HWND>(lParam);
				if (readyStart)
				{
					if (readyDashboard)
					{
						if (readyDashboardBody)
						{
							for (auto const & child_pair : DashboardWData.children.at(L"container")->children)
							{
								for (auto const & child_pair2 : child_pair.second->children)
								{
									if (subjectHWND == child_pair2.second->handle) return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, child_pair2.second->color);
								}
							}
						}
					}
				}
				break;
			}
			case WM_ERASEBKGND:  // becase roundrect corners
			{
				break;
			}
			case WM_LBUTTONUP:
			{
				if (StartCreated)
				{
					for (auto const & child_pair : StartWData.children)
					{
						if (child_pair.second->handle == hwnd)
						{
							StartPage::RowsWM_LButtonUp(&StartWData, &CurrentProfile, ColorPalette, hwnd);
							break;
						}
					}
				}
				break;
			}
			case WM_VSCROLL:
			{
				if (readyStart)
				{
					if (readyDashboard)
					{
						if (readyDashboardBody)
						{
							if (DashboardWData.children.at(L"container")->visible)
							{
								Dashboard::MenuWM_VScroll(&DashboardWData, hwnd, wParam);
								return 0;
							}
							else if (readyLessonPageBody)
							{
								if ( (LessonPageWData.children.find(L"LBOX") != LessonPageWData.children.end()) && (hwnd == LessonPageWData.children.at(L"LBOX")->handle) )
								{
									LP->LBoxWM_VScroll(hwnd, wParam);
									return 0;
								}
								else if ( (LessonPageWData.children.find(L"BIGBOX") != LessonPageWData.children.end()) && (hwnd == LessonPageWData.children.at(L"BIGBOX")->handle) )
								{
									LP->BigBoxWM_VScroll(hwnd, wParam);
									return 0;
								}
							}
						}
					}
				}
				break;
			}
			case WM_COMMAND:
			{
				const HWND subjectHWND = reinterpret_cast<HWND>(lParam);
				const UINT_PTR state = wParam;
				if (state == STN_CLICKED)
				{
					if (DashboardCreated)
					{
						for (auto const & menu_pair : DashboardWData.children.at(L"container")->children)
						{
							for (auto const & menu_item_pair : menu_pair.second->children)
							{
								if (menu_item_pair.first.find(L"lesson_") != std::wstring::npos && subjectHWND == menu_item_pair.second->handle)
								{
									SendMessage(DashboardWData.handle, Message, wParam, lParam); // if this is a lesson menu item, refer the message to the main window
									return 0;
								}
								else if (menu_item_pair.first.find(L"lessongroup_") != std::wstring::npos && subjectHWND == menu_item_pair.second->handle)
								{
									const size_t underscorePos = menu_item_pair.first.find(L'_');
									const std::wstring lessonGroupArrowName = L"lessongrouparrow" + menu_item_pair.first.substr(underscorePos);
									SendMessage(DashboardWData.children.at(L"container")->children.at(menu_pair.first)->children.at(lessonGroupArrowName)->handle, WM_LBUTTONUP, 0, 0); // if this is a lesson group menu item, refer the message to the lesson group arrow
									return 0;
								}
							}
						}
					}
				}
				break;
			}
			default:
				return DefWindowProc(hwnd, Message, wParam, lParam);
		}
		return 0;
	}

	LRESULT CALLBACK ContainerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) // container window procedure
	{
		switch (uMsg)
		{
			case WM_PAINT:
			{
				PAINTSTRUCT ps = { 0 };
				HDC hdc = BeginPaint(hwnd, &ps);
				if (readyStart)
				{
					if (readyDashboard)
					{
						if (hwnd == DashboardWData.children.at(L"container")->handle)
							Dashboard::WM_Paint(hwnd, hdc, 0, DashboardWData.children.at(L"container")->color, &(*gui.corbelButton));
						else if (readyLessonPage)
						{
							if (hwnd == LessonPageWData.children.at(L"container")->handle)
								GUI::WM_Paint(hwnd, hdc, ColorPalette.TextColor, ColorPalette.LightGray, LessonPageWData.children.at(L"container")->color, &(*gui.verdanaNormal));
						}
					}
				}
				EndPaint(hwnd, &ps);
				break;
			}
			case WM_HSCROLL:
			{
				if (readyStart)
				{
					if (readyDashboard)
					{
						if (readyDashboardBody && !readyLessonPage) Dashboard::ContainerWM_HScroll(&DashboardWData, LanguageMap.size(), hwnd, wParam);
						if (readyLessonPage) DefWindowProc(hwnd, uMsg, wParam, lParam);
					}
				}
				break;
			}
			case WM_CTLCOLORSTATIC:
			{
				HDC hdc = reinterpret_cast<HDC>(wParam);
				HWND subjectHWND = reinterpret_cast<HWND>(lParam);
				if ((LessonPageWData.children.find(L"container") != LessonPageWData.children.end())
					&& (LessonPageWData.children.at(L"container")->children.find(L"CCMessage") != LessonPageWData.children.at(L"container")->children.end())
					&& (subjectHWND == LessonPageWData.children.at(L"container")->children.at(L"CCMessage")->handle))
				{
					return GUI::WM_CTLColorStatic(hdc, ColorPalette.TextColor, ColorPalette.LightGray);
				}
			}
			default:
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		return 0;
	}

	LRESULT CALLBACK ImageWndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) // Image window procedure
	{
		switch (Message)
		{
			case WM_PAINT:
			{
				PAINTSTRUCT ps = { 0 };
				HDC hdc = BeginPaint(hwnd, &ps);
				if (readyStart)
				{
					if (readyDashboard)
					{
						if (readyDashboardBody)
						{
							bool done = false;
							for (auto const & child_pair : DashboardWData.children.at(L"container")->children)
							{
								for (auto const & child_pair2 : child_pair.second->children)
								{
									if (child_pair2.first == L"arrow" && hwnd == child_pair2.second->handle)
									{
										Dashboard::HeaderArrowWM_Paint(&DashboardWData, child_pair.first.substr(child_pair.first.find(L'_') + 1), hdc, child_pair2.second->width, child_pair2.second->height);
										done = true;
										break;
									}
									else if (child_pair2.first.find(L"lessongrouparrow_") != std::wstring::npos && hwnd == child_pair2.second->handle)
									{
										Dashboard::LessonGroupArrowWM_Paint(child_pair, child_pair2, hdc);
										done = true;
										break;
									}
								}
								if (done) break;
							}
						}
					}
				}
				EndPaint(hwnd, &ps);
				break;
			}
			case WM_LBUTTONUP:
			{
				if (readyStart)
				{
					if (readyDashboard)
					{
						if (readyDashboardBody)
						{
							bool done = false;
							for (auto const & menu_pair : DashboardWData.children.at(L"container")->children)
							{
								for (auto const & arrow_pair : menu_pair.second->children)
								{
									if (arrow_pair.first == L"arrow" && hwnd == arrow_pair.second->handle)
									{
										Dashboard::HeaderArrowWM_LButtonUp(DashboardWData, gui, menu_pair, arrow_pair, hwnd, ColorPalette.BoxColor);
										done = true;
										break;
									}
									else if (arrow_pair.first.find(L"lessongrouparrow_") != std::wstring::npos && hwnd == arrow_pair.second->handle)
									{
										Dashboard::LessonGroupWM_LButtonUp(DashboardWData, LanguageMap, menu_pair, arrow_pair, hwnd, &(*gui.corbelButton));
										done = true;
										break;
									}
								}
								if (done) break;
							}
						}
					}
				}
				break;
			}
			default:
				return DefWindowProc(hwnd, Message, wParam, lParam);
		}
		return 0;
	}

	LRESULT CALLBACK SCEditSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		switch (uMsg)
		{
			case WM_LBUTTONDOWN:
			{
				if (LP->getSCBoxReadOnly())
				{
					HideCaret(hwnd);
				}
				else return DefSubclassProc(hwnd, uMsg, wParam, lParam);
				break;
			}
			case WM_CHAR:
			{
				const LRESULT ret = DefSubclassProc(hwnd, uMsg, wParam, lParam);
				if (wParam == VK_RETURN)
				{
					LP->SCEditSHUpdate();
				}
				return ret;
			}
			default:
				return DefSubclassProc(hwnd, uMsg, wParam, lParam);
		}
		return 0;
	}
}