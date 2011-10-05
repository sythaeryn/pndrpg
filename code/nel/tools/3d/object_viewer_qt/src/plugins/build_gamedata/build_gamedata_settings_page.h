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
class QSignalMapper;
class QListWidget;
class QLineEdit;

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

	enum DirButtonType
	{
		// Tool List Widget Tool Buttons
		TOOL_ADD,
		TOOL_DEL,
		TOOL_UP,
		TOOL_DWN,
		
		// Exe/Dll/Cfg List Widget Tool Buttons
		EXE_ADD,
		EXE_DEL,
		EXE_UP,
		EXE_DWN,

		// General Page Tool Buttons
		PYTHON_TB,
		SCRIPT_TB,
		WORKSPC_TB,
		DATABASE_TB,

		LVLDSN_TB,
		LVLDSN_DFN_TB,
		LVLDSN_WRLD_TB,
		PRIMS_TB,
		GAMEDEV_TB,
		DATACOMMON_TB,

		OUTPUT_EXPORT_TB,
		OUTPUT_INSTALL_TB,
		OUTPUT_DATASHARD_TB,
		OUTPUT_CLIENTDEV_TB,
		OUTPUT_CLIENTPATCH_TB,
		OUTPUT_CLIENTINSTALL_TB
	};

private Q_SLOTS:
	void buttonClicked(int buttonId);

private:
	void readSettings();
	void writeSettings();

	void pathDialogForListWidget(QListWidget *listWidget);
	void pathDialogForLineEdit(QLineEdit *lineEditWidget);
	void fileDialogForLineEdit(QLineEdit *lineEditWidget);

	QWidget *m_currentPage;
	Ui::BuildGamedataSettingsPage m_ui;
	BuildGamedataPlugin *m_buildGamedataPlugin;
	QSignalMapper *m_directoryButtonMapper;
};

} // namespace BuildGamedata

#endif // BUILD_GAMEDATA_SETTINGS_PAGE_H
