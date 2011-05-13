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

#ifndef SHEET_ID_VIEW_PLUGIN_H
#define SHEET_ID_VIEW_PLUGIN_H

#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"
#include "nel/misc/app_context.h"

#include <QtCore/QObject>

#include "ais_editor_main_window.h"

namespace NLMISC
{
class CLibraryContext;
}

namespace NLQT
{
class IPluginSpec;
}

namespace AisEditorPluginNS
{

class AisEditorPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	virtual ~AisEditorPlugin();
	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();

	void addAutoReleasedObject(QObject *obj);

	void setNelContext(NLMISC::INelContext *nelContext);

	QString name() const;
	QString version() const;
	QString vendor() const;
	QString description() const;
	QStringList dependencies() const;

//private Q_SLOTS:
//	void execMessageBox();

protected:
	NLMISC::CLibraryContext *_LibContext;

private:
	ExtensionSystem::IPluginManager *_plugMan;
	QList<QObject *> _autoReleaseObjects;

};

class CAISEditorContext: public Core::IContext
{
	Q_OBJECT
public:
	CAISEditorContext(QObject *parent = 0): IContext(parent)
	{
		m_aisEditorMainWindow = new AisEditorMainWindow();
	}
	virtual ~CAISEditorContext() {}

	virtual QString id() const
	{
		return QLatin1String("AISEditorContext");
	}
	virtual QString trName() const
	{
		return tr("AI Script Editor");
	}
	virtual QIcon icon() const
	{
		return QIcon();
	}
	virtual QWidget *widget()
	{
		return m_aisEditorMainWindow;
	}
	AisEditorMainWindow *m_aisEditorMainWindow;
};

} // namespace AisEditorPluginNS

#endif // SHEET_ID_VIEW_PLUGIN_H
