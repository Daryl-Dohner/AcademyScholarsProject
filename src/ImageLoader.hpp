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

#ifndef IMAGELOADER_HPP
#define IMAGELOADER_HPP

// STL Headers
#include <string>

// Windows Headers
#define UNICODE
#include <wincodec.h>

namespace ASP
{
	class ImageLoader
	{
	private:
		//const wchar_t * imagePath;
		std::wstring imagePath;
		bool factoryCreated;
		bool isLoaded;
		bool isRendered;
		bool isRotated;
		IWICImagingFactory* imagingFactory;
		IWICBitmapDecoder* decoder;
		IWICBitmapFrameDecode* frame;
		IWICFormatConverter* convertedFrame;
		IWICBitmapScaler* scaler;
		IWICBitmapFlipRotator* rotator;
		unsigned int height;
		unsigned int width;
		const unsigned int bitsPerPixel = 32;
	public:
		ImageLoader() noexcept : ImageLoader(L"") {}; // default ctor
		explicit ImageLoader(const wchar_t *) noexcept; // parameterized ctor
		ImageLoader(ImageLoader&&) = delete; // move ctor
		ImageLoader(const ImageLoader&) = delete; // copy ctor
		ImageLoader& operator=(ImageLoader&&) = delete; // move assignment operator
		ImageLoader& operator=(const ImageLoader&) = delete; // copy assignment op
		~ImageLoader() noexcept; // dtor
		/***/
		bool getIsLoaded() noexcept; // accessor for isLoaded (loadImage() successful?)
		bool getIsRendered() noexcept; // accessor for isRendered (renderImage() successful?)
		bool getIsRotated() noexcept; // accessor for isRotated (rotateImage() successful?)
		void loadImage();
		void renderImage(HDC, unsigned int, unsigned int, unsigned int, unsigned int);
		void rotateImage(WICBitmapTransformOptions);
		void clearImage(HDC, COLORREF, unsigned int, unsigned int) noexcept;
		/***/
		void UnInit() noexcept; // free up interfaces if error
	};
}

#endif