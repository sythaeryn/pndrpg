// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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

#include "gui_editor_window.h"
#include "gui_editor_constants.h"

#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/core.h"
#include "../core/menu_manager.h"

#include <nel/misc/debug.h>

#include <QtCore/QSettings>
#include <QtGui/QFileDialog>
#include <QDockWidget>
#include <QMessageBox>
#include "../../3rdparty/qtpropertybrowser/QtTreePropertyBrowser"

#include "widget_properties.h"
#include "widget_properties_parser.h"
#include "widget_hierarchy.h"
#include "link_list.h"
#include "proc_list.h"
#include "project_file_parser.h"
#include "project_window.h"
#include "nelgui_widget.h"

namespace GUIEditor
{
	QString _lastDir;
	std::map< std::string, SWidgetInfo > widgetInfo;
	SProjectFiles projectFiles;

	GUIEditorWindow::GUIEditorWindow(QWidget *parent) :
	QMainWindow(parent)
	{
		m_ui.setupUi(this);
		m_undoStack   = new QUndoStack(this);
		widgetProps   = new CWidgetProperties;
		linkList      = new LinkList;
		procList      = new ProcList;
		projectWindow = new ProjectWindow;
		connect( projectWindow, SIGNAL( projectFilesChanged() ), this, SLOT( onProjectFilesChanged() ) );
		viewPort      = new NelGUIWidget;
		setCentralWidget( viewPort );

		createMenus();
		readSettings();

		CWidgetPropParser parser;

		parser.setWidgetPropMap( &widgetInfo );
		parser.parseGUIWidgets();
		widgetProps->setupWidgetInfo( &widgetInfo );

		QDockWidget *dock = new QDockWidget( "Widget Hierarchy", this );
		dock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
		hierarchyView = new WidgetHierarchy;
		dock->setWidget( hierarchyView );
		addDockWidget( Qt::LeftDockWidgetArea, dock );

		dock = new QDockWidget( "Widget Properties", this );
		dock->setAllowedAreas(  Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

		QtTreePropertyBrowser *propBrowser = new QtTreePropertyBrowser;
		browserCtrl.setupWidgetInfo( widgetInfo );
		browserCtrl.setBrowser( propBrowser );
		dock->setWidget( propBrowser );
		addDockWidget( Qt::RightDockWidgetArea, dock );

		viewPort->init();

		connect( viewPort, SIGNAL( guiLoadComplete() ), hierarchyView, SLOT( onGUILoaded() ) );
		connect( viewPort, SIGNAL( guiLoadComplete() ), procList, SLOT( onGUILoaded() ) );
		connect( viewPort, SIGNAL( guiLoadComplete() ), linkList, SLOT( onGUILoaded() ) );
		connect( hierarchyView, SIGNAL( selectionChanged( std::string& ) ),
			&browserCtrl, SLOT( onSelectionChanged( std::string& ) ) );
	}
	
	GUIEditorWindow::~GUIEditorWindow()
	{
		writeSettings();

		delete widgetProps;
		widgetProps = NULL;

		delete linkList;
		linkList = NULL;

		delete procList;
		procList = NULL;

		delete projectWindow;
		projectWindow = NULL;

		delete viewPort;
		viewPort = NULL;

		// no deletion needed for these, since dockwidget owns them
		hierarchyView = NULL;
		propBrowser   = NULL;
	}
	
	QUndoStack *GUIEditorWindow::undoStack() const
	{
		return m_undoStack;
	}
	
	void GUIEditorWindow::open()
	{
		QString fileName = QFileDialog::getOpenFileName( this,
											tr( "Open GUI XML files" ),
											_lastDir,
											tr( "All XML files (*.xml)" ) );
		
		setCursor( Qt::WaitCursor );
		if( !fileName.isEmpty() )
		{
			_lastDir = QFileInfo( fileName ).absolutePath();
		}
		else
		{
			QMessageBox::critical( this,
				tr( "Error opening project file" ),
				tr( "Cannot open the specified project file!" ) );

			setCursor( Qt::ArrowCursor );
			return;
		}

		CProjectFileParser parser;
		if( !parser.parseProjectFile( fileName.toStdString() ) )
		{
			QMessageBox::critical( this,
				tr( "Error parsing project file" ),
				tr( "There was an error while parsing the project file. Not a project file?" ) );
			setCursor( Qt::ArrowCursor );
			return;
		}
		projectFiles.clearAll();
		parser.getProjectFiles( projectFiles );
		currentProject = projectFiles.projectName.c_str();
		projectWindow->setupFiles( projectFiles );
		if( viewPort->parse( projectFiles ) )
		{
			hierarchyView->buildHierarchy( projectFiles.masterGroup );
		}
		else
		{
			QMessageBox::critical( this,
				tr( "Error parsing GUI XML files" ),
				tr( "There was an error while parsing the GUI XML files. See the log file for details." ) );
		}

		setCursor( Qt::ArrowCursor );
	}

	void GUIEditorWindow::onProjectFilesChanged()
	{
		setCursor( Qt::WaitCursor );

		projectWindow->updateFiles( projectFiles );
		if( !viewPort->parse( projectFiles ) )
		{
			QMessageBox::critical( this,
				tr( "Error parsing GUI XML files" ),
				tr( "There was an error while parsing the GUI XML files. See the log file for details." ) );
		}

		setCursor( Qt::ArrowCursor );
	}


	void GUIEditorWindow::createMenus()
	{
		Core::MenuManager *mm = Core::ICore::instance()->menuManager();
		//QAction *newAction = mm->action( Core::Constants::NEW );
		QAction *saveAction = mm->action( Core::Constants::SAVE );

		//if( newAction != NULL )
		//	newAction->setEnabled( true );
		if( saveAction != NULL )
			saveAction->setEnabled( true );

		QMenu *menu = mm->menu( Core::Constants::M_TOOLS );
		if( menu != NULL )
		{
			QMenu *m = menu->addMenu( "GUI Editor" );

			QAction *a = new QAction( "Widget Properties", this );
			connect( a, SIGNAL( triggered( bool ) ), widgetProps, SLOT( show() ) );
			m->addAction( a );

			a = new QAction( "Link Editor", this );
			connect( a, SIGNAL( triggered( bool ) ), linkList, SLOT( show() ) );
			m->addAction( a );

			a = new QAction( "Procedure Editor", this );
			connect( a, SIGNAL( triggered( bool ) ), procList, SLOT( show() ) );
			m->addAction( a );

			a = new QAction( "Project Window", this );
			connect( a, SIGNAL( triggered( bool ) ), projectWindow, SLOT( show() ) );
			m->addAction( a );
		}
	}
	
	void GUIEditorWindow::readSettings()
	{
		QSettings *settings = Core::ICore::instance()->settings();
		settings->beginGroup(Constants::GUI_EDITOR_SECTION);
		settings->endGroup();
	}
	
	void GUIEditorWindow::writeSettings()
	{
		QSettings *settings = Core::ICore::instance()->settings();
		settings->beginGroup(Constants::GUI_EDITOR_SECTION);
		settings->endGroup();
		settings->sync();
	}
}
