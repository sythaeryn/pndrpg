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

#ifndef NL_X11_WINDOW_H
#define NL_X11_WINDOW_H

#include "window.h"
#include "nel/misc/event_emitter.h"

namespace NLMISC {

class CX11Display;
class CX11System;

class CX11Window : public IWindow
{
protected:
	CX11Window(CX11Display *display);
	virtual ~CX11Window();

	// inherited from IWindow
	virtual bool nativeCreate();
	virtual bool nativeDestroy();

	virtual bool nativeSetStyle(EWindowStyle style);
	virtual EWindowStyle nativeGetStyle() const;

	virtual bool nativeSetTitle(const ucstring &title);
	virtual ucstring nativeGetTitle() const;
	
	virtual bool nativeSetIcons(const std::vector<NLMISC::CBitmap> &icons);
	
	virtual bool nativeSetPosition(sint x, sint y);
	virtual bool nativeGetPosition(sint &x, sint &y) const;
	
	virtual bool nativeSetSize(sint width, sint height);
	virtual bool nativeGetSize(sint &width, sint &height) const;
	
	virtual bool nativeShow(bool show = true);

	virtual bool nativeIsActive() const;

	virtual void nativeSetProgressTotal(sint total);
	virtual sint nativeGetProgressTotal() const;

	virtual void nativeSetProgressValue(sint value);
	virtual sint nativeGetProgressValue() const;

	virtual IDisplay* nativeGetDisplay() { return (IDisplay*)_Display; }

	virtual NLMISC::IEventEmitter* nativeGetEventEmitter() { return NULL; }

	// specific methods
	bool processMessage(const XEvent &e);

private:
	CX11Display* _Display;

	Window _Window;
	sint _Width, _Height;
	sint _X, _Y;
	bool _Active;

	sint _DecorationWidth;
	sint _DecorationHeight;

	std::string _Title;

	NLMISC::CUnixEventEmitter _EventEmitter;
};

}

#endif
