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
#include <utility> // std::make_unique
#include <tuple> // std::get
#include <thread>

// program headers
#include "LessonPage.hpp"
#include "LessonParser.hpp"
#include "SyntaxHighlighter.hpp"
#include "GUI.hpp"
#include "WProc.hpp"
#include "CodeChecker.hpp"

namespace ASP
{
	LessonPage::~LessonPage()
	{
		if (this->syntaxHighlighterThread->joinable()) this->syntaxHighlighterThread->join();
	}

	std::wstring LessonPage::getTitleStr() const noexcept
	{
		return this->curLangName + L": " + this->curLessonName;
	}

	bool LessonPage::getSCBoxReadOnly() const noexcept
	{
		return this->lessonData.SCReadOnly;
	}

	void LessonPage::SCEditSHUpdate()
	{
		SCEditBox->SHUpdate();
	}

	void LessonPage::loadLesson()
	{
		try
		{
			const std::wstring lessonFilePath = (L"Languages\\" + this->curLangID + L"_" + this->curLangName + L"\\" + this->curLGID + L" " + this->curLGName + L"\\" + this->curLangName + L"_" + this->curLessonID + L".txt");
			LessonParser Parser(lessonFilePath, this->langs);
			Parser.parse();
			this->lessonData = Parser.getLessonData();
			Parser.debugLessonData(this->lessonData);
		}
		catch (int err)
		{
			const std::wstring errStr = L"LessonParser error: " + std::to_wstring(err);
			Error(errStr.c_str());
			std::wcout << errStr << L'\n';
		}
	}

	void LessonPage::createLessonPageBody(WindowData& wData, const Palette& ColorPalette, const HFONT font)
	{
		if (!this->lessonData.LBox && !this->lessonData.SCBox && !this->lessonData.BigBox)
		{
			const std::wstring errStr = L"No Lesson Elements To Create";
			Error(errStr.c_str());
			std::wcout << errStr << L'\n';
		}
		// common metrics
		Metric bodyWidth = static_cast<Metric>(wData.width * 0.8);
		Metric bodyX = (wData.width - bodyWidth) / 2;
		Metric spacerX = static_cast<Metric>(wData.width * 0.05);
		Metric columnWidth = (bodyWidth - spacerX) / 2;
		Metric rColumnX = bodyX + columnWidth + spacerX;
		Metric spacerY = static_cast<Metric>(wData.height * 0.05);
		Metric bodyY = wData.children.at(L"title")->y + wData.children.at(L"title")->height;
		Metric bottomY = wData.children.at(L"copyright")->y - spacerY;
		Metric bodyHeight = bottomY - bodyY;
		// pick which gui elements to create based on the lesson data
		if (this->lessonData.BigBox) this->createBigBox(wData, ColorPalette.LightGray, bodyX, bodyY, bodyWidth, bodyHeight);
		else
		{
			if (this->lessonData.LBox) this->createLBox(wData, ColorPalette.LightGray, bodyX, bodyY, bodyWidth, bodyHeight, spacerX, columnWidth);
			if (this->lessonData.SCBox) this->createSCBox(wData, ColorPalette.Black, font, rColumnX, bodyY, bodyWidth, bodyHeight, spacerX, columnWidth);
			if (this->lessonData.CCButton) this->createCCButton(wData, ColorPalette.BoxColor, bodyX, bodyY, bodyWidth, bodyHeight, spacerX, columnWidth);
		}
	}

	void LessonPage::createLBox(WindowData& wData, const COLORREF BkColor, Metric bodyX, Metric bodyY, Metric bodyWidth, Metric bodyHeight, Metric spacerX, Metric columnWidth) const
	{
		HWND hLBox = CreateWindowEx(0, L"Light Groutfit", lessonData.LBoxData.c_str(), WS_VISIBLE | WS_CHILD | WM_VSCROLL, bodyX, bodyY, columnWidth, bodyHeight, wData.handle, nullptr, nullptr, nullptr);
		std::unique_ptr<WindowData> wd_ptr_LBox = std::make_unique<WindowData>(hLBox, ChildList(), BkColor, bodyHeight, columnWidth, bodyX, bodyY, &wData, true);
		CheckEmplace(wData.children.emplace(L"LBOX", std::move(wd_ptr_LBox)), L"LBOX");

		SCROLLINFO SCBoxScroll = {};
		SCBoxScroll.cbSize = sizeof(SCROLLINFO);
		SCBoxScroll.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
		SCBoxScroll.nMin = 0; // the range start.  It's zero-based, so start at 0
		SCBoxScroll.nMax = 1; // the last position in the range.  
		SCBoxScroll.nPos = 0;
		SCBoxScroll.nPage = 2; // have nPage > nMax to prevent the scrollbar from appearing initially
		SetScrollInfo(hLBox, SB_VERT, &SCBoxScroll, false);
		GetScrollInfo(hLBox, SB_VERT, &SCBoxScroll);
	}

