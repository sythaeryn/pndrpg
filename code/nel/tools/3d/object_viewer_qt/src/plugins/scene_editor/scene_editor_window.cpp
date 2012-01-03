// Object Viewer Qt - Scene Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2012  Dzmitry Kamiahin <dnk-88@tut.by>
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

// Project includes
#include "scene_editor_window.h"
#include "scene_editor_constants.h"

#include "../core/icore.h"
#include "../core/core_constants.h"

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QFileDialog>

namespace SceneEditor
{

QString LastDir;

SceneEditorWindow::SceneEditorWindow(QWidget *parent)
	: QMainWindow(parent)
{
	m_ui.setupUi(this);

	m_undoStack = new QUndoStack(this);

	createMenus();
	readSettings();
}

SceneEditorWindow::~SceneEditorWindow()
{
	writeSettings();
}

QUndoStack *SceneEditorWindow::undoStack() const
{
	return m_undoStack;
}

void SceneEditorWindow::open()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(this,
							tr("Open NeL Ligo land file"), LastDir,
							tr("All NeL Ligo land files (*.land)"));

	setCursor(Qt::WaitCursor);
	if (!fileNames.isEmpty())
	{
		QStringList list = fileNames;
		LastDir = QFileInfo(list.front()).absolutePath();
	}
	setCursor(Qt::ArrowCursor);
}

void SceneEditorWindow::createMenus()
{
	Core::MenuManager *menuManager = Core::ICore::instance()->menuManager();
}

void SceneEditorWindow::readSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::SCENE_EDITOR_SECTION);
	settings->endGroup();
}

void SceneEditorWindow::writeSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::SCENE_EDITOR_SECTION);
	settings->endGroup();
	settings->sync();
}

} /* namespace SceneEditor */
