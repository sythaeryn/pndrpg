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

#include "win32_window.h"

#ifdef NL_OS_WINDOWS
#include "win32_display.h"
#include "win32_cursor.h"
#include "win32_system.h"
#include "win_event_emitter.h"
#include "di_event_emitter.h"

#include "nel/misc/thread.h"
#include "nel/misc/mutex.h"

#define NOMINMAX
#include <windows.h>

#ifdef _WIN32_WINNT_WIN7
	// only supported by Windows 7 Platform SDK
	#include <ShObjIdl.h>
	#define TASKBAR_PROGRESS 1
#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

#ifdef TASKBAR_PROGRESS

class CWindows7Progress : public NLMISC::IRunnable
{
public:
	CWindows7Progress(HWND window):_TaskbarList(NULL), _Window(window), _Total(0), _Value(0)
	{
	}

	void run()
	{
		// initialize COM
		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (FAILED(hr)) return;

		// instanciate the taskbar control COM object
		hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_TaskbarList));

		// error can be ignored because Windows versions before Windows 7 doesn't support it
		if (FAILED(hr) || !_TaskbarList) return;

		// wait until notified by another thread
		while(_Event.wait() && _Window)
		{
			nldebug("progress notified %d/%d", _Value, _Total);

			if (_Total)
			{
				// update the taskbar progress
				hr = _TaskbarList->SetProgressValue(_Window, (ULONGLONG)_Value, (ULONGLONG)_Total);
			}
			else
			{
				// don't update anymore the progress
				hr = _TaskbarList->SetProgressState(_Window, _Value == 0 ? TBPF_INDETERMINATE:TBPF_NOPROGRESS);
			}
		}

		// release the interface
		_TaskbarList->Release();

		// uninitialize COM
		CoUninitialize();
	}

	void setTotal(sint total)
	{
		_Total = total;
		_Event.notify();
	}

	sint getTotal() const
	{
		return _Total;
	}

	void setValue(sint value)
	{
		_Value = value;
		_Event.notify();
	}

	sint getValue() const
	{
		return _Value;
	}

	void setWindow(HWND wnd)
	{
		_Window = wnd;
		_Event.notify();
	}

private:
	ITaskbarList3* _TaskbarList;
	NLMISC::CSynchronizedEvent _Event;
	HWND _Window;
	sint _Total, _Value;
};

#endif

CWin32Window::CWin32Window(CWin32Display *display):IWindow(), _Display(display), _Handle(NULL),
	_Width(0), _Height(0), _X(0), _Y(0), _Focused(false),
	_Progress(NULL), _ProgressThread(NULL)
{
	_DefaultCursor = NULL;
	_BlankCursor = NULL;

	// ati specific : try to retrieve driver version
//	retrieveATIDriverVersion();
}

CWin32Window::~CWin32Window()
{
}

bool CWin32Window::processMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
/*
	if (_EventEmitter.getNumEmitters() > 1) // is direct input running ?
	{
		// flush direct input messages if any
		NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1))->poll();
	}
*/

	if(message == WM_SIZE)
	{
		if (_Style & EWSWindowed)
		{
			RECT rect;
			GetClientRect(_Handle, &rect);

			_Width = (uint)(rect.right-rect.left);
			_Height = (uint)(rect.bottom-rect.top);

			nldebug("width = %d, height = %d", _Width, _Height);

			if (wParam == SIZE_MAXIMIZED)
			{
			}
			else if (wParam == SIZE_RESTORED)
			{
			}

			std::list<IWindowSizeListener*>::const_iterator it = _SizeListeners.begin(), iend = _SizeListeners.end();

			while (it != iend)
			{
				(*it)->onWindowSizeChanged(_Width, _Height);

				++it;
			}
		}
	}
	else if(message == WM_MOVE)
	{
		if (_Style & EWSWindowed)
		{
			RECT rect;
			GetWindowRect(_Handle, &rect);
			_X = (sint)rect.left;
			_Y = (sint)rect.top;

			nldebug("x = %d, y = %d", _X, _Y);

			std::list<IWindowPositionListener*>::const_iterator it = _PositionListeners.begin(), iend = _PositionListeners.end();

			while (it != iend)
			{
				(*it)->onWindowPositionChanged(_X, _Y);

				++it;
			}
		}
	}
	else if (message == WM_ACTIVATE)
	{
		_Focused = LOWORD(wParam) != WA_INACTIVE;

		nldebug("focused = %s", _Focused ? "yes":"no");
	}

	bool trapMessage = false;

	if (_EventEmitter.getNumEmitters() > 0)
	{
		NLMISC::CWinEventEmitter *we = NLMISC::safe_cast<NLMISC::CWinEventEmitter *>(_EventEmitter.getEmitter(0));
		// Process the message by the emitter
		trapMessage = we->processMessage(_Handle, message, wParam, lParam);
	}

	return trapMessage;
}

