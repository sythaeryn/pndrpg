// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

// Project includes
#include "build_gamedata_plugin.h"
#include "build_gamedata_view.h"
#include "build_gamedata_settings_page.h"
#include "../core/icore.h"
#include "../core/imenu_manager.h"
#include "../core/core_constants.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

// NeL includes
#include "nel/misc/debug.h"

using namespace BuildGamedata;

BuildGamedataPlugin::BuildGamedataPlugin(QWidget *parent): QObject(parent)
{
}

BuildGamedataPlugin::~BuildGamedataPlugin()
{
	Q_FOREACH(QObject *obj, m_autoReleaseObjects)
	{
		m_plugMan->removeObject(obj);
	}
	qDeleteAll(m_autoReleaseObjects);
	m_autoReleaseObjects.clear();
}

bool BuildGamedataPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	m_plugMan = pluginManager;
	m_buildGamedataSettingsPage = new CBuildGamedataSettingsPage(this, this);
	addAutoReleasedObject(m_buildGamedataSettingsPage);
	return true;
}

void BuildGamedataPlugin::extensionsInitialized()
{
	Core::IMenuManager *menuManager = Core::ICore::instance()->menuManager();

	QMenu *viewMenu = menuManager->menu(Core::Constants::M_VIEW);
	QMainWindow *wnd = Core::ICore::instance()->mainWindow();

	
	m_buildGamedataView = new BuildGamedataView();
	wnd->addDockWidget(Qt::RightDockWidgetArea, m_buildGamedataView);
	m_buildGamedataView->hide();

	viewMenu->addAction(m_buildGamedataView->toggleViewAction());
}

void BuildGamedataPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	_LibContext = new NLMISC::CLibraryContext(*nelContext);
}

void BuildGamedataPlugin::addAutoReleasedObject(QObject *obj)
{
	m_plugMan->addObject(obj);
	m_autoReleaseObjects.prepend(obj);
}

QString BuildGamedataPlugin::name() const
{
	return "Build Gamedata";
}

QString BuildGamedataPlugin::version() const
{
	return "1.0";
}

QString BuildGamedataPlugin::vendor() const
{
	return "Ryzom Core";
}

QString BuildGamedataPlugin::description() const
{
	return "Build Gamedata Plugin";
}

QStringList BuildGamedataPlugin::dependencies() const
{
	QStringList list;
	list.append(Core::Constants::OVQT_CORE_PLUGIN);
	return list;
}

Q_EXPORT_PLUGIN(BuildGamedataPlugin)
