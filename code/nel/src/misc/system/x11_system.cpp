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
#include "x11_system.h"
#include "x11_window.h"
#include "x11_display.h"

#include <X11/extensions/Xinerama.h>

CX11System::CX11System():ISystem(), _Instance(NULL), _Class(0)
{
	nativeInit();
}

CX11System::~CX11System()
{
	nativeUninit();
}

bool CX11System::init()
{
	_Display = XOpenDisplay(NULL);

	if (_Display == NULL)
	{
		nlerror ("XOpenDisplay failed on '%s'", getenv("DISPLAY"));
	}
	else
	{
		nldebug("3D: XOpenDisplay on '%s' OK", getenv("DISPLAY"));
	}

	_xrandr_version = 0;

#ifdef HAVE_XRANDR
	_OldSizeID = 0;
	sint xrandr_major, xrandr_minor;
	if (XRRQueryVersion(_Display, &xrandr_major, &xrandr_minor))
	{
		_xrandr_version = xrandr_major * 100 + xrandr_minor;
		nlinfo("3D: XRandR %d.%d found", xrandr_major, xrandr_minor);
	}

	if (XQueryExtension(_Display, "RANDR", &Version, &Version, &Version))
	{
		// ok
	}

#endif // HAVE_XRANDR

	_xvidmode_version = 0;

#ifdef XF86VIDMODE
	sint event = 0, error = -1, vm_major = 0, vm_minor = 0;
	if (XF86VidModeQueryExtension(_Display, &event, &error) && XF86VidModeQueryVersion(_Display, &vm_major, &vm_minor))
	{
		_xvidmode_version = vm_major * 100 + vm_minor;
		nlinfo("3D: XF86VidMode %d.%d found", vm_major, vm_minor);
	}
#endif

	_xrender_version = 0;

#ifdef HAVE_XRENDER
	sint xrender_major, xrender_event, xrender_error;
	if (XQueryExtension(_Display, "RENDER", &xrender_major, &xrender_event, &xrender_error) &&
		XRenderQueryExtension(_Display, &xrender_event, &xrender_error))
	{
		sint xrender_minor = 0;
		XRenderQueryVersion(_Display, &xrender_major, &xrender_minor);
		_xrender_version = xrender_major * 100 + xrender_minor;
		nlinfo("3D: XRender %d.%d found", xrender_major, xrender_minor);
	}
#endif // HAVE_XRENDER

	_AlphaBlendedCursorSupported = false;

#ifdef HAVE_XCURSOR
	if (!_AlphaBlendedCursorSupported && XcursorSupportsARGB(_dpy))
		_AlphaBlendedCursorSupported = true;
#endif // HAVE_XCURSOR

	if (!_AlphaBlendedCursorSupported && _xrender_version > 0)
		_AlphaBlendedCursorSupported = true;

	// XInput Extension available?
	sint opcode, event, error;
	if (!XQueryExtension(_Display, "XInputExtension", &opcode, &event, &error))
	{
		printf("X Input extension not available.\n");
		return -1;
	}

	// Which version of XI2? We support 2.0
	int major = 2, minor = 0;
	if (XIQueryVersion(_Display, &major, &minor) == BadRequest)
	{
		printf("XI2 not available. Server supports %d.%d\n", major, minor);
		return -1;
	}

	nldebug("3D: Available X Extensions:");

	if (DebugLog)
	{
		// list all supported extensions
		sint nextensions = 0;
		char **extensions = XListExtensions(_Display, &nextensions);

		for(sint i = 0; i < nextensions; ++i)
		{
			if(i%5==0) DebugLog->displayRaw("3D:     ");
			DebugLog->displayRaw(NLMISC::toString("%s ", extensions[i]).c_str());
			if(i%5==4) DebugLog->displayRaw("\n");
		}

		DebugLog->displayRaw("\n");

		XFreeExtensionList(extensions);
	}

	// set default X errors handler
	XSetErrorHandler(nelXErrorsHandler);

	// define Atoms
	XA_WM_STATE = XInternAtom(_Display, "_NET_WM_STATE", False);
	XA_WM_STATE_FULLSCREEN = XInternAtom(_Display, "_NET_WM_STATE_FULLSCREEN", False);
	XA_WM_ICON = XInternAtom(_Display, "_NET_WM_ICON", False);
	XA_WM_WINDOW_TYPE = XInternAtom(_Display, "_NET_WM_WINDOW_TYPE", False);
	XA_WM_WINDOW_TYPE_NORMAL = XInternAtom(_Display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
	XA_FRAME_EXTENTS = XInternAtom(_Display, "_NET_FRAME_EXTENTS", False);

	int screens = ScreenCount(_Display);

	for(int i = 0; i < screens; ++i)
	{
	}
		if (device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
		{
			CWin32Display *display = new CWin32Display(this, ucstring((ucchar*)device.DeviceName));

			nldebug("Found %s", ucstring((ucchar*)device.DeviceString).toString().c_str());

			_Displays.push_back(display);
		}

		++iDevNum;
	}

/*
    DWORD  StateFlags;
    WCHAR  DeviceID[128];
    WCHAR  DeviceKey[128];
*/

	return true;
}

bool CX11System::nativeUninit()
{
	// destroy all displays
	for(size_t i = 0; i < _Displays.size(); ++i)
	{
		delete _Displays[i];
		_Displays[i] = NULL;
	}

	if (!UnregisterClassW(getClassName(), _Instance))
	{
		printError();
		return false;
	}

	return true;
}

IDisplay* CX11System::nativeGetDisplay(sint display)
{
	if (display >= 0 && display < _Displays.size()) return _Displays[display];

	return NULL;
}

bool CX11System::nativeCopyTextToClipboard(const ucstring &text)
{
	if (!text.size()) return false;

	_CopiedString = text;

	// NeL window is the owner of clipboard
	XSetSelectionOwner(_Display,  XA_CLIPBOARD, _win, CurrentTime);

	// check we are owning the clipboard
	if (XGetSelectionOwner(_Display, XA_CLIPBOARD) != _win)
	{
		nlwarning("Can't aquire selection");
		return false;
	}

	_SelectionOwned = true;

	return true;
}

bool CX11System::nativePasteTextFromClipboard(ucstring &text)
{
	// check if we own the selection
	if (_SelectionOwned)
	{
		text = _CopiedString;
		return true;
	}

	// check if there is a data in clipboard
	if (XGetSelectionOwner(_Display, XA_CLIPBOARD) == None)
		return false;

	// request supported methods
	XConvertSelection(_Display, XA_CLIPBOARD, XA_TARGETS, XA_NEL_SEL, _win, CurrentTime);

	// don't return result now
	return false;
}

/*
IDriver::TMessageBoxId CX11System::systemMessageBox (const char* message, const char* title, IDriver::TMessageBoxType type, TMessageBoxIcon icon)
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

bool CX11System::convertBitmapToIcon(const NLMISC::CBitmap &bitmap, std::vector<long> &icon)
{
	// get bitmap width and height
	uint width = bitmap.getWidth();
	uint height = bitmap.getHeight();

	// icon position for bitmap
	uint pos = (uint)icon.size();

	// extend icon_data size for bitmap
	icon.resize(pos + 2 + width*height);

	// set bitmap width and height
	icon[pos++] = width;
	icon[pos++] = height;

	// convert RGBA to ARGB
	CObjectVector<uint8> pixels = bitmap.getPixels();
	for(uint j = 0; j < pixels.size(); j+=4)
		icon[pos++] = pixels[j] << 16 | pixels[j+1] << 8 | pixels[j+2] | pixels[j+3] << 24;

	return true;
}

void CX11System::printError()
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
		unicodeSupported = true;
	}
	return unicodeSupported;
}

uint CSystemUtils::getCurrentColorDepth()
{
	uint depth = 0;

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

	XkbDescPtr keyboard = XkbGetKeyboard(_dpy, XkbAllComponentsMask, XkbUseCoreKbd);

	// symbols are on the form of, check with setxkbmap -print
	// pc_us_se_2_il_3_nec_vndr/jp_4_inet(evdev)_group(alt_shift_toggle)
	char *str = XGetAtomName(_dpy, keyboard->names->symbols);
	std::string symbols(str);
	XFree(str);

	string::size_type first = symbols.find("_");

	// remove the first pc_
	symbols = symbols.remove(0, first + 1);

	first = symbols.find("_");

	// add the first layout to the list
	std::vector<std::string> keyboardLayouts;
	std::string layout = symbols.substr(0, first);
	keyboardLayouts.push_back(layout);

	// and remove it and the subsequent "_"
	symbols = symbols.remove(0, first + 1);

	for(int i = 1; i <= XkbNumKbdGroups; i++) {
		QString nextNumber = QString::number(i + 1);
		if(!symbols.contains(nextNumber))
			break;

		int nextNumberIndex = symbols.indexOf(nextNumber);

		// get the layout minus the "_" and number
		layout =  symbols.left(nextNumberIndex - 1);

		keyboardLayouts << layout;
		// and remove it and the subsequent "_", number and "_"
		symbols = symbols.remove(0, nextNumberIndex + 2);
	}

	XkbFreeKeyboard(keyboard, XkbAllComponentsMask, true);

	XkbStateRec state;

	// TODO: check the return value, which is almost always zero
	// http://tronche.com/gui/x/xlib/introduction/errors.html#Status
	XkbGetState(m_display, XkbUseCoreKbd, &state);

	sint index = state.group;

	if (index < 0 || index > XkbMaxKbdGroup)
		index = 0;

	std::string layout = keyboardLayouts[index];

*/

#endif
