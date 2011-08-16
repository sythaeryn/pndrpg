// Object Viewer Qt - Georges Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Adrian Jaekel <aj at elane2k dot com>
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
#include "georges_editor_plugin.h"
#include "georges_editor_form.h"

#include "../core/icore.h"
#include "../core/core_constants.h"

// NeL includes
#include "nel/misc/debug.h"

// Qt includes
#include <QtCore/QObject>

namespace Plugin
{

GeorgesEditorPlugin::~GeorgesEditorPlugin()
{
	Q_FOREACH(QObject *obj, m_autoReleaseObjects)
	{
		m_plugMan->removeObject(obj);
	}
	qDeleteAll(m_autoReleaseObjects);
	m_autoReleaseObjects.clear();
}

bool GeorgesEditorPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	m_plugMan = pluginManager;

	addAutoReleasedObject(new GeorgesEditorContext(this));
	return true;
}

void GeorgesEditorPlugin::extensionsInitialized()
{
}

void GeorgesEditorPlugin::shutdown()
{
}

void GeorgesEditorPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	m_libContext = new NLMISC::CLibraryContext(*nelContext);
}

QString GeorgesEditorPlugin::name() const
{
	return tr("Georges Editor");
}

QString GeorgesEditorPlugin::version() const
{
	return "0.3";
}

QString GeorgesEditorPlugin::vendor() const
{
	return "aquiles";
}

QString GeorgesEditorPlugin::description() const
{
	return tr("Tool to create & edit sheets or forms.");
}

QStringList GeorgesEditorPlugin::dependencies() const
{
	QStringList list;
	// TODO
	//list.append(Core::Constants::OVQT_CORE_PLUGIN);
	//list.append("ObjectViewer");
	return list;
}

void GeorgesEditorPlugin::addAutoReleasedObject(QObject *obj)
{
	m_plugMan->addObject(obj);
	m_autoReleaseObjects.prepend(obj);
}

GeorgesEditorContext::GeorgesEditorContext(QObject *parent)
	: IContext(parent),
	  m_georgesEditorForm(0)
{
	m_georgesEditorForm = new GeorgesEditorForm();
}

QUndoStack *GeorgesEditorContext::undoStack()
{
	return m_georgesEditorForm->undoStack();
}

void GeorgesEditorContext::open()
{
	m_georgesEditorForm->open();
}

QWidget *GeorgesEditorContext::widget()
{
	return m_georgesEditorForm;
}

}

Q_EXPORT_PLUGIN(Plugin::GeorgesEditorPlugin)