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

#ifndef NL_DISPLAY_H
#define NL_DISPLAY_H

#include "nel/misc/types_nl.h"

namespace NLMISC {

class ISystem;
class IWindow;

struct SDisplayMode
{
	uint16 Width;
	uint16 Height;
	uint8 Depth;
	uint8 Frequency;

	SDisplayMode(uint16 width = 0, uint16 height = 0, uint8 depth = 0, uint8 frequency = 0):
		Width(width), Height(height), Depth(depth), Frequency(frequency)
	{
	}

	void reset()
	{
		Width = 0;
		Height = 0;
		Depth = 0;
		Frequency = 0;
	}

	bool operator == (const SDisplayMode& mode) const
	{
		return Width == mode.Width && Height == mode.Height && Depth == mode.Depth && Frequency == mode.Frequency;
	}
};

class IDisplay;
class CWindow;

class CDisplay
{
public:
	CWindow* createWindow();
	bool destroyWindow(sint window);

	bool setCurrentMode(const SDisplayMode &mode);
	bool getCurrentMode(SDisplayMode &mode);

	bool getModes(std::vector<SDisplayMode> &modes);

	bool saveMode();
	bool restoreMode();

	CWindow* getWindow(sint window = -1);

	uint16 getWidth() const;
	uint16 getHeight() const;
	uint8 getDepth() const;

	friend class CSystem;

protected:
	CDisplay(IDisplay *display);
	virtual ~CDisplay();

	IDisplay* _Display;

	std::map<sint, CWindow*> _Windows;
	CWindow*		_CurrentWindow;
	SDisplayMode	_CurrentMode;
};

class IDisplay
{
public:
	IDisplay();
	virtual ~IDisplay();

	virtual IWindow* createWindow() =0;

	virtual bool setCurrentMode(const SDisplayMode &mode) =0;
	virtual bool getCurrentMode(SDisplayMode &mode) =0;

	virtual bool getModes(std::vector<SDisplayMode> &modes) =0;

	virtual bool saveMode() =0;
	virtual bool restoreMode() =0;
};

}

#endif
