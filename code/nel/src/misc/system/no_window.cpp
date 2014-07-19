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

#include "no_window.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

CNoWindow::CNoWindow(CNoDisplay *display):IWindow(), _Display(display),
	_Width(0), _Height(0), _X(0), _Y(0), _Active(false), _Title(ucstring("NeL Window"))
{
}

CNoWindow::~CNoWindow()
{
}

bool CNoWindow::nativeSetIcons(const std::vector<NLMISC::CBitmap> &bitmaps)
{
	return true;
}

bool CNoWindow::nativeCreate()
{
	return true;
}

bool CNoWindow::nativeDestroy()
{
	return true;
}

EWindowStyle CNoWindow::nativeGetStyle() const
{
	return EWSWindowed;
}

bool CNoWindow::nativeSetStyle(EWindowStyle windowStyle)
{
	return true;
}

bool CNoWindow::nativeSetTitle(const ucstring &title)
{
	_Title = title;

	return true;
}

ucstring CNoWindow::nativeGetTitle() const
{
	return _Title;
}

bool CNoWindow::nativeSetPosition(sint x, sint y)
{
	_X = x;
	_Y = y;

	return true;
}

bool CNoWindow::nativeShow(bool show)
{
	return true;
}

bool CNoWindow::nativeGetSize(sint &width, sint &height) const
{
	width = _Width;
	height = _Height;

	return true;
}

bool CNoWindow::nativeSetSize(sint width, sint height)
{
	_Width = width;
	_Height = height;

	return true;
}

bool CNoWindow::nativeGetPosition(sint &x, sint &y) const
{
	x = _X;
	y = _Y;

	return true;
}

bool CNoWindow::nativeIsActive() const
{
	return _Active;
}

void CNoWindow::nativeSetProgressTotal(sint total)
{
}

sint CNoWindow::nativeGetProgressTotal() const
{
	return -1;
}

void CNoWindow::nativeSetProgressValue(sint value)
{
}

sint CNoWindow::nativeGetProgressValue() const
{
	return -1;
}

}

/*

// ***************************************************************************
IDriver::TMessageBoxId IDriver::systemMessageBox (const char* message, const char* title, IDriver::TMessageBoxType type, IDriver::TMessageBoxIcon icon)
{
	static const char* icons[iconCount]=
	{
		"",
		"WAIT:\n",
		"QUESTION:\n",
		"HEY!\n",
		"",
		"WARNING!\n",
		"ERROR!\n",
		"INFORMATION:\n",
		"STOP:\n"
	};
	static const char* messages[typeCount]=
	{
		"Press any key...",
		"(O)k or (C)ancel ?",
		"(Y)es or (N)o ?",
		"(A)bort (R)etry (I)gnore ?",
		"(Y)es (N)o (C)ancel ?",
		"(R)etry (C)ancel ?"
	};
	printf ("%s%s\n%s", icons[icon], title, message);
	for(;;)
	{
		printf ("\n%s", messages[type]);
		int c=getchar();
		if (type==okType)
			return okId;
		switch (c)
		{
		case 'O':
		case 'o':
			if ((type==okType)||(type==okCancelType))
				return okId;
			break;
		case 'C':
		case 'c':
			if ((type==yesNoCancelType)||(type==okCancelType)||(type==retryCancelType))
				return cancelId;
			break;
		case 'Y':
		case 'y':
			if ((type==yesNoCancelType)||(type==yesNoType))
				return yesId;
			break;
		case 'N':
		case 'n':
			if ((type==yesNoCancelType)||(type==yesNoType))
				return noId;
			break;
		case 'A':
		case 'a':
			if (type==abortRetryIgnoreType)
				return abortId;
			break;
		case 'R':
		case 'r':
			if (type==abortRetryIgnoreType)
				return retryId;
			break;
		case 'I':
		case 'i':
			if (type==abortRetryIgnoreType)
				return ignoreId;
			break;
		}
	}
	nlassert (0);		// no!
	return okId;
}

*/