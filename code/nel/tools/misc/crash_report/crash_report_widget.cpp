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


#include "crash_report_widget.h"
#include "crash_report_socket.h"
#include "crash_report_data.h"
#include <QTimer>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QFile>
#include <QPushButton>
#include <QHBoxLayout>

CCrashReportWidget::CCrashReportWidget( QWidget *parent ) :
QWidget( parent )
{
	m_developerMode = false;
	m_forceSend = false;

	m_ui.setupUi( this );

	m_socket = new CCrashReportSocket( this );

	QTimer::singleShot( 1, this, SLOT( onLoad() ) );

	connect( m_ui.emailCB, SIGNAL( stateChanged( int ) ), this, SLOT( onCBClicked() ) );

	connect( m_socket, SIGNAL( reportSent() ), this, SLOT( onReportSent() ) );
	connect( m_socket, SIGNAL( reportFailed() ), this, SLOT( onReportFailed() ) );
}

CCrashReportWidget::~CCrashReportWidget()
{
	m_socket = NULL;
}

void CCrashReportWidget::setup( const std::vector< std::pair< std::string, std::string > > &params )
{
	for( int i = 0; i < params.size(); i++ )
	{
		const std::pair< std::string, std::string > &p = params[ i ];
		const std::string &k = p.first;
		const std::string &v = p.second;

		if( k == "log" )
		{
			m_fileName = v.c_str();
		}
		else
		if( k == "host" )
		{
			m_socket->setURL( v.c_str() );
		}
		else
		if( k == "title" )
		{
			setWindowTitle( v.c_str() );
		}
		else
		if( k == "dev" )
		{
			m_developerMode = true;
		}
		else
		if( k == "sendreport" )
		{
			m_forceSend = true;
		}
	}

	QHBoxLayout *hbl = new QHBoxLayout( this );

	if( m_developerMode )
	{
		QPushButton *alwaysIgnoreButton = new QPushButton( tr( "Always Ignore" ), this );
		QPushButton *ignoreButton = new QPushButton( tr( "Ignore" ), this );
		QPushButton *abortButton = new QPushButton( tr( "Abort" ), this );
		QPushButton *breakButton = new QPushButton( tr( "Break" ), this );

		hbl->addWidget( alwaysIgnoreButton );
		hbl->addWidget( ignoreButton );
		hbl->addWidget( abortButton );
		hbl->addWidget( breakButton );

		m_ui.gridLayout->addLayout( hbl, 6, 0, 1, 3 );

		connect( alwaysIgnoreButton, SIGNAL( clicked( bool ) ), this, SLOT( onAlwaysIgnoreClicked() ) );
		connect( ignoreButton, SIGNAL( clicked( bool ) ), this, SLOT( onIgnoreClicked() ) );
		connect( abortButton, SIGNAL( clicked( bool ) ), this, SLOT( onAbortClicked() ) );
		connect( breakButton, SIGNAL( clicked( bool ) ), this, SLOT( onBreakClicked() ) );
	}
	else
	{
		QPushButton *sendButton = new QPushButton( tr( "Send report" ), this );
		connect( sendButton, SIGNAL( clicked( bool ) ), this, SLOT( onSendClicked() ) );
		hbl->addWidget( sendButton );

		if( !m_forceSend )
		{
			QPushButton *cancelButton = new QPushButton( tr( "Don't send report" ), this );
			connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( onCancelClicked() ) );
			hbl->addWidget( cancelButton );
		}

		m_ui.gridLayout->addLayout( hbl, 6, 0, 1, 3 );
	}
}

void CCrashReportWidget::onLoad()
{
	return;

	if( !checkSettings() )
	{
		close();
		return;
	}

	QFile f( m_fileName );
	bool b = f.open( QFile::ReadOnly | QFile::Text );
	if( !b )
	{
		QMessageBox::information( this,
									tr( "No log file found" ),
									tr( "There was no log file found, therefore nothing to report. Exiting..." ) );
		close();
		return;
	}

	QTextStream ss( &f );
	m_ui.reportEdit->setPlainText( ss.readAll() );
	f.close();
}

void CCrashReportWidget::onSendClicked()
{
	QApplication::setOverrideCursor( Qt::WaitCursor );

	SCrashReportData data;
	data.description = m_ui.descriptionEdit->toPlainText();
	data.report = m_ui.reportEdit->toPlainText();
	data.email = m_ui.emailEdit->text();

	m_socket->sendReport( data );
}

void CCrashReportWidget::onCancelClicked()
{
	removeAndQuit();
}

void CCrashReportWidget::onCBClicked()
{
	m_ui.emailEdit->setEnabled( m_ui.emailCB->isChecked() );
}

void CCrashReportWidget::onAlwaysIgnoreClicked()
{
	m_returnValue = ERET_ALWAYS_IGNORE;
	close();
}

void CCrashReportWidget::onIgnoreClicked()
{
	m_returnValue = ERET_IGNORE;
	close();
}

void CCrashReportWidget::onAbortClicked()
{
	m_returnValue = ERET_ABORT;
	close();
}

void CCrashReportWidget::onBreakClicked()
{
	m_returnValue = ERET_BREAK;
	close();
}


void CCrashReportWidget::onReportSent()
{
	QApplication::setOverrideCursor( Qt::ArrowCursor );

	QMessageBox::information( this,
								tr( "Report sent" ),
								tr( "The report has been sent." ) );

	removeAndQuit();
}

void CCrashReportWidget::onReportFailed()
{
	QApplication::setOverrideCursor( Qt::ArrowCursor );

	QMessageBox::information( this,
								tr( "Report failed" ),
								tr( "Failed to send the report..." ) );

	removeAndQuit();
}

bool CCrashReportWidget::checkSettings()
{
	if( m_fileName.isEmpty() )
	{
		QMessageBox::information( this,
									tr( "No log file specified." ),
									tr( "No log file specified. Exiting..." ) );
		return false;
	}

	if( m_socket->url().isEmpty() )
	{
		QMessageBox::information( this,
									tr( "No host specified." ),
									tr( "No host specified. Exiting..." ) );
		return false;
	}

	return true;
}

void CCrashReportWidget::removeAndQuit()
{
	QFile::remove( m_fileName );
	close();
}

