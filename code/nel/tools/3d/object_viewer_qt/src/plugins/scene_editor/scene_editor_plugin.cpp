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
#include "scene_editor_plugin.h"
#include "scene_editor_window.h"

#include "../core/icore.h"
#include "../core/core_constants.h"

// NeL includes
#include "nel/misc/debug.h"

// Qt includes
#include <QtCore/QObject>

namespace SceneEditor
{

SceneEditorPlugin::~SceneEditorPlugin()
{
	Q_FOREACH(QObject *obj, m_autoReleaseObjects)
	{
		m_plugMan->removeObject(obj);
	}
	qDeleteAll(m_autoReleaseObjects);
	m_autoReleaseObjects.clear();
}

bool SceneEditorPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	m_plugMan = pluginManager;

	addAutoReleasedObject(new SceneEditorContext(this));
	return true;
}

void SceneEditorPlugin::extensionsInitialized()
{
}

void SceneEditorPlugin::shutdown()
{
}

void SceneEditorPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	m_libContext = new NLMISC::CLibraryContext(*nelContext);
}

void SceneEditorPlugin::addAutoReleasedObject(QObject *obj)
{
	m_plugMan->addObject(obj);
	m_autoReleaseObjects.prepend(obj);
}

SceneEditorContext::SceneEditorContext(QObject *parent)
	: IContext(parent),
	  m_sceneEditorWindow(0)
{
	m_sceneEditorWindow = new SceneEditorWindow();
}

QWidget *SceneEditorContext::widget()
{
	return m_sceneEditorWindow;
}

QUndoStack *SceneEditorContext::undoStack()
{
	return m_sceneEditorWindow->undoStack();
}

void SceneEditorContext::open()
{
	m_sceneEditorWindow->open();
}

}

Q_EXPORT_PLUGIN(SceneEditor::SceneEditorPlugin)