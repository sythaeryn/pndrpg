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

#include "build_gamedata_settings_page.h"


// Qt includes
#include <QtGui/QWidget>
#include <QFileDialog>
#include <QSignalMapper>
#include <QSettings>

// NeL includes

// Project includes
#include "build_gamedata_constants.h"
#include "../core/icore.h"

namespace BuildGamedata
{

QString lastDir = ".";

CBuildGamedataSettingsPage::CBuildGamedataSettingsPage(BuildGamedataPlugin *plugin, QObject *parent)
	: IOptionsPage(parent),
	  m_currentPage(NULL),
	  m_buildGamedataPlugin(plugin)
{
	m_directoryButtonMapper = new QSignalMapper();
}

QString CBuildGamedataSettingsPage::id() const
{
	return QLatin1String("BuildGamedata");
}

QString CBuildGamedataSettingsPage::trName() const
{
	return tr("Build Gamedata");
}

QString CBuildGamedataSettingsPage::category() const
{
	return QLatin1String("Build Gamedata");
}

QString CBuildGamedataSettingsPage::trCategory() const
{
	return tr("Build Gamedata");
}

QIcon CBuildGamedataSettingsPage::categoryIcon() const
{
        return QIcon();
}

QWidget *CBuildGamedataSettingsPage::createPage(QWidget *parent)
{
	m_currentPage = new QWidget(parent);
	m_ui.setupUi(m_currentPage);

	// Read in the settings information.
	readSettings();

	/*
	 * Connections for the tool directory list widget and its tool buttons.
	 */
	connect(m_ui.toolDirectoryAddTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.toolDirectoryAddTB, TOOL_ADD);
	connect(m_ui.toolDirectoryRemoveTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.toolDirectoryRemoveTB, TOOL_DEL);
	connect(m_ui.toolDirectoryUpTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.toolDirectoryUpTB, TOOL_UP);
	connect(m_ui.toolDirectoryDownTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.toolDirectoryDownTB, TOOL_DWN);

	/*
	 * Connections for the exe/dll/cfg directory list widget and its tool buttons.
	 */
	connect(m_ui.exeDllCfgDirAddTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.exeDllCfgDirAddTB, EXE_ADD);
	connect(m_ui.exeDllCfgDirRemoveTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.exeDllCfgDirRemoveTB, EXE_DEL);
	connect(m_ui.exeDllCfgDirUpTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.exeDllCfgDirUpTB, EXE_UP);
	connect(m_ui.exeDllCfgDirDownTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.exeDllCfgDirDownTB, EXE_DWN);

	/*
	 * Connections on 'General' page for tool buttons.
	 */
	connect(m_ui.pythonExeTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.pythonExeTB, PYTHON_TB);
	connect(m_ui.scriptDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.scriptDirectoryTB, SCRIPT_TB);
	connect(m_ui.workspaceDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.workspaceDirectoryTB, WORKSPC_TB);
	connect(m_ui.databaseDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.databaseDirectoryTB, DATABASE_TB);

	/*
	 * Connections on 'Level Design' page for tool buttons.
	 */
	connect(m_ui.levelDesignDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.levelDesignDirectoryTB, LVLDSN_TB);
	connect(m_ui.levelDesignDfnDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.levelDesignDfnDirectoryTB, LVLDSN_DFN_TB);
	connect(m_ui.levelDesignWorldDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.levelDesignWorldDirectoryTB, LVLDSN_WRLD_TB);
	connect(m_ui.primitivesDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.primitivesDirectoryTB, PRIMS_TB);
	connect(m_ui.gamedevDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.gamedevDirectoryTB, GAMEDEV_TB);
	connect(m_ui.dataCommonDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.dataCommonDirectoryTB, DATACOMMON_TB);

	/*
	 * Connections on 'Output Paths' page for tool buttons.
	 */
	connect(m_ui.exportBuildDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.exportBuildDirectoryTB, OUTPUT_EXPORT_TB);
	connect(m_ui.installDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.installDirectoryTB, OUTPUT_INSTALL_TB);
	connect(m_ui.dataShardDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.dataShardDirectoryTB, OUTPUT_DATASHARD_TB);
	connect(m_ui.clientDevDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.clientDevDirectoryTB, OUTPUT_CLIENTDEV_TB);
	connect(m_ui.clientPatchDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.clientPatchDirectoryTB, OUTPUT_CLIENTPATCH_TB);
	connect(m_ui.clientInstallDirectoryTB, SIGNAL(clicked()), m_directoryButtonMapper, SLOT(map()));
	m_directoryButtonMapper->setMapping(m_ui.clientInstallDirectoryTB, OUTPUT_CLIENTINSTALL_TB);

	// Final connection for the mega signal mapper.
	connect(m_directoryButtonMapper, SIGNAL(mapped(int)), this, SLOT(buttonClicked(int)));

	return m_currentPage;
}

void CBuildGamedataSettingsPage::apply()
{
	writeSettings();
}

void CBuildGamedataSettingsPage::buttonClicked(int buttonId)
{
	switch(buttonId)
	{
	case TOOL_ADD:
		pathDialogForListWidget(m_ui.toolDirectoriesLW);
		break;
	case TOOL_DEL:
		removePath(m_ui.toolDirectoriesLW);
		break;
	case TOOL_UP:
		upPath(m_ui.toolDirectoriesLW);
		break;
	case TOOL_DWN:
		downPath(m_ui.toolDirectoriesLW);
		break;
	case EXE_ADD:
		pathDialogForListWidget(m_ui.exeDllCfgDirectoriesLW);
		break;
	case EXE_DEL:
		removePath(m_ui.exeDllCfgDirectoriesLW);
		break;
	case EXE_UP:
		upPath(m_ui.exeDllCfgDirectoriesLW);
		break;
	case EXE_DWN:
		downPath(m_ui.exeDllCfgDirectoriesLW);
		break;

	case PYTHON_TB:
		fileDialogForLineEdit(m_ui.pythonExeLE);
		break;
	case SCRIPT_TB:
		pathDialogForLineEdit(m_ui.scriptDirectoryLE);
		break;
	case WORKSPC_TB:
		pathDialogForLineEdit(m_ui.workspaceDirectoryLE);
		break;
	case DATABASE_TB:
		pathDialogForLineEdit(m_ui.databaseDirectoryLE);
		break;


	case LVLDSN_TB:
		pathDialogForLineEdit(m_ui.levelDesignDirectoryLE);
		break;
	case LVLDSN_DFN_TB:
		pathDialogForLineEdit(m_ui.levelDesignDfnDirectoryLE);
		break;
	case LVLDSN_WRLD_TB:
		pathDialogForLineEdit(m_ui.levelDesignWorldDirectoryLE);
		break;
	case PRIMS_TB:
		pathDialogForLineEdit(m_ui.primitivesDirectoryLE);
		break;
	case GAMEDEV_TB:
		pathDialogForLineEdit(m_ui.gamedevDirectoryLE);
		break;
	case DATACOMMON_TB:
		pathDialogForLineEdit(m_ui.dataCommonDirectoryLE);
		break;

	case OUTPUT_EXPORT_TB:
		pathDialogForLineEdit(m_ui.exportBuildDirectoryLE);
		break;
	case OUTPUT_INSTALL_TB:
		pathDialogForLineEdit(m_ui.installDirectoryLE);
		break;
	case OUTPUT_DATASHARD_TB:
		pathDialogForLineEdit(m_ui.dataShardDirectoryLE);
		break;
	case OUTPUT_CLIENTDEV_TB:
		pathDialogForLineEdit(m_ui.clientDevDirectoryLE);
		break;
	case OUTPUT_CLIENTPATCH_TB:
		pathDialogForLineEdit(m_ui.clientPatchDirectoryLE);
		break;
	case OUTPUT_CLIENTINSTALL_TB:
		pathDialogForLineEdit(m_ui.clientInstallDirectoryLE);
		break;
	};
}

void CBuildGamedataSettingsPage::pathDialogForLineEdit(QLineEdit *lineEditWidget)
{
	QString prevDir = (lineEditWidget->text().isEmpty()?lastDir:lineEditWidget->text());
	QString path = QFileDialog::getExistingDirectory(m_currentPage, "", prevDir);
	if(!path.isEmpty())
	{
		lineEditWidget->setText(path);
		lastDir = path;
	}
}

void CBuildGamedataSettingsPage::fileDialogForLineEdit(QLineEdit *lineEditWidget)
{
	QString prevDir = (lineEditWidget->text().isEmpty()?lastDir:lineEditWidget->text());
	QString path = QFileDialog::getOpenFileName(m_currentPage, "", prevDir);
	if(!path.isEmpty())
	{
		lineEditWidget->setText(path);
		lastDir = path;
	}
}

void CBuildGamedataSettingsPage::pathDialogForListWidget(QListWidget *listWidget)
{
	QString path = QFileDialog::getExistingDirectory(m_currentPage, "", lastDir);
	if(!path.isEmpty())
	{
		QListWidgetItem *newItem = new QListWidgetItem();
		newItem->setText(path);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		listWidget->addItem(newItem);
		lastDir = path;
	}
}

void CBuildGamedataSettingsPage::upPath(QListWidget *listWidget)
{
	int currentRow = listWidget->currentRow();
	if (!(currentRow == 0))
	{
		QListWidgetItem *item = listWidget->takeItem(currentRow);
		listWidget->insertItem(--currentRow, item);
		listWidget->setCurrentRow(currentRow);
	}
}

void CBuildGamedataSettingsPage::downPath(QListWidget *listWidget)
{
	int currentRow = listWidget->currentRow();
	if (!(currentRow == listWidget->count()-1))
	{
		QListWidgetItem *item = listWidget->takeItem(currentRow);
		listWidget->insertItem(++currentRow, item);
		listWidget->setCurrentRow(currentRow);
	}
}

void CBuildGamedataSettingsPage::removePath(QListWidget *listWidget)
{
	QListWidgetItem *removeItem = listWidget->takeItem(listWidget->currentRow());
	if (!removeItem)
		delete removeItem;
}

void CBuildGamedataSettingsPage::readSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(BuildGamedata::Constants::BUILD_GAMEDATA_SECTION);

