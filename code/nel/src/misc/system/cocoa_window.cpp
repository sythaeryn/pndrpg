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

#include "cocoa_window.h"
//#include "cocoa_display.h"
//#include "cocoa_system.h"

/*

CCocoaWindow::CCocoaWindow(CCocoaDisplay *display):IWindow(display)
{
}

CCocoaWindow::~CCocoaWindow()
{
}

bool CCocoaWindow::processMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == WM_SIZE)
	{
		if (_Style == EWSWindowed)
		{
			RECT rect;
			GetClientRect (_Handle, &rect);

//			_CurrentMode.Width = (uint16)(rect.right-rect.left);
//			_CurrentMode.Height = (uint16)(rect.bottom-rect.top);
		}
	}
	else if(message == WM_MOVE)
	{
		if (_Style == EWSWindowed)
		{
			RECT rect;
			GetWindowRect(_Handle, &rect);
//			_WindowX = rect.left;
//			_WindowY = rect.top;
		}
	}
	else if (message == WM_ACTIVATE)
	{
		WORD fActive = LOWORD(wParam);

		if (fActive == WA_INACTIVE)
		{
//			_WndActive = false;
		}
		else
		{
//			_WndActive = true;
		}
	}

	bool trapMessage = false;

	if (_EventEmitter.getNumEmitters() > 0)
	{
		CWinEventEmitter *we = NLMISC::safe_cast<CWinEventEmitter *>(driver->_EventEmitter.getEmitter(0));
		// Process the message by the emitter
		we->setHWnd(hWnd);
		trapMessage = we->processMessage(hWnd, message, wParam, lParam);
	}

	return trapMessage;
}

bool CCocoaWindow::nativeSetIcons(const std::vector<NLMISC::CBitmap> &bitmaps)
{
	if (_Handle == NULL) return false;

	static HICON winIconBig = NULL;
	static HICON winIconSmall = NULL;

	if (winIconBig)
	{
		DestroyIcon(winIconBig);
		winIconBig = NULL;
	}

	if (winIconSmall)
	{
		DestroyIcon(winIconSmall);
		winIconSmall = NULL;
	}

	sint smallIndex = -1;
	uint smallWidth = GetSystemMetrics(SM_CXSMICON);
	uint smallHeight = GetSystemMetrics(SM_CYSMICON);

	sint bigIndex = -1;
	uint bigWidth = GetSystemMetrics(SM_CXICON);
	uint bigHeight = GetSystemMetrics(SM_CYICON);

	// find icons with the exact size
	for(uint i = 0; i < bitmaps.size(); ++i)
	{
		if (smallIndex == -1 &&	bitmaps[i].getWidth() == smallWidth &&	bitmaps[i].getHeight() == smallHeight)
			smallIndex = i;

		if (bigIndex == -1 && bitmaps[i].getWidth() == bigWidth && bitmaps[i].getHeight() == bigHeight)
			bigIndex = i;
	}

	// find icons with taller size (we will resize them)
	for(uint i = 0; i < bitmaps.size(); ++i)
	{
		if (smallIndex == -1 && bitmaps[i].getWidth() >= smallWidth && bitmaps[i].getHeight() >= smallHeight)
			smallIndex = i;

		if (bigIndex == -1 && bitmaps[i].getWidth() >= bigWidth && bitmaps[i].getHeight() >= bigHeight)
			bigIndex = i;
	}

	if (smallIndex > -1)
		CWin32System::convertBitmapToIcon(bitmaps[smallIndex], winIconSmall, smallWidth, smallHeight, 32);

	if (bigIndex > -1)
		CWin32System::convertBitmapToIcon(bitmaps[bigIndex], winIconBig, bigWidth, bigHeight, 32);

	return true;
}

bool CCocoaWindow::nativeCreate()
{
	// create the OpenGL window
	_Handle = CreateWindowW(L"NLClass", L"NeL Window", WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, GetModuleHandle(NULL), NULL);

	if (_Handle == NULL)
	{
		CWin32System::printError();
		return false;
	}

	// associate OpenGL driver to window
	SetWindowLongPtr(_Handle, GWLP_USERDATA, (LONG_PTR)this);

//	_CurrentMode.Width = width;
//	_CurrentMode.Height = height;

	// Must destroy this window
//	_DestroyWindow = true;

//	setWindowTitle(ucstring("NeL window"));

//	createCursors();

	return true;
}

bool CCocoaWindow::nativeDestroy()
{
	if (_Handle == NULL) return false;

//	releaseCursors();

	// make sure window icons are deleted
//	std::vector<NLMISC::CBitmap> bitmaps;
//	setWindowIcon(bitmaps);

	if (!DestroyWindow(_Handle))
	{
		CWin32System::printError();
		return false;
	}

	_Handle = NULL;

	return true;
}

EWindowStyle CCocoaWindow::nativeGetStyle() const
{
	return EWSWindowed;
}

bool CCocoaWindow::nativeSetStyle(EWindowStyle windowStyle)
{
	// don't change window style, if we did not create the window
	if (_Handle == NULL) return false;

	if (nativeGetStyle() == windowStyle) return true;

	// get current style
	LONG dwStyle = GetWindowLong(_Handle, GWL_STYLE);

	// prepare new style
	LONG dwNewStyle = WS_CLIPCHILDREN|WS_CLIPSIBLINGS;

	// get window current state
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);

	bool isMaximized = GetWindowPlacement(_Handle, &wndpl) && (wndpl.showCmd == SW_SHOWMAXIMIZED);
	bool isVisible = false;

	if (windowStyle == EWSWindowed)
	{
		dwNewStyle |= WS_OVERLAPPEDWINDOW;

		// if we can't resize window, remove maximize box and resize anchors
//		if (!_Resizable) dwNewStyle ^= WS_MAXIMIZEBOX|WS_THICKFRAME;

		isVisible = (dwStyle & WS_VISIBLE) != 0;
	}
	else if (windowStyle == EWSFullscreen)
	{
		dwNewStyle |= WS_POPUP;
		isVisible = true;
	}

	if (isVisible)
		dwNewStyle |= WS_VISIBLE;

	if (dwStyle != dwNewStyle)
		SetWindowLong(_Handle, GWL_STYLE, dwNewStyle);

//	if (windowStyle == EWSMaximized && isVisible && !isMaximized)
//		ShowWindow(_hWnd, SW_SHOWMAXIMIZED);
//	else if (isMaximized && isVisible)
//		ShowWindow(_hWnd, SW_RESTORE);

//	_CurrentMode.Windowed = (windowStyle == EWSWindowed);

	return true;
}

bool CCocoaWindow::nativeSetTitle(const ucstring &title)
{
	if (_Handle == NULL) return false;

	if (!SetWindowTextW(_Handle, (wchar_t*)title.c_str()))
	{
		CWin32System::printError();
		return false;
	}

	return true;
}

// ***************************************************************************
bool CCocoaWindow::nativeSetPosition(sint x, sint y)
{
//	_WindowX = x;
//	_WindowY = y;

	if (_Handle == NULL || _Style == EWSFullscreen)	return false;

	if (!SetWindowPos(_Handle, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE))
	{
		CWin32System::printError();
		return false;
	}

	return true;
}

// ***************************************************************************
bool CCocoaWindow::nativeShow(bool show)
{
	// don't change window visibility, if we didn't create the window
	if (_Handle == EmptyWindow) return false;

//	_WindowVisible = show;

	if (!ShowWindow(_Handle, show ? SW_SHOW:SW_HIDE))
	{
		CWin32System::printError();
		return false;
	}

	return true;
}

bool CCocoaWindow::nativeGetSize(sint &width, sint &height) const
{
//	width = _CurrentMode.Width;
//	height = _CurrentMode.Height;
	return true;
}

bool CCocoaWindow::nativeSetSize(sint width, sint height)
{
	if (_Handle == NULL) return false;

	// resize the window
	RECT rc;
	SetRect (&rc, 0, 0, width, height);
	AdjustWindowRectEx(&rc, (DWORD)GetWindowLongW(_Handle, GWL_STYLE), GetMenu(_Handle) != NULL, (DWORD)GetWindowLongW(_Handle, GWL_EXSTYLE));
	UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
	// set position to (0, 0) if fullscreen
//	if (_CurrentMode.Windowed)
//		flags |= SWP_NOMOVE;
	SetWindowPos(_Handle, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, flags);

	// init window width and height
	RECT clientRect;
	if (!GetClientRect(_Handle, &clientRect))
	{
		CWin32System::printError();
		return false;
	}

//	_CurrentMode.Width = uint16(clientRect.right-clientRect.left);
//	_CurrentMode.Height = uint16(clientRect.bottom-clientRect.top);

	if (!GetWindowRect(_Handle, &clientRect))
	{
		CWin32System::printError();
		return false;
	}

//	_WindowX = clientRect.left;
//	_WindowY = clientRect.top;

	return true;
}

bool CCocoaWindow::nativeGetPosition(sint &x, sint &y) const
{
//	x = _WindowX;
//	y = _WindowY;

	return true;
}

// --------------------------------------------------
bool CCocoaWindow::nativeIsActive() const
{
	if (_Handle == NULL) return false;

	bool res = true;

	return res;
}

void CWin32Window::setMousePos(float x, float y)
{
	sint x1 = (sint)((float)_Width*x);
	sint y1 = (sint)((float)_Height*(1.0f-y));

	// CG wants absolute coordinates related to first screen's top left

	// get the first screen's (conaints menubar) rect (this is not mainScreen)
	NSRect firstScreenRect = [[[NSScreen screens] objectAtIndex:0] frame];

	// get the rect (position, size) of the window
	NSRect windowRect;
	if([containerView() isInFullScreenMode])
		windowRect = [[[containerView() window] screen] frame];
	else
		windowRect = [[containerView() window] frame];

	// get the view's rect for height and width
	NSRect viewRect = [containerView() frame];

	// set the cursor position
	CGDisplayErr error = CGDisplayMoveCursorToPoint(
		kCGDirectMainDisplay, CGPointMake(
			windowRect.origin.x + (viewRect.size.width * x),
			firstScreenRect.size.height - windowRect.origin.y -
				viewRect.size.height + ((1.0 - y) * viewRect.size.height)));

	if(error != kCGErrorSuccess)
		nlerror("cannot set mouse position");
}

void CWin32Window::setCapture(bool capture)
{
	// no need to capture
	_MouseCaptured = b;
}

void CWin32Window::showCursor(bool show)
{
	// Mac OS manages a show/hide counter for the cursor, so hiding the cursor
	// twice requires two calls to "show" to make the cursor visible again.
	// Since other platforms seem to not do this, the functionality is masked here
	// by only calling hide if the cursor is visible and only calling show if
	// the cursor was hidden.

	CGDisplayErr error  = kCGErrorSuccess;
	static bool visible = true;

	if(b && !visible)
	{
		error = CGDisplayShowCursor(kCGDirectMainDisplay);
		visible = true;
	}
	else if(!b && visible)
	{
		error = CGDisplayHideCursor(kCGDirectMainDisplay);
		visible = false;
	}

	if(error != kCGErrorSuccess)
		nlerror("cannot show / hide cursor");
}

*/