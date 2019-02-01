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

#ifndef SYNTAXHIGHLIGHTER_HPP
#define SYNTAXHIGHLIGHTER_HPP

// STL headers
#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>

// Windows Headers
#define UNICODE
#include <windows.h>

namespace ASP
{
	using namespace std::string_literals;
	class SyntaxHighlighter
	{
		public:
			using Instruction = std::tuple<size_t, size_t, COLORREF, std::wstring>; // {beginPos, endPos, color, text}
		private:
			using Rule = std::tuple<std::wstring, std::wstring, std::wstring, std::wstring>; // {mode, param1, param2, color}
			std::wstring plainText = L"";
			std::wstring language = L"";
			std::wstring langID = L"";
			std::unordered_map<std::wstring, COLORREF> colorMap{ {L"red"s, RGB(0xFF, 0, 0)},
																{L"yellow"s, RGB(0xFF, 0xFF, 0)},
																{L"green"s, RGB(0, 0xFF, 0)},
																{L"cyan"s, RGB(0, 0xFF, 0xFF)},
																{L"blue"s, RGB(0, 0, 0xFF)},
																{L"magenta"s, RGB(0xFF, 0, 0xFF)},
																{L"white"s, RGB(0xFF, 0xFF, 0xFF)} };
			COLORREF defaultColor = 0;
			std::vector<Rule> rules{};
			std::vector<Instruction> instructions{};
			unsigned int numNewLines = 0;
			void LoadRules(void);
			void generateHighlightingInstructions(void);
			void fillGaps(void);
			void accountForNewLines(void);
			void pruneEmptyInstructions(void);
			static bool sortInstructions(Instruction lhs, Instruction rhs);
		public:
			const Instruction newLineCommand{ std::wstring::npos, 0, 0, L"" };
			SyntaxHighlighter() noexcept {};
			SyntaxHighlighter(const std::wstring& _language, const std::wstring& _langID, const std::wstring& _plainText, COLORREF defColor = -1) noexcept;
			SyntaxHighlighter(const SyntaxHighlighter& other) noexcept;
			SyntaxHighlighter& operator=(const SyntaxHighlighter& other) noexcept;
			std::vector<SyntaxHighlighter::Instruction> getInstructions(void) const noexcept;
			unsigned int countNewLineInstructions(void) const noexcept;
			void DebugRules(void) const;
			void DebugInstructions(void) const;
	};
}

#endif