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

#ifndef NL_SYSTEM_H
#define NL_SYSTEM_H

#include "nel/misc/types_nl.h"

#include "display.h"
#include "window.h"

namespace NLMISC {

class ISystem;
class IDisplay;
class IWindow;

class CDisplay;

class CSystem
{
public:
	static bool setSystem(ISystem *system);

	CDisplay* getDisplay(sint display = -1);

	bool isAlphaBlendedCursorSupported() const;

	static CSystem* instance();
	static void release();

protected:
	CSystem(ISystem *system = NULL);
	virtual ~CSystem();

	ISystem* _System;
	std::vector<CDisplay*> _Displays;
};

class ISystem
{
protected:
	ISystem();
	virtual ~ISystem();

	virtual bool init() =0;
	virtual bool uninit() =0;

//	virtual IDisplay* getDisplay(sint display) =0;
	virtual bool getDisplays(std::vector<IDisplay*> &displays) =0;
	virtual bool isAlphaBlendedCursorSupported() const =0;

	friend CSystem;
};

}

#endif