	void LessonPage::createSCBox(WindowData& wData, const COLORREF BkColor, const HFONT font, Metric bodyX, Metric bodyY, Metric bodyWidth, Metric bodyHeight, Metric spacerX, Metric columnWidth)
	{
		Metric SCHeight = static_cast<Metric>(bodyHeight * 0.75);

		// start the syntax highlighter
		/*if (this->lessonData.SCReadOnly)
		{
			this->syntaxHighlighter = std::make_unique<SyntaxHighlighter>(SyntaxHighlighter());
			this->syntaxHighlighterThread = new std::thread(&LessonPage::setSyntaxHighlighter, this);

			// Create the window
			HWND hSCBox = CreateWindowEx(0, L"Groutfit", this->lessonData.SCBoxData.c_str(), WS_VISIBLE | WS_CHILD | WS_VSCROLL, bodyX, bodyY, columnWidth, SCHeight, wData.handle, nullptr, nullptr, nullptr);
			std::unique_ptr<WindowData> wd_ptr_SCBox = std::make_unique<WindowData>(hSCBox, ChildList(), BkColor, SCHeight, columnWidth, bodyX, bodyY, &wData, true);
			CheckEmplace(wData.children.emplace(L"SCBOX", std::move(wd_ptr_SCBox)), L"SCBOX");

			SCROLLINFO SCBoxScroll = {};
			SCBoxScroll.cbSize = sizeof(SCROLLINFO);
			SCBoxScroll.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
			SCBoxScroll.nMin = 0; // the range start.  It's zero-based, so start at 0
			SCBoxScroll.nMax = 1; // the last position in the range.  
			SCBoxScroll.nPos = 0;
			SCBoxScroll.nPage = 2; // have nPage > nMax to prevent the scrollbar from appearing initially
			SetScrollInfo(hSCBox, SB_VERT, &SCBoxScroll, false);
			GetScrollInfo(hSCBox, SB_VERT, &SCBoxScroll);
		}
		else
		{*/
			this->SCEditBox = std::make_unique<SCEdit>(SCEdit(wData, this->lessonData.SCLang, this->lessonData.SCLangID, this->lessonData.SCBoxData, bodyX, bodyY, columnWidth, SCHeight, wData.handle, BkColor, this->defTextColor, font, this->lessonData.SCReadOnly));
		//}
	}

	void LessonPage::createBigBox(WindowData& wData, const COLORREF BkColor, Metric bodyX, Metric bodyY, Metric bodyWidth, Metric bodyHeight) const
	{
		HWND hBigBox = CreateWindowEx(0, L"Light Groutfit", lessonData.BigBoxData.c_str(), WS_VISIBLE | WS_CHILD, bodyX, bodyY, bodyWidth, bodyHeight, wData.handle, nullptr, nullptr, nullptr);
		std::unique_ptr<WindowData> wd_ptr_BigBox = std::make_unique<WindowData>(hBigBox, ChildList(), BkColor, bodyHeight, bodyWidth, bodyX, bodyY, &wData, true);
		CheckEmplace(wData.children.emplace(L"BIGBOX", std::move(wd_ptr_BigBox)), L"BIGBOX");
	}

