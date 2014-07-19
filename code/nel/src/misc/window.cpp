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

#include "stdmisc.h"

#include "nel/misc/window.h"
#include "nel/misc/display.h"
#include "nel/misc/system.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

static sint _NextId = 0;

CWindow::CWindow(CDisplay *display, IWindow *window):_Display(display), _Window(window), _Id(-1)
{
	nlassert(window != NULL);

	_Id = _NextId++;
}

CWindow::~CWindow()
{
	delete _Window;
	_Window = NULL;
}

bool CWindow::destroy()
{
	releaseCursors();

	// make sure window icons are deleted
	std::vector<NLMISC::CBitmap> bitmaps;
	setIcons(bitmaps);

	if (!_Window->destroy()) return false;

	return true;
}

bool CWindow::setIcon(const NLMISC::CBitmap &icon)
{
	std::vector<NLMISC::CBitmap> icons;
	icons.push_back(icon);
	
	return setIcons(icons);
}

bool CWindow::setIcons(const std::vector<NLMISC::CBitmap> &icons)
{
	return _Window->setIcons(icons);
}

sint CWindow::getStyle() const
{
	return _Window->_Style;
}

bool CWindow::setStyle(sint style)
{
	if (_Window->_Style == style) return true;

	// remove fullscreen style if also windowed
	if (style & (EWSWindowed|EWSFullscreen)) style ^= EWSFullscreen;

	if (_Window->setStyle(style))
	{
		_Window->_Style = style;

		return true;
	}

	return false;
}

bool CWindow::setTitle(const ucstring &title)
{
	return _Window->setTitle(title);
}

bool CWindow::setPosition(sint x, sint y)
{
	return _Window->setPosition(x, y);
}

bool CWindow::show(bool show)
{
	return _Window->show(show);
}

bool CWindow::hide()
{
	return show(false);
}

bool CWindow::getSize(uint &width, uint &height) const
{
	return _Window->getSize(width, height);
}

bool CWindow::setSize(uint width, uint height)
{
	return _Window->setSize(width, height);
}

bool CWindow::getPosition(sint &x, sint &y) const
{
	return _Window->getPosition(x, y);
}

bool CWindow::isActive() const
{
	return _Window->isActive();
}

void CWindow::setProgressTotal(sint total)
{
	_Window->setProgressTotal(total);
}

sint CWindow::getProgressTotal() const
{
	return _Window->getProgressTotal();
}

void CWindow::setProgressValue(sint value)
{
	_Window->setProgressValue(value);
}

sint CWindow::getProgressValue() const
{
	return _Window->getProgressValue();
}

NLMISC::IEventEmitter* CWindow::getEventEmitter()
{
	return _Window->getEventEmitter();
}

bool CWindow::copyTextToClipboard(const ucstring &text)
{
	return _Window->copyTextToClipboard(text);
}

bool CWindow::pasteTextFromClipboard(ucstring &text)
{
	return _Window->pasteTextFromClipboard(text);
}

NLMISC::IMouseDevice* CWindow::enableLowLevelMouse(bool enable, bool exclusive)
{
	return _Window->enableLowLevelMouse(enable, exclusive);
}

NLMISC::IKeyboardDevice* CWindow::enableLowLevelKeyboard(bool enable)
{
	return _Window->enableLowLevelKeyboard(enable);
}

NLMISC::IInputDeviceManager* CWindow::getLowLevelInputDeviceManager()
{
	return _Window->getLowLevelInputDeviceManager();
}

uint CWindow::getDoubleClickDelay(bool hardwareMouse)
{
	return _Window->getDoubleClickDelay(hardwareMouse);
}

bool CWindow::addCursor(const std::string &name, const NLMISC::CBitmap &cursorBitmap)
{
	if (!CSystem::instance()->isAlphaBlendedCursorSupported()) return false;

	CCursor *cursor = NULL;
	TCursorMap::iterator it = _Cursors.find(name);

	if (it == _Cursors.end())
	{
		cursor = new CCursor(this, _Window->createCursor());

		_Cursors[name] = cursor;
	}
	else
	{
		cursor = it->second;

		// erase possible previous cursor
		cursor->reset();
	}

	cursor->setBitmap(cursorBitmap);

	if (name == _CurrName)
	{
		updateCursor();
	}

	return true;
}

bool CWindow::setCursor(const std::string &name, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY, bool forceRebuild)
{
	// don't update cursor if it's hidden or if custom cursors are not suppported
	if (!CSystem::instance()->isAlphaBlendedCursorSupported() || _CurrName == "none") return false;

	_CurrName = name;

	// same than 'CViewRenderer::drawRotFlipBitmapTiled
	if (rot > 3) rot = 3;

	_CurrentCursor.Col = col;
	_CurrentCursor.Rot = rot;
	_CurrentCursor.HotSpotX = hotSpotX;
	_CurrentCursor.HotSpotY = hotSpotY;

	return updateCursor(forceRebuild);
}

