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

#ifndef NL_WIN32_SYSTEM_H
#define NL_WIN32_SYSTEM_H

#include "nel/misc/system.h"
#include "nel/misc/bitmap.h"

#ifdef NL_OS_WINDOWS

namespace NLMISC {

class CWin32Window;
class CWin32Display;
class CSystem;

class CWin32System : public ISystem
{
public:
	CWin32System();
	virtual ~CWin32System();

	virtual bool init();
	virtual bool uninit();

	virtual bool getDisplays(std::vector<IDisplay*> &displays);
	virtual bool isAlphaBlendedCursorSupported() const { return _AlphaBlendedCursorSupported; }

	LPCWSTR getClassName() const;

	friend CWin32Window;
	friend CWin32Display;
	friend CSystem;

	static bool convertBitmapToIcon(const NLMISC::CBitmap &bitmap, HICON &icon, uint iconWidth, uint iconHeight, uint iconDepth, const NLMISC::CRGBA &col = NLMISC::CRGBA::White, sint hotSpotX = 0, sint hotSpotY = 0, bool cursor = false);
	static void printError();

private:
	HINSTANCE _Instance;
	ATOM _Class;
	bool _AlphaBlendedCursorSupported;
	ucstring _ClassName;
};

}

#endif

#endif
