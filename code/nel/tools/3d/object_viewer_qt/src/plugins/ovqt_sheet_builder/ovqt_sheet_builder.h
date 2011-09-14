// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef PLUGIN_H
#define PLUGIN_H

#include "../../extension_system/iplugin.h"

#include "nel/misc/app_context.h"

#include <QtCore/QObject>

namespace NLMISC
{
class CLibraryContext;
}

namespace ExtensionSystem
{
class IPluginSpec;
}

namespace Plugin
{

class SheetBuilderPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:
	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();

	void setNelContext(NLMISC::INelContext *nelContext);

	QString name() const;
	QString version() const;
	QString vendor() const;
	QString description() const;
	QStringList dependencies() const;

	void buildSheet(bool clean);

private Q_SLOTS:
	void execBuilderDialog();

protected:
	NLMISC::CLibraryContext *_LibContext;

private:
	ExtensionSystem::IPluginManager *_plugMan;

};

} // namespace Plugin

#endif // PLUGIN_H
