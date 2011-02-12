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

#include "plugin_manager.h"

#include <QtCore/QDir>

#include <nel/misc/debug.h>

#include "plugin_spec.h"

namespace ExtensionSystem
{

CPluginManager::CPluginManager(QObject *parent)
	:IPluginManager(parent),
	 _settings(0)
{
}

CPluginManager::~CPluginManager()
{
	stopAll();
	deleteAll();
	qDeleteAll(_pluginSpecs);
}

void CPluginManager::addObject(QObject *obj)
{
	QWriteLocker lock(&_lock);
	if (obj == 0)
	{
		nlwarning("trying to add null object");
		return;
	}
	if (_allObjects.contains(obj))
	{
		nlwarning("trying to add duplicate object");
		return;
	}
	nlinfo(QString("addObject: " + obj->objectName()).toStdString().c_str());

	_allObjects.append(obj);

	Q_EMIT objectAdded(obj);
}

void CPluginManager::removeObject(QObject *obj)
{
	if (obj == 0)
	{
		nlwarning("trying to remove null object");
		return;
	}

	if (!_allObjects.contains(obj))
	{
		nlinfo(QString("object not in list: " + obj->objectName()).toStdString().c_str());
		return;
	}
	nlinfo(QString("removeObject: " + obj->objectName()).toStdString().c_str());

	Q_EMIT aboutToRemoveObject(obj);
	QWriteLocker lock(&_lock);
	_allObjects.removeAll(obj);
}

QList<QObject *> CPluginManager::allObjects() const
{
	return _allObjects;
}

void CPluginManager::loadPlugins()
{
	Q_FOREACH (CPluginSpec *spec, _pluginSpecs)
	setPluginState(spec, State::Loaded);

	Q_FOREACH (CPluginSpec *spec, _pluginSpecs)
	setPluginState(spec, State::Initialized);

	QListIterator<CPluginSpec *> it(_pluginSpecs);
	it.toBack();
	while (it.hasPrevious())
		setPluginState(it.previous(), State::Running);

	Q_EMIT pluginsChanged();
}

QStringList CPluginManager::getPluginPaths() const
{
	return _pluginPaths;
}

void CPluginManager::setPluginPaths(const QStringList &paths)
{
	_pluginPaths = paths;
	readPluginPaths();
}

QList<IPluginSpec *> CPluginManager::plugins() const
{
	return _ipluginSpecs;
}

void CPluginManager::setSettings(QSettings *settings)
{
	_settings = settings;
}

QSettings *CPluginManager::settings() const
{
	return _settings;
}

void CPluginManager::readSettings()
{
}

void CPluginManager::writeSettings()
{
}

void CPluginManager::readPluginPaths()
{
	qDeleteAll(_pluginSpecs);
	_pluginSpecs.clear();
	_ipluginSpecs.clear();

	QStringList pluginsList;
	QStringList searchPaths = _pluginPaths;
	while (!searchPaths.isEmpty())
	{
		const QDir dir(searchPaths.takeFirst());
#ifdef Q_OS_WIN
		const QFileInfoList files = dir.entryInfoList(QStringList() << QString("*.dll"), QDir::Files);
#else
		const QFileInfoList files = dir.entryInfoList(QStringList() << QString("*.so"), QDir::Files);
#endif
		Q_FOREACH (const QFileInfo &file, files)
		pluginsList << file.absoluteFilePath();
		const QFileInfoList dirs = dir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
		Q_FOREACH (const QFileInfo &subdir, dirs)
		searchPaths << subdir.absoluteFilePath();
	}

	Q_FOREACH (const QString &pluginFile, pluginsList)
	{
		CPluginSpec *spec = new CPluginSpec;
		spec->setFileName(pluginFile);
		spec->_pluginManager = this;
		_pluginSpecs.append(spec);
		_ipluginSpecs.append(spec);
	}

	Q_EMIT pluginsChanged();
}

void CPluginManager::setPluginState(CPluginSpec *spec, int destState)
{
	if (spec->hasError())
		return;
	if (destState == State::Running)
	{
		spec->initializeExtensions();
		return;
	}
	else if (destState == State::Deleted)
	{
		spec->kill();
		return;
	}

	if (destState == State::Loaded)
		spec->loadLibrary();
	else if (destState == State::Initialized)
		spec->initializePlugin();
	else if (destState == State::Stopped)
		spec->stop();
}

void CPluginManager::stopAll()
{
	Q_FOREACH (CPluginSpec *spec, _pluginSpecs)
	setPluginState(spec, State::Stopped);
}

void CPluginManager::deleteAll()
{
	QListIterator<CPluginSpec *> it(_pluginSpecs);
	it.toBack();
	while (it.hasPrevious())
	{
		setPluginState(it.previous(), State::Deleted);
	}
}

}; // namespace NLQT