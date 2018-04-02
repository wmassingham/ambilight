#include <stdio.h>
#include <tchar.h>
#include "SerialClass.h"
#include <string>

//#include "stdafx.h"
#include <windows.h>
#include <chrono>
#include <thread>
//#include <iostream>

#pragma comment(lib, "user32.lib") 
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "gdiplus.lib")


#include "DXGIManager.h"

/* Globals */
int hres = 0;
int vres = 0;
Serial* SP;
BYTE* ScreenData = 0;

DXGIManager* g_DXGIManager = new DXGIManager();

void getScreen() {

	HDC hScreen = GetDC(GetDesktopWindow());
	hres = GetDeviceCaps(hScreen, HORZRES);
	vres = GetDeviceCaps(hScreen, VERTRES);

	HDC hdcMem = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, hres, vres);
	HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);
	BitBlt(hdcMem, 0, 0, hres, vres, hScreen, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hOld);

	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biWidth = hres;
	bmi.biHeight = -vres;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY;

	if (ScreenData) { free(ScreenData); }
	ScreenData = (BYTE*)malloc(4 * hres * vres);
	if (!ScreenData) { DebugBreak(); }

	GetDIBits(hdcMem, hBitmap, 0, vres, ScreenData, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

	ReleaseDC(GetDesktopWindow(), hScreen);
	DeleteDC(hdcMem);
	DeleteObject(hBitmap);

}

int getScreen2() {

	g_DXGIManager->SetCaptureSource(CSMonitor1);

	RECT rcDim;
	g_DXGIManager->GetOutputRect(rcDim);

	DWORD dwWidth = rcDim.right - rcDim.left;
	DWORD dwHeight = rcDim.bottom - rcDim.top;

	hres = dwWidth;
	vres = dwHeight;

	printf("dwWidth=%d dwHeight=%d\n", dwWidth, dwHeight);

	DWORD dwBufSize = dwWidth*dwHeight * 4;

	//BYTE* pBuf = new BYTE[dwBufSize];
	if (ScreenData) { free(ScreenData); }
	ScreenData = (BYTE*)malloc(dwBufSize);

	HRESULT hr = S_OK;
	
	CComPtr<IWICImagingFactory> spWICFactory = NULL;
	hr = spWICFactory.CoCreateInstance(CLSID_WICImagingFactory);

	if (FAILED(hr)) {
		return hr;
	}

	int i = 0;
	do {
		hr = g_DXGIManager->GetOutputBits(ScreenData, rcDim);
		/*
		if (hr == DXGI_ERROR_ACCESS_LOST) {
			delete[] g_DXGIManager;
			g_DXGIManager = new DXGIManager();
		}
		*/
		i++;
	} while (hr == DXGI_ERROR_WAIT_TIMEOUT || i < 2);

	if (FAILED(hr)) {
		printf("GetOutputBits failed with hr=0x%08x\n", hr);
		return hr;
	}

	//delete[] pBuf;

	return S_OK;

}

inline int PosR(int x, int y) { return ScreenData[4 * ((y*hres) + x) + 2]; }
inline int PosG(int x, int y) { return ScreenData[4 * ((y*hres) + x) + 1]; }
inline int PosB(int x, int y) { return ScreenData[4 * ((y*hres) + x)    ]; }

BOOL WINAPI consoleHandler(DWORD signal) {

	//if (signal == CTRL_C_EVENT) {
		printf("Ctrl-C handled\n"); // do cleanup
		SP->ECF(SETDTR);
		Sleep(10);
		SP->ECF(CLRDTR);
	//}

	return TRUE;
}

// application reads from the specified serial port and reports the collected data
//int _tmain(int argc, _TCHAR* argv[]) {
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	//printf("Welcome to the serial test app!\n");
	if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
		printf("\nERROR: Could not set control handler");
		return 1;
	}
	//FreeConsole();

	CoInitialize(NULL);

	SP = new Serial("\\\\.\\COM4");    // adjust as needed

	if (SP->IsConnected()) { printf("Connected\n"); }

	//Sleep(10000);

	while (SP->IsConnected()) {

		HRESULT hr = S_OK;

		hr = getScreen2();
		if (hr == DXGI_ERROR_ACCESS_LOST ||
			hr == DXGI_ERROR_INVALID_CALL) {
			return hr;
		} else {
			printf("Got screen OK, %08x\n", hr);
		}

		const int numCols = 10;
		const int colWidth = hres / numCols;
		const int stride = 10; // number of pixels to skip between samples

		SP->WriteData("\xff", 1); // write marker, must be called each time we restart drawing

		// loop through each column
		for (int i = numCols; i > 0; i--) {

			int r=0, g=0, b=0;

			// loop through pixels within each column
			for (int j = colWidth*i-1; j < (colWidth*i)+colWidth; j += stride) {
				
				//int ry = PosR(j, 80);
				//int gy = PosG(j, 80);
				//int by = PosB(j, 80);
				int ry = 0, gy = 0, by = 0;

				// loop vertically through column
				for (int k = 80; k < vres; k += stride) {

					ry += PosR(j, k);
					gy += PosG(j, k);
					by += PosB(j, k);

				}

				r += ry / (vres / stride);
				g += gy / (vres / stride);
				b += by / (vres / stride);

			}

			// Average each sub-column together
			r /= colWidth / stride;
			g /= colWidth / stride;
			b /= colWidth / stride;

			//printf("%x %x %x\n", (unsigned char)r, (unsigned char)g, (unsigned char)b);
			//printf("%d", sizeof(char));

			unsigned char f[] = { r, g, b };

			if (!SP->WriteData(&f, 3)) { printf("Problem writing to serial port!\n"); };

		}

		Sleep(10);

	}

	CoUninitialize();

	return 0;

}
