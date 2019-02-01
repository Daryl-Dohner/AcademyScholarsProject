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
#include <iostream>
#include <tuple>
#include <algorithm> // std::sort

// Windows Headers
#define UNICODE
#include <windows.h>

// Project Headers
#include "SyntaxHighlighter.hpp"
#include "misc.hpp"
#include "gui.hpp"

namespace ASP
{
	SyntaxHighlighter::SyntaxHighlighter(const std::wstring& _language, const std::wstring& _langID, const std::wstring& _plainText, const COLORREF defColor) noexcept : language(_language), langID(_langID), plainText(_plainText)
	{
		if (defColor == -1) this->defaultColor = this->colorMap.at(L"white");
		else this->defaultColor = defColor;
		try
		{
			this->LoadRules();
			this->DebugRules();
			this->generateHighlightingInstructions();
		}
		catch (int _errno)
		{
			const std::wstring errStr = L"SyntaxHighlighter error: " + std::to_wstring(_errno);
			Error(errStr.c_str());
			std::wcout << errStr << L'\n';
		}
	}

	SyntaxHighlighter::SyntaxHighlighter(const SyntaxHighlighter& other) noexcept : plainText(other.plainText), language(other.language), defaultColor(other.defaultColor), rules(other.rules), instructions(other.instructions) {}

	SyntaxHighlighter& SyntaxHighlighter::operator=(const SyntaxHighlighter& other) noexcept
	{
		this->plainText = other.plainText;
		this->language = other.language;
		this->defaultColor = other.defaultColor;
		this->rules = other.rules;
		this->instructions = other.instructions;
		return *this;
	}

	bool SyntaxHighlighter::sortInstructions(Instruction lhs, Instruction rhs)
	{
		return std::get<0>(lhs) < std::get<0>(rhs);
	}

