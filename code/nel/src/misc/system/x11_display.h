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

#ifndef NL_X11_DISPLAY_H
#define NL_X11_DISPLAY_H

#include "display.h"

class CX11System;

class CX11Display : public IDisplay
{
protected:
	CX11Display(CX11System *system, const ucstring &name);
	virtual ~CX11Display();

	IWindow* nativeCreateWindow();
	bool nativeDestroyWindow(sint window);

	CX11System* getSystem() { return _System; }

	bool nativeSetCurrentMode(const SDisplayMode &mode);
	bool nativeGetCurrentMode(SDisplayMode &mode);

	bool nativeGetModes(std::vector<SDisplayMode> &modes);

	friend class CWin32Window;
	friend class CWin32System;

private:
	CX11System* _System;

	Display _Display;

	std::map<sint, CX11Window*> _Windows;
};

#endif
