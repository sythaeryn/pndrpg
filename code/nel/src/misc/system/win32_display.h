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

#ifndef NL_WIN32_DISPLAY_H
#define NL_WIN32_DISPLAY_H

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS

#include "nel/misc/display.h"
#include "nel/misc/ucstring.h"

namespace NLMISC {

class CWin32System;

class CWin32Display : public IDisplay
{
public:
	CWin32Display(CWin32System *system, const ucstring &name);
	virtual ~CWin32Display();

	// from IDisplay
	virtual IWindow* createWindow();

	virtual bool setCurrentMode(const SDisplayMode &mode);
	virtual bool getCurrentMode(SDisplayMode &mode);

	virtual bool getModes(std::vector<SDisplayMode> &modes);

	virtual bool saveMode();
	virtual bool restoreMode();

	CWin32System* getSystem() { return _System; }

	friend class CWin32Window;
	friend class CWin32System;

private:
	bool setMode(DEVMODEW *mode, DWORD flags);

	CWin32System* _System;
	ucstring _Name;
};

}

#endif

#endif
