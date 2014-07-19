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

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
#include "x11_window.h"
#include "x11_display.h"

#ifdef HAVE_XRANDR
#	include <X11/extensions/Xrandr.h>
#endif // HAVE_XRANDR

#ifdef HAVE_XRENDER
#	include <X11/extensions/Xrender.h>
#endif // HAVE_XRENDER

#include <X11/Xatom.h>

#define _NET_WM_STATE_REMOVE	0
#define _NET_WM_STATE_ADD	1

static Atom XA_WM_STATE = 0;
static Atom XA_WM_STATE_FULLSCREEN = 0;
static Atom XA_WM_ICON = 0;
static Atom XA_WM_WINDOW_TYPE = 0;
static Atom XA_WM_WINDOW_TYPE_NORMAL = 0;
static Atom XA_FRAME_EXTENTS = 0;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

sint nelXErrorsHandler(Display *dpy, XErrorEvent *e)
{
	char buf[80];
	XGetErrorText(dpy, e->error_code, buf, sizeof(buf));
	nlwarning("3D: XError: %s", buf);
	return 1;
}

CX11Window::CX11Window(CX11Display *display):IWindow(), _Display(display),
	_Width(0), _Height(0), _X(0), _Y(0), _Active(false),
	_DecorationWidth(0), _DecorationHeight(0), _Title(ucstring("NeL Window"))
{
}

CX11Window::~CX11Window()
{
}

bool CX11Window::processMessage(const XEvent &e)
{
	switch(e.type)
	{
		case DestroyNotify:
		break;

		case MapNotify:
		_Active = true;
		break;

		case UnmapNotify:
		_Active = false;
		break;

		case EnterNotify:
//		_MouseCaptured = true;
		break;

		case LeaveNotify:
//		_MouseCaptured = false;
//		int mode;		/* NotifyNormal, NotifyGrab, NotifyUngrab */

		break;

		case Expose:
//		nlwarning("Expose event");
		break;

		case ConfigureNotify:

		if (_CurrentMode.Windowed && _Active)
		{
			// first time setting decoration sizes
			if ((_DecorationWidth == -1) || (_DecorationWidth == 0))
			{
				Atom type_return = 0;
				int format_return = 0;
				unsigned long nitems_return = 0;
				unsigned long bytes_after_return = 0;
				long *data = NULL;

				int status = XGetWindowProperty(_Display->getDisplay(), _Window, XA_FRAME_EXTENTS, 0, 4, False, XA_CARDINAL, &type_return, &format_return, &nitems_return, &bytes_after_return, (unsigned char**)&data);

				// succeeded to retrieve decoration size
				if (status == Success && type_return == XA_CARDINAL && format_return == 32 && nitems_return == 4 && data)
				{
					_DecorationWidth = data[0];
					_DecorationHeight = data[2];
				}
				else
				{
					// use difference between current position and previous one (set by application)
					_DecorationWidth = e.xconfigure.x - _X;
					_DecorationHeight = e.xconfigure.y - _Y;
				}
				
				// don't allow negative decoration sizes
				if (_DecorationWidth < 0) _DecorationWidth = 0;
				if (_DecorationHeight < 0) _DecorationHeight = 0;
			}

			_Width = e.xconfigure.width;
			_Height = e.xconfigure.height;
			_X = e.xconfigure.x - _DecorationWidth;
			_Y = e.xconfigure.y - _DecorationHeight;
		}

		break;

		default:

		// Process the message by the emitter
		return _EventEmitter.processMessage(e);
	}

	return true;
}

bool CX11Window::nativeSetIcons(const std::vector<NLMISC::CBitmap> &bitmaps)
{
	std::vector<long> icon_data;

	if (!bitmaps.empty())
	{
		// process each bitmap
		for(uint i = 0; i < bitmaps.size(); ++i)
		{
			convertBitmapToIcon(bitmaps[i], icon_data);
		}
	}

	if (!icon_data.empty())
	{
		// change window icon
		XChangeProperty(_Display->getDisplay(), _Window, XA_WM_ICON, XA_CARDINAL, 32, PropModeReplace, (const unsigned char *) &icon_data[0], icon_data.size());
	}
	else
	{
		// delete window icon if no bitmap is available
		XDeleteProperty(_Display->getDisplay(), _Window, XA_WM_ICON);
	}

	return true;
}

