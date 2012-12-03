// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "../stdmisc.h"

#include "win32_system.h"

#ifdef NL_OS_WINDOWS
#include "win32_window.h"
#include "win32_display.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool trapMessage = false;

//	nldebug("HWND = %p", hWnd);

	NLMISC::CWin32Window *pWindow = (NLMISC::CWin32Window*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

	if (pWindow)
	{
		trapMessage = pWindow->processMessage(message, wParam, lParam);
	}
	else
	{
		nldebug("unknown windows with message %u (0x%x)", message, message);
	}

	// we don't want Windows to erase background
	if (message == WM_ERASEBKGND)
	{
		return TRUE;
	}

	if (message == WM_SYSCOMMAND)
	{
		switch (wParam)
		{
#ifdef NL_DISABLE_MENU
			// disable menu (F10, ALT and ALT+SPACE key doesn't freeze or open the menu)
			case SC_KEYMENU:
#endif // NL_DISABLE_MENU

			// Screensaver Trying To Start?
			case SC_SCREENSAVE:

			// Monitor Trying To Enter Powersave?
			case SC_MONITORPOWER:

			// Prevent From Happening
			return 0;

			default:
			break;
		}
	}

	return trapMessage ? 0 : DefWindowProcW(hWnd, message, wParam, lParam);
}

namespace NLMISC {

CWin32System::CWin32System():ISystem(), _Instance(NULL), _Class(0), _AlphaBlendedCursorSupported(false),
	_ClassName("NLClassV1")
{
}

CWin32System::~CWin32System()
{
}

bool CWin32System::init()
{
	if (_Class) return true;

	_Instance = GetModuleHandle(NULL);

	HCURSOR _DefaultCursor = NULL;
	HICON windowIcon = NULL;

	WNDCLASSEXW wc;
    wc.cbSize			= sizeof(WNDCLASSEXW);
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;//| CS_DBLCLKS;
	wc.lpfnWndProc		= (WNDPROC)WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= _Instance;
	wc.hIcon			= windowIcon;
	wc.hCursor			= _DefaultCursor;
	wc.hbrBackground	= WHITE_BRUSH;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= getClassName();
	wc.hIconSm			= windowIcon;

	_Class = RegisterClassExW(&wc);

	if (!_Class)
	{
		_ClassName = _ClassName + "a";
		wc.lpszClassName = getClassName();

		_Class = RegisterClassExW(&wc);
	}

	if (!_Class)
	{
		printError();

		return false;
	}

	// Support starts with windows 2000 (not only from XP as seen in most docs)
	// NB : Additionnaly, could query D3D caps to know if
	// color hardware cursor is supported, not only emulated,
	// but can't be sure that using the win32 api 'SetCursor' uses the same resources
	// So far, seems to be supported on any modern card used by the game anyway ...
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osvi))
	{
		if (osvi.dwMajorVersion	>= 5) _AlphaBlendedCursorSupported = true;
	}

	return true;
}

bool CWin32System::uninit()
{
/*
	// destroy all displays
	for(size_t i = 0; i < _Displays.size(); ++i)
	{
		delete _Displays[i];
		_Displays[i] = NULL;
	}
*/

	if (!UnregisterClassW(getClassName(), _Instance))
	{
		printError();
		return false;
	}

	return true;
}

static BOOL CALLBACK DisplayMonitorEnumProc(HMONITOR hMonitor, HDC hDC, LPRECT lprcMonitor, LPARAM data)
{
    RECT rc = *lprcMonitor;
    // you have the rect which has coordinates of the monitor

    return TRUE;
}

