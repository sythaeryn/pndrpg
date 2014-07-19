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

#ifndef NL_X11_SYSTEM_H
#define NL_X11_SYSTEM_H

#include "system.h"

#include "nel/misc/bitmap.h"

class CX11Window;
class CX11Display;
class CSystem;

class CX11System : public ISystem
{
protected:
	CX11System();
	virtual ~CX11System();

	virtual bool nativeInit();
	virtual bool nativeUninit();

	virtual IDisplay* nativeGetDisplay(sint display);

	virtual bool nativeCopyTextToClipboard(const ucstring &text);
	virtual bool nativePasteTextFromClipboard(ucstring &text);

	friend CX11Window;
	friend CX11Display;
	friend CSystem;

	static bool convertBitmapToIcon(const NLMISC::CBitmap &bitmap, std::vector<long> &icon);
	static void printError();

private:
	HINSTANCE _Instance;
	ATOM _Class;

	std::vector<CX11Display*> _Displays;
};

#endif
