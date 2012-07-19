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
#include "link_editor.h"
#include "proc_editor.h"
#include "project_file_parser.h"
#include "project_window.h"
#include "nelgui_widget.h"

namespace GUIEditor
{
	QString _lastDir;
	std::map< std::string, SWidgetInfo > widgetInfo;

	GUIEditorWindow::GUIEditorWindow(QWidget *parent) :
	QMainWindow(parent)
	{
		m_ui.setupUi(this);
		m_undoStack   = new QUndoStack(this);
		widgetProps   = new CWidgetProperties;
		linkEditor    = new LinkEditor;
		procEditor    = new ProcEditor;
		projectWindow = new ProjectWindow;
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
		WidgetHierarchy *ha = new WidgetHierarchy;
		dock->setWidget( ha );
		addDockWidget( Qt::LeftDockWidgetArea, dock );

		dock = new QDockWidget( "Widget Properties", this );
		dock->setAllowedAreas(  Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
		QtTreePropertyBrowser *tb = new QtTreePropertyBrowser;
		browserCtrl.setBrowser( tb );
		browserCtrl.setup();
		dock->setWidget( tb );
		addDockWidget( Qt::RightDockWidgetArea, dock );

		viewPort->init();
	}
	
	GUIEditorWindow::~GUIEditorWindow()
	{
		writeSettings();

		delete widgetProps;
		widgetProps = NULL;

		delete linkEditor;
		linkEditor = NULL;

		delete procEditor;
		procEditor = NULL;

		delete projectWindow;
		projectWindow = NULL;

		delete viewPort;
		viewPort = NULL;
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
		SProjectFiles projectFiles;
		parser.getProjectFiles( projectFiles );
		currentProject = parser.getProjectName().c_str();
		projectWindow->setupFiles( projectFiles );

		setCursor( Qt::ArrowCursor );
	}


	void GUIEditorWindow::createMenus()
	{
		Core::MenuManager *mm = Core::ICore::instance()->menuManager();
		QMenu *menu = mm->menu( Core::Constants::M_TOOLS );
		if( menu != NULL )
		{
			QMenu *m = menu->addMenu( "GUI Editor" );

			QAction *a = new QAction( "Widget Properties", this );
			connect( a, SIGNAL( triggered( bool ) ), widgetProps, SLOT( show() ) );
			m->addAction( a );

			a = new QAction( "Link Editor", this );
			connect( a, SIGNAL( triggered( bool ) ), linkEditor, SLOT( show() ) );
			m->addAction( a );

			a = new QAction( "Proc Editor", this );
			connect( a, SIGNAL( triggered( bool ) ), procEditor, SLOT( show() ) );
			m->addAction( a );

			a = new QAction( "Project Window", this );
			connect( a, SIGNAL( triggered( bool ) ), projectWindow, SLOT( show() ) );
			m->addAction( a );

			a = new QAction( "Clear Viewport", this );
			connect( a, SIGNAL( triggered( bool ) ), viewPort, SLOT( clear() ) );
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