bool CWin32System::getDisplays(std::vector<IDisplay*> &displays)
{
	DWORD iDevNum = 0;
	DISPLAY_DEVICEW device;
	device.cb = sizeof(DISPLAY_DEVICEW);

	EnumDisplayMonitors(NULL, NULL, DisplayMonitorEnumProc, (LONG_PTR)this);

/*
	while (EnumDisplayDevicesW(NULL, iDevNum, &device, 0))
	{
		if (device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
		{
			CWin32Display *display = new CWin32Display(this, ucstring((ucchar*)device.DeviceName));

			nldebug("Found %s", ucstring((ucchar*)device.DeviceString).toString().c_str());

			displays.push_back(display);
		}

		++iDevNum;
	}
*/

	return true;
}

LPCWSTR CWin32System::getClassName() const
{
	return (wchar_t*)_ClassName.c_str();
}
/*
IDriver::TMessageBoxId CWin32System::systemMessageBox (const char* message, const char* title, IDriver::TMessageBoxType type, TMessageBoxIcon icon)
{
	H_AUTO_OGL(CDriverGL_systemMessageBox);

	switch (::MessageBox (NULL, message, title, ((type==retryCancelType)?MB_RETRYCANCEL:
										(type==yesNoCancelType)?MB_YESNOCANCEL:
										(type==okCancelType)?MB_OKCANCEL:
										(type==abortRetryIgnoreType)?MB_ABORTRETRYIGNORE:
										(type==yesNoType)?MB_YESNO|MB_ICONQUESTION:MB_OK)|

										((icon==handIcon)?MB_ICONHAND:
										(icon==questionIcon)?MB_ICONQUESTION:
										(icon==exclamationIcon)?MB_ICONEXCLAMATION:
										(icon==asteriskIcon)?MB_ICONASTERISK:
										(icon==warningIcon)?MB_ICONWARNING:
										(icon==errorIcon)?MB_ICONERROR:
										(icon==informationIcon)?MB_ICONINFORMATION:
										(icon==stopIcon)?MB_ICONSTOP:0)))
										{
											case IDOK:
												return okId;
											case IDCANCEL:
												return cancelId;
											case IDABORT:
												return abortId;
											case IDRETRY:
												return retryId;
											case IDIGNORE:
												return ignoreId;
											case IDYES:
												return yesId;
											case IDNO:
												return noId;
										}
	nlstop;

	return okId;
}
*/

bool CWin32System::convertBitmapToIcon(const NLMISC::CBitmap &bitmap, HICON &icon, uint iconWidth, uint iconHeight, uint iconDepth, const NLMISC::CRGBA &col, sint hotSpotX, sint hotSpotY, bool cursor)
{
	NLMISC::CBitmap src = bitmap;

	// resample bitmap if necessary
	if (src.getWidth() != iconWidth || src.getHeight() != iconHeight)
	{
		src.resample(iconWidth, iconHeight);
	}
	NLMISC::CBitmap colorBm;
	colorBm.resize(iconWidth, iconHeight, NLMISC::CBitmap::RGBA);
	const NLMISC::CRGBA *srcColorPtr = (NLMISC::CRGBA *) &(src.getPixels()[0]);
	const NLMISC::CRGBA *srcColorPtrLast = srcColorPtr + (iconWidth * iconHeight);
	NLMISC::CRGBA *destColorPtr = (NLMISC::CRGBA *) &(colorBm.getPixels()[0]);
	static uint8 alphaThreshold = 127;
	do
	{
		destColorPtr->modulateFromColor(*srcColorPtr, col);
		std::swap(destColorPtr->R, destColorPtr->B);
		++ srcColorPtr;
		++ destColorPtr;
	}
	while (srcColorPtr != srcColorPtrLast);
	//
	HBITMAP colorHbm = NULL;
	HBITMAP maskHbm = NULL;
	//
	if (iconDepth == 16)
	{
		std::vector<uint16> colorBm16(iconWidth * iconHeight);
		const NLMISC::CRGBA *src32 = (const NLMISC::CRGBA *) &colorBm.getPixels(0)[0];

		for (uint k = 0; k < colorBm16.size(); ++k)
		{
			colorBm16[k] = ((uint16)(src32[k].R&0xf8)>>3) | ((uint16)(src32[k].G&0xfc)<<3) | ((uint16)(src32[k].B & 0xf8)<<8);
		}

		colorHbm = CreateBitmap(iconWidth, iconHeight, 1, 16, &colorBm16[0]);
		std::vector<uint8> bitMask((iconWidth * iconHeight + 7) / 8, 0);

		for (uint k = 0;k < colorBm16.size(); ++k)
		{
			if (src32[k].A <= alphaThreshold)
			{
				bitMask[k / 8] |= (0x80 >> (k & 7));
			}
		}

		maskHbm = CreateBitmap(iconWidth, iconHeight, 1, 1, &bitMask[0]);
	}
	else
	{
		colorHbm = CreateBitmap(iconWidth, iconHeight, 1, 32, &colorBm.getPixels(0)[0]);
		maskHbm = CreateBitmap(iconWidth, iconHeight, 1, 32, &colorBm.getPixels(0)[0]);
	}

	ICONINFO iconInfo;
	iconInfo.fIcon = cursor ? FALSE:TRUE;
	iconInfo.xHotspot = (DWORD) hotSpotX;
	iconInfo.yHotspot = (DWORD) hotSpotY;
	iconInfo.hbmMask = maskHbm;
	iconInfo.hbmColor = colorHbm;

	if (colorHbm && maskHbm)
	{
		icon = CreateIconIndirect(&iconInfo);
	}

	//
	if (colorHbm) DeleteObject(colorHbm);
	if (maskHbm) DeleteObject(maskHbm);

	return true;
}

