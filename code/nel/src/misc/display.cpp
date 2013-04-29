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

#include "nel/misc/display.h"
#include "nel/misc/window.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

CDisplay::CDisplay(IDisplay *display):_Display(display), _CurrentWindow(NULL)
{
	_Display->getCurrentMode(_CurrentMode);
}

CDisplay::~CDisplay()
{
	std::map<sint, CWindow*>::const_iterator it = _Windows.begin(), iend = _Windows.end();

	if (it != iend)
	{
		if (it->second) delete it->second;

		++it;
	}

	if (_Display)
	{
		delete _Display;
		_Display = NULL;
	}
}

CWindow* CDisplay::createWindow()
{
	IWindow *nativeWindow = _Display->createWindow();

	if (!nativeWindow) return NULL;

	CWindow *window = new CWindow(this, nativeWindow);

	_Windows[window->getId()] = window;

	_CurrentWindow = window;

	return window;
}

bool CDisplay::destroyWindow(sint window)
{
	std::map<sint, CWindow*>::iterator it = _Windows.find(window);

	if (it != _Windows.end())
	{
		if (!it->second) return false;

		if (_CurrentWindow == it->second) _CurrentWindow = NULL;

//		delete it->second;

//		it->second = NULL;
	}

	return true;
}

bool CDisplay::setCurrentMode(const SDisplayMode &mode)
{
	// if switching exactly to the same screen mode, don't change it
	if (_CurrentMode == mode) return true;

	if (!_Display->setCurrentMode(mode)) return false;

	_CurrentMode = mode;

	for(size_t i = 0; i < _Windows.size(); ++i)
	{
		// update windows too
		CWindow *window = _Windows[i];

		if (!window) continue;

		// when changing window style, it's possible system change window size too
		window->setStyle(EWSFullscreen);

		window->setSize(mode.Width, mode.Height);
		window->setPosition(0, 0);

		// set color depth for custom cursor
		window->updateCursor(true);
	}

	return true;
}

bool CDisplay::getCurrentMode(SDisplayMode &mode)
{
	if (!_Display->getCurrentMode(mode)) return false;

	_CurrentMode = mode;

	return true;
}

struct CModeSorter
{
	bool operator()(const SDisplayMode &mode1, const SDisplayMode &mode2) const
	{
		if (mode1.Width == mode2.Width)
		{
			if (mode1.Height == mode2.Height) return mode1.Frequency < mode2.Frequency;

			return mode1.Height < mode2.Height;
		}

		return mode1.Width < mode2.Width;
	}
};

bool CDisplay::getModes(std::vector<SDisplayMode> &modes)
{
	modes.clear();

	if (!_Display->getModes(modes)) return false;

	std::sort(modes.begin(), modes.end(), CModeSorter());

	return true;
}

bool CDisplay::saveMode()
{
	return _Display->saveMode();
}

bool CDisplay::restoreMode()
{
	return _Display->restoreMode();
}

CWindow* CDisplay::getWindow(sint window)
{
	if (window < 0)
	{
		if (_CurrentWindow != NULL) return _CurrentWindow;

		// return first not NULL CWindow*
		std::map<sint, CWindow*>::const_iterator it = _Windows.begin(), iend = _Windows.end();

		while(it != iend)
		{
			if (it->second)
			{
				_CurrentWindow = it->second;
				return it->second;
			}

			++it;
		}
	}

	if (window >= 0 && window < _Windows.size()) return _Windows[window];

	return NULL;
}

uint16 CDisplay::getWidth() const
{
	return _CurrentMode.Width;
}

uint16 CDisplay::getHeight() const
{
	return _CurrentMode.Height;
}

uint8 CDisplay::getDepth() const
{
	return _CurrentMode.Depth;
}

IDisplay::IDisplay()
{
}

IDisplay::~IDisplay()
{
}

}