bool CX11Window::nativeCreate()
{
	if (_visual_info == NULL)
		return false;

	nlWindow root = RootWindow(_Display->getDisplay(), DefaultScreen(_Display->getDisplay()));

	XSetWindowAttributes attr;
	attr.background_pixel = BlackPixel(_Display->getDisplay(), DefaultScreen(_Display->getDisplay()));
	attr.colormap = XCreateColormap(_Display->getDisplay(), root, _visual_info->visual, AllocNone);
	int attr_flags = CWBackPixel | CWColormap;

	window = XCreateWindow (_Display->getDisplay(), root, 0, 0, mode.Width, mode.Height, 0, _visual_info->depth, InputOutput, _visual_info->visual, attr_flags, &attr);

	if (window == EmptyWindow)
	{
		nlerror("3D: XCreateWindow() failed");
		return false;
	}

	// normal window type
	XChangeProperty(_Display->getDisplay(), window, XA_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (const unsigned char*)&XA_WM_WINDOW_TYPE_NORMAL, 1);

	// set WM hints
	XWMHints *wm_hints = XAllocWMHints();

	if (wm_hints)
	{
		wm_hints->flags = StateHint | InputHint;
		wm_hints->initial_state = NormalState;
		wm_hints->input = True;

		XSetWMHints(_Display->getDisplay(), window, wm_hints);
		XFree(wm_hints);
	}
	else
	{
		nlwarning("3D: Couldn't allocate XWMHints");
	}

	// set class hints
	XClassHint *class_hints = XAllocClassHint();

	if (class_hints)
	{
		class_hints->res_name = (char*)"NeL";
		class_hints->res_class = (char*)"nel";

		XSetClassHint(_Display->getDisplay(), window, class_hints);
		XFree(class_hints);
	}
	else
	{
		nlwarning("3D: Couldn't allocate XClassHint");
	}

	_EventEmitter.init (_Display->getDisplay(), _Window, this);

	return true;
}

bool CX11Window::nativeDestroy()
{
	_EventEmitter.closeIM();

	if (_DestroyWindow && _Window)
		XDestroyWindow(_Display->getDisplay(), _Window);

	// Ungrab the keyboard (probably not necessary);
//	XUnmapWindow(_Display->getDisplay(), _Window);
	XSync(_Display->getDisplay(), True);
	XUngrabKeyboard(_Display->getDisplay(), CurrentTime);

	return true;
}

EWindowStyle CX11Window::nativeGetStyle() const
{
	return EWSWindowed;
}

bool CX11Window::nativeSetStyle(EWindowStyle windowStyle)
{
	XWindowAttributes attr;
	XGetWindowAttributes(_Display->getDisplay(), _Window, &attr);

	// if window is mapped use events else properties
	if (attr.map_state != IsUnmapped)
	{
		// Toggle fullscreen
		XEvent xev;
		xev.xclient.type = ClientMessage;
		xev.xclient.serial = 0;
		xev.xclient.send_event = True;
		xev.xclient.display = _Display->getDisplay();
		xev.xclient.window = _Window;
		xev.xclient.message_type = XA_WM_STATE;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = windowStyle == EWSFullscreen ? _NET_WM_STATE_ADD:_NET_WM_STATE_REMOVE;
		xev.xclient.data.l[1] = XA_WM_STATE_FULLSCREEN;
		xev.xclient.data.l[2] = 0;
		xev.xclient.data.l[3] = 1; // 1 for Application, 2 for Page or Taskbar, 0 for old source
		xev.xclient.data.l[4] = 0;
		if (!XSendEvent(_Display->getDisplay(), XDefaultRootWindow(_Display->getDisplay()), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev))
		{
			nlwarning("3D: Failed to toggle to fullscreen");
			return false;
		}
	}
	else
	{
		if (windowStyle == EWSFullscreen)
		{
			// set state property to fullscreen
			XChangeProperty(_Display->getDisplay(), _Window, XA_WM_STATE, XA_ATOM, 32, PropModeReplace, (const unsigned char*)&XA_WM_STATE_FULLSCREEN, 1);
		}
		else
		{
			// delete state property
			XDeleteProperty(_Display->getDisplay(), _Window, XA_WM_STATE);
		}
	}

	return true;
}

bool CX11Window::nativeSetTitle(const ucstring &title)
{
	_Title = title.toUtf8();

#ifdef X_HAVE_UTF8_STRING
	// UTF8 properties
	Xutf8SetWMProperties (_Display->getDisplay(), _Window, (char*)_Title.c_str(), (char*)_Title.c_str(), NULL, 0, NULL, NULL, NULL);
#else
	// standard properties
	XTextProperty text_property;
	if (XStringListToTextProperty((char**)&_Title.c_str(), 1, &text_property) != 0)
	{
		XSetWMProperties (_Display->getDisplay(), _Window, &text_property, &text_property,  NULL, 0, NULL, NULL, NULL);
	}
	else
	{
		nlwarning("3D: Can't convert title to TextProperty");
	}
#endif

	return true;
}

ucstring CX11Window::nativeGetTitle() const
{
	return _Title;
}

bool CX11Window::nativeSetPosition(sint x, sint y)
{
	_X = x;
	_Y = y;

	if (_CurrentMode.Windowed)
	{
		// first time requesting decoration sizes
		if (_X && _Y && !_DecorationWidth && !_DecorationHeight && _WndActive)
		{
			_DecorationWidth = -1;
			_DecorationHeight = -1;
		}

		XMoveWindow(_Display->getDisplay(), _Window, x, y);
	}

	return true;
}

