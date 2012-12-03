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

#include "win32_display.h"

#ifdef NL_OS_WINDOWS
#include "win32_system.h"
#include "win32_window.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

CWin32Display::CWin32Display(CWin32System *system, const ucstring &name):IDisplay(), _System(system), _Name(name)
{
}

CWin32Display::~CWin32Display()
{
}

IWindow* CWin32Display::createWindow()
{
	CWin32Window *window = new CWin32Window(this);

	if (window->create()) return window;

	nlwarning("Unable to create window");

	return NULL;
}

bool CWin32Display::getCurrentMode(SDisplayMode &mode)
{
	DEVMODEW devmode;
	devmode.dmSize = sizeof(DEVMODEW);
	devmode.dmDriverExtra = 0;

	if (EnumDisplaySettingsW((wchar_t*)_Name.c_str(), ENUM_CURRENT_SETTINGS, &devmode) == 0)
	{
		CWin32System::printError();
		return false;
	}

	mode.Depth = (uint8)devmode.dmBitsPerPel;
	mode.Frequency = devmode.dmDisplayFrequency;
	mode.Width = (uint16)devmode.dmPelsWidth;
	mode.Height = (uint16)devmode.dmPelsHeight;
	// devmode.dmDisplayFlags

	return true;
}

bool CWin32Display::setMode(DEVMODEW *mode, DWORD flags)
{
	LONG res = ChangeDisplaySettingsExW((wchar_t*)_Name.c_str(), mode, NULL, flags, NULL);

	if (res == DISP_CHANGE_SUCCESSFUL) return true;

	std::string message;

	switch(res)
	{
#ifdef DISP_CHANGE_BADDUALVIEW
		case DISP_CHANGE_BADDUALVIEW:
		message = "The settings change was unsuccessful because the system is DualView capable.";
		break;
#endif
		case DISP_CHANGE_BADFLAGS:
		message = "An invalid set of flags was passed in.";
		break;

		case DISP_CHANGE_BADMODE:
		message = "The graphics mode is not supported.";
		break;

		case DISP_CHANGE_BADPARAM:
		message = "An invalid parameter was passed in. This can include an invalid flag or combination of flags.";
		break;

		case DISP_CHANGE_FAILED:
		message = "The display driver failed the specified graphics mode.";
		break;

		case DISP_CHANGE_NOTUPDATED:
		message = "Unable to write settings to the registry.";
		break;

		case DISP_CHANGE_RESTART:
		message = "The computer must be restarted for the graphics mode to work.";
		break;

		default:
		message = NLMISC::toString("Unknown error %d", (sint)res);
		break;
	}

	nlwarning("Unable to set display mode: %s", message.c_str());

	return false;
}

bool CWin32Display::setCurrentMode(const SDisplayMode &mode)
{
	DEVMODEW devMode;
	memset(&devMode, 0, sizeof(DEVMODEW));
	devMode.dmSize        = sizeof(DEVMODEW);
	devMode.dmDriverExtra = 0;
	devMode.dmFields      = DM_PELSWIDTH | DM_PELSHEIGHT;
	devMode.dmPelsWidth   = mode.Width;
	devMode.dmPelsHeight  = mode.Height;

	if (mode.Depth > 0)
	{
		devMode.dmBitsPerPel  = mode.Depth;
		devMode.dmFields     |= DM_BITSPERPEL;
	}

	if (mode.Frequency > 0)
	{
		devMode.dmDisplayFrequency  = mode.Frequency;
		devMode.dmFields           |= DM_DISPLAYFREQUENCY;
	}

//	DM_POSITION
//	DM_DISPLAYFLAGS

	return setMode(&devMode, CDS_FULLSCREEN);
}

bool CWin32Display::getModes(std::vector<SDisplayMode> &modes)
{
	sint modeIndex = 0;

	DEVMODEW devMode;
	devMode.dmSize = sizeof(DEVMODEW);
	devMode.dmDriverExtra = 0;

	while (EnumDisplaySettingsW((wchar_t*)_Name.c_str(), modeIndex, &devMode))
	{
		// Keep only 16 and 32 bits
		if ((devMode.dmBitsPerPel == 16) || (devMode.dmBitsPerPel == 32))
		{
			// Add this mode
			SDisplayMode mode;
			mode.Width = (uint16)devMode.dmPelsWidth;
			mode.Height = (uint16)devMode.dmPelsHeight;
			mode.Depth = (uint8)devMode.dmBitsPerPel;
			mode.Frequency = (uint8)devMode.dmDisplayFrequency;

			modes.push_back(mode);
		}

		// Mode index
		modeIndex++;
	}

	return true;
}

bool CWin32Display::saveMode()
{
	// don't need to save it because Windows will use default desktop resolution
	return true;
}

bool CWin32Display::restoreMode()
{
	// restore default display mode
	return setMode(NULL, 0);
}

/*

	// Monitor color parameters backup
	bool							_NeedToRestaureGammaRamp;
	uint16							_GammaRampBackuped[3*256];

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

}

#endif