bool CWindow::updateCursor(bool forceRebuild)
{
	// cursor has to be changed next time
	if (_CurrName.empty()) return false;

	TCursorMap::iterator it = _Cursors.find(_CurrName);

	CCursor *cursor = NULL;

	if (it != _Cursors.end())
	{
		// Update cursor if modified or not already built
		cursor = it->second;

		sint hotSpotX = (sint) (cursor->HotspotScale * (_CurrentCursor.HotSpotX - cursor->HotspotOffsetX));
		sint hotSpotY = (sint) (cursor->HotspotScale * ((cursor->OrigHeight - _CurrentCursor.HotSpotY) - cursor->HotspotOffsetY));

		if (!cursor->isValid() ||
			cursor->HotSpotX != hotSpotX || cursor->HotSpotY != hotSpotY ||
			cursor->Col != _CurrentCursor.Col || cursor->Rot != _CurrentCursor.Rot ||
			cursor->ColorDepth != _Display->getDepth() ||
			forceRebuild
		   )
		{
			cursor->reset();
			cursor->Col = _CurrentCursor.Col;
			cursor->Rot = _CurrentCursor.Rot;
			cursor->HotSpotX = hotSpotX;
			cursor->HotSpotY = hotSpotY;
			cursor->ColorDepth = _Display->getDepth() == 32 ? ColorDepth32:ColorDepth16;
			cursor->buildCursor();
		}
	}

	if (_Window->isSystemCursorInClientArea() || _Window->isSystemCursorCaptured() || forceRebuild)
	{
		_Window->setCursor(cursor ? cursor->getNative():NULL);
	}

	return true;
}

bool CWindow::createCursors()
{
	return _Window->createCursors();
}

bool CWindow::releaseCursors()
{
	// release custom cursors
	TCursorMap::const_iterator it = _Cursors.begin(), iend = _Cursors.end();

	while(it != iend)
	{
		delete it->second;

		++it;
	}

	_Cursors.clear();

	// release system cursors
	return _Window->releaseCursors();
}

void CWindow::setSystemArrow()
{
	if (_Window->isSystemCursorInClientArea() || _Window->isSystemCursorCaptured())
	{
		_Window->setCursor(NULL);
	}
}

void CWindow::showCursor(bool show)
{
	if (show)
	{
		// update current hardware icon to avoid to have the plain arrow
		updateCursor(true);
	}

	_Window->showCursor(show);
}

bool CWindow::isSystemCursorCaptured()
{
	return _Window->isSystemCursorCaptured();
}

void* CWindow::getNativePointer() const
{
	return _Window->getNativePointer();
}

void CWindow::addSizeListener(IWindowSizeListener *listener)
{
	if (std::find(_Window->_SizeListeners.begin(), _Window->_SizeListeners.end(), listener) == _Window->_SizeListeners.end()) return;

	_Window->_SizeListeners.push_back(listener);
}

void CWindow::removeSizeListener(IWindowSizeListener *listener)
{
	_Window->_SizeListeners.remove(listener);
}

void CWindow::addStyleListener(IWindowStyleListener *listener)
{
	if (std::find(_Window->_StyleListeners.begin(), _Window->_StyleListeners.end(), listener) == _Window->_StyleListeners.end()) return;

	_Window->_StyleListeners.push_back(listener);
}

void CWindow::removeStyleListener(IWindowStyleListener *listener)
{
	_Window->_StyleListeners.remove(listener);
}

void CWindow::addPositionListener(IWindowPositionListener *listener)
{
	if (std::find(_Window->_PositionListeners.begin(), _Window->_PositionListeners.end(), listener) == _Window->_PositionListeners.end()) return;

	_Window->_PositionListeners.push_back(listener);
}

void CWindow::removePositionListener(IWindowPositionListener *listener)
{
	_Window->_PositionListeners.remove(listener);
}

void CWindow::addFocusListener(IWindowFocusListener *listener)
{
	if (std::find(_Window->_FocusListeners.begin(), _Window->_FocusListeners.end(), listener) == _Window->_FocusListeners.end()) return;

	_Window->_FocusListeners.push_back(listener);
}

void CWindow::removeFocusListener(IWindowFocusListener *listener)
{
	_Window->_FocusListeners.remove(listener);
}

uint CWindow::getWidth() const
{
	uint width, height;
	
	if (_Window->getSize(width, height)) return width;

	return -1;
}

uint CWindow::getHeight() const
{
	uint width, height;
	
	if (_Window->getSize(width, height)) return height;

	return -1;
}

IWindow::IWindow():_Style(EWSWindowed), _Title(ucstring("NeL Window"))
{
}

IWindow::~IWindow()
{
}

}
