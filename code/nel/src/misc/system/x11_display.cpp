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
#include "x11_display.h"
#include "x11_system.h"
#include "x11_window.h"

CX11Display::CX11Display(CX11System *system, const ucstring &name):IDisplay(), _System(system), _Display(0)
{
}

CX11Display::~CX11Display()
{
	std::map<sint, CWin32Window*>::const_iterator it = _Windows.begin(), iend = _Windows.end();

	if (it != iend)
	{
		if (it->second)
		{
			it->second->destroy();
			delete it->second;
		}

		++it;
	}

	// restore default X errors handler
	XSetErrorHandler(NULL);

	XCloseDisplay(_Display);
	_Display = NULL;
}

IWindow* CX11Display::nativeCreateWindow()
{
	CWin32Window *window = new CWin32Window(this);
	
	if (window->create())
	{
		_Windows[window->getId()] = window;
		return window;
	}

	nlwarning("Unable to create window");

	return NULL;
}

bool CX11Display::nativeDestroyWindow(sint window)
{
	std::map<sint, CWin32Window*>::iterator it = _Windows.find(window);

	if (it != _Windows.end())
	{
		if (!it->second) return false;

		it->second->destroy();

		delete it->second;

		it->second = NULL;
	}

	return true;
}

bool CX11Display::nativeGetCurrentMode(SDisplayMode &mode)
{
	bool found = false;
	int screen = DefaultScreen(_dpy);

#ifdef HAVE_XRANDR

	if (!found && _xrandr_version > 0)
	{
		XRRScreenConfiguration *screen_config = XRRGetScreenInfo(_dpy, RootWindow(_dpy, screen));

		if (screen_config)
		{
			int nsizes;
			XRRScreenSize *sizes = XRRConfigSizes(screen_config, &nsizes);
			if (nsizes > 0)
			{
				Rotation cur_rotation;
				SizeID size = XRRConfigCurrentConfiguration(screen_config, &cur_rotation);

				mode.Windowed = _CurrentMode.Windowed;
				mode.OffScreen = false;
				mode.Depth = (uint) DefaultDepth(_dpy, screen);
				mode.Frequency = 0;
				mode.Width = sizes[size].width;
				mode.Height = sizes[size].height;

				found = true;

				nlinfo("3D: Current XRandR mode %d: %dx%d, %dbit", size, mode.Width, mode.Height, mode.Depth);
			}
			else
			{
				nlwarning("3D: No XRandR modes available");
			}

			XRRFreeScreenConfigInfo(screen_config);
		}
		else
		{
			nlwarning("3D: XRRGetScreenInfo failed");
		}
	}

#endif // HAVE_XRANDR

#ifdef XF86VIDMODE

	if (!found && _xvidmode_version > 0)
	{
		sint pixelClock;
		XF86VidModeModeLine xmode;

		if (XF86VidModeGetModeLine(_dpy, screen, &pixelClock, &xmode))
		{
			mode.Windowed = _CurrentMode.Windowed;
			mode.OffScreen = false;
			mode.Depth = (uint) DefaultDepth(_dpy, screen);
			mode.Frequency = 1000 * pixelClock / (xmode.htotal * xmode.vtotal) ;
			mode.Width = xmode.hdisplay;
			mode.Height = xmode.vdisplay;
			nlinfo("3D: Current XF86VidMode mode: %dx%d, %d Hz, %dbit", mode.Width, mode.Height, mode.Frequency, mode.Depth);

			found = true;
		}
		else
		{
			nlwarning("3D: XF86VidModeGetModeLine failed, cannot get current video mode");
		}
	}

#endif // XF86VidMode

	if (!found)
	{
		mode.Windowed = _CurrentMode.Windowed;
		mode.OffScreen = _CurrentMode.OffScreen;
		mode.Depth = (uint) DefaultDepth(_dpy, screen);
		mode.Frequency = 0;
		mode.Width = DisplayWidth(_dpy, screen);
		mode.Height = DisplayHeight(_dpy, screen);

		found = true;

		nldebug("Current mode: %dx%d, %d Hz, %dbit", mode.Width, mode.Height, mode.Frequency, mode.Depth);
	}

	return true;
}