bool CWin32Window::setIcons(const std::vector<NLMISC::CBitmap> &bitmaps)
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

	if (winIconBig)
	{
		SendMessage(_Handle, WM_SETICON, 0 /* ICON_SMALL */, (LPARAM)winIconSmall);
		SendMessage(_Handle, WM_SETICON, 1 /* ICON_BIG */, (LPARAM)winIconBig);
	}
	else
	{
		SendMessage(_Handle, WM_SETICON, 0 /* ICON_SMALL */, (LPARAM)winIconSmall);
		SendMessage(_Handle, WM_SETICON, 1 /* ICON_BIG */, (LPARAM)winIconSmall);
	}

	return true;
}

bool CWin32Window::create()
{
	_Handle = CreateWindowW(_Display->getSystem()->getClassName(), (wchar_t*)_Title.c_str(),
		WS_OVERLAPPEDWINDOW/*|WS_CLIPCHILDREN|WS_CLIPSIBLINGS*/,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		HWND_DESKTOP, NULL, GetModuleHandle(NULL), NULL);

	if (_Handle == NULL)
	{
		CWin32System::printError();
		return false;
	}

	// associate OpenGL driver to window
	SetWindowLongPtr(_Handle, GWLP_USERDATA, (LONG_PTR)this);

	removeEmitters();

	createEmitters();

#ifdef TASKBAR_PROGRESS
	_Progress = new CWindows7Progress(_Handle);

	_ProgressThread = NLMISC::IThread::create(_Progress);
	_ProgressThread->start();
	_ProgressThread->setPriority(NLMISC::ThreadPriorityLowest);
#endif

	// Must destroy this window
//	_DestroyWindow = true;

//	setWindowTitle(ucstring("NeL window"));

//	createCursors();

	return true;
}

bool CWin32Window::destroy()
{
	if (_Handle == NULL) return false;

#ifdef TASKBAR_PROGRESS
	if (_Progress)
	{
		_Progress->setWindow(NULL);
		_ProgressThread->wait();

		delete _ProgressThread;
		_ProgressThread = NULL;

		delete _Progress;
		_Progress = NULL;
	}
#endif

	if (!DestroyWindow(_Handle))
	{
		CWin32System::printError();
		return false;
	}

	_Handle = NULL;

	return true;
}

bool CWin32Window::setStyle(sint windowStyle)
{
	// get current style
	LONG dwStyle = GetWindowLong(_Handle, GWL_STYLE);

	// prepare new style
	LONG dwNewStyle = WS_CLIPCHILDREN|WS_CLIPSIBLINGS;

	// get window current state
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);

	bool isMaximized = GetWindowPlacement(_Handle, &wndpl) && (wndpl.showCmd == SW_SHOWMAXIMIZED);
	bool isVisible = false;

	if (windowStyle & EWSWindowed)
	{
		dwNewStyle |= WS_OVERLAPPEDWINDOW;

		// if we can't resize window, remove maximize box and resize anchors
//		if (!_Resizable) dwNewStyle ^= WS_MAXIMIZEBOX|WS_THICKFRAME;

		isVisible = (dwStyle & WS_VISIBLE) != 0;
	}
	else if (windowStyle & EWSFullscreen)
	{
		dwNewStyle |= WS_POPUP;
		isVisible = true;
	}

	if (isVisible)
		dwNewStyle |= WS_VISIBLE;

	if (dwStyle != dwNewStyle)
		SetWindowLong(_Handle, GWL_STYLE, dwNewStyle);

	if (windowStyle & EWSMaximized /* && isVisible */ && !isMaximized)
	{
		ShowWindow(_Handle, SW_SHOWMAXIMIZED);
	}
	else if (isMaximized && isVisible)
	{
		ShowWindow(_Handle, SW_RESTORE);
	}

	return true;
}

bool CWin32Window::setTitle(const ucstring &title)
{
	if (_Handle == NULL) return false;

	_Title = title;

	if (!SetWindowTextW(_Handle, (wchar_t*)_Title.c_str()))
	{
		CWin32System::printError();
		return false;
	}

	return true;
}

