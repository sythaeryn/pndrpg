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

#include "stdopengl.h"
#include "driver_opengl.h"
#include "driver_opengl_extension.h"
#include "driver_opengl_vertex_buffer_hard.h"

using namespace std;
using namespace NLMISC;

namespace NL3D {

#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
#endif

static GfxMode toGfxMode(const SDisplayMode& mode)
{
	GfxMode res;
	res.Width = mode.Width;
	res.Height = mode.Height;
	res.Depth = mode.Depth;
	res.Frequency = mode.Frequency;
	res.AntiAlias = -1;
	res.OffScreen = false;
	res.Windowed = false;

	return res;
}

static SDisplayMode toDisplayMode(const GfxMode& mode)
{
	SDisplayMode res;
	res.Width = mode.Width;
	res.Height = mode.Height;
	res.Depth = mode.Depth;
	res.Frequency = mode.Frequency;

	return res;
}

// ***************************************************************************
bool CDriverGL::unInit()
{
	H_AUTO_OGL(CDriverGL_unInit)

	return true;
}

bool CDriverGL::setDisplay(CWindow* wnd, const GfxMode &mode, bool show, bool resizable) throw(EBadDisplay)
{
	H_AUTO_OGL(CDriverGL_setDisplay);

	_Resizable = resizable;

	// Init pointers
	_PBuffer = NULL;
	_hRC = NULL;
	_hDC = NULL;

	// Driver caps.
	//=============
	// Retrieve the WGL extensions before init the driver.

	if (wnd == NULL)
	{
		_Display = CSystem::instance()->getDisplay();

		// create dummy window
		_Window = _Display->createWindow();
	}
	else
	{
		_Window = wnd;
		_Display = _Window->getDisplay();
	}

	_hDC = GetDC((HWND)_Window->getNativePointer());

	_WndActive = true;

	// setup pixel format structure with main flags (extended ones will be set later)
	memset(&_pfd, 0, sizeof(_pfd));
	_pfd.nSize			= sizeof(_pfd);
	_pfd.nVersion		= 1;
	_pfd.dwFlags		= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	_pfd.iPixelType		= PFD_TYPE_RGBA;
	_pfd.iLayerType		= PFD_MAIN_PLANE;
	_pfd.cDepthBits		= 16; // 16;
	_pfd.cAlphaBits		= 8; // 8;
	_pfd.cStencilBits	= 0; // 8;

	// choose best suited depth Buffer
	if (_Display->getDepth() <= 16)
	{
		_pfd.cColorBits = 16;
	}
	else
	{
		_pfd.cColorBits = 24;
	}

	OSVERSIONINFO osvi = {0};
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (!GetVersionEx(&osvi))
		return false;

	// When running under Windows Vista or later support desktop composition.
	if (osvi.dwMajorVersion > 6 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 0))
		_pfd.dwFlags |=  PFD_SUPPORT_COMPOSITION;

	sint pf = 0;

	// init pixel format
	if (!choosePixelFormat(pf))
	{
		nlwarning("Can't find suitable pixel format");
		_Display->destroyWindow(_Window->getId());
		return false;
	}

	if (!SetPixelFormat(_hDC, pf, &_pfd))
	{
		nlwarning("SetPixelFormat failed");
		return false;
	}

	createContext();

	// choose real pixel format with antialiasing
	if (!choosePixelFormat(pf, mode.AntiAlias))
	{
		nlwarning("Can't find suitable pixel format");
		_Display->destroyWindow(_Window->getId());
		return false;
	}

	destroyContext();

	// destroy dummy window
	_Display->destroyWindow(_Window->getId());

	// create the real window
	_Window = _Display->createWindow();

	_hDC = GetDC((HWND)_Window->getNativePointer());

	if (!SetPixelFormat(_hDC, pf, &_pfd))
	{
		nlwarning("SetPixelFormat failed");
		return false;
	}

	createContext();

	// setup OpenGL structures
	if (!setupDisplay())
		return false;

	// setup window size and screen mode
	if (!setMode(mode))
		return false;

	if (show || !mode.Windowed)
		_Window->show();

	_CurrentMode = mode;

	return true;
}

