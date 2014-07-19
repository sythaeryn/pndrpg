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

#ifndef NL_WIN32_WINDOW_H
#define NL_WIN32_WINDOW_H

#include "nel/misc/window.h"
#include "nel/misc/cursor.h"
#include "nel/misc/event_emitter_multi.h"

#ifdef NL_OS_WINDOWS

#include "nel/misc/thread.h"

#define NOMINMAX
#include <windows.h>

namespace NLMISC {

class CWin32Display;
class CWin32System;
class CWindows7Progress;

class CWin32Window : public IWindow
{
public:
	CWin32Window(CWin32Display *display);
	virtual ~CWin32Window();

	// inherited from IWindow
	virtual bool create();
	virtual bool destroy();

	virtual bool setStyle(sint style);
	virtual bool setTitle(const ucstring &title);
	
	virtual bool setIcons(const std::vector<NLMISC::CBitmap> &icons);
	
	virtual bool setPosition(sint x, sint y);
	virtual bool getPosition(sint &x, sint &y) const;
	
	virtual bool setSize(uint width, uint height);
	virtual bool getSize(uint &width, uint &height) const;
	
	virtual bool show(bool show = true);

	virtual bool isActive() const;

	virtual void setProgressTotal(sint total);
	virtual sint getProgressTotal() const;

	virtual void setProgressValue(sint value);
	virtual sint getProgressValue() const;

	virtual IDisplay* getDisplay() { return (IDisplay*)_Display; }

	virtual NLMISC::IEventEmitter* getEventEmitter() { return &_EventEmitter; }

	virtual bool copyTextToClipboard(const ucstring &text);
	virtual bool pasteTextFromClipboard(ucstring &text);

	virtual IMouseDevice* enableLowLevelMouse(bool enable, bool exclusive);
	virtual IKeyboardDevice* enableLowLevelKeyboard(bool enable);
	virtual IInputDeviceManager* getLowLevelInputDeviceManager();
	virtual uint getDoubleClickDelay(bool hardwareMouse);

	virtual bool setCursor(ICursor *cursor);

	virtual bool createCursors();
	virtual bool releaseCursors();

	virtual ICursor* createCursor();

	virtual void showCursor(bool show);

	virtual bool isSystemCursorInClientArea();
	virtual bool isSystemCursorCaptured();

	virtual void* getNativePointer() const;


	// TODO
	virtual bool convertBitmapToCursor(const NLMISC::CBitmap &bitmap, HCURSOR &cursor, uint iconWidth, uint iconHeight, uint iconDepth, const NLMISC::CRGBA &col, sint hotSpotX, sint hotSpotY);

	virtual void setMousePos(float x, float y);
	virtual void setCapture(bool capture);

	// specific methods
	bool processMessage(UINT message, WPARAM wParam, LPARAM lParam);

	HWND getHandle() { return _Handle; }

protected:
	bool createEmitters();
	bool removeEmitters();

	friend CWin32System;
	friend CWin32Display;

private:
	CWin32Display*				_Display;
	HWND						_Handle;

	uint						_Width, _Height;
	sint						_X, _Y;
	bool						_Focused;

	NLMISC::CEventEmitterMulti	_EventEmitter;

	CWindows7Progress*			_Progress;
	NLMISC::IThread*			_ProgressThread;

	HCURSOR						_DefaultCursor;
	HCURSOR						_BlankCursor;
};

}

#endif

#endif
