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

#ifndef LESSONPARSER_HPP
#define LESSONPARSER_HPP

// STL headers
#include <string>
#include <vector>
#include <unordered_map>

// Program headers
#include "misc.hpp"

namespace ASP
{
	class LessonParser
	{
		public:
			// typedefs
			using strvec = std::vector<std::wstring>;
			using func = size_t(LessonParser::*)(const size_t);
			using funcmap = std::unordered_map<std::wstring, func>;
			struct LessonData
			{
				// title
				std::wstring Title = L"";

				// LBOX
				bool LBox = false;
				std::wstring LBoxData = L"";

				// SCBOX
				bool SCBox = false;
				std::wstring SCBoxData = L"";
				std::wstring SCLang = L"";
				std::wstring SCLangID = L"";
				bool SCReadOnly = false;

				// BIGBOX
				bool BigBox = false;
				std::wstring BigBoxData = L"";

				// CCBUTTON
				bool CCButton = false;
				std::vector<std::wstring> CCButtonData{};

				// RCBUTTON
				bool RCButton = false;
				std::wstring RCButtonData = L"";
			};
		private:
			// member vars
			std::wstring filePath = L"";
			LangList* langs = nullptr;
			strvec fileLines{};
			funcmap commandMap{};
			LessonData lessonData{};
			// member functions
			void LoadFile(void);
			// commands
			size_t findEND(const size_t); // find the END command, signifying the end of the text for LBOX or SCBOX
			size_t TITLE(const size_t); // Lesson Title
			size_t LBOX(const size_t); // Left Info Box
			size_t SCBOX(const size_t); // Source Code Box
			size_t BIGBOX(const size_t); // Full Page Info Box
			size_t CCBUTTON(const size_t); // Code Check Button
		public:
			LessonParser() noexcept = default;
			LessonParser(const std::wstring& filename, LangList* _langs) noexcept;
			void parse(const size_t = 0);
			LessonData getLessonData(void) const noexcept;
			static void debugLessonData(const LessonData&);
	};
}

#endif