	m_ui.pythonExeLE->setText(settings->value(Constants::SETTING_PYTHON_EXE_PATH, Constants::SETTING_PYTHON_EXE_PATH_DEFAULT).toString());
	m_ui.scriptDirectoryLE->setText(settings->value(Constants::SETTING_SCRIPT_PATH, Constants::SETTING_SCRIPT_PATH_DEFAULT).toString());
	m_ui.workspaceDirectoryLE->setText(settings->value(Constants::SETTING_WORKSPACE_PATH, Constants::SETTING_WORKSPACE_PATH_DEFAULT).toString());
	m_ui.databaseDirectoryLE->setText(settings->value(Constants::SETTING_DATABASE_PATH, Constants::SETTING_DATABASE_PATH_DEFAULT).toString());
	m_ui.levelDesignDirectoryLE->setText(settings->value(Constants::SETTING_LEVELDESIGN_PATH, Constants::SETTING_LEVELDESIGN_PATH_DEFAULT).toString());
	m_ui.levelDesignDfnDirectoryLE->setText(settings->value(Constants::SETTING_LEVELDESIGN_DFN_PATH, Constants::SETTING_LEVELDESIGN_DFN_PATH_DEFAULT).toString());
	m_ui.levelDesignWorldDirectoryLE->setText(settings->value(Constants::SETTING_LEVELDESIGN_WORLD_PATH, Constants::SETTING_LEVELDESIGN_WORLD_PATH_DEFAULT).toString());
	m_ui.primitivesDirectoryLE->setText(settings->value(Constants::SETTING_PRIMITIVES_PATH, Constants::SETTING_PRIMITIVES_PATH_DEFAULT).toString());
	m_ui.gamedevDirectoryLE->setText(settings->value(Constants::SETTING_GAMEDEV_PATH, Constants::SETTING_GAMEDEV_PATH_DEFAULT).toString());
	m_ui.dataCommonDirectoryLE->setText(settings->value(Constants::SETTING_DATA_COMMON_PATH, Constants::SETTING_DATA_COMMON_PATH_DEFAULT).toString());
	m_ui.exportBuildDirectoryLE->setText(settings->value(Constants::SETTING_EXPORT_PATH, Constants::SETTING_EXPORT_PATH_DEFAULT).toString());
	m_ui.installDirectoryLE->setText(settings->value(Constants::SETTING_INSTALL_PATH, Constants::SETTING_INSTALL_PATH_DEFAULT).toString());
	m_ui.dataShardDirectoryLE->setText(settings->value(Constants::SETTING_DATA_SHARD_PATH, Constants::SETTING_DATA_SHARD_PATH_DEFAULT).toString());
	m_ui.clientDevDirectoryLE->setText(settings->value(Constants::SETTING_CLIENT_DEV_PATH, Constants::SETTING_CLIENT_DEV_PATH_DEFAULT).toString());
	m_ui.clientPatchDirectoryLE->setText(settings->value(Constants::SETTING_CLIENT_PATCH_PATH, Constants::SETTING_CLIENT_PATCH_PATH_DEFAULT).toString());
	m_ui.clientInstallDirectoryLE->setText(settings->value(Constants::SETTING_CLIENT_INSTALL_PATH, Constants::SETTING_CLIENT_INSTALL_PATH_DEFAULT).toString());
	
