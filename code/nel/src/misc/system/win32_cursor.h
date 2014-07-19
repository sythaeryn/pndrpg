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

#ifndef NL_WIN32_CURSOR_H
#define NL_WIN32_CURSOR_H

#include "nel/misc/cursor.h"

namespace NLMISC {

class CWin32Window;

class CWin32Cursor : public ICursor
{
public:
	CWin32Cursor(CWin32Window *window);
	virtual ~CWin32Cursor();

	virtual bool isValid() const;
	virtual bool getBestSize(uint srcWidth, uint srcHeight, uint &dstWidth, uint &dstHeight);
	virtual bool buildCursor(const NLMISC::CBitmap &bitmap, uint iconWidth, uint iconHeight, uint iconDepth, const NLMISC::CRGBA &col, sint hotSpotX, sint hotSpotY);
	virtual bool reset();

	HCURSOR getCursor() { return _Cursor; }

protected:
	CWin32Window* _Window;
	HCURSOR _Cursor;
};

}

#endif
