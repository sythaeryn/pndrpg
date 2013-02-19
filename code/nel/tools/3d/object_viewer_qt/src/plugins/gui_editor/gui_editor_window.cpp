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
#include "widget_info_tree.h"
#include "widget_properties_parser.h"
#include "widget_hierarchy.h"
#include "widget_serializer.h"
#include "link_list.h"
#include "proc_list.h"
#include "project_file_parser.h"
#include "project_file_serializer.h"
#include "project_window.h"
#include "nelgui_widget.h"

namespace GUIEditor
{
	QString _lastDir;
	std::map< std::string, SWidgetInfo > widgetInfo;
	SProjectFiles projectFiles;
	CProjectFileParser projectParser;

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

		widgetInfoTree = new CWidgetInfoTree;

		createMenus();
		readSettings();

		CWidgetPropParser parser;

		parser.setWidgetPropMap( &widgetInfo );
		parser.setWidgetInfoTree( widgetInfoTree );
		parser.parseGUIWidgets();
		widgetProps->setupWidgetInfo( widgetInfoTree );

		QDockWidget *dock = new QDockWidget( "Widget Hierarchy", this );
		dock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
		hierarchyView = new WidgetHierarchy;
		dock->setWidget( hierarchyView );
		addDockWidget( Qt::LeftDockWidgetArea, dock );

		dock = new QDockWidget( "Widget Properties", this );
		dock->setAllowedAreas(  Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

		QtTreePropertyBrowser *propBrowser = new QtTreePropertyBrowser;
		browserCtrl.setupWidgetInfo( widgetInfoTree );
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

		delete widgetInfoTree;
		widgetInfoTree = NULL;
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
		if( fileName.isEmpty() )
		{
			setCursor( Qt::ArrowCursor );
			return;
		}
		
		_lastDir = QFileInfo( fileName ).absolutePath();

		projectParser.clear();

		std::string projectFileName = fileName.toStdString();
		if( !projectParser.parseProjectFile( projectFileName ) )
		{
			QMessageBox::critical( this,
				tr( "Error parsing project file" ),
				tr( "There was an error while parsing the project file. Not a project file?" ) );
			setCursor( Qt::ArrowCursor );
			return;
		}
		projectFiles.clearAll();
		projectParser.getProjectFiles( projectFiles );
		currentProject = projectFiles.projectName.c_str();
		currentProjectFile = fileName;
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

	void GUIEditorWindow::newDocument()
	{
	}

	void GUIEditorWindow::save()
	{
		if( currentProject.isEmpty() )
			return;

		CProjectFileSerializer serializer;
		serializer.setFile( currentProjectFile.toStdString() );
		if( !serializer.serialize( projectFiles ) )
		{
			QMessageBox::critical( this,
							tr( "Failed to save project" ),
							tr( "There was an error while trying to save the project." ) );
			return;
		}

		// Can't save old projects any further, since the widgets are in multiple files in them
		// using templates, styles and whatnot. There's no way to restore the original XML structure
		// after it's loaded
		if( projectParser.getProjectVersion() == OLD )
			return;
	}

	void GUIEditorWindow::saveAs()
	{
		if( currentProject.isEmpty() )
			return;

		QString dir = 
			QFileDialog::getExistingDirectory( this, tr( "Save project as..." ) );

		if( dir.isEmpty() )
			return;

		projectFiles.guiFiles.clear();
		projectFiles.guiFiles.push_back( "ui_" + projectFiles.projectName + ".xml"  );
		projectFiles.version = NEW;

		QString newFile =
			dir + "/" + projectFiles.projectName.c_str() + ".xml";

		CProjectFileSerializer serializer;
		serializer.setFile( newFile.toStdString() );
		if( !serializer.serialize( projectFiles ) )
		{
			QMessageBox::critical( this,
							tr( "Failed to save project" ),
							tr( "There was an error while trying to save the project." ) );
			return;
		}

		std::string guiFile =
			dir.toStdString() + "/" + "ui_" + projectFiles.projectName + ".xml";

		WidgetSerializer widgetSerializer;
		widgetSerializer.setFile( guiFile );
		widgetSerializer.setActiveGroup( projectFiles.activeGroup );
		if( !widgetSerializer.serialize( projectFiles.masterGroup ) )
		{
			QMessageBox::critical( this,
							tr( "Failed to save project" ),
							tr( "There was an error while trying to save the project." ) );
			return;
		}

		QMessageBox::information( this,
						tr( "Save successful" ),
						tr( "Project saved successfully!" ) );

	}

	bool GUIEditorWindow::close()
	{
		if( currentProject.isEmpty() )
			return true;

		QMessageBox::StandardButton reply = QMessageBox::question( this,
											tr( "Closing project" ),
											tr( "Are you sure you want to close this project?" ),
											QMessageBox::Yes | QMessageBox::No );
		if( reply != QMessageBox::Yes )
			return false;

		projectFiles.clearAll();
		projectWindow->clear();
		hierarchyView->clearHierarchy();
		viewPort->reset();
		browserCtrl.clear();
		linkList->clear();
		procList->clear();
		currentProject = "";
		currentProjectFile = "";
		projectParser.clear();

		return true;
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
		QAction *saveAsAction = mm->action( Core::Constants::SAVE_AS );
		QAction *closeAction = mm->action( Core::Constants::CLOSE );

		//if( newAction != NULL )
		//	newAction->setEnabled( true );
		if( saveAction != NULL )
			saveAction->setEnabled( true );
		if( saveAsAction != NULL )
			saveAsAction->setEnabled( true );
		if( closeAction != NULL )
			closeAction->setEnabled( true );

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