bool CDriverGL::setMode(const GfxMode& mode)
{
	H_AUTO_OGL(CDriverGL_setMode);

	_Window->setSize(mode.Width, mode.Height);

	sint style = mode.Windowed ? EWSWindowed:EWSFullscreen;
	if (_Resizable) style |= EWSResizable;
	_Window->setStyle(style);

	if (!mode.Windowed)
	{
		return _Display->setCurrentMode(toDisplayMode(mode));
	}

	return _Display->restoreMode();
}

bool CDriverGL::getModes(std::vector<GfxMode> &modes)
{
	H_AUTO_OGL(CDriverGL_getModes);

	std::vector<SDisplayMode> sModes;

	if (!_Display->getModes(sModes)) return false;

	for(size_t i = 0; i < sModes.size(); ++i)
	{
		modes.push_back(toGfxMode(sModes[i]));
	}

	return true;
}

bool CDriverGL::getCurrentScreenMode(GfxMode &mode)
{
	H_AUTO_OGL(CDriverGL_getCurrentScreenMode);

	SDisplayMode sMode;

	if (!_Display->getCurrentMode(sMode)) return false;

	mode = toGfxMode(sMode);

	return true;
}

/*
bool CDriverGL::setScreenMode(const GfxMode &mode)
{
	H_AUTO_OGL(CDriverGL_setScreenMode);

	if (mode.Windowed)
	{
		// if fullscreen, switch back to desktop screen mode
		if (!_CurrentMode.Windowed)
			restoreScreenMode();

		return true;
	}

	// save previous screen mode only if switching from windowed to fullscreen
	if (_CurrentMode.Windowed)
		saveScreenMode();

	return true;
}
*/
// ***************************************************************************
bool CDriverGL::destroyContext()
{
	H_AUTO_OGL(CDriverGL_destroyContext);

	if (_hDC)
		wglMakeCurrent(_hDC, NULL);

	// Then delete.
	// wglMakeCurrent(NULL,NULL);

	if (_hRC)
	{
		wglDeleteContext(_hRC);
		_hRC = NULL;
	}

	if (_hDC)
	{
		HWND hWnd = (HWND)_Window->getNativePointer();
		ReleaseDC(hWnd, _hDC);
		_hDC = NULL;
	}

	return true;
}