	m_ui.toolSuffixLE->setText(settings->value(Constants::SETTING_TOOL_SUFFIX, Constants::SETTING_TOOL_SUFFIX_DEFAULT).toString());

	settings->endGroup();

	/*
	 * Read in the tool directories.
	 */
	QStringList toolDefaults;
	toolDefaults << Constants::SETTING_TOOL_DIRECTORIES_DEFAULT_1;
	toolDefaults << Constants::SETTING_TOOL_DIRECTORIES_DEFAULT_2;
	readDirectorySettings(Constants::SETTING_TOOL_DIRECTORIES, m_ui.toolDirectoriesLW, toolDefaults);

	QStringList exeDefaults;
	exeDefaults << Constants::SETTING_EXE_DIRECTORIES_DEFAULT_1;
	exeDefaults << Constants::SETTING_EXE_DIRECTORIES_DEFAULT_2;
	exeDefaults << Constants::SETTING_EXE_DIRECTORIES_DEFAULT_3;
	exeDefaults << Constants::SETTING_EXE_DIRECTORIES_DEFAULT_4;
	exeDefaults << Constants::SETTING_EXE_DIRECTORIES_DEFAULT_5;
	exeDefaults << Constants::SETTING_EXE_DIRECTORIES_DEFAULT_6;
	exeDefaults << Constants::SETTING_EXE_DIRECTORIES_DEFAULT_7;
	readDirectorySettings(Constants::SETTING_EXE_DIRECTORIES, m_ui.exeDllCfgDirectoriesLW, exeDefaults);
}