bool CX11Display::nativeSetCurrentMode(const SDisplayMode &mode)
{
	bool found = false;

	if (mode.width == 0 || mode.height == 0)
	{
		int screen = DefaultScreen(_Display);

	#ifdef HAVE_XRANDR

		if (!res && _xrandr_version > 0)
		{
			Window root = RootWindow(_dpy, screen);

			XRRScreenConfiguration *screen_config = XRRGetScreenInfo(_dpy, root);

			if (screen_config)
			{
				Rotation saved_rotation;
				SizeID size = XRRConfigCurrentConfiguration(screen_config, &saved_rotation);
				if (XRRSetScreenConfig(_dpy, screen_config, root, _OldSizeID, saved_rotation, CurrentTime) == RRSetConfigSuccess)
				{
					nlinfo("3D: Switching back to XRandR mode %d", _OldSizeID);
					res = true;
				}

				XRRFreeScreenConfigInfo(screen_config);
			}
			else
			{
				nlwarning("3D: XRRGetScreenInfo failed");
			}
		}

	#endif // HAVE_XRANDR

	#if defined(XF86VIDMODE)

		if (!res && _xvidmode_version > 0)
		{
			XF86VidModeModeInfo info;
			nlinfo("3D: Switching back to original mode");

			// This is UGLY
			info.dotclock = _OldDotClock;
			info.hdisplay = _OldScreenMode.hdisplay;
			info.hsyncstart = _OldScreenMode.hsyncstart;
			info.hsyncend = _OldScreenMode.hsyncend;
			info.htotal = _OldScreenMode.htotal;
			info.vdisplay = _OldScreenMode.vdisplay;
			info.vsyncstart = _OldScreenMode.vsyncstart;
			info.vsyncend = _OldScreenMode.vsyncend;
			info.vtotal = _OldScreenMode.vtotal;
			info.flags = _OldScreenMode.flags;
			info.privsize = _OldScreenMode.privsize;
			info.c_private = _OldScreenMode.c_private;

			nlinfo("3D: Switching back mode to %dx%d", info.hdisplay, info.vdisplay);
			XF86VidModeSwitchToMode(_dpy, screen, &info);
			nlinfo("3D: Switching back viewport to %d,%d",_OldX, _OldY);
			res = XF86VidModeSetViewPort(_dpy, screen, _OldX, _OldY);
		}

	#endif // XF86VIDMODE
	}
	else
	{

#ifdef HAVE_XRANDR

	if (!found && _xrandr_version > 0)
	{
		int screen = DefaultScreen(_dpy);
		Window root = RootWindow(_dpy, screen);

		XRRScreenConfiguration *screen_config = XRRGetScreenInfo(_dpy, root);

		if (screen_config)
		{
			Rotation saved_rotation;
			SizeID cur_size = XRRConfigCurrentConfiguration(screen_config, &saved_rotation);

			sint nsizes;
			XRRScreenSize *sizes = XRRConfigSizes(screen_config, &nsizes);
			sint size = -1;

			for (sint i = 0; i < nsizes; ++i)
			{
				if (sizes[i].width == mode.Width && sizes[i].height == mode.Height)
				{
					size = i;
					break;
				}
			}

			if (size > -1)
			{
				if (XRRSetScreenConfig(_dpy, screen_config, root, size, saved_rotation, CurrentTime) == RRSetConfigSuccess)
				{
					nlinfo("3D: Switching to XRandR mode %d: %dx%d", size, sizes[size].width, sizes[size].height);
					found = true;
				}
				else
				{
					nlwarning("3D: XRRSetScreenConfig failed for mode %d: %dx%d", size, sizes[size].width, sizes[size].height);
				}
			}
			else
			{
				nlwarning("3D: No corresponding screen mode");
			}

			XRRFreeScreenConfigInfo(screen_config);
		}
		else
		{
			nlwarning("3D: XRRGetScreenInfo failed");
		}
	}

#endif // HAVE_XRANDR

#if defined(XF86VIDMODE)

	if (!found && _xvidmode_version > 0)
	{
		// Find the requested mode and use it
		XF86VidModeModeInfo **modes;
		int nmodes;
		if (XF86VidModeGetAllModeLines(_dpy, DefaultScreen(_dpy), &nmodes, &modes))
		{
			for (int i = 0; i < nmodes; i++)
			{
				const uint16 freq = modeInfoToFrequency(modes[i]);

				nldebug("3D: Available mode - %dx%d %d Hz", modes[i]->hdisplay, modes[i]->vdisplay, (int)freq);
				if (modes[i]->hdisplay == mode.Width && modes[i]->vdisplay == mode.Height /* && freq == mode.Frequency */)
				{
					if (XF86VidModeSwitchToMode(_dpy, DefaultScreen(_dpy), modes[i]))
					{
						nlinfo("3D: XF86VidMode Switching to mode %dx%d", modes[i]->hdisplay, modes[i]->vdisplay);
						XF86VidModeSetViewPort(_dpy, DefaultScreen(_dpy), 0, 0);
						found = true;
					}
					break;
				}
			}
			XFree(modes);
		}
	}

#endif // XF86VIDMODE

	if (!found)
		return false;

	return false;
}

bool CX11Display::nativeGetModes(std::vector<SDisplayMode> &modes)
{
	bool found = false;
	int screen = DefaultScreen(_dpy);

#if defined(HAVE_XRANDR)
	if (!found && _xrandr_version >= 100)
	{
		XRRScreenConfiguration *screen_config = XRRGetScreenInfo(_dpy, RootWindow(_dpy, screen));

		if (screen_config)
		{
			// retrieve the list of resolutions
			int nsizes = 0;
			XRRScreenSize *sizes = XRRConfigSizes(screen_config, &nsizes);

			if (nsizes > 0)
			{
//				nldebug("3D: %d available XRandR modes:", nsizes);
				for (sint i = 0; i < nsizes; ++i)
				{
					// Add this mode
					GfxMode mode;
					mode.Width = sizes[i].width;
					mode.Height = sizes[i].height;
					mode.Frequency = 0;
					modes.push_back(mode);
//					nldebug("3D:   Mode %d: %dx%d", i, mode.Width, mode.Height);
				}

				found = true;
			}
			else
			{
				nlwarning("3D: No XRandR modes available");
			}

			XRRFreeScreenConfigInfo(screen_config);
		}
		else
		{
			nlwarning("3D: XRRGetScreenInfo failed");
		}
	}
#endif

#ifdef XF86VIDMODE
	if (!found && _xvidmode_version > 0)
	{
		int nmodes;
		XF86VidModeModeInfo **ms;
		if (XF86VidModeGetAllModeLines(_dpy, screen, &nmodes, &ms))
		{
//			nlinfo("3D: %d available XF86VidMode modes:", nmodes);
			for (int j = 0; j < nmodes; j++)
			{
				// Add this mode
				GfxMode mode;
				mode.Width = (uint16)ms[j]->hdisplay;
				mode.Height = (uint16)ms[j]->vdisplay;
				mode.Frequency = modeInfoToFrequency(ms[j]);
//				nlinfo("3D:   Mode %d: %dx%d, %d Hz", j, mode.Width, mode.Height, mode.Frequency);
				modes.push_back (mode);
			}
			XFree(ms);
		}
		else
		{
			nlwarning("3D: XF86VidModeGetAllModeLines failed");
		}
	}
#endif // XF86VIDMODE

	if (!found)
	{
		// Add current screen mode
		GfxMode mode;
		mode.Width = DisplayWidth(_dpy, screen);
		mode.Height = DisplayHeight(_dpy, screen);
		mode.Frequency = 0;
		modes.push_back(mode);
	}

	return true;
}

/*

	// save gamma
	// Backup monitor color parameters
	HDC dc = CreateDC ("DISPLAY", NULL, NULL, NULL);
	if (dc)
	{
		_NeedToRestaureGammaRamp = GetDeviceGammaRamp (dc, _GammaRampBackuped) != FALSE;

		// Release the DC
		ReleaseDC (NULL, dc);
	}
	else
	{
		nlwarning ("(CDriverGL::init): can't create DC");
	}

	// restore gamma
	// Restaure monitor color parameters
	if (_NeedToRestaureGammaRamp)
	{
		HDC dc = CreateDC ("DISPLAY", NULL, NULL, NULL);
		if (dc)
		{
			if (!SetDeviceGammaRamp (dc, _GammaRampBackuped))
				nlwarning ("(CDriverGL::release): SetDeviceGammaRamp failed");

			// Release the DC
			ReleaseDC (NULL, dc);
		}
		else
		{
			nlwarning ("(CDriverGL::release): can't create DC");
		}
	}

	// set monitor color properties
	// Get a DC
	HDC dc = CreateDC ("DISPLAY", NULL, NULL, NULL);
	if (dc)
	{
		// The ramp
		WORD ramp[256*3];

		// For each composant
		uint c;
		for( c=0; c<3; c++ )
		{
			uint i;
			for( i=0; i<256; i++ )
			{
				// Floating value
				float value = (float)i / 256;

				// Contrast
				value = (float) max (0.0f, (value-0.5f) * (float) pow (3.f, properties.Contrast[c]) + 0.5f );

				// Gamma
				value = (float) pow (value, (properties.Gamma[c]>0) ? 1 - 3 * properties.Gamma[c] / 4 : 1 - properties.Gamma[c] );

				// Luminosity
				value = value + properties.Luminosity[c] / 2.f;
				ramp[i+(c<<8)] = (WORD)min ((int)65535, max (0, (int)(value * 65535)));
			}
		}

		// Set the ramp
		bool result = SetDeviceGammaRamp (dc, ramp) != FALSE;

		// Release the DC
		ReleaseDC (NULL, dc);

		// Returns result
		return result;
	}
	else
	{
		nlwarning ("(CDriverGL::setMonitorColorProperties): can't create DC");
	}

*/

#endif
