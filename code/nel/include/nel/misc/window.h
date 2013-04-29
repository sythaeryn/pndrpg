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

#ifndef NL_WINDOW_H
#define NL_WINDOW_H

#include "nel/misc/types_nl.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/event_emitter.h"
#include "nel/misc/mouse_device.h"
#include "nel/misc/keyboard_device.h"
#include "nel/misc/input_device_manager.h"
#include "cursor.h"

#include <vector>

namespace NLMISC {

enum EWindowStyle
{
	EWSNone = 0,
	EWSWindowed = 1,
	EWSFullscreen = 2,
	EWSMaximized = 4,
	EWSResizable = 8
};

class IWindowSizeListener
{
public:
	virtual void onWindowSizeChanged(uint width, uint height) =0;
};

class IWindowStyleListener
{
public:
	virtual void onWindowStyleChanged(int style) =0;
};

class IWindowPositionListener
{
public:
	virtual void onWindowPositionChanged(sint x, sint y) =0;
};

class IWindowFocusListener
{
public:
	virtual void onWindowFocusChanged(bool focused) =0;
};

class IDisplay;
class IWindow;

class CDisplay;

struct CStrCaseUnsensitiveCmp
{
	bool operator()(const std::string &lhs, const std::string &rhs) const
	{
		return NLMISC::nlstricmp(lhs, rhs) < 0;
	}
};

typedef std::map<std::string, CCursor*, CStrCaseUnsensitiveCmp> TCursorMap;

class CWindow
{
public:
	CWindow(CDisplay *display, IWindow *window);
	virtual ~CWindow();

	bool destroy();

	sint getId() const { return _Id; }

	bool setStyle(sint style);
	sint getStyle() const;

	bool setTitle(const ucstring &title);
	ucstring getTitle() const;
	
	bool setIcon(const NLMISC::CBitmap &icon);
	bool setIcons(const std::vector<NLMISC::CBitmap> &icons);
	
	bool setPosition(sint x, sint y);
	bool getPosition(sint &x, sint &y) const;

	bool setSize(uint width, uint height);
	bool getSize(uint &width, uint &height) const;
	
	bool show(bool show = true);
	bool hide();
	
	bool isActive() const;

	void setProgressTotal(sint total);
	sint getProgressTotal() const;

	void setProgressValue(sint value);
	sint getProgressValue() const;
	
	IWindow* getNative() { return _Window; }
	CDisplay* getDisplay() { return _Display; }

	NLMISC::IEventEmitter* getEventEmitter();

	bool copyTextToClipboard(const ucstring &text);
	bool pasteTextFromClipboard(ucstring &text);

	NLMISC::IMouseDevice* enableLowLevelMouse(bool enable, bool exclusive);
	NLMISC::IKeyboardDevice* enableLowLevelKeyboard(bool enable);
	NLMISC::IInputDeviceManager* getLowLevelInputDeviceManager();
	uint getDoubleClickDelay(bool hardwareMouse);

	bool addCursor(const std::string &name, const NLMISC::CBitmap &cursorBitmap);
	bool setCursor(const std::string &name, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY, bool forceRebuild = false);
	bool updateCursor(bool forceRebuild = false);

	bool createCursors();
	bool releaseCursors();

	void setSystemArrow();

	void showCursor(bool show = true);

	bool isSystemCursorCaptured();

	void* getNativePointer() const;

	// listeners
	void addSizeListener(IWindowSizeListener *listener);
	void removeSizeListener(IWindowSizeListener *listener);

	void addStyleListener(IWindowStyleListener *listener);
	void removeStyleListener(IWindowStyleListener *listener);

	void addPositionListener(IWindowPositionListener *listener);
	void removePositionListener(IWindowPositionListener *listener);

	void addFocusListener(IWindowFocusListener *listener);
	void removeFocusListener(IWindowFocusListener *listener);

	// helpers
	uint getWidth() const;
	uint getHeight() const;

	friend class CSystem;
	friend class IDisplay;

protected:
	IWindow*		_Window;
	CDisplay*		_Display;

	sint			_Id;

	TCursorMap		_Cursors;
	CCursor			_CurrentCursor;
	std::string		_CurrName;
};

class IWindow
{
public:
	IWindow();
	virtual ~IWindow();

	virtual bool create() =0;
	virtual bool destroy() =0;

	virtual bool setStyle(sint style) =0;
	virtual bool setTitle(const ucstring &title) =0;
	
	virtual bool setIcons(const std::vector<NLMISC::CBitmap> &icons) =0;
	
	virtual bool setPosition(sint x, sint y) =0;
	virtual bool getPosition(sint &x, sint &y) const =0;
	
	virtual bool setSize(uint width, uint height) =0;
	virtual bool getSize(uint &width, uint &height) const =0;
	
	virtual bool show(bool show = true) =0;

	virtual bool isActive() const =0;

	virtual void setProgressTotal(sint total) =0;
	virtual sint getProgressTotal() const =0;

	virtual void setProgressValue(sint value) =0;
	virtual sint getProgressValue() const =0;

	virtual IDisplay* getDisplay() =0;

	virtual NLMISC::IEventEmitter* getEventEmitter() =0;

	virtual bool copyTextToClipboard(const ucstring &text) =0;
	virtual bool pasteTextFromClipboard(ucstring &text) =0;

	virtual NLMISC::IMouseDevice* enableLowLevelMouse(bool enable, bool exclusive) =0;
	virtual NLMISC::IKeyboardDevice* enableLowLevelKeyboard(bool enable) =0;
	virtual NLMISC::IInputDeviceManager* getLowLevelInputDeviceManager() =0;
	virtual uint getDoubleClickDelay(bool hardwareMouse) =0;

	virtual bool setCursor(ICursor *cursor) =0;

	virtual bool createCursors() =0;
	virtual bool releaseCursors() =0;

	virtual ICursor* createCursor() =0;

	virtual void showCursor(bool show) =0;

	virtual bool isSystemCursorInClientArea() =0;
	virtual bool isSystemCursorCaptured() =0;

	virtual void* getNativePointer() const =0;

	friend CWindow;

protected:
	sint		_Style;
	ucstring	_Title;

	std::list<IWindowSizeListener*> _SizeListeners;
	std::list<IWindowStyleListener*> _StyleListeners;
	std::list<IWindowPositionListener*> _PositionListeners;
	std::list<IWindowFocusListener*> _FocusListeners;
};

}

#endif
