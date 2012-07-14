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

#ifndef GUI_EDITOR_WINDOW_H
#define GUI_EDITOR_WINDOW_H

#include "ui_gui_editor_window.h"
#include <QtGui/QUndoStack>
#include <QXmlStreamReader>
#include <QFile>
#include "widget_info.h"
#include "property_browser_ctrl.h"

namespace GUIEditor
{

	class CWidgetProperties;
	class LinkEditor;
	class ProcEditor;

	class GUIEditorWindow: public QMainWindow
	{
		Q_OBJECT
public:
		GUIEditorWindow(QWidget *parent = 0);
		
		~GUIEditorWindow();
		
		QUndoStack *undoStack() const;
		
		Q_SIGNALS:

public Q_SLOTS:
		void open();

private Q_SLOTS:
		void parse();

private:
		void createMenus();

		void readSettings();

		void writeSettings();

		void parseGUIWidgets();
		void parseGUIWidget( const QString &file );
		void parseGUIWidgetXML( QFile &file );
		QString parseGUIWidgetHeader( QXmlStreamReader &reader );
		void parseGUIWidgetProperties( QXmlStreamReader &reader, const QString &widgetName );

		QUndoStack *m_undoStack;

		Ui::GUIEditorWindow m_ui;
		CWidgetProperties *widgetProps;
		LinkEditor *linkEditor;
		ProcEditor *procEditor;
		
		CPropBrowserCtrl browserCtrl;
	};

}

#endif
