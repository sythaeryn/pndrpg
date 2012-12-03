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


#include "nel/misc/types_nl.h"

#include <cstdlib>

#include <Windows.h>

#if defined(_MSC_VER) && defined(_DEBUG)
	#define _CRTDBG_MAP_ALLOC
	#ifdef _malloca
		#undef _malloca
	#endif
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#ifdef realloc
		#undef realloc
	#endif
#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

#include "nel/misc/debug.h"
#include "nel/misc/string_common.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/event_server.h"
#include "nel/misc/events.h"
#include "nel/misc/game_device_events.h"
#include "nel/misc/file.h"
#include "nel/misc/system.h"

using namespace std;
using namespace NLMISC;

class CEventsListener : public NLMISC::IEventListener
{
public:
	CEventsListener()
	{
		_Closed = false;
		_Progress = 0;
		_Total = 100;
		_Window = NULL;
	}

	virtual ~CEventsListener()
	{
	}

	void setWindow(CWindow *window)
	{
		_Window = window;

		if(_Window) _Window->setProgressTotal(_Total);
	}

	void addToServer(CEventServer& server)
	{
		server.addListener(EventGDMouseMove,	this);
		server.addListener(EventMouseMoveId,	this);
		server.addListener(EventMouseDownId,	this);
		server.addListener(EventMouseUpId,		this);
		server.addListener(EventMouseWheelId,	this);
		server.addListener(EventKeyDownId,		this);
		server.addListener(EventKeyUpId,		this);
		server.addListener(EventDestroyWindowId,this);
		server.addListener(EventCloseWindowId,this);
		server.addListener(EventSetFocusId,		this);
		server.addListener(EventDisplayChangeId,	this);
	}

	void removeFromServer (CEventServer& server)
	{
		server.removeListener(EventGDMouseMove,	this);
		server.removeListener(EventMouseMoveId,	this);
		server.removeListener(EventMouseDownId,	this);
		server.removeListener(EventMouseUpId,	this);
		server.removeListener(EventMouseWheelId,this);
		server.removeListener(EventKeyDownId,	this);
		server.removeListener(EventKeyUpId,		this);
		server.removeListener(EventDestroyWindowId,this);
		server.removeListener(EventCloseWindowId,this);
		server.removeListener(EventSetFocusId,		this);
	}

	bool closed() const { return _Closed; }
	sint getProgress() const { return _Progress; }
	sint getTotal() const { return _Total; }

protected:
	virtual void operator ()(const CEvent& event)
	{
		// Window closed.
		if(event == EventDestroyWindowId)
		{
		}

		if(event == EventCloseWindowId)
		{
			_Closed = true;
		}

		// Event from the Mouse (ANGLE)
		if(event == EventGDMouseMove)
		{
		}
		// Event from the Mouse (MOVE)
		else if(event == EventMouseMoveId)
		{
		}
		// Event from the Mouse (DOWN BUTTONS)
		else if(event == EventMouseDownId)
		{
		}
		// Event from the Mouse (UP BUTTONS)
		else if(event == EventMouseUpId)
		{
		}
		// Event from the Mouse (WHEEL)
		else if(event == EventMouseWheelId)
		{
			CEventMouseWheel *wheelEvent = dynamic_cast<CEventMouseWheel*>(event.clone());

			_Progress += wheelEvent->Direction ? -1:1;
			NLMISC::clamp(_Progress, 0, _Total);

			if(_Window)	_Window->setProgressValue(_Progress);

			delete wheelEvent;
		}

		if (event==EventSetFocusId)
		{
		}
	}

	bool _Closed;
	sint _Progress;
	sint _Total;
	CWindow* _Window;
};

int main (int argc, char **argv)
{
#if defined(_MSC_VER) && defined(_DEBUG)
	_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

//	NLMISC::CApplicationContext *appContext = new NLMISC::CApplicationContext();
	NLMISC::CApplicationContext appContext;

	NLMISC::CEventServer EventServer;
	CEventsListener EventsListener;

	CWindow *window = CSystem::instance()->getDisplay()->createWindow();

	EventServer.addEmitter(window->getEventEmitter());
	EventsListener.addToServer(EventServer);
	EventsListener.setWindow(window);

	// IWindow tests
	window->show();
	window->setTitle(ucstring("NeL System Window"));

	NLMISC::CIFile file;

	if (file.open("d:/ryzom_public/code/ryzom/client/unix/ryzom_client.png"))
	{
		NLMISC::CBitmap bitmap;
		bitmap.load(file);
		window->setIcon(bitmap);
	}

	window->setPosition(300, 300);
	window->setSize(200, 200);
//	window->setStyle(EWSMaximized);

#if 0
	// IDisplay tests
	IDisplay *display = window->getDisplay();

	SDisplayMode mode;

	if (display->getCurrentMode(mode))
	{
		nldebug("getCurrentMode: ok");
	}

	std::vector<SDisplayMode> modes;

	if (display->getModes(modes))
	{
		nldebug("getModes: ok");
	}

//	mode.width = 1025;
//	mode.height = 768;

	if (display->setCurrentMode(mode))
	{
		nldebug("setCurrentMode: ok");
	}
#endif

#if 1
	// ISystem tests
	ucstring str1("bonjour"), str2;

	if (window->copyTextToClipboard(str1))
	{
		if (window->pasteTextFromClipboard(str2))
		{
			if (str1 == str2)
			{
				nldebug("copy-paste: ok");
			}
		}
	}
#endif

#if 1
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		EventServer.pump(true);

		if (EventsListener.closed()) break;
	}
#else
	while(!EventsListener.closed())
	{
		EventServer.pump(true);
	}
#endif

//	CSystem::instance()->getDisplay(0)->destroyWindow(window->getId());

	CSystem::instance()->release();

	destroyDebug();
	CLog::releaseProcessName();

//	delete appContext;

	CInstanceCounterManager::releaseInstance();

	return EXIT_SUCCESS;
}
