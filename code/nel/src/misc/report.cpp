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

#include <sstream>

#include "nel/misc/common.h"
#include "nel/misc/ucstring.h"

#include "nel/misc/report.h"
#include "nel/misc/path.h"

#ifdef NL_OS_WINDOWS
#	include <windowsx.h>
#	include <winuser.h>
#endif // NL_OS_WINDOWS

#define NL_NO_DEBUG_FILES 1

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

void setReportEmailFunction(void *emailFunction)
{
	// DEPRECATED
	// no-op
}

// Contents of crash report
static string ReportBody;
// Host url for crash report
static std::string ReportPostUrl = "";
// Title for the crash report window
static std::string ReportWindowTitle = "";

void setReportPostUrl(const std::string &postUrl)
{
	ReportPostUrl = postUrl;
}

// Launch the crash report application
static void doSendReport()
{
    std::string filename;

    filename = /*getLogDirectory() + */ "report_"; // FIXME: Should use log directory
    filename += NLMISC::toString( int( time( NULL ) ) );
    filename += ".txt";

    std::stringstream params;
    params << "-log ";
    params << filename; // FIXME: Escape the filepath with quotes
    if (!ReportPostUrl.empty())
    {
		params << " -host ";
		params << ReportPostUrl;
	}
    if (!ReportWindowTitle.empty())
    {
		params << " -title ";
		params << ReportWindowTitle; // FIXME: Escape the title with quotes and test
	}

    std::ofstream f;
    f.open( filename.c_str() );
    if( !f.good() )
        return;

    f << ReportBody;

    f.close();

#ifdef NL_OS_WINDOWS
    NLMISC::launchProgram( "crash_report.exe", params.str() );
#else
    NLMISC::launchProgram( "crash_report", params.str() );
#endif

    // Added because NLSMIC::launcProgram needs time to launch
    nlSleep( 2 * 1000 );
    
}

#if defined(FINAL_VERSION) || !defined(NL_OS_WINDOWS)

// For FINAL_VERSION, simply launch the crash report and exit the application
TReportResult report(const std::string &title, const std::string &header, const std::string &subject, const std::string &body, bool enableCheckIgnore, uint debugButton, bool ignoreButton, sint quitButton, bool sendReportButton, bool &ignoreNextTime, const string &attachedFile)
{
	ReportWindowTitle = title.empty() ? "Nel Crash Report" : title;
	ReportBody = addSlashR(body);

	doSendReport();

#	if defined(FINAL_VERSION) // TODO: This behaviour is used in the old report code when Quitting the application is the default crash report behaviour. Needs testing.
#		ifdef NL_OS_WINDOWS
#		ifndef NL_COMP_MINGW
	// disable the Windows popup telling that the application aborted and disable the dr watson report.
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#		endif
#		endif
	// quit without calling atexit or static object dtors.
	abort();
#	endif

	return ReportQuit;
}

#else

// Windows specific version for DEV builds, first shows a dialog box for debugging

static HWND sendReport=NULL;
#define DELETE_OBJECT(a) if((a)!=NULL) { DeleteObject (a); a = NULL; }

static HWND checkIgnore = NULL;
static HWND debug = NULL;
static HWND ignore = NULL;
static HWND quit = NULL;
static HWND dialog = NULL;

static bool NeedExit;
static TReportResult Result;
static bool IgnoreNextTime;
static bool CanSendMailReport = false;

static bool DebugDefaultBehavior, QuitDefaultBehavior;

static void maybeSendReport()
{
	if (CanSendMailReport && SendMessage(sendReport, BM_GETCHECK, 0, 0) != BST_CHECKED)
	{
		doSendReport();
#ifndef NL_NO_DEBUG_FILES
		CFile::createEmptyFile(getLogDirectory() + "report_sent");
#endif
	}
	else
	{
#ifndef NL_NO_DEBUG_FILES
		CFile::createEmptyFile(getLogDirectory() + "report_refused");
#endif
-	}
}

static LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//MSGFILTER *pmf;

	if (message == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND) lParam == checkIgnore)
		{
			IgnoreNextTime = !IgnoreNextTime;
		}
		else if ((HWND) lParam == debug)
		{
			maybeSendReport();
			NeedExit = true;
			Result = ReportDebug;
			if (DebugDefaultBehavior)
			{
				NLMISC_BREAKPOINT;
			}
		}
		else if ((HWND) lParam == ignore)
		{
			maybeSendReport();
			NeedExit = true;
			Result = ReportIgnore;
		}
		else if ((HWND) lParam == quit)
		{
			maybeSendReport();
			NeedExit = true;
			Result = ReportQuit;

			if (QuitDefaultBehavior)
			{
				// ace: we cannot call exit() because it's call the static object dtor and can crash the application
				// if the dtor call order is not good.
				//exit(EXIT_SUCCESS);
#ifdef NL_OS_WINDOWS
#ifndef NL_COMP_MINGW
				// disable the Windows popup telling that the application aborted and disable the dr watson report.
				_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
#endif
				// quit without calling atexit or static object dtors.
				abort();
			}
		}
	}
	else if (message == WM_CHAR)
	{
		if (wParam == 27)
		{
			// ESC -> ignore
			maybeSendReport();
			NeedExit = true;
			Result = ReportIgnore;
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

TReportResult report(const std::string &title, const std::string &header, const std::string &subject, const std::string &body, bool enableCheckIgnore, uint debugButton, bool ignoreButton, sint quitButton, bool sendReportButton, bool &ignoreNextTime, const string &attachedFile)
{
	// register the window
	static bool AlreadyRegister = false;
	if (!AlreadyRegister)
	{
		WNDCLASSW wc;
		memset (&wc,0,sizeof(wc));
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetModuleHandle(NULL);
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)COLOR_WINDOW;
		wc.lpszClassName	= L"NLReportWindow";
		wc.lpszMenuName		= NULL;
		if (!RegisterClassW(&wc)) return ReportError;
		AlreadyRegister = true;
	}

	ReportWindowTitle = title.empty() ? "Nel Crash Report" : title;
	ucstring formatedTitle = ucstring::makeFromUtf8(ReportWindowTitle);

	// create the window
	dialog = CreateWindowW (L"NLReportWindow", (LPCWSTR)formatedTitle.c_str(), WS_DLGFRAME | WS_CAPTION /*| WS_THICKFRAME*/, CW_USEDEFAULT, CW_USEDEFAULT, 456, 400,  NULL, NULL, GetModuleHandle(NULL), NULL);

	// create the font
	HFONT font = CreateFont (-12, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");

	// create the edit control
	HWND edit = CreateWindowW (L"EDIT", NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_READONLY | ES_LEFT | ES_MULTILINE, 7, 70, 429, 212, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLongPtr(dialog, GWLP_HINSTANCE), NULL);
	SendMessage (edit, WM_SETFONT, (WPARAM) font, TRUE);

	// set the edit text limit to lot of :)
	SendMessage (edit, EM_LIMITTEXT, ~0U, 0);

	ReportBody = addSlashR(body);

	// set the message in the edit text
	SendMessage (edit, WM_SETTEXT, (WPARAM)0, (LPARAM)ReportBody.c_str());

	if (enableCheckIgnore)
	{
		// create the combo box control
		checkIgnore = CreateWindowW (L"BUTTON", L"Don't display this report again", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_CHECKBOX, 7, 290, 429, 18, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLongPtr(dialog, GWLP_HINSTANCE), NULL);
		SendMessage (checkIgnore, WM_SETFONT, (WPARAM) font, TRUE);

		if(ignoreNextTime)
		{
			SendMessage (checkIgnore, BM_SETCHECK, BST_CHECKED, 0);
		}
	}

	// create the debug button control
	debug = CreateWindowW (L"BUTTON", L"Debug", WS_CHILD | WS_VISIBLE, 7, 315, 75, 25, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLongPtr(dialog, GWLP_HINSTANCE), NULL);
	SendMessage (debug, WM_SETFONT, (WPARAM) font, TRUE);

	if (debugButton == 0)
		EnableWindow(debug, FALSE);

	// create the ignore button control
	ignore = CreateWindowW (L"BUTTON", L"Ignore", WS_CHILD | WS_VISIBLE, 75+7+7, 315, 75, 25, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLongPtr(dialog, GWLP_HINSTANCE), NULL);
	SendMessage (ignore, WM_SETFONT, (WPARAM) font, TRUE);

	if (ignoreButton == 0)
		EnableWindow(ignore, FALSE);

	// create the quit button control
	quit = CreateWindowW (L"BUTTON", L"Quit", WS_CHILD | WS_VISIBLE, 75+75+7+7+7, 315, 75, 25, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLongPtr(dialog, GWLP_HINSTANCE), NULL);
	SendMessage (quit, WM_SETFONT, (WPARAM) font, TRUE);

	if (quitButton == 0)
		EnableWindow(quit, FALSE);

	// create the debug button control
	sendReport = CreateWindowW (L"BUTTON", L"Don't send the report", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 7, 315+32, 429, 18, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLongPtr(dialog, GWLP_HINSTANCE), NULL);
	SendMessage (sendReport, WM_SETFONT, (WPARAM) font, TRUE);

	string formatedHeader;
	if (header.empty())
	{
		formatedHeader = "This application stopped to display this report.";
	}
	else
	{
		formatedHeader = header;
	}

	// ace don't do that because it s slow to try to send a mail
	CanSendMailReport = sendReportButton && !ReportPostUrl.empty();

	if (CanSendMailReport)
		formatedHeader += " Send report will only email the contents of the box below. Please, send it to help us (it could take few minutes to send the email, be patient).";
	else
		EnableWindow(sendReport, FALSE);

	ucstring uc = ucstring::makeFromUtf8(formatedHeader);

	// create the label control
	HWND label = CreateWindowW (L"STATIC", (LPCWSTR)uc.c_str(), WS_CHILD | WS_VISIBLE /*| SS_WHITERECT*/, 7, 7, 429, 51, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLongPtr(dialog, GWLP_HINSTANCE), NULL);
	SendMessage (label, WM_SETFONT, (WPARAM) font, TRUE);


	DebugDefaultBehavior = debugButton==1;
	QuitDefaultBehavior = quitButton==1;

	IgnoreNextTime = ignoreNextTime;

	// show until the cursor really show :)
	while (ShowCursor(TRUE) < 0)
		;

	SetWindowPos (dialog, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

	SetFocus(dialog);
	SetForegroundWindow(dialog);

	NeedExit = false;

	while(!NeedExit)
	{
		MSG	msg;
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		nlSleep (1);
	}

	// set the user result
	ignoreNextTime = IgnoreNextTime;

	ShowWindow(dialog, SW_HIDE);



	DELETE_OBJECT(sendReport)
	DELETE_OBJECT(quit)
	DELETE_OBJECT(ignore)
	DELETE_OBJECT(debug)
	DELETE_OBJECT(checkIgnore)
	DELETE_OBJECT(edit)
	DELETE_OBJECT(label)
	DELETE_OBJECT(dialog)

	return Result;
}

#endif


} // NLMISC
