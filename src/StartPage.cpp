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

// STL Headers
#include <string>
#include <sstream>
#include <algorithm> // std::sort

// Windows Headers
#define UNICODE
#include <windows.h>

// Program Headers
#include "StartPage.hpp"
#include "misc.hpp"
#include "Profiles.hpp"
#include "WProc.hpp"
#include "AppData.hpp"

namespace ASP
{
	bool StartPage::TableVectorSorter(HWND lhs, HWND rhs) noexcept
	{
		RECT WindRectL = {};
		RECT WindRectR = {};
		GetWindowRect(lhs, &WindRectL);
		GetWindowRect(rhs, &WindRectR);
		return WindRectL.top < WindRectR.top;
	}

	void StartPage::CreateStartBody(WindowData *data, const Palette& ColorPalette, const GUI::Pages page, bool *NewProfilesAllowed) // Start Page Body
	{
		if (page == GUI::Pages::START)
		{
			const int Nwidth = static_cast<int>(data->width * 0.4);
			const int Nheight = static_cast<int>(data->height * 0.05);
			const int x = (data->width - Nwidth) / 2;
			int y = data->children.at(L"title")->y + (data->children.at(L"title")->height) + static_cast<int>(data->height * 0.1);
			const wchar_t * class1 = L"Groutfit";
			const wchar_t * class2 = L"Light Groutfit";
			Profiles Prof;
			std::vector<Profiles::Profile> ExistingProfiles;
			if (!Prof.LoadProfiles(&ExistingProfiles))
			{
				SendMessage(data->handle, WM_DESTROY, 0, 0);
			}
			std::vector<HWND> handles;
			for (int i = 0; i < Profiles::MaxProfiles; i++)
			{
				const wchar_t * WClass = (i % 2 == 1) ? class2 : class1;
				const COLORREF Color = (i % 2) ? ColorPalette.LightGray : ColorPalette.BoxColor;
				std::wostringstream ss;
				ss << WClass << i;
				std::wstring key = ss.str();
				HWND h = CreateWindowEx(0, WClass, L"", SS_CENTER | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD | SS_NOTIFY, x, y, Nwidth, Nheight, data->handle, nullptr, nullptr, nullptr);
				handles.push_back(h);
				std::unique_ptr<WindowData> wd_ptr = std::make_unique<WindowData>(h, ChildList(), Color, Nheight, Nwidth, x, y, data, true);
				CheckEmplace(data->children.emplace(key, std::move(wd_ptr)), L" start page " + key);
				std::unique_ptr<WindowData> dummy_ptr_delete = std::make_unique<WindowData>();
				std::unique_ptr<WindowData> dummy_ptr_go = std::make_unique<WindowData>();
				CheckEmplace(data->children.at(key)->children.emplace(L"DeleteButton", std::move(dummy_ptr_delete)), key + L" delete button");
				CheckEmplace(data->children.at(key)->children.emplace(L"GoButton", std::move(dummy_ptr_go)), key + L" go button");
				y += Nheight;
			}
			for (size_t i = 0; i < ExistingProfiles.size(); i++)
			{
				const Profiles::Profile curProfile = ExistingProfiles[i];
				SetWindowText(handles[i], static_cast<const wchar_t*>(curProfile.profile_name));
			}
			if (ExistingProfiles.size() == Profiles::MaxProfiles)
			{
				*NewProfilesAllowed = false;
			}
		}
	}

	void StartPage::UpdateStartBody(WindowData *data, GUI::Pages page, bool *NewProfilesAllowed) // Updates the text of the start body
	{
		if (page == GUI::Pages::NEW_PROFILE)
		{
			// get relevant windows
			std::vector<HWND> RelevantChildren = {};
			for (auto const & Child : data->children)
			{
				std::wstring key = Child.first;
				if (key.find(L"Groutfit") != std::wstring::npos) // is it one of said windows
				{
					RelevantChildren.push_back(data->children.at(key)->handle);
				}
			}
			std::sort(RelevantChildren.begin(), RelevantChildren.end(), StartPage::TableVectorSorter);

			// get profiles
			std::vector<Profiles::Profile> ExistingProfiles = {};
			Profiles Prof;
			Prof.LoadProfiles(&ExistingProfiles);

			//update the window text
			for (size_t i = 0; i < ExistingProfiles.size(); i++)
			{
				SetWindowText(RelevantChildren[i], ExistingProfiles[i].profile_name);
				InvalidateRect(RelevantChildren[i], nullptr, false);
			}
			for (size_t i = ExistingProfiles.size(); i <= Profiles::MaxProfiles; i++)
			{
				std::wcout << L"yo\n";
				SetWindowText(RelevantChildren[i], L"");
				InvalidateRect(RelevantChildren[i], nullptr, false);
			}
			if (ExistingProfiles.size() < Profiles::MaxProfiles) *NewProfilesAllowed = true;
			else *NewProfilesAllowed = true;
		}
	}