bool CWin32Window::setPosition(sint x, sint y)
{
	if (_Handle == NULL || _Style & EWSFullscreen) return false;

	if (!SetWindowPos(_Handle, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE))
	{
		CWin32System::printError();
		return false;
	}

	return true;
}

bool CWin32Window::show(bool show)
{
	// don't change window visibility, if we didn't create the window
	if (_Handle == EmptyWindow) return false;

//	_WindowVisible = show;

	BOOL res = ShowWindow(_Handle, show ? SW_SHOW:SW_HIDE);

	// if res == FALSE, window was hidden
	// if res == TRUE, window was shown
/*
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);

	if (GetWindowPlacement(_win, &wndpl))
	{
		switch(wndpl.showCmd)
		{
			case SW_HIDE:
			case SW_SHOWNORMAL:

			if (show)
			{
				ShowWindow(_win, SW_SHOW);
				SetForegroundWindow(_win);
			}

			break;

			case SW_SHOW:
			case SW_SHOWNA:
			case SW_SHOWNOACTIVATE:
			case SW_SHOWMAXIMIZED:

			if (!show)
				ShowWindow(_win, SW_HIDE);

			break;

			case SW_MINIMIZE:

			ShowWindow(_win, show ? SW_RESTORE:SW_HIDE);

			if (show)
				SetForegroundWindow(_win);

			break;

			case SW_RESTORE: break;
			case SW_SHOWMINIMIZED: break;
			case SW_SHOWMINNOACTIVE: break;
			default: break;
		}
	}


*/
	return true;
}

bool CWin32Window::getSize(uint &width, uint &height) const
{
	width = _Width;
	height = _Height;

	return true;
}

bool CWin32Window::setSize(uint width, uint height)
{
	// resize the window
	RECT rc;
	SetRect (&rc, 0, 0, width, height);
	AdjustWindowRectEx(&rc, (DWORD)GetWindowLongW(_Handle, GWL_STYLE), GetMenu(_Handle) != NULL, (DWORD)GetWindowLongW(_Handle, GWL_EXSTYLE));

	UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;

	// set position to (0, 0) if fullscreen
	if (_Style & EWSWindowed) flags |= SWP_NOMOVE;

	SetWindowPos(_Handle, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, flags);

	// init window width and height
//	RECT clientRect;
//	if (!GetClientRect(_Handle, &clientRect))
//	{
//		CWin32System::printError();
//		return false;
//	}

//	_CurrentMode.Width = uint16(clientRect.right-clientRect.left);
//	_CurrentMode.Height = uint16(clientRect.bottom-clientRect.top);

//	if (!GetWindowRect(_Handle, &clientRect))
//	{
//		CWin32System::printError();
//		return false;
//	}

//	_WindowX = clientRect.left;
//	_WindowY = clientRect.top;

	return true;
}

bool CWin32Window::getPosition(sint &x, sint &y) const
{
	x = _X;
	y = _Y;

	return true;
}

bool CWin32Window::isActive() const
{
	if (_Handle == NULL) return false;

	return (IsWindow(_Handle) != FALSE);
}

void CWin32Window::setProgressTotal(sint total)
{
#ifdef TASKBAR_PROGRESS
	if (_Progress) _Progress->setTotal(total);
#endif
}

sint CWin32Window::getProgressTotal() const
{
#ifdef TASKBAR_PROGRESS
	if (_Progress) return _Progress->getTotal();
#endif

	return -1;
}

void CWin32Window::setProgressValue(sint value)
{
#ifdef TASKBAR_PROGRESS
	if (_Progress) _Progress->setValue(value);
#endif
}

sint CWin32Window::getProgressValue() const
{
#ifdef TASKBAR_PROGRESS
	if (_Progress) return _Progress->getValue();
#endif

	return -1;
}

bool CWin32Window::copyTextToClipboard(const ucstring &text)
{
	if (!text.size()) return false;

	bool res = false;

	if (OpenClipboard(NULL))
	{
		// check if unicode format is supported by clipboard
		bool isUnicode = (IsClipboardFormatAvailable(CF_UNICODETEXT) == TRUE);

		// allocates a buffer to copy text in global memory
		HGLOBAL mem = GlobalAlloc(GHND|GMEM_DDESHARE, (text.size()+1) * (isUnicode ? 2:1));

		if (mem)
		{
			// create a lock on this buffer
			void *hLock = GlobalLock(mem);

			// copy text to this buffer
			if (isUnicode)
				wcscpy((wchar_t*)hLock, (const wchar_t*)text.c_str());
			else
				strcpy((char*)hLock, text.toString().c_str());

			// unlock buffer
			GlobalUnlock(mem);

			// empty clipboard
			EmptyClipboard();

			// set new data to clipboard in the right format
			SetClipboardData(isUnicode ? CF_UNICODETEXT:CF_TEXT, mem);

			res = true;
		}

		CloseClipboard();
	}

	return res;
}