	void LessonPage::createCCButton(WindowData& wData, const COLORREF BkColor, Metric bodyX, Metric bodyY, Metric bodyWidth, Metric bodyHeight, Metric spacerX, Metric columnWidth) const
	{
		// Y-Axis Metrics
		Metric SCHeight = wData.children.at(L"SCBOX")->height;
		Metric CCHeight = (bodyHeight - SCHeight) / 3;
		Metric CCy = bodyY + SCHeight + ((bodyHeight - SCHeight - CCHeight) / 6);

		// X-Axis Metrics
		Metric CCWidth = columnWidth / 3;
		Metric CCx = bodyX + columnWidth + spacerX + ((columnWidth - CCWidth) / 2);

		// Make It So!
		HWND hCCButton = CreateWindowEx(0, L"STATIC", L"Check!", WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE | SS_NOTIFY, CCx, CCy, CCWidth, CCHeight, wData.handle, nullptr, nullptr, nullptr);
		std::unique_ptr<WindowData> wd_ptr_CCButton = std::make_unique<WindowData>(hCCButton, ChildList(), BkColor, CCHeight, CCWidth, CCx, CCy, &wData, true);
		CheckEmplace(wData.children.emplace(L"CCBUTTON", std::move(wd_ptr_CCButton)), L"CCBUTTON");
		SetWindowSubclass(hCCButton, ButtonSubclass, 0, 0);
	}

	void LessonPage::LBoxWM_Paint(const WindowData& data, const HWND hwnd, const HDC hdc, const COLORREF TextColor, const COLORREF BkColor, const COLORREF BorderColor, const HFONT font, const double xMarginDivisor, const double yMarginDivisor, const unsigned int textLength, const unsigned int textAlignX, const unsigned int textAlignY, const bool multiLine)
	{
		SCROLLINFO LBoxScroll = {};
		LBoxScroll.cbSize = sizeof(SCROLLINFO);
		LBoxScroll.fMask = SIF_POS;
		GetScrollInfo(hwnd, SB_VERT, &LBoxScroll);

		TEXTMETRIC textMetrics{};
		GetTextMetrics(hdc, &textMetrics);
		Metric lineHeight = textMetrics.tmHeight + textMetrics.tmExternalLeading;

		wchar_t* WindowText = new wchar_t[textLength];
		GetWindowText(hwnd, WindowText, textLength);

		// redraw client rect
		SetBkColor(hdc, BkColor);
		RECT rect;
		GetClientRect(hwnd, &rect);
		HBRUSH hBrush = CreateSolidBrush(BkColor);
		HPEN x = CreatePen(BS_SOLID, 1, BorderColor);
		SelectObject(hdc, hBrush);
		SelectObject(hdc, x);
		RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 9, 9);
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
		SelectObject(hdc, GetStockObject(NULL_PEN));
		DeleteObject(hBrush);
		DeleteObject(x);

		// redraw text
		const int rWidth = rect.right - rect.left;
		const int rHeight = rect.bottom - rect.top;
		rect.top -= LBoxScroll.nPos * lineHeight;
		RECT nRect = { rect.left + static_cast<LONG>(rWidth / xMarginDivisor), rect.top + static_cast<LONG>(rHeight / yMarginDivisor), rect.right - static_cast<LONG>(rWidth / xMarginDivisor), rect.bottom - static_cast<LONG>(rHeight / yMarginDivisor) };
		LOGFONT lf;
		GetObject(font, sizeof(LOGFONT), &lf);
		HFONT hf = CreateFontIndirect(&lf);
		HFONT oldf = static_cast<HFONT>(SelectObject(hdc, hf));
		SetTextColor(hdc, TextColor);
		unsigned int multiLineFlag = multiLine ? DT_WORDBREAK : DT_SINGLELINE;
		this->LBoxContentHeight = DrawTextEx(hdc, WindowText, -1, &nRect, textAlignX | textAlignY | multiLineFlag | DT_CALCRECT, nullptr);
		DrawTextEx(hdc, WindowText, -1, &nRect, textAlignX | textAlignY | multiLineFlag, nullptr);
		SelectObject(hdc, oldf);
		DeleteObject(hf);
		delete[] WindowText;

		Metric numLines = this->LBoxContentHeight / lineHeight;
		Metric LBoxHeight = data.children.at(L"LBOX")->height;
		Metric marginY = static_cast<Metric>(LBoxHeight / yMarginDivisor);
		Metric linesPerMarginY = static_cast<Metric>(std::round(marginY*1.0 / lineHeight));
		Metric maxVisibleLines = (LBoxHeight + 2 * marginY) / lineHeight;