	void StartPage::UpdateSelectedProfileButtons(WindowData *data, const Palette& ColorPalette, std::wstring& SelectedProfileName) // Add buttons to the selected profile row
	{
		for (auto const & child_pair : data->children) //iterate through all the windows
		{
			if (child_pair.first.find(L"Groutfit") != std::wstring::npos) //only the ones that we care about
			{
				//step one: clear any existing profile buttons
				for (auto const & buttons_pair : child_pair.second->children)
				{
					wchar_t * ButtonText = new wchar_t[17];
					GetWindowText(buttons_pair.second->handle, ButtonText, 17);
					if ((std::wstring(ButtonText) == L"X") || (std::wstring(ButtonText) == L"GO"))
					{
						ShowWindow(buttons_pair.second->handle, SW_HIDE);
						DestroyWindow(buttons_pair.second->handle);
					}
					InvalidateRect(child_pair.second->handle, nullptr, false);
					delete[] ButtonText;
				}
				//step two: set the profile buttons for the selected profile row
				wchar_t * WindowText = new wchar_t[Profiles::MaxProfileNameLength + 1];
				GetWindowText(child_pair.second->handle, WindowText, Profiles::MaxProfileNameLength + 1);
				if (std::wstring(WindowText) == SelectedProfileName)
				{
					//step 2.1: common metrics
					const int Height = child_pair.second->height / 2;
					const int TotalWidth = child_pair.second->width / 4;
					const int X = child_pair.second->width - TotalWidth;
					const int Y = ((child_pair.second->height - Height) / 2);
					//step 2.2: delete button
					const int NheightDelete = Height;
					const int NwidthDelete = TotalWidth / 3;
					const int xDelete = X + TotalWidth / 12;
					const int yDelete = Y;
					HWND hDelete = CreateWindowEx(0, L"STATIC", L"X", SS_CENTER | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD | SS_NOTIFY, xDelete, yDelete, NwidthDelete, NheightDelete, child_pair.second->handle, nullptr, nullptr, nullptr);
					data->children.at(child_pair.first)->children.erase(L"DeleteButton");
					std::unique_ptr<WindowData> wd_ptr_del = std::make_unique<WindowData>(hDelete, ChildList(), ColorPalette.BlueButtonColor, NheightDelete, NwidthDelete, xDelete, yDelete, data, true);
					CheckEmplace(data->children.at(child_pair.first)->children.emplace(L"DeleteButton", std::move(wd_ptr_del)), child_pair.first + L" delete button");
					SetWindowSubclass(hDelete, BlueButtonSubclass, 0, 0);
					//step 2.3: GO button
					const int NheightGO = Height;
					const int NwidthGO = TotalWidth / 3;
					const int xGO = X + NwidthDelete + (TotalWidth / 3) - TotalWidth / 12;
					const int yGO = Y;
					HWND hGO = CreateWindowEx(0, L"STATIC", L"GO", SS_CENTER | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD | SS_NOTIFY, xGO, yGO, NwidthGO, NheightGO, child_pair.second->handle, nullptr, nullptr, nullptr);
					data->children.at(child_pair.first)->children.erase(L"GoButton");
					std::unique_ptr<WindowData> wd_ptr_go = std::make_unique<WindowData>(hGO, ChildList(), ColorPalette.BlueButtonColor, NheightGO, NwidthGO, xGO, yGO, data, true);
					CheckEmplace(data->children.at(child_pair.first)->children.emplace(L"GoButton", std::move(wd_ptr_go)), child_pair.first + L" go button");
					SetWindowSubclass(hGO, BlueButtonSubclass, 0, 0);
				}
				delete[] WindowText;
			}
		}
	}

	void StartPage::CreateNewProfileButton(WindowData *data, const Palette& ColorPalette, GUI::Pages page)
	{
		if (page == GUI::Pages::START)
		{
			int Nwidth = static_cast<int>(data->width * 0.1);
			int Nheight = static_cast<int>(Nwidth * 0.33333);
			int x = (data->width / 2) - (Nwidth / 2);
			std::wstringstream key_stream;
			std::wstring key_class;
			if ((Profiles::MaxProfiles - 1) % 2 == 0)
			{
				key_class = L"Groutfit";
			}
			else if ((Profiles::MaxProfiles - 1) % 2 == 1)
			{
				key_class = L"Light Groutfit";
			}
			key_stream << (key_class) << (Profiles::MaxProfiles - 1);
			const int LastWindowY = data->children.at(key_stream.str())->y;
			const int LastWindowHeight = data->children.at(key_stream.str())->height;
			const int y = LastWindowY + LastWindowHeight + static_cast<int>(data->height * 0.01);
			HWND h = CreateWindowEx(0, L"STATIC", L"New Profile", SS_CENTER | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD | SS_NOTIFY, x, y, Nwidth, Nheight, data->handle, nullptr, nullptr, nullptr);
			std::unique_ptr<WindowData> wd_ptr = std::make_unique<WindowData>(h, ChildList(), ColorPalette.BoxColor, Nheight, Nwidth, x, y, data, true);
			CheckEmplace(data->children.emplace(L"NewProfileButton", std::move(wd_ptr)), L"new profile button");
			SetWindowSubclass(h, ButtonSubclass, 0, 0);
		}
	}