bool CWin32Window::pasteTextFromClipboard(ucstring &text)
{
	bool res = false;

	if (OpenClipboard(NULL))
	{
		// check if unicode format is supported by clipboard
		bool isUnicode = (IsClipboardFormatAvailable(CF_UNICODETEXT) == TRUE);

		// get data from clipboard (if not of this type, they are converted)
		// warning, this code can't be debuggued in VC++ IDE, hObj will be always NULL
		HANDLE hObj = GetClipboardData(isUnicode ? CF_UNICODETEXT:CF_TEXT);

		if (hObj)
		{
			// create a lock on clipboard data
			void *hLock = GlobalLock(hObj);

			if (hLock != NULL)
			{
				// retrieve clipboard data
				if (isUnicode)
					text = (const ucchar*)hLock;
				else
					text = (const char*)hLock;

				// unlock data
				GlobalUnlock(hObj);

				res = true;
			}
		}

		CloseClipboard();
	}

	return res;
}

NLMISC::IMouseDevice* CWin32Window::enableLowLevelMouse(bool enable, bool exclusive)
{
	NLMISC::IMouseDevice *res = NULL;

	NLMISC::CDIEventEmitter *diee = NULL;

	if (_EventEmitter.getNumEmitters() > 1)
		diee = NLMISC::safe_cast<CDIEventEmitter *>(_EventEmitter.getEmitter(1));

	if (enable)
	{
		try
		{
			if (diee)
				res = diee->getMouseDevice(exclusive);
		}
		catch (const EDirectInput &)
		{
		}
	}
	else
	{
		if (diee)
			diee->releaseMouse();
	}

	return res;
}

NLMISC::IKeyboardDevice* CWin32Window::enableLowLevelKeyboard(bool enable)
{
	NLMISC::IKeyboardDevice *res = NULL;
	NLMISC::CDIEventEmitter *diee = NULL;

	if (_EventEmitter.getNumEmitters() > 1)
		diee = NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1));

	if (enable)
	{
		try
		{
			if (diee)
				res = diee->getKeyboardDevice();
		}
		catch (const EDirectInput &)
		{
		}
	}
	else
	{
		if (diee)
			diee->releaseKeyboard();
	}

	return res;
}

NLMISC::IInputDeviceManager* CWin32Window::getLowLevelInputDeviceManager()
{
	NLMISC::IInputDeviceManager *res = NULL;

	if (_EventEmitter.getNumEmitters() > 1)
		res = NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1));


	return res;
}

uint CWin32Window::getDoubleClickDelay(bool hardwareMouse)
{
	uint res = 250;

	NLMISC::IMouseDevice *md = NULL;

	if (_EventEmitter.getNumEmitters() >= 2)
	{
		NLMISC::CDIEventEmitter *diee = NLMISC::safe_cast<CDIEventEmitter *>(_EventEmitter.getEmitter(1));
		if (diee->isMouseCreated())
		{
			try
			{
				md = diee->getMouseDevice(hardwareMouse);
			}
			catch (const EDirectInput &)
			{
				// could not get device ..
			}
		}
	}

	if (md)
	{
		res = md->getDoubleClickDelay();
	}
	else
	{
		// try to read the good value from windows
		res = ::GetDoubleClickTime();
	}

	return res;
}