		if (this->LBoxContentHeight > 0)
		{
			LBoxScroll.cbSize = sizeof(SCROLLINFO);
			LBoxScroll.fMask = SIF_RANGE | SIF_PAGE;
			LBoxScroll.nMin = 0; // the range start.  It's zero-based, so start at 0
			LBoxScroll.nMax = numLines + 2*linesPerMarginY; // the last position in the range.  
			LBoxScroll.nPos = 0;
			LBoxScroll.nPage = maxVisibleLines; // have nPage > nMax to prevent the scrollbar from appearing initially
			SetScrollInfo(hwnd, SB_VERT, &LBoxScroll, false);
			GetScrollInfo(hwnd, SB_VERT, &LBoxScroll);
		}
	}

	void LessonPage::LBoxWM_VScroll(const HWND hwnd, const WPARAM wParam) const
	{
		if (this->lessonData.LBox) GUI::WM_VScroll(hwnd, wParam, this->LBoxContentHeight);
	}

	void LessonPage::BigBoxWM_Paint(const WindowData& data, const HWND hwnd, const HDC hdc, const COLORREF TextColor, const COLORREF BkColor, const COLORREF BorderColor, const HFONT font, const double xMarginDivisor, const double yMarginDivisor, const unsigned int textLength, const unsigned int textAlignX, const unsigned int textAlignY, const bool multiLine)
	{
		SCROLLINFO BigBoxScroll = {};
		BigBoxScroll.cbSize = sizeof(SCROLLINFO);
		BigBoxScroll.fMask = SIF_POS;
		GetScrollInfo(hwnd, SB_VERT, &BigBoxScroll);

		TEXTMETRIC textMetrics{};
		GetTextMetrics(hdc, &textMetrics);
		Metric lineHeight = textMetrics.tmHeight + textMetrics.tmExternalLeading;

		wchar_t* WindowText = new wchar_t[textLength];
		GetWindowText(hwnd, WindowText, textLength);

		// redraw client rect
		SetBkColor(hdc, BkColor);
		RECT rect;
		GetClientRect(hwnd, &rect);
		HBRUSH hBrush = CreateSolidBrush(BkColor);
		HPEN x = CreatePen(BS_SOLID, 1, BorderColor);
		SelectObject(hdc, hBrush);
		SelectObject(hdc, x);
		RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 9, 9);
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
		SelectObject(hdc, GetStockObject(NULL_PEN));
		DeleteObject(hBrush);
		DeleteObject(x);

		// redraw text
		const int rWidth = rect.right - rect.left;
		const int rHeight = rect.bottom - rect.top;
		rect.top -= BigBoxScroll.nPos * lineHeight;
		RECT nRect = { rect.left + static_cast<LONG>(rWidth / xMarginDivisor), rect.top + static_cast<LONG>(rHeight / yMarginDivisor), rect.right - static_cast<LONG>(rWidth / xMarginDivisor), rect.bottom - static_cast<LONG>(rHeight / yMarginDivisor) };
		LOGFONT lf;
		GetObject(font, sizeof(LOGFONT), &lf);
		HFONT hf = CreateFontIndirect(&lf);
		HFONT oldf = static_cast<HFONT>(SelectObject(hdc, hf));
		SetTextColor(hdc, TextColor);
		unsigned int multiLineFlag = multiLine ? DT_WORDBREAK : DT_SINGLELINE;
		this->BigBoxContentHeight = DrawTextEx(hdc, WindowText, -1, &nRect, textAlignX | textAlignY | multiLineFlag | DT_CALCRECT, nullptr);
		DrawTextEx(hdc, WindowText, -1, &nRect, textAlignX | textAlignY | multiLineFlag, nullptr);
		SelectObject(hdc, oldf);
		DeleteObject(hf);
		delete[] WindowText;

		Metric numLines = this->BigBoxContentHeight / lineHeight;
		Metric BigBoxHeight = data.children.at(L"BIGBOX")->height;
		Metric marginY = static_cast<Metric>(BigBoxHeight / yMarginDivisor);
		Metric linesPerMarginY = static_cast<Metric>(std::round(marginY*1.0 / lineHeight));
		Metric maxVisibleLines = (BigBoxHeight + 2 * marginY) / lineHeight;

		if (this->BigBoxContentHeight > 0)
		{
			BigBoxScroll.cbSize = sizeof(SCROLLINFO);
			BigBoxScroll.fMask = SIF_RANGE | SIF_PAGE;
			BigBoxScroll.nMin = 0; // the range start.  It's zero-based, so start at 0
			BigBoxScroll.nMax = numLines + 2 * linesPerMarginY; // the last position in the range.  
			BigBoxScroll.nPos = 0;
			BigBoxScroll.nPage = maxVisibleLines; // have nPage > nMax to prevent the scrollbar from appearing initially
			SetScrollInfo(hwnd, SB_VERT, &BigBoxScroll, false);
			GetScrollInfo(hwnd, SB_VERT, &BigBoxScroll);
		}
	}

	void LessonPage::BigBoxWM_VScroll(const HWND hwnd, const WPARAM wParam) const
	{
		if (this->lessonData.BigBox) GUI::WM_VScroll(hwnd, wParam, this->BigBoxContentHeight);
	}

	void LessonPage::StaticSCBoxWM_Paint(const HWND hwnd, const HDC hdc, const COLORREF BkColor, const COLORREF BorderColor, const HFONT font)
	{
		// redraw client rect
		SetBkColor(hdc, BkColor);
		RECT rect;
		GetClientRect(hwnd, &rect);
		HBRUSH hBrush = CreateSolidBrush(BkColor);
		HPEN x = CreatePen(BS_SOLID, 1, BorderColor);
		SelectObject(hdc, hBrush);
		SelectObject(hdc, x);
		RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 9, 9);
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
		SelectObject(hdc, GetStockObject(NULL_PEN));
		DeleteObject(hBrush);
		DeleteObject(x);

		//-redraw-text-
		LOGFONT lf;
		GetObject(font, sizeof(LOGFONT), &lf);
		HFONT hf = CreateFontIndirect(&lf);
		HFONT oldf = static_cast<HFONT>(SelectObject(hdc, hf));
		//-Metrics-----
		LOGFONT fontData = {};
		GetObject(font, sizeof(LOGFONT), &fontData);
		//Metric letterWidth = fontData.lfWidth;
		SIZE size = {};
		GetTextExtentPoint32(hdc, L"M", 1, &size);
		Metric letterWidth = size.cx;
		Metric letterHeight = fontData.lfHeight;
		Metric spacerY = letterHeight / 3;
		Metric SCBoxWidth = rect.right - rect.left;
		Metric SCBoxHeight = rect.bottom - rect.top;
		Metric marginX = static_cast<Metric>(SCBoxWidth * 0.05);
		Metric marginY = static_cast<Metric>(SCBoxHeight * 0.05);
		unsigned int X = marginX;
		unsigned int Y = marginY;
		//-Thread-Sync-and-Scrollbar-
		SCROLLINFO SCBoxScroll{};
		SCBoxScroll.cbSize = sizeof(SCROLLINFO);
		Metric maxVisibleLines = static_cast<unsigned int>(std::round( (SCBoxHeight - marginY * 2.0) / (letterHeight + spacerY) ) );
		if (this->syntaxHighlighterThread->joinable()) // this stuff only happens on the first paint
		{
			this->syntaxHighlighterThread->join();
			//-----
			SCBoxScroll.cbSize = sizeof(SCROLLINFO);
			SCBoxScroll.fMask = SIF_RANGE | SIF_PAGE;
			SCBoxScroll.nMin = 0; // the range start.  It's zero-based, so start at 0
			SCBoxScroll.nMax = this->syntaxHighlighter->countNewLineInstructions(); // the last position in the range.  
			SCBoxScroll.nPage = maxVisibleLines;
			SetScrollInfo(hwnd, SB_VERT, &SCBoxScroll, false);
		}
		SCBoxScroll.fMask = SIF_POS;
		GetScrollInfo(hwnd, SB_VERT, &SCBoxScroll);
		//-------------
		std::wcout << L"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" << std::dec;
		size_t firstLine = SCBoxScroll.nPos;
		size_t lastLine = firstLine + maxVisibleLines;
		size_t lineCounter = 0;
		for (auto const & instruction : this->syntaxHighlighter->getInstructions())
		{
			if (lineCounter >= lastLine) break;
			auto[startPos, endPos, TextColor, text] = instruction;
			if (instruction == this->syntaxHighlighter->newLineCommand)
			{
				if (lineCounter >= firstLine)
				{
					X = marginX;
					Y += letterHeight + spacerY;
				}
				lineCounter++;
				continue;
			}
			if (lineCounter < firstLine) continue;
			SetTextColor(hdc, TextColor);
			wchar_t * cText = new wchar_t[text.length() + 1];
			wcscpy_s(cText, text.length() + 1, text.c_str());
			cText[text.length()] = L'\0'; // ensure null terminator...gotta love C code :'(
			Metric width = static_cast<Metric>(letterWidth * text.length());
			RECT textRect = { static_cast<int>(X),
							  static_cast<int>(Y),
							  static_cast<int>(X + width),
							  static_cast<int>(Y + letterHeight) };
			std::wcout << X << L'\t' << Y << L'\t' << letterWidth << L" * " << text.length() << L" = " << width << L'\t' << letterHeight << L'\t' << std::hex << TextColor << std::dec << L"\t|" << cText << L"|\n";
			DrawTextEx(hdc, cText, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP, nullptr);
			X += width; // adjust the X-value so the next set of text begins right after this one
			delete[] cText;
		}
		SelectObject(hdc, oldf);
		DeleteObject(hf);
		this->SCBoxContentHeight = Y + (letterHeight * 2) + marginY;
	}

	void LessonPage::StaticSCBoxWM_VScroll(const HWND hwnd, const WPARAM wParam) const
	{
		if (this->lessonData.SCBox && this->lessonData.SCReadOnly) GUI::WM_VScroll(hwnd, wParam, this->SCBoxContentHeight);
	}

	void LessonPage::setSyntaxHighlighter()
	{
		this->syntaxHighlighter = std::make_unique<SyntaxHighlighter>(SyntaxHighlighter(this->lessonData.SCLang, this->lessonData.SCLangID, this->lessonData.SCBoxData, this->defTextColor));
	}

	std::wstring LessonPage::CheckCode(WindowData& data) const
	{
		using namespace std::string_literals;
		std::wstring result = L""s;
		
		// get the code from the SCBOX
		const size_t codeBufSize = GetWindowTextLength(data.children.at(L"SCBOX")->handle);
		std::vector<wchar_t> codeBuf(codeBufSize + 1, 0);
		GetWindowText(data.children.at(L"SCBOX")->handle, &codeBuf[0], static_cast<int>(codeBuf.size()) );
		const std::wstring code = std::wstring(&codeBuf[0]);
		if (code.empty())
		{
			result += L"\n\t-No code was entered."s;
			return result;
		}

		// CodeChecker processing
		CodeChecker CC(this->curLangName, this->curLangID, code);
		CC.Init();
		CodeChecker::Error error = CC.getErrorState();
		if (error != CodeChecker::Error::ALLGOOD)
		{
			result += L"\n\t-CodeChecker Error ("s + std::to_wstring(static_cast<unsigned int>(error)).c_str() + L")"s;
			return result;
		}
		const std::wstring identMsg = CC.Identify();
		error = CC.getErrorState();
		if (error != CodeChecker::Error::ALLGOOD)
		{
			result += L"\n\t-CodeChecker Error ("s + std::to_wstring(static_cast<unsigned int>(error)).c_str() + L")"s;
			return result;
		}
		const std::wstring csMsg = CC.CheckSyntax();
		error = CC.getErrorState();
		if (error != CodeChecker::Error::ALLGOOD)
		{
			result += L"\n\t-CodeChecker Error ("s + std::to_wstring(static_cast<unsigned int>(error)).c_str() + L")"s;
			return result;
		}
		if (csMsg != L"SUCCESS") result += L"\n\t" + csMsg;
		for (auto const & query : this->lessonData.CCButtonData)
		{
			const std::wstring qMsg = CC.Query(query);
			error = CC.getErrorState();
			if (error != CodeChecker::Error::ALLGOOD)
			{
				result += L"\n\t-CodeChecker Error ("s + std::to_wstring(static_cast<unsigned int>(error)).c_str() + L")"s;
				return result;
			}
			if (qMsg != L"SUCCESS")
			{
				result += L"\n\t" + qMsg;
				break;
			}
		}
		return result;
	}

	void LessonPage::CCButtonMessage(WindowData& data, const GUI& gui, const Palette& ColorPalette, const std::wstring& message, const bool correct) const
	{
		if ( (data.children.find(L"container") == data.children.end())
			|| (data.children.at(L"container")->children.find(L"CCMessage") == data.children.at(L"container")->children.end()) ) // if the window hasn't been created yet, create it.  Else, just update the text.
		{
			// Y-Axis Metrics
			Metric SCy = data.children.at(L"SCBOX")->y;
			Metric SCHeight = data.children.at(L"SCBOX")->height;
			Metric CCy = data.children.at(L"CCBUTTON")->y;
			Metric CCHeight = data.children.at(L"CCBUTTON")->height;
			Metric SpacerHeight = CCy - (SCy + SCHeight);
			Metric ContY = CCy + CCHeight + SpacerHeight;
			Metric ContHeight = (2 * CCHeight) - (2 * SpacerHeight); // The space is 3 * CCHeight tall, so subtract away 1 CCHeight and 2 spacers and that's your MWHeight
			Metric MWHeight = ContHeight - 4;
			Metric MWy = 2;

			// X-Axis Metrics
			Metric SCx = data.children.at(L"SCBOX")->x;
			Metric SCWidth = data.children.at(L"SCBOX")->width;
			Metric ContWidth = static_cast<Metric>(SCWidth * 0.9375);
			Metric ContX = SCx + (SCWidth - ContWidth) / 2;
			Metric MWWidth = ContWidth - 4;
			Metric MWx = 2;

			// Make It So!
			const COLORREF BorderColor = correct ? ColorPalette.Green : ColorPalette.Red;
			HWND hCont = CreateWindowEx(0, L"Container", L"", WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, ContX, ContY, ContWidth, ContHeight, data.handle, nullptr, nullptr, nullptr);
			std::unique_ptr<WindowData> wd_ptr_cont = std::make_unique<WindowData>(hCont, ChildList(), BorderColor, ContHeight, ContWidth, ContX, ContY, &data, true);
			CheckEmplace(data.children.emplace(L"container", std::move(wd_ptr_cont)), L"CCMessage container");

			HWND hMessageWindow = CreateWindowEx(0, L"STATIC", message.c_str(), WS_VISIBLE | WS_CHILD | SS_LEFT, MWx, MWy, MWWidth, MWHeight, hCont, nullptr, nullptr, nullptr);
			SendMessage(hMessageWindow, WM_SETFONT, reinterpret_cast<WPARAM>(&(*gui.verdanaNormal)), TRUE);
			InvalidateRect(hMessageWindow, nullptr, false);
			std::unique_ptr<WindowData> wd_ptr_MessageWindow = std::make_unique<WindowData>(hMessageWindow, ChildList(), data.color, CCHeight, MWWidth, MWx, MWy, &(*data.children.at(L"container")), true);
			CheckEmplace(data.children.at(L"container")->children.emplace(L"CCMessage", std::move(wd_ptr_MessageWindow)), L"CCMessage");
		}
		else
		{
			data.children.at(L"container")->color = correct ? ColorPalette.Green : ColorPalette.Red;
			SetWindowText(data.children.at(L"container")->children.at(L"CCMessage")->handle, message.c_str());
			InvalidateRect(data.children.at(L"container")->handle, nullptr, false);
		}
	}

	void LessonPage::CCButtonWM_LButtonUp(WindowData& data, const GUI& gui, const Palette& ColorPalette) const
	{
		const std::wstring result = this->CheckCode(data);
		if (result.empty()) this->CCButtonMessage(data, gui, ColorPalette, L"Success!  You may now advance to the next lesson.", true);
		else this->CCButtonMessage(data, gui, ColorPalette, L"Your code contains error(s):" + result, false);
	}

	/*void LessonPage::WM_Paint(const HWND hwnd, const HDC hdc, const COLORREF TextColor, const COLORREF BkColor, HFONT font)
	{
		RECT rect;
		GetClientRect(hwnd, &rect);
		SendMessage(hwnd, WM_ERASEBKGND, reinterpret_cast<WPARAM>(hdc), reinterpret_cast<LPARAM>(nullptr));
		HBRUSH hBrush = CreateSolidBrush(BkColor);
		HPEN hPen = CreateSolidPe
		SelectObject(hdc, hBrush);
		SelectObject(hdc, hPen);
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
		RECT nRect = { static_cast<int>(rect.left + ((rect.right - rect.left) * 0.05)),
		rect.top,
		static_cast<int>(rect.left + (rect.right - rect.left) * 0.5),
		rect.bottom };
		DrawTextEx(hdc, WindowText, -1, &nRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE, nullptr);
		delete[] WindowText;
	}*/

	void LessonPage::Uninit()
	{
		if (this->SCEditBox) this->SCEditBox->Uninit();
	}
}