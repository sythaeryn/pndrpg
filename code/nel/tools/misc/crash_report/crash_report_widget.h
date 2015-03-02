// Nel MMORPG framework - Error Reporter
//
// Copyright (C) 2015 Laszlo Kis-Adam
// Copyright (C) 2010 Ryzom Core <http://ryzomcore.org/>
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


#ifndef RCERROR_WIDGET
#define RCERROR_WIDGET


#include "ui_crash_report_widget.h"
#include <vector>
#include <string>

class CCrashReportSocket;

class CCrashReportWidget : public QWidget
{
	Q_OBJECT
public:
	CCrashReportWidget( QWidget *parent = NULL );
	~CCrashReportWidget();

	void setFileName( const char *fn ){ m_fileName = fn; }

	void setup( const std::vector< std::pair< std::string, std::string > > &params );
	
private Q_SLOTS:
	void onLoad();
	void onSendClicked();
	void onCancelClicked();
	void onCBClicked();

	void onAlwaysIgnoreClicked();
	void onIgnoreClicked();
	void onAbortClicked();
	void onBreakClicked();
	
	void onReportSent();
	void onReportFailed();

private:
	bool checkSettings();
	void removeAndQuit();

	Ui::CrashReportWidget m_ui;
	QString m_fileName;
	CCrashReportSocket *m_socket;
	bool m_developerMode;
	bool m_forceSend;

};

#endif