bool CWin32Window::convertBitmapToCursor(const NLMISC::CBitmap &bitmap, HCURSOR &cursor, uint iconWidth, uint iconHeight, uint iconDepth, const NLMISC::CRGBA &col, sint hotSpotX, sint hotSpotY)
{
#if defined(NL_OS_WINDOWS)

	return CWin32System::convertBitmapToIcon(bitmap, cursor, iconWidth, iconHeight, iconDepth, col, hotSpotX, hotSpotY, true);

#elif defined(NL_OS_UNIX) && !defined(NL_OS_MAC)

	CBitmap src = bitmap;

	// resample bitmap if necessary
	if (src.getWidth() != iconWidth || src.getHeight() != iconHeight)
	{
		src.resample(iconWidth, iconHeight);
	}

	CBitmap colorBm;
	colorBm.resize(iconWidth, iconHeight, CBitmap::RGBA);
	const CRGBA *srcColorPtr = (CRGBA *) &(src.getPixels()[0]);
	const CRGBA *srcColorPtrLast = srcColorPtr + (iconWidth * iconHeight);
	CRGBA *destColorPtr = (CRGBA *) &(colorBm.getPixels()[0]);

	do
	{
		// colorize icon
		destColorPtr->modulateFromColor(*srcColorPtr, col);

		// X11 wants BGRA pixels : swap red and blue channels
		std::swap(destColorPtr->R, destColorPtr->B);

		// premultiplied alpha
		if (destColorPtr->A < 255)
		{
			destColorPtr->R = (destColorPtr->R * destColorPtr->A) / 255;
			destColorPtr->G = (destColorPtr->G * destColorPtr->A) / 255;
			destColorPtr->B = (destColorPtr->B * destColorPtr->A) / 255;
		}

		++ srcColorPtr;
		++ destColorPtr;
	}
	while (srcColorPtr != srcColorPtrLast);

#ifdef HAVE_XCURSOR

	if (XcursorSupportsARGB(_dpy))
	{
		XcursorImage *image = XcursorImageCreate(iconWidth, iconHeight);

		if (!image)
		{
			nlwarning("Failed to create a XcusorImage with size %ux%u", iconWidth, iconHeight);
		}
		else
		{
			image->xhot = (uint)hotSpotX;
			image->yhot = (uint)hotSpotY;

			memcpy(image->pixels, &colorBm.getPixels(0)[0], colorBm.getSize()*4);

			cursor = XcursorImageLoadCursor(_dpy, image);

			XcursorImageDestroy(image);
		}
	}

#endif // HAVE_XCURSOR

#ifdef HAVE_XRENDER

	if (_xrender_version > 0)
	{
		// use malloc() because X will free() data itself
		CRGBA *src32 = (CRGBA*)malloc(colorBm.getSize()*4);
		memcpy(src32, &colorBm.getPixels(0)[0], colorBm.getSize()*4);

		uint size = iconWidth * iconHeight;

		sint screen = DefaultScreen(_dpy);
		Visual *visual = DefaultVisual(_dpy, screen);

		if (!visual)
		{
			nlwarning("Failed to get a default visual for screen %d", screen);
			return false;
		}

		// Create the icon image
		XImage* image = XCreateImage(_dpy, visual, 32, ZPixmap, 0, (char*)src32, iconWidth, iconHeight, 32, 0);

		if (!image)
		{
			nlwarning("Failed to set the window's icon");
			return false;
		}

		// Create the icon pixmap
		Pixmap pixmap = XCreatePixmap(_dpy, _win, iconWidth, iconHeight, 32 /* defDepth */);

		if (!pixmap)
		{
			nlwarning("Failed to create a pixmap %ux%ux%d", iconWidth, iconHeight, 32);
			return false;
		}

		// Create the icon graphic contest
		GC gc = XCreateGC(_dpy, pixmap, 0, NULL);

		if (!gc)
		{
			nlwarning("Failed to create a GC");
			return false;
		}

		sint res = XPutImage(_dpy, pixmap, gc, image, 0, 0, 0, 0, iconWidth, iconHeight);

		if (res)
		{
			nlwarning("XPutImage failed with code %d", res);
		}

		if (!XFreeGC(_dpy, gc))
		{
			nlwarning("XFreeGC failed");
		}

		if (image->data)
		{
			free(image->data);
			image->data = NULL;
		}

		XDestroyImage(image);

		XRenderPictFormat *format = XRenderFindStandardFormat(_dpy, PictStandardARGB32);

		if (!format)
		{
			nlwarning("Failed to find a standard format");
			return false;
		}

		Picture picture = XRenderCreatePicture(_dpy, pixmap, format, 0, 0);

		if (!picture)
		{
			nlwarning("Failed to create picture");
			return false;
		}

		cursor = XRenderCreateCursor(_dpy, picture, (uint)hotSpotX, (uint)hotSpotY);

		if (!cursor)
		{
			nlwarning("Failed to create cursor");
			return false;
		}

		XRenderFreePicture(_dpy, picture);

		if (!XFreePixmap(_dpy, pixmap))
		{
			nlwarning("XFreePixmap failed");
		}

		return true;
	}

#endif // HAVE_XRENDER

	return false;

#else

	return false;

#endif
}

