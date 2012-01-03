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

#ifndef SCENE_EDITOR_PLUGIN_H
#define SCENE_EDITOR_PLUGIN_H

// Project includes
#include "scene_editor_constants.h"
#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"

// NeL includes
#include "nel/misc/app_context.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QIcon>

namespace NLMISC
{
class CLibraryContext;
}

namespace ExtensionSystem
{
class IPluginSpec;
}

namespace SceneEditor
{
class SceneEditorWindow;

class SceneEditorPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	virtual ~SceneEditorPlugin();

	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();
	void shutdown();
	void setNelContext(NLMISC::INelContext *nelContext);

	void addAutoReleasedObject(QObject *obj);

protected:
	NLMISC::CLibraryContext *m_libContext;

private:
	ExtensionSystem::IPluginManager *m_plugMan;
	QList<QObject *> m_autoReleaseObjects;
};

class SceneEditorContext: public Core::IContext
{
	Q_OBJECT
public:
	SceneEditorContext(QObject *parent = 0);
	virtual ~SceneEditorContext() {}

	virtual QString id() const
	{
		return QLatin1String("SceneEditorContext");
	}
	virtual QString trName() const
	{
		return tr("SceneEditor");
	}
	virtual QIcon icon() const
	{
		return QIcon();
	}
	virtual QWidget *widget();
	
	virtual void open();
	virtual QUndoStack *undoStack();

	SceneEditorWindow *m_sceneEditorWindow;
};

} // namespace SceneEditor

#endif // SCENE_EDITOR_PLUGIN_H
