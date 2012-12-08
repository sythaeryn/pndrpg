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

#include "win32_cursor.h"
#include "win32_system.h"
#include "win32_window.h"

namespace NLMISC {

CWin32Cursor::CWin32Cursor(CWin32Window *window):ICursor(), _Window(window), _Cursor(NULL)
{
}

CWin32Cursor::~CWin32Cursor()
{
}

bool CWin32Cursor::isValid() const
{
	return _Cursor != NULL;
}

bool CWin32Cursor::getBestSize(uint /* srcWidth */, uint /* srcHeight */, uint &dstWidth, uint &dstHeight)
{
	// Windows provides default size for cursors
	dstWidth = (uint)GetSystemMetrics(SM_CXCURSOR);
	dstHeight = (uint)GetSystemMetrics(SM_CYCURSOR);

	return true;
}

bool CWin32Cursor::buildCursor(const NLMISC::CBitmap &bitmap, uint iconWidth, uint iconHeight, uint iconDepth, const NLMISC::CRGBA &col, sint hotSpotX, sint hotSpotY)
{
	reset();

	return CWin32System::convertBitmapToIcon(bitmap, _Cursor, iconWidth, iconHeight, iconDepth, col, hotSpotX, hotSpotY, true);
}

bool CWin32Cursor::reset()
{
	if (_Cursor)
	{
		DestroyIcon(_Cursor);
		_Cursor = NULL;
#if defined(NL_OS_UNIX)
		XFreeCursor(Dpy, Cursor);
		XSync(Dpy, False);
#endif
	}

	return true;
}

}