bool CX11Window::nativeShow(bool show)
{
	if (show)
	{
		XMapRaised(_Display->getDisplay(), _Window);

		// fix window position if windows manager want to impose them
		setWindowPos(_WindowX, _WindowY);
	}
	else
	{
		XUnmapWindow(_Display->getDisplay(), _Window);
	}

	return true;
}

bool CX11Window::nativeGetSize(sint &width, sint &height) const
{
	width = _Width;
	height = _Height;

	return true;
}

bool CX11Window::nativeSetSize(sint width, sint height)
{
	_Width = width;
	_Height = height;

	if (!_Resizable)
	{
		// Update WM hints (disallow resizing)
		XSizeHints *size_hints = XAllocSizeHints();

		size_hints->flags = PMinSize | PMaxSize;
		size_hints->min_width = width;
		size_hints->min_height = height;
		size_hints->max_width = width;
		size_hints->max_height = height;

		XSetWMNormalHints(_Display->getDisplay(), _Window, size_hints);
		XFree(size_hints);
	}
	else
	{
//		XSetWMNormalHints(_Display->getDisplay(), _Window, StdHints);
	}

	if (width != _CurrentMode.Width || height != _CurrentMode.Height)
	{
		// resize the window
		XResizeWindow(_Display->getDisplay(), _Window, width, height);

//		_CurrentMode.Width = width;
//		_CurrentMode.Height = height;
	}

	return true;
}

bool CX11Window::nativeGetPosition(sint &x, sint &y) const
{
	x = _X;
	y = _Y;

	return true;
}

bool CX11Window::nativeIsActive() const
{
	// check if our window is still active
	XWindowAttributes attr;
	Status status = XGetWindowAttributes(_Display->getDisplay(), _Window, &attr);

	res = (status == 1);

	return _Active;
}

void CX11Window::nativeSetProgressTotal(sint total)
{
}

sint CX11Window::nativeGetProgressTotal() const
{
	return -1;
}

void CX11Window::nativeSetProgressValue(sint value)
{
}

sint CX11Window::nativeGetProgressValue() const
{
	return -1;
}

bool CX11Window::releaseCursors()
{
	XUndefineCursor(_dpy, _win);
	XFreeCursor(_dpy, _BlankCursor);

	_Cursors.clear();
}

void CX11Window::setMousePos(float x, float y)
{
	sint x1 = (sint)((float)_Width*x);
	sint y1 = (sint)((float)_Height*(1.0f-y));

	XWarpPointer (_dpy, None, _win, None, None, None, None, x1, y1);
}

void CWin32Window::setCapture(bool capture)
{
	if(b /* && isSystemCursorInClientArea() && !isSystemCursorCaptured()*/) // capture the cursor.
	{
		// capture the cursor
		XGrabPointer(_dpy, _win, True, 0, GrabModeAsync, GrabModeAsync, _win, None, CurrentTime);
		_MouseCaptured = true;
	}
	else if (!b/* && isSystemCursorCaptured()*/)
	{
		// release the cursor
		XUngrabPointer(_dpy, CurrentTime);
		_MouseCaptured = false;
	}
}

void CWin32Window::showCursor(bool show)
{
	if (!b)
	{
		XDefineCursor(_dpy, _win, _BlankCursor);
		_CurrName = "none";
	}
	else
	{
		_CurrName = "";
	}

	// update current hardware icon to avoid to have the plain arrow
	updateCursor(true);
}

bool CX11Window::setCursor(ICursor *cursor)
{
	if (cursorHandle == _DefaultCursor)
	{
		XUndefineCursor(_dpy, _win);
	}
	else
	{
		XDefineCursor(_dpy, _win, cursorHandle);
	}
}

bool CX11Window::createCursors()
{
	_DefaultCursor = None;

	if (_dpy && _win && _BlankCursor == EmptyCursor)
	{
		// create blank cursor
		char bm_no_data[] = { 0,0,0,0,0,0,0,0 };
		Pixmap pixmap_no_data = XCreateBitmapFromData (_dpy, _win, bm_no_data, 8, 8);
		XColor black;
		memset(&black, 0, sizeof (XColor));
		black.flags = DoRed | DoGreen | DoBlue;
		_BlankCursor = XCreatePixmapCursor (_dpy, pixmap_no_data, pixmap_no_data, &black, &black, 0, 0);
		XFreePixmap(_dpy, pixmap_no_data);
	}
}

bool CWin32Window::getBestCursorSize(uint srcWidth, uint srcHeight, uint &dstWidth, uint &dstHeight)
{
	Status status = XQueryBestCursor(_Display, _Handle, srcWidth, srcHeight, &dstWidth, &dstHeight);

	if (!status)
	{
		nlwarning("XQueryBestCursor failed");
	}

	return true;
}

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
# include <X11/Xatom.h>
# ifdef HAVE_XRENDER
#  include <X11/extensions/Xrender.h>
# endif // HAVE_XRENDER
# ifdef HAVE_XCURSOR
#  include <X11/Xcursor/Xcursor.h>
# endif // HAVE_XCURSOR
#endif // defined(NL_OS_UNIX) && !defined(NL_OS_MAC)

#endif
