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

#ifndef PROFILE_HPP
#define PROFILE_HPP

// STL headers
#include <string>
#include <vector>

namespace ASP
{
	class Profiles // to make dealing with the profiles file easier and more organized
	{
		public:
			static const wchar_t * ProfileFileName; // profiles file filename
			std::wstring SelectedProfile; // for the start page
			static const unsigned int MaxProfiles = 5; // must be an odd number
			static const unsigned int MaxProfileNameLength = 20;
			struct Profile // profile structure (45 bytes) DATA SIZE IS FAKE NEWS NOW B/C WCHAR_T
			{
				char profile_id; // 1 bytes
				wchar_t profile_name[Profiles::MaxProfileNameLength + 1]; // 42 bytes
				char cur_lang_id; // 1 byte
				char cur_lesson_id; // 1 byte
			};
			bool LoadProfiles(std::vector<Profiles::Profile>*); // reads the profile blocks from the file and puts in into the provided std::vector
			bool CreateNewProfile(wchar_t[Profiles::MaxProfileNameLength + 1]);  // creates a new profile and writes it to the file.
			//bool DebugData(void); //reads the profile blocks from the file and outputs them to the console for debugging
			bool CheckName(std::wstring); //make sure no illegal chars
	};
}

#endif
