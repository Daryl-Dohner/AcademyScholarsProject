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
#include <iostream>

// Windows Headers
#define UNICODE
#include <windows.h>
#include <wincodec.h>
#include <objbase.h>

// Project Headers
#include "ImageLoader.hpp"
#include "misc.hpp"

namespace ASP
{
	ImageLoader::ImageLoader(const wchar_t * _imagePath) noexcept : imagePath(_imagePath), isLoaded(false), isRendered(false), isRotated(false), factoryCreated(false), imagingFactory(nullptr), decoder(nullptr), frame(nullptr), convertedFrame(nullptr), scaler(nullptr), rotator(nullptr), height(0), width(0)
	{
		HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
		if (FAILED(hr)) return;
		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<void**>(&this->imagingFactory));
		if (SUCCEEDED(hr) && (this->imagingFactory != nullptr)) this->factoryCreated = true;
	}

	ImageLoader::~ImageLoader() noexcept
	{
		this->UnInit();
		CoUninitialize();
	}

	bool ImageLoader::getIsLoaded() noexcept
	{
		return this->isLoaded;
	}

	bool ImageLoader::getIsRendered() noexcept
	{
		return this->isRendered;
	}

	bool ImageLoader::getIsRotated() noexcept
	{
		return this->isRotated;
	}

	void ImageLoader::loadImage()
	{
		try
		{
			if (!this->factoryCreated) return;
			HRESULT hr = this->imagingFactory->CreateDecoderFromFilename(this->imagePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
			if (FAILED(hr)) throw 0;

			hr = decoder->GetFrame(0, &frame);
			if (FAILED(hr)) throw 1;
			hr = frame->GetSize(&this->width, &this->height);
			if (FAILED(hr)) throw 2;

			hr = this->imagingFactory->CreateFormatConverter(&this->convertedFrame);
			if (FAILED(hr)) throw 3;
			hr = this->convertedFrame->Initialize(frame, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
			if (FAILED(hr)) return;

			this->isLoaded = true;
		}
		catch (...)
		{
			this->UnInit();
			throw;
		}
	}

	void ImageLoader::renderImage(HDC hdc, unsigned int x, unsigned int y, unsigned int renderWidth, unsigned int renderHeight)
	{
		if (!this->isLoaded) return;
		HBITMAP hImage = nullptr;
		HDC memDC = nullptr;
		try
		{
			HRESULT hr = this->imagingFactory->CreateBitmapScaler(&this->scaler);
			if (FAILED(hr)) throw 4;
			if (!isRotated) hr = this->scaler->Initialize(this->convertedFrame, renderWidth, renderHeight, WICBitmapInterpolationModeFant);
			else this->scaler->Initialize(this->rotator, renderWidth, renderHeight, WICBitmapInterpolationModeFant);
			if (FAILED(hr)) throw 5;

			HDC screenDC = GetDC(nullptr);
			if (!screenDC) throw 6;
			memDC = CreateCompatibleDC(screenDC);
			if (!memDC) throw 7;

			BITMAPINFO imageInfo = {};
			imageInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			imageInfo.bmiHeader.biWidth = renderWidth;
			imageInfo.bmiHeader.biHeight = -static_cast<int>(renderHeight);
			imageInfo.bmiHeader.biPlanes = 1;
			imageInfo.bmiHeader.biBitCount = this->bitsPerPixel;
			imageInfo.bmiHeader.biCompression = BI_RGB;

			void* imageData = nullptr;
			hImage = CreateDIBSection(screenDC, &imageInfo, DIB_RGB_COLORS, &imageData, nullptr, 0);

			ReleaseDC(nullptr, screenDC);

			HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(memDC, hImage));

			const unsigned int strideDataSize = 4 * (renderWidth * this->bitsPerPixel) / 32;
			const unsigned int imgDataSize = strideDataSize * height;
			hr = this->scaler->CopyPixels(nullptr, strideDataSize, imgDataSize, static_cast<BYTE*>(imageData));
			if (FAILED(hr)) throw 8;

			BLENDFUNCTION bf;
			bf.AlphaFormat = AC_SRC_ALPHA;
			bf.BlendFlags = 0;
			bf.BlendOp = AC_SRC_OVER;
			bf.SourceConstantAlpha = 255;
			if (!AlphaBlend(hdc, x, y, renderWidth, renderHeight, memDC, 0, 0, renderWidth, renderHeight, bf)) throw;

			SelectObject(memDC, hOldBitmap);
			DeleteDC(memDC);

			//SetDIBitsToDevice(hdc, x, y, renderWidth, renderHeight, 0, 0, 0, renderHeight, imageData, &imageInfo, DIB_RGB_COLORS);
			DeleteObject(hImage);
			this->isRendered = true;
		}
		catch (...)
		{
			if (hImage) DeleteObject(hImage);
			this->UnInit();
			throw;
		}
	}

	void ImageLoader::rotateImage(WICBitmapTransformOptions deg)
	{
		if (!this->isLoaded) return;
		try
		{
			HRESULT hr = imagingFactory->CreateBitmapFlipRotator(&rotator);
			if (FAILED(hr)) throw 9;

			hr = rotator->Initialize(convertedFrame, deg);
			if (FAILED(hr)) throw 10;

			this->isRotated = true;
		}
		catch (...)
		{
			this->UnInit();
			throw;
		}
	}

	void ImageLoader::clearImage(HDC hdc, COLORREF bkColor, unsigned int rWidth, unsigned int rHeight) noexcept
	{
		const RECT rect = { 0, 0, static_cast<int>(rWidth), static_cast<int>(rHeight) };
		FillRect(hdc, &rect, CreateSolidBrush(bkColor));
	}

	void ImageLoader::UnInit() noexcept
	{
		if (this->convertedFrame)
		{
			this->convertedFrame->Release();
			this->convertedFrame = nullptr;
		}
		if (this->frame)
		{
			this->frame->Release();
			this->frame = nullptr;
		}
		if (this->decoder)
		{
			this->decoder->Release();
			this->decoder = nullptr;
		}
		if (this->scaler)
		{
			this->scaler->Release();
			this->scaler = nullptr;
		}
		if (this->rotator)
		{
			this->rotator->Release();
			this->rotator = nullptr;
		}
		this->width = 0;
		this->height = 0;
	}
}