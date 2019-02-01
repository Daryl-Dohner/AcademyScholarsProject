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

#ifndef APPDATA_HPP
#define APPDATA_HPP

//STL headers
#include <string>

namespace ASP
{
	class AppData // to make dealing with the app data file easier and more organized
	{
		private:
			static const wchar_t * AppDataFileName; // app data file filename
		public:
			struct Data // App Data structure (2 bytes)
			{
				char ProfileCounter; // 1 byte; the amount of profiles ever created
				char NumProfiles; // 1 byte; the amount of profiles currently in existence
			};
			bool ReadData(AppData::Data*) noexcept; // reads the data from the file and puts in into the provided struct
			bool WriteData(const AppData::Data&) noexcept; // writes the data from the provided struct into the (truncated) file
			bool DebugData(bool NewConsole = false); // reads the data from the file and outputs it to the console for debugging
			void ResetFile(void) noexcept; // resets the data file
	};
}

#endif
