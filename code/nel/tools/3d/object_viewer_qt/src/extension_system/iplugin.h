/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QtPlugin>
#include <QtCore/QString>

#include "iplugin_manager.h"

namespace NLQT
{

class IPlugin
{
public:
	virtual ~IPlugin();

	virtual bool initialize(IPluginManager *pluginManager, QString *errorString) = 0;
	virtual void extensionsInitialized() = 0;
	virtual void shutdown() { }

	virtual QString name() const = 0;
	virtual QString version() const = 0;
	virtual QString vendor() const = 0;
	virtual QString description() const = 0;
};

}; //namespace NLQT

Q_DECLARE_INTERFACE(NLQT::IPlugin, "com.ryzom.dev.ObjectViewerQt.IPlugin/0.7")

#endif // IPLUGIN_H
