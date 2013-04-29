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

#include "nel/misc/system.h"
#include "nel/misc/window.h"
#include "nel/misc/display.h"

#if defined(NL_OS_WINDOWS)
#include "system/win32_system.h"
#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

CSystem* _SystemInstance = NULL;

CSystem::CSystem(ISystem *system):_System(system)
{
	if (!system)
	{
		// use default native platforms
#if defined(NL_OS_WINDOWS)
		_System = new CWin32System();
#elif defined(NL_OS_MAC)
		_System = new CCocoaSystem();
#elif defined(NL_OS_UNIX)
		_System = new CX11System();
#else
		_System = new CNoSystem();
#endif
	}

	if (!_System->init()) return;

	std::vector<IDisplay*> displays;

	if (_System->getDisplays(displays))
	{
		for(size_t i = 0; i < displays.size(); ++i)
		{
			_Displays.push_back(new CDisplay(displays[i]));
		}
	}
}

CSystem::~CSystem()
{
	for(size_t i = 0; i < _Displays.size(); ++i)
	{
		delete _Displays[i];
		_Displays[i] = NULL;
	}

	if (_System)
	{
		if (!_System->uninit()) return;

		delete _System;
		_System = NULL;
	}
}

bool CSystem::setSystem(ISystem *system)
{
	if (_SystemInstance)
	{
		_SystemInstance->release();
	}

	_SystemInstance = new CSystem(system);

	return true;
}

CDisplay* CSystem::getDisplay(sint display)
{
	if (display < 0) display = 0;

	if (display >= 0 && display < _Displays.size()) return _Displays[display];

	return NULL;
}

bool CSystem::isAlphaBlendedCursorSupported() const
{
	return _System->isAlphaBlendedCursorSupported();
}

CSystem* CSystem::instance()
{
	if (_SystemInstance == NULL)
	{
		_SystemInstance = new CSystem();

		nlwarning("New CSystem singleton %p", _SystemInstance);
	}

	return _SystemInstance;
}

void CSystem::release()
{
	if (_SystemInstance)
	{
		delete _SystemInstance;
		_SystemInstance = NULL;
	}
}

ISystem::ISystem()
{
}

ISystem::~ISystem()
{
}

}