void CBuildGamedataSettingsPage::readDirectorySettings(const char *settingKey, QListWidget *listWidget, QStringList defaults)
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(BuildGamedata::Constants::BUILD_GAMEDATA_SECTION);

	QStringList paths = settings->value(settingKey).toStringList();
	if(paths.size() < 1)
		paths = defaults;

	Q_FOREACH(QString path, paths)
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(path);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		listWidget->addItem(newItem);
	}
	settings->endGroup();
}

void CBuildGamedataSettingsPage::writeSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(BuildGamedata::Constants::BUILD_GAMEDATA_SECTION);

	settings->setValue(Constants::SETTING_PYTHON_EXE_PATH, m_ui.pythonExeLE->text());
	settings->setValue(Constants::SETTING_SCRIPT_PATH, m_ui.scriptDirectoryLE->text());
	settings->setValue(Constants::SETTING_WORKSPACE_PATH, m_ui.workspaceDirectoryLE->text());
	settings->setValue(Constants::SETTING_DATABASE_PATH, m_ui.databaseDirectoryLE->text());
	settings->setValue(Constants::SETTING_LEVELDESIGN_PATH, m_ui.levelDesignDirectoryLE->text());
	settings->setValue(Constants::SETTING_LEVELDESIGN_DFN_PATH, m_ui.levelDesignDfnDirectoryLE->text());
	settings->setValue(Constants::SETTING_LEVELDESIGN_WORLD_PATH, m_ui.levelDesignWorldDirectoryLE->text());
	settings->setValue(Constants::SETTING_PRIMITIVES_PATH, m_ui.primitivesDirectoryLE->text());
	settings->setValue(Constants::SETTING_GAMEDEV_PATH, m_ui.gamedevDirectoryLE->text());
	settings->setValue(Constants::SETTING_DATA_COMMON_PATH, m_ui.dataCommonDirectoryLE->text());
	settings->setValue(Constants::SETTING_EXPORT_PATH, m_ui.exportBuildDirectoryLE->text());
	settings->setValue(Constants::SETTING_INSTALL_PATH, m_ui.installDirectoryLE->text());
	settings->setValue(Constants::SETTING_DATA_SHARD_PATH, m_ui.dataShardDirectoryLE->text());
	settings->setValue(Constants::SETTING_CLIENT_DEV_PATH, m_ui.clientDevDirectoryLE->text());
	settings->setValue(Constants::SETTING_CLIENT_PATCH_PATH, m_ui.clientPatchDirectoryLE->text());
	settings->setValue(Constants::SETTING_CLIENT_INSTALL_PATH, m_ui.clientInstallDirectoryLE->text());
	
	settings->setValue(Constants::SETTING_TOOL_SUFFIX, m_ui.toolSuffixLE->text());

	settings->endGroup();
	settings->sync();

	writeDirectorySettings(Constants::SETTING_TOOL_DIRECTORIES, m_ui.toolDirectoriesLW);
	writeDirectorySettings(Constants::SETTING_EXE_DIRECTORIES, m_ui.exeDllCfgDirectoriesLW);
}

void CBuildGamedataSettingsPage::writeDirectorySettings(const char *settingKey, QListWidget *listWidget)
{
	QStringList paths;
	for (int i = 0; i < listWidget->count(); ++i)
		paths << listWidget->item(i)->text();

	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(BuildGamedata::Constants::BUILD_GAMEDATA_SECTION);
	settings->setValue(settingKey, paths);
	settings->endGroup();
	settings->sync();
}

} /* namespace BuildGamedata */
