// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#ifndef BUILD_GAMEDATA_SETTINGS_PAGE_H
#define BUILD_GAMEDATA_SETTINGS_PAGE_H

#include <QtCore/QObject>
#include <QStringListModel>

#include "../core/ioptions_page.h"

#include "ui_build_gamedata_settings_page.h"

class QWidget;

namespace BuildGamedata
{

class BuildGamedataPlugin;

/**
@class CBuildGamedataSettingsPage
*/
class CBuildGamedataSettingsPage : public Core::IOptionsPage
{
	Q_OBJECT
public:
	CBuildGamedataSettingsPage(BuildGamedataPlugin *plugin, QObject *parent = 0);
	virtual ~CBuildGamedataSettingsPage() {}

	virtual QString id() const;
	virtual QString trName() const;
	virtual QString category() const;
	virtual QString trCategory() const;
	QIcon categoryIcon() const;
	virtual QWidget *createPage(QWidget *parent);

	virtual void apply();
	virtual void finish() {}

private Q_SLOTS:
	void addToolDirectory();

private:
	QWidget *m_currentPage;
	Ui::BuildGamedataSettingsPage m_ui;
	BuildGamedataPlugin *m_buildGamedataPlugin;
	QStringListModel *m_toolsDirListViewModel;
};

} // namespace BuildGamedata

#endif // BUILD_GAMEDATA_SETTINGS_PAGE_H