bool CWin32Window::isSystemCursorInClientArea()
{
	if (_Style != EWSWindowed)
	{
		return IsWindowVisible(_Handle) != FALSE;
	}
	else
	{
		POINT cursPos;
		// the mouse should be in the client area of the window
		if (!GetCursorPos(&cursPos))
		{
			return false;
		}
		HWND wnd = WindowFromPoint(cursPos);
		if (wnd != _Handle)
		{
			return false; // not the same window
		}
		// want that the mouse be in the client area
		RECT clientRect;
		if (!GetClientRect(_Handle, &clientRect))
		{
			return false;
		}
		POINT tl, br;
		tl.x = clientRect.left;
		tl.y = clientRect.top;
		br.x = clientRect.right;
		br.y = clientRect.bottom;
		if (!ClientToScreen(_Handle, &tl))
		{
			return false;
		}
		if (!ClientToScreen(_Handle, &br))
		{
			return false;
		}
		if ((cursPos.x < tl.x) || (cursPos.x >= br.x) || (cursPos.y < tl.y) || (cursPos.y >= br.y))
		{
			return false;
		}
	}

	return true;
}

bool CWin32Window::isSystemCursorCaptured()
{
	return GetCapture() == _Handle;
}

void* CWin32Window::getNativePointer() const
{
	return _Handle;
}

bool CWin32Window::setCursor(ICursor *cursor)
{
	HCURSOR hCursor = cursor ? NLMISC::safe_cast<CWin32Cursor*>(cursor)->getCursor():NULL;

	if (hCursor == NULL) hCursor = _DefaultCursor;

//	if (CInputHandlerManager::getInstance()->hasFocus())
	SetCursor(hCursor);

	// set default mouse icon to the last one
	SetClassLongPtr(_Handle, GCLP_HCURSOR, (LONG_PTR)hCursor);

	return true;
}

bool CWin32Window::createCursors()
{
	_DefaultCursor = LoadCursor(NULL, IDC_ARROW);
	_BlankCursor = NULL;

	return true;
}

bool CWin32Window::releaseCursors()
{
	SetClassLongPtr(_Handle, GCLP_HCURSOR, 0);

	return true;
}

ICursor* CWin32Window::createCursor()
{
	return new CWin32Cursor(this);
}

void CWin32Window::showCursor(bool show)
{
	if (show)
	{
		while (ShowCursor(TRUE) < 0);
	}
	else
	{
		while (ShowCursor(FALSE) >= 0);
	}
}

void CWin32Window::setMousePos(float x, float y)
{
	// NeL window coordinate to MSWindows coordinates
	POINT pt;
	pt.x = (sint)((float)_Width*x);
	pt.y = (sint)((float)_Height*(1.0f-y));
	ClientToScreen (_Handle, &pt);
	SetCursorPos(pt.x, pt.y);
}

void CWin32Window::setCapture(bool capture)
{
	if (capture && isSystemCursorInClientArea() && !isSystemCursorCaptured())
	{
		SetCapture(_Handle);
	}
	else if (!capture && isSystemCursorCaptured())
	{
		// if hardware mouse and not in client area, then force to update its aspect by updating its pos
		if (!isSystemCursorInClientArea())
		{
			// force update
			showCursor(true);
		}

		ReleaseCapture();
	}
}

bool CWin32Window::createEmitters()
{
	NLMISC::CWinEventEmitter *we = new NLMISC::CWinEventEmitter();
	we->setHWnd(_Handle);

	// setup the event emitter, and try to retrieve a direct input interface
	_EventEmitter.addEmitter(we, true);

	/// try to get direct input
	try
	{
		NLMISC::CDIEventEmitter *diee = NLMISC::CDIEventEmitter::create(GetModuleHandle(NULL), _Handle, we);
		if (diee)
		{
			_EventEmitter.addEmitter(diee, true);
		}
	}
	catch(const NLMISC::EDirectInput &e)
	{
		nlinfo(e.what());
	}

	return true;
}

bool CWin32Window::removeEmitters()
{
	/// release old emitter
	while (_EventEmitter.getNumEmitters() != 0)
	{
		_EventEmitter.removeEmitter(_EventEmitter.getEmitter(_EventEmitter.getNumEmitters() - 1));
	}

	return true;
}

}

#endif