void CWin32System::printError()
{
	DWORD dwErrorCode = GetLastError();

//	LPVOID lpMsgBuf;
//	LPVOID lpDisplayBuf;

	LPWSTR lpErrorText = NULL;
//	DWORD res = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, dwErrorCode, 0, lpErrorText, MAX_PATH, 0);
	DWORD res = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpErrorText, 0, NULL);

	if (res > 0)
	{
		ucstring strError = (ucchar*)lpErrorText;

		LocalFree(lpErrorText);
	
		nlwarning("WIN32 Error %u: %s", dwErrorCode, strError.toUtf8().c_str());
	}

    // Display the error message and exit the process

//    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
//        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
//    StringCchPrintf((LPTSTR)lpDisplayBuf, 
//        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
//        TEXT("%s failed with error %d: %s"), 
//        lpszFunction, dw, lpMsgBuf); 
//    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

//    
//    LocalFree(lpDisplayBuf);
}

/*

	// is_azerty
	uint16 klId = uint16((uint32)GetKeyboardLayout(0) & 0xFFFF);
	// 0x040c is French, 0x080c is Belgian
	if (klId == 0x040c || klId == 0x080c)
		return true;

// Key in registry
static string RootKey;
static const uint32 KeyMaxLength = 1024;

bool CSystemUtils::supportUnicode()
{
	static bool init = false;
	static bool unicodeSupported = false;
	if (!init)
	{
		init = true;
#ifdef NL_OS_WINDOWS
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		// get Windows version
		if (GetVersionEx(&osvi))
		{
			if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
			{
				// unicode is supported since Windows NT 4.0
				if (osvi.dwMajorVersion >= 4)
				{
					unicodeSupported = true;
				}
			}
		}
#else
		unicodeSupported = true;
#endif
	}
	return unicodeSupported;
}

bool CSystemUtils::isAzertyKeyboard()
{
#ifdef NL_OS_WINDOWS
	uint16 klId = uint16((uint32)GetKeyboardLayout(0) & 0xFFFF);
	// 0x040c is French, 0x080c is Belgian
	if (klId == 0x040c || klId == 0x080c)
		return true;
#endif
	return false;
}

bool CSystemUtils::isScreensaverEnabled()
{
	bool res = false;
#ifdef NL_OS_WINDOWS
//	old code, is not working anymore
//	BOOL bRetValue;
//	SystemParametersInfoA(SPI_GETSCREENSAVEACTIVE, 0, &bRetValue, 0);
//	res = (bRetValue == TRUE);
	HKEY hKeyScreenSaver = NULL;
	LSTATUS lReturn = RegOpenKeyExA(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop"), 0, KEY_QUERY_VALUE, &hKeyScreenSaver);
	if (lReturn == ERROR_SUCCESS)
	{
		DWORD dwType = 0L;
		DWORD dwSize = KeyMaxLength;
		unsigned char Buffer[KeyMaxLength] = {0};

		lReturn = RegQueryValueExA(hKeyScreenSaver, TEXT("SCRNSAVE.EXE"), NULL, &dwType, NULL, &dwSize);
		// if SCRNSAVE.EXE is present, check also if it's empty
		if (lReturn == ERROR_SUCCESS)
			res = (Buffer[0] != '\0');
	}
	RegCloseKey(hKeyScreenSaver);
#endif
	return res;
}

bool CSystemUtils::enableScreensaver(bool screensaver)
{
	bool res = false;
#ifdef NL_OS_WINDOWS
	res = (SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, screensaver ? TRUE:FALSE, NULL, 0) == TRUE);
#endif
	return res;
}

std::string CSystemUtils::getRootKey()
{
	return RootKey;
}

void CSystemUtils::setRootKey(const std::string &root)
{
	RootKey = root;
}

string CSystemUtils::getRegKey(const string &Entry)
{
	string ret;
#ifdef NL_OS_WINDOWS
	HKEY hkey;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, RootKey.c_str(), 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		DWORD	dwType	= 0L;
		DWORD	dwSize	= KeyMaxLength;
		unsigned char	Buffer[KeyMaxLength];

		if(RegQueryValueEx(hkey, Entry.c_str(), NULL, &dwType, Buffer, &dwSize) != ERROR_SUCCESS)
		{
			nlwarning("Can't get the reg key '%s'", Entry.c_str());
		}
		else
		{
			ret = (char*)Buffer;
		}
		RegCloseKey(hkey);
	}
	else
	{
		nlwarning("Can't get the reg key '%s'", Entry.c_str());
	}
#endif
	return ret;
}

bool CSystemUtils::setRegKey(const string &ValueName, const string &Value)
{
	bool res = false;
#ifdef NL_OS_WINDOWS
	HKEY hkey;
	DWORD dwDisp;

	if (RegCreateKeyExA(HKEY_CURRENT_USER, RootKey.c_str(), 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisp) == ERROR_SUCCESS)
	{
		if (RegSetValueExA(hkey, ValueName.c_str(), 0L, REG_SZ, (const BYTE *)Value.c_str(), (DWORD)(Value.size())+1) == ERROR_SUCCESS)
			res = true;
		RegCloseKey(hkey);
	}
	else
	{
		nlwarning("Can't set the reg key '%s' '%s'", ValueName.c_str(), Value.c_str());
	}
#endif

	return res;
}

uint CSystemUtils::getCurrentColorDepth()
{
	uint depth = 0;

#ifdef NL_OS_WINDOWS
	HWND desktopWnd = GetDesktopWindow();
	if (desktopWnd)
	{
		HDC desktopDC = GetWindowDC(desktopWnd);
		if (desktopDC)
		{
			depth = (uint) GetDeviceCaps(desktopDC, BITSPIXEL);
			ReleaseDC(desktopWnd, desktopDC);
		}
	}
#else
	depth = 24; // temporary fix for compilation under Linux

	Display *display = XOpenDisplay(NULL);
	if (display)
	{
		depth = (uint) DefaultDepth(display, DefaultScreen(display));
		XCloseDisplay(display);
	}

	return depth;
}

} // NLMISC

*/

}

#endif