	void StartPage::DeleteButtonWM_LButtonUp(WindowData* WData, const Palette& ColorPalette, const HWND hwnd, const HWND child_pair_HWND, const GUI::Pages page, bool *NewProfileAllowed)
	{
		wchar_t* ProfileName = new wchar_t[Profiles::MaxProfileNameLength + 1];
		GetWindowText(child_pair_HWND, ProfileName, Profiles::MaxProfileNameLength + 1);
		std::wostringstream ConfirmQuestion;
		ConfirmQuestion << L"Do you really want to delete profile \"" << ProfileName << L"\"? This action is not reversible.";
		int ConfirmChoice = MessageBox(hwnd, ConfirmQuestion.str().c_str(), L"Delete Profile", MB_ICONEXCLAMATION | MB_YESNO);
		if (ConfirmChoice == IDYES) // if yes, then delete the profile and force update
		{
			Profiles Prof;
			// step one: update the main profiles file (profiles.bin)
			std::vector<Profiles::Profile> AllTheProfiles = {};
			if (!Prof.LoadProfiles(&AllTheProfiles)) return;
			HANDLE proFILE = CreateFile(Profiles::ProfileFileName, GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS | TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (proFILE == INVALID_HANDLE_VALUE)
			{
				Error(L"Load Profile (W)");
				return;
			}
			DWORD BytesWritten = 0;
			for (auto const & aProfile : AllTheProfiles) // this loop rewrites the profiles.bin by pushing all the profile data 
			{
				if (std::wstring(aProfile.profile_name) == std::wstring(ProfileName)) continue;
				if (!WriteFile(proFILE, &aProfile, sizeof(Profiles::Profile), &BytesWritten, nullptr))
				{
					Error(L"Write Profile (W)");
					std::wcout << L"Profile Error Dump || " << aProfile.cur_lang_id << L" || " << aProfile.cur_lesson_id << L" || " << aProfile.profile_id << L" || " << aProfile.profile_name << L'\n';
				}
			}
			CloseHandle(proFILE);
			//step two: update the app data file (data.bin)
			AppData AD = AppData();
			AppData::Data data = {};
			AD.ReadData(&data);
			data.NumProfiles--;
			AD.WriteData(data);

			//step three: update the table on the profiles page
			StartPage::UpdateStartBody(WData, page, NewProfileAllowed);
			std::wstring TestString = L"Impossible String @#$";
			StartPage::UpdateSelectedProfileButtons(WData, ColorPalette, TestString);
		}
		//do nothing if the answer is no or the box is closed
		delete[] ProfileName;
	}

	void StartPage::GoButtonWM_LButtonUp(const HWND subjectHWND, const HWND parentHWND)
	{
		SendMessage(parentHWND, WM_COMMAND, 0, reinterpret_cast<LPARAM>(subjectHWND));
	}

	void StartPage::RowsWM_Paint(Profiles* CurrentProfile, const Palette& ColorPalette, const HWND hwnd, const HDC hdc, const COLORREF TextColor, COLORREF BkColor, COLORREF BorderColor, const HFONT font, const double xMarginDivisor, const double yMarginDivisor, const unsigned int textLength)
	{
		SendMessage(hwnd, WM_ERASEBKGND, reinterpret_cast<WPARAM>(hdc), reinterpret_cast<LPARAM>(nullptr));
		wchar_t* WindowText = new wchar_t[Profiles::MaxProfileNameLength + 1];
		GetWindowText(hwnd, WindowText, Profiles::MaxProfileNameLength + 1);
		if ((CurrentProfile->SelectedProfile == std::wstring(WindowText)) && (CurrentProfile->SelectedProfile != L"")) // if this row is selected, make it blue
		{
			BkColor = ColorPalette.BoxColorSelected;
			BorderColor = ColorPalette.BorderColorSelected;
		}
		delete[] WindowText;
		GUI::WM_Paint(hwnd, hdc, TextColor, BkColor, BorderColor, font, xMarginDivisor, yMarginDivisor, textLength, DT_LEFT);
	}

	void StartPage::RowsWM_LButtonUp(WindowData* data, Profiles* CurrentProfile, const Palette& ColorPalette, const HWND hwnd)
	{
		wchar_t* WindowText = new wchar_t[Profiles::MaxProfileNameLength + 1];
		GetWindowText(hwnd, WindowText, Profiles::MaxProfileNameLength + 1);
		if (std::wstring(WindowText).empty()) // if empty row, do nothing
		{
			delete[] WindowText;
			return;
		}
		CurrentProfile->SelectedProfile = std::wstring(WindowText);
		for (auto const & child_pair2 : data->children)
		{
			if (child_pair2.first.find(L"Groutfit") != std::wstring::npos)
			{
				InvalidateRect(child_pair2.second->handle, nullptr, false);
			}
		}
		StartPage::UpdateSelectedProfileButtons(data, ColorPalette, CurrentProfile->SelectedProfile);
		delete[] WindowText;
	}
}