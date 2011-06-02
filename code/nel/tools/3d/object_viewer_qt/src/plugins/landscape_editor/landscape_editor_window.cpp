// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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
#include "landscape_editor_window.h"
#include "landscape_editor_constants.h"
#include "zone_list_model.h"

#include "../core/icore.h"
#include "../core/imenu_manager.h"
#include "../core/core_constants.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QFileDialog>

namespace LandscapeEditor
{
QString _lastDir;

LandscapeEditorWindow::LandscapeEditorWindow(QWidget *parent)
	: QMainWindow(parent)
{
	m_ui.setupUi(this);

	m_undoStack = new QUndoStack(this);
	/*
		m_zoneBuilder = new ZoneBuilder();
		m_zoneBuilder->init("e:/-nel-/install/continents/newbieland", false);
		m_ui.zoneListWidget->setModel(m_zoneBuilder->zoneModel());
	*/
	createMenus();
	readSettings();
}

LandscapeEditorWindow::~LandscapeEditorWindow()
{
	writeSettings();
}

QUndoStack *LandscapeEditorWindow::undoStack() const
{
	return m_undoStack;
}

void LandscapeEditorWindow::open()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(this,
							tr("Open NeL Ligo land file"), _lastDir,
							tr("All NeL Ligo land files (*.land)"));

	setCursor(Qt::WaitCursor);
	if (!fileNames.isEmpty())
	{
		QStringList list = fileNames;
		_lastDir = QFileInfo(list.front()).absolutePath();
	}
	setCursor(Qt::ArrowCursor);
}

void LandscapeEditorWindow::createMenus()
{
	Core::IMenuManager *menuManager = Core::ICore::instance()->menuManager();
}

void LandscapeEditorWindow::readSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::LANDSCAPE_EDITOR_SECTION);
	settings->endGroup();
}

void LandscapeEditorWindow::writeSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::LANDSCAPE_EDITOR_SECTION);
	settings->endGroup();
	settings->sync();
}

} /* namespace LandscapeEditor */