// --------------------------------------------------
bool CDriverGL::choosePixelFormat(sint &pf, sint antiAlias)
{
	uint samples = 0;

	// check for ARBMultisample
	if (_Extensions.ARBMultisample)
	{
		sint maxSamples = 0;

		// get maximum samples value
		glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxSamples);

		if (antiAlias < 0)
			samples = maxSamples;
		else if (antiAlias > 0)
			samples = antiAlias;
	}
	else
	{
		// disable multisample if extension doesn't exist
		antiAlias = 0;
	}

	pf = 0;

	// use wglChoosePixelFormat if WGLARBPixelFormat else ChoosePixelFormat
	if (!_Extensions.WGLARBPixelFormat)
	{
		pf = ChoosePixelFormat(_hDC, &_pfd);
		if (pf == 0)
		{
			nlwarning("ChoosePixelFormat failed");
			return false;
		}
	}
	else
	{
		sint i = 0;
		sint iAttributes[40];

/*
	WGL_DRAW_TO_BITMAP_ARB         boolean     exact
	WGL_NEED_PALETTE_ARB           boolean     exact
	WGL_NEED_SYSTEM_PALETTE_ARB    boolean     exact
	WGL_SWAP_LAYER_BUFFERS_ARB     boolean     exact
	WGL_SWAP_METHOD_ARB            enum        exact
	WGL_NUMBER_OVERLAYS_ARB        integer     minimum
	WGL_NUMBER_UNDERLAYS_ARB       integer     minimum
	WGL_SHARE_DEPTH_ARB            boolean     exact
	WGL_SHARE_STENCIL_ARB          boolean     exact
	WGL_SHARE_ACCUM_ARB            boolean     exact
	WGL_SUPPORT_GDI_ARB            boolean     exact
	WGL_PIXEL_TYPE_ARB             enum        exact
	WGL_ACCUM_BITS_ARB             integer     minimum
	WGL_ACCUM_RED_BITS_ARB         integer     minimum
	WGL_ACCUM_GREEN_BITS_ARB       integer     minimum
	WGL_ACCUM_BLUE_BITS_ARB        integer     minimum
	WGL_ACCUM_ALPHA_BITS_ARB       integer     minimum
	WGL_AUX_BUFFERS_ARB            integer     minimum
*/

		iAttributes[i++] = WGL_SUPPORT_OPENGL_ARB;
		iAttributes[i++] = GL_TRUE;

		iAttributes[i++] = WGL_ACCELERATION_ARB;
		iAttributes[i++] = WGL_FULL_ACCELERATION_ARB;

		iAttributes[i++] = WGL_DOUBLE_BUFFER_ARB;
		iAttributes[i++] = GL_TRUE;

		if (false /* _CurrentMode.OffScreen */)
		{
			iAttributes[i++] = WGL_DRAW_TO_PBUFFER_ARB;
			iAttributes[i++] = GL_TRUE;
		}
		else
		{
			iAttributes[i++] = WGL_DRAW_TO_WINDOW_ARB;
			iAttributes[i++] = GL_TRUE;
		}

		if (_Display->getDepth() > 16)
		{
			iAttributes[i++] = WGL_COLOR_BITS_ARB;
			iAttributes[i++] = 24;

			iAttributes[i++] = WGL_RED_BITS_ARB;
			iAttributes[i++] = 8;

			iAttributes[i++] = WGL_GREEN_BITS_ARB;
			iAttributes[i++] = 8;

			iAttributes[i++] = WGL_BLUE_BITS_ARB;
			iAttributes[i++] = 8;

			iAttributes[i++] = WGL_ALPHA_BITS_ARB;
			iAttributes[i++] = 8; // 8;

			iAttributes[i++] = WGL_DEPTH_BITS_ARB;
			iAttributes[i++] = 16; // 16;

			iAttributes[i++] = WGL_STENCIL_BITS_ARB;
			iAttributes[i++] = 0; // 8;
		}
		else
		{
			iAttributes[i++] = WGL_COLOR_BITS_ARB;
			iAttributes[i++] = 16;

			iAttributes[i++] = WGL_RED_BITS_ARB;
			iAttributes[i++] = 4;

			iAttributes[i++] = WGL_GREEN_BITS_ARB;
			iAttributes[i++] = 4;

			iAttributes[i++] = WGL_BLUE_BITS_ARB;
			iAttributes[i++] = 4;

			iAttributes[i++] = WGL_ALPHA_BITS_ARB;
			iAttributes[i++] = 4;

			iAttributes[i++] = WGL_DEPTH_BITS_ARB;
			iAttributes[i++] = 16;

			iAttributes[i++] = WGL_STENCIL_BITS_ARB;
			iAttributes[i++] = 8;
		}

		if (samples)
		{
			iAttributes[i++] = WGL_SAMPLE_BUFFERS_ARB;
			iAttributes[i++] = GL_TRUE;

			iAttributes[i++] = WGL_SAMPLES_ARB;
			iAttributes[i++] = samples;

//			iAttributes[i++] = WGL_COLOR_SAMPLES_NV;
//			iAttributes[i++] = 8;

//			iAttributes[i++] = WGL_COVERAGE_SAMPLES_NV;
//			iAttributes[i++] = 8;
		}

		iAttributes[i++] = 0;
		iAttributes[i++] = 0;

		float fAttributes[] = { 0, 0 };
		uint numFormats = 0;

		// First We Check To See If We Can Get A Pixel Format For 4 Samples
		sint valid = nwglChoosePixelFormatARB(_hDC, iAttributes, fAttributes, 1, &pf, &numFormats);

		// If We Returned True, And Our Format Count Is Greater Than 1
		if (!valid || numFormats < 1)
		{
			nlwarning("nwglChoosePixelFormatARB failed");
			return false;
		}

		_AntiAlias = antiAlias;
	}

	return true;
}

// --------------------------------------------------
bool CDriverGL::createContext()
{
	if (_hRC)
		wglDeleteContext(_hRC);

	_hRC = wglCreateContext(_hDC);

	if (_hRC == NULL)
	{
		nlwarning("wglCreateContext failed");
		return false;
	}

	// make the rendering context our current rendering context
	if (!wglMakeCurrent(_hDC, _hRC))
	{
		nlwarning("wglMakeCurrent failed");
		return false;
	}

	if (!_Extensions.WGLARBPixelFormat)
		registerExtensions();

	return true;
}

// --------------------------------------------------
bool CDriverGL::activate()
{
	H_AUTO_OGL(CDriverGL_activate);

	HGLRC hglrc = wglGetCurrentContext();

	if (hglrc != _hRC)
		wglMakeCurrent(_hDC, _hRC);

	return true;
}

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