	void SyntaxHighlighter::LoadRules()
	{
		const std::wstring fileName = L"Languages/" + this->langID + L"_" +  this->language + L"/" + GUI::ReservedLessonFileNames[0];
		HANDLE rulesFile = CreateFile(fileName.c_str(), GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (rulesFile == INVALID_HANDLE_VALUE)
		{
			CloseHandle(rulesFile);
			throw 0;
		}
		const unsigned int FileSize = GetFileSize(rulesFile, nullptr);
		wchar_t * buffer = new wchar_t[FileSize/sizeof(wchar_t) + 1];
		ZeroMemory(buffer, FileSize + sizeof(wchar_t));
		DWORD BytesRead = 0;
		if (!ReadFile(rulesFile, buffer, FileSize, &BytesRead, nullptr)) throw 1;
		const std::wstring dataStr = std::wstring(buffer, buffer + (FileSize/sizeof(wchar_t)));
		delete[] buffer;
		std::wistringstream iss(dataStr);
		std::wstring line = L"";
		const std::wstring delim = L"\t|=|\t";
		while (std::getline(iss, line))
		{
			if (line.at(line.length() - 1) == L'\r') line = line.substr(0, line.length() - 1);
			const size_t delim1 = line.find(delim);
			const size_t delim2 = line.find(delim, delim1 + 1);
			const size_t delim3 = line.find(delim, delim2 + 1);
			if (delim1 == std::wstring::npos || delim2 == std::wstring::npos || delim3 == std::wstring::npos) continue;
			const std::wstring type = line.substr(0, delim1);
			const std::wstring param1 = line.substr(delim1 + delim.length(), delim2 - (delim1 + delim.length()) );
			const std::wstring param2 = line.substr(delim2 + delim.length(), delim3 - (delim2 + delim.length()) );
			const std::wstring color = line.substr(delim3 + delim.length());
			try
			{
				this->rules.push_back(Rule(type, param1, param2, color));
			}
			catch (std::exception& e)
			{
				std::wcout << e.what();
			}
		}
		CloseHandle(rulesFile);
	}

	void SyntaxHighlighter::generateHighlightingInstructions()
	{
		std::wstring rawText = this->plainText;
		for (const SyntaxHighlighter::Rule& rule : this->rules)
		{
			auto[mode, param1, param2, colorParam] = rule;
			if (mode == L"KEY")
			{
				size_t offset = 0;
				while (offset < rawText.size() - param1.length()) // updates even when the text is removed
				{
					const size_t foundPos = rawText.find(param1);
					if (foundPos == std::wstring::npos) offset = std::wstring::npos;
					else
					{
						const size_t endPos = foundPos + param1.length();
						const std::wstring test = L"" + colorParam;
						if (this->colorMap.find(test) == this->colorMap.end())
						{
							MessageBox(nullptr, L"hi", L"", MB_OK);
						}
						const COLORREF color = this->colorMap.at(colorParam);
						const Instruction instr{ foundPos, endPos, color, rawText.substr(foundPos, endPos - foundPos)};
						this->instructions.push_back(instr);
						rawText.replace(foundPos, endPos - foundPos, endPos - foundPos, '\0'); // replace the processed characters with null characters so they don't get reprocessed and the index associations are maintained
					}
				}
			}
			else if (mode == L"DELIM")
			{
				size_t offset = 0;
				while (offset < rawText.size() - param1.length()) // updates even when the text is removed
				{
					const size_t foundPosStart = rawText.find(param1, offset);
					if (foundPosStart == std::wstring::npos) offset = std::wstring::npos;
					else if (  (foundPosStart > 0 && rawText.at(foundPosStart - 1) == L'\\') 
							&& !(foundPosStart > 1 && rawText.at(foundPosStart - 2) == L'\\')) // if the character is escaped (not to be confused with a '\\' before it) skip over it
					{
						offset = foundPosStart + param1.length();
					}
					else
					{
						size_t foundPosEnd = rawText.find(param2, foundPosStart + param1.length());
						while ((foundPosEnd != std::wstring::npos)
						   &&  (foundPosEnd >= 1 && (rawText.at(foundPosEnd - 1) == L'\\'))
						   && !(foundPosEnd >= 2 && (rawText.at(foundPosEnd - 2) == L'\\'))) // if the character is escaped (not to be confused with a '\\' before it) skip over it)
						{
							foundPosEnd = rawText.find(param2, foundPosEnd + param2.length());
						}
						if (foundPosEnd == std::wstring::npos) offset = std::wstring::npos;
						else
						{
							const size_t startPos = foundPosStart + param1.length();
							const size_t endPos = foundPosEnd;
							if (startPos == endPos)
							{
								offset = endPos;
								continue;
							}
							const COLORREF color = this->colorMap.at(colorParam);
							const Instruction instr{ startPos, endPos, color, rawText.substr(startPos, endPos - startPos)};
							this->instructions.push_back(instr);
							rawText.replace(startPos, endPos - startPos, endPos - startPos, L'\0'); // replace the processed characters with null characters so they don't get reprocessed and the index associations are maintained
							offset = endPos + param2.length();
						}
					}
				}
			}
			else if (mode == L"DEFAULT")
			{
				this->defaultColor = this->colorMap.at(colorParam);
			}
		}
		std::sort(this->instructions.begin(), this->instructions.end(), &sortInstructions);
		this->DebugInstructions();
		this->fillGaps();
		this->DebugInstructions();
		this->accountForNewLines();
		this->DebugInstructions();
		this->pruneEmptyInstructions();
		this->DebugInstructions();
	}

	std::vector<SyntaxHighlighter::Instruction> SyntaxHighlighter::getInstructions() const noexcept
	{
		return this->instructions;
	}

	unsigned int SyntaxHighlighter::countNewLineInstructions() const noexcept
	{
		return this->numNewLines;
	}

	void SyntaxHighlighter::DebugRules() const
	{
		std::wcout << L"\n----------------\n";
		std::wcout << std::hex;
		for (auto const & elem : this->rules) std::wcout << std::get<0>(elem) << L'\t' << std::get<1>(elem) << L'\t'
														 << std::get<2>(elem) << L'\t' << std::get<3>(elem) << L'\n';
		std::wcout << L"----------------\n";
	}

	void SyntaxHighlighter::DebugInstructions() const
	{
		std::wcout << L"\n----------------\n";
		for (auto const & instr : this->instructions) std::wcout << std::dec << std::get<0>(instr) << L'\t' << std::get<1>(instr)
																 << L'\t' << std::hex << std::get<2>(instr) << L"\t|"
																 << std::get<3>(instr) << L"|\n";
		std::wcout << L"----------------\n";
	}

	void SyntaxHighlighter::fillGaps()
	{
		for(long long index = this->instructions.size() - 2; index >= 0; index--)
		{
			const size_t endPosFirstInstr = std::get<1>(this->instructions[index]);
			const size_t startPosSecondInstr = std::get<0>(this->instructions[index + 1]);
			if (endPosFirstInstr != startPosSecondInstr)
			{
				Instruction gapInstr{endPosFirstInstr, startPosSecondInstr, this->defaultColor, this->plainText.substr(endPosFirstInstr, startPosSecondInstr - endPosFirstInstr)};
				this->instructions.insert(this->instructions.begin() + index + 1, gapInstr);
			}
		}
		// account for possible gap before the first instr
		const size_t firstElemStartPos = (this->instructions.size() > 0) ? std::get<0>(this->instructions[0]) : 0;
		if (firstElemStartPos != 0)
		{
			Instruction instr{ 0, firstElemStartPos, this->defaultColor, this->plainText.substr(0, firstElemStartPos) };
			this->instructions.insert(this->instructions.begin(), instr);
		}
		// account for possible gap after the last instr
		const size_t lastElemEndPos = (this->instructions.size() > 0) ? std::get<1>( this->instructions[this->instructions.size() - 1] ) : this->plainText.length();
		if (lastElemEndPos != this->plainText.length())
		{
			Instruction instr{ lastElemEndPos, this->plainText.length(), this->defaultColor, this->plainText.substr(lastElemEndPos) };
			this->instructions.push_back(instr);
		}
	}

	void SyntaxHighlighter::accountForNewLines()
	{
		for (unsigned int index = 0; index < this->instructions.size(); index++)
		{
			auto[startPos, endPos, color, text] = this->instructions[index];
			const std::wstring newLineToken = L"\r\n";
			const size_t foundPos = text.find(newLineToken);
			if (text == newLineToken)
			{
				this->instructions[index] = newLineCommand;
				numNewLines++;
			}
			else if (foundPos != std::wstring::npos)
			{
				const Instruction updatedInstruction{startPos, startPos + foundPos, color, text.substr(0, foundPos)};
				size_t startPosNew = startPos + foundPos + newLineToken.length();
				const Instruction newInstruction{ startPosNew, endPos, color, text.substr(startPosNew - startPos) };
				this->instructions[index] = updatedInstruction;
				this->instructions.insert(this->instructions.begin() + index + 1, newLineCommand);
				this->instructions.insert(this->instructions.begin() + index + 2, newInstruction);
				numNewLines++;
			}
		}
	}

	void SyntaxHighlighter::pruneEmptyInstructions()
	{
		for (long long index = this->instructions.size() - 1; index >= 0; index--)
		{
			auto[pos1, pos2, color, text] = this->instructions[index];
			if (text.empty() && pos1 != std::wstring::npos) this->instructions.erase(this->instructions.begin() + index);
		}
	}
}