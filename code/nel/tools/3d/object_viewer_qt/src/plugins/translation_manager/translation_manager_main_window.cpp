// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Emanuel Costea <cemycc@gmail.com>
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

// Plugin includes
#include "translation_manager_main_window.h"
#include "translation_manager_constants.h"
#include "ftp_selection.h"

// Core includes
#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/menu_manager.h"
#include "../../extension_system/iplugin_spec.h"

// Qt includes
#include <QtCore/QFileInfo>
#include <QtCore/QEvent>
#include <QtCore/QSettings>
#include <QtCore/QSignalMapper>
#include <QtCore/QResource>
#include <QtGui/QMessageBox>
#include <QtGui/QErrorMessage>
#include <QtGui/QTableWidget>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QFileDialog>
#include <QtGui/QMenuBar>
#include <QtGui/QCloseEvent>

namespace TranslationManager
{

/**
 * Main Window Constructor
 * Set the signals for subwindows and settings and the toolbar
 */
CMainWindow::CMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	_ui.setupUi(this);

	_ui.mdiArea->closeAllSubWindows();
	windowMapper = new QSignalMapper(this);
	connect(windowMapper, SIGNAL(mapped(QWidget *)), this, SLOT(setActiveSubWindow(QWidget *)));

	connect(_ui.mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(windowActivated(QMdiSubWindow *)));
	editorWindow = new CEditorBase();

	initialize_settings["georges"] = false;
	initialize_settings["ligo"] = false;


	undoStack = new QUndoStack(this);

	connect(Core::ICore::instance(), SIGNAL(changeSettings()), this, SLOT(readSettings()));
	readSettings();
	createToolbar();

	

}

/**
 * Create toolbar
 * Set the actions and the window menu
 */
void CMainWindow::createToolbar()
{
	// File menu
	openAct = new QAction(QIcon(Core::Constants::ICON_OPEN), "&Open file(s)...", this);
	_ui.toolBar->addAction(openAct);
	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
	saveAct = new QAction(QIcon(Core::Constants::ICON_SAVE), "&Save...", this);
	_ui.toolBar->addAction(saveAct);
	connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
	saveAsAct = new QAction(QIcon(Core::Constants::ICON_SAVE_AS), "&Save as...", this);
	_ui.toolBar->addAction(saveAsAct);
	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	// Words Extraction Menu
	QMenu *wordsExtractionMenu = new QMenu("&Words extraction...");
	QSignalMapper *wordsExtractionMapper = new QSignalMapper(this);
	wordsExtractionMenu->setIcon(QIcon(Core::Constants::ICON_SETTINGS));
	_ui.toolBar->addAction(wordsExtractionMenu->menuAction());	
	connect(wordsExtractionMapper, SIGNAL(mapped(QString)), this, SLOT(extractMenu(QString)));

	// extract bot names
	QAction *extractBotNamesAct = wordsExtractionMenu->addAction("&Extract bot names...");
	extractBotNamesAct->setStatusTip(tr("Extract bot names from primitives."));
	connect(extractBotNamesAct, SIGNAL(triggered()), wordsExtractionMapper, SLOT(map()));
	wordsExtractionMapper->setMapping(extractBotNamesAct, QString(Constants::WK_BOTNAMES));

	// extract item words
	QAction *extractItemWordsAct = wordsExtractionMenu->addAction("&Extract item words...");
	extractItemWordsAct->setStatusTip(tr("Extract item words"));
	connect(extractItemWordsAct, SIGNAL(triggered()), wordsExtractionMapper, SLOT(map()));
	wordsExtractionMapper->setMapping(extractItemWordsAct, QString(Constants::WK_ITEM));

	// extract creature words
	QAction *extractCreatureWordsAct = wordsExtractionMenu->addAction(tr("&Extract creature words..."));
	extractCreatureWordsAct->setStatusTip(tr("Extract creature words"));
	connect(extractCreatureWordsAct, SIGNAL(triggered()), wordsExtractionMapper, SLOT(map()));
	wordsExtractionMapper->setMapping(extractCreatureWordsAct, QString(Constants::WK_CREATURE));

	// extract sbrick words
	QAction *extractSbrickWordsAct = wordsExtractionMenu->addAction("&Extract sbrick words...");
	extractSbrickWordsAct->setStatusTip(tr("Extract sbrick words"));
	connect(extractSbrickWordsAct, SIGNAL(triggered()), wordsExtractionMapper, SLOT(map()));
	wordsExtractionMapper->setMapping(extractSbrickWordsAct, QString(Constants::WK_SBRICK));

	// extract sphrase words
	QAction *extractSphraseWordsAct = wordsExtractionMenu->addAction("&Extract sphrase words...");
	extractSphraseWordsAct->setStatusTip(tr("Extract sphrase words"));
	connect(extractSphraseWordsAct, SIGNAL(triggered()), wordsExtractionMapper, SLOT(map()));
	wordsExtractionMapper->setMapping(extractSphraseWordsAct, QString(Constants::WK_SPHRASE));

	// extract place and region names
	QAction *extractPlaceNamesAct = wordsExtractionMenu->addAction("&Extract place names...");
	extractPlaceNamesAct->setStatusTip(tr("Extract place names from primitives"));
	connect(extractPlaceNamesAct, SIGNAL(triggered()), wordsExtractionMapper, SLOT(map()));
	wordsExtractionMapper->setMapping(extractPlaceNamesAct, QString(Constants::WK_PLACE));

	// Merge options
	QAction *mergeSingleFileAct = wordsExtractionMenu->addAction("&Merge worksheet file...");
	mergeSingleFileAct->setStatusTip(tr("Merge worksheet file from local or remote directory"));
	connect(mergeSingleFileAct, SIGNAL(triggered()), this, SLOT(mergeSingleFile()));

	// Windows menu
	Core::ICore *core = Core::ICore::instance();
	Core::MenuManager *menuManager = core->menuManager();
	windowMenu = menuManager->menuBar()->addMenu("Window");
	updateWindowsList();
	connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowsList()));

	// Undo, Redo actions
	QAction *undoAction = menuManager->action(Core::Constants::UNDO);
	if (undoAction != 0)
		_ui.toolBar->addAction(undoAction);

	QAction *redoAction = menuManager->action(Core::Constants::REDO);
	if (redoAction != 0)
		_ui.toolBar->addAction(redoAction);
}

void CMainWindow::windowActivated(QMdiSubWindow *window) 
{
	Core::ICore *core = Core::ICore::instance();	
	core->contextManager()->updateCurrentContext();	
}

/**
 * Set the active subwindow
 * Convert the window to QMdiSubWindow and set active window on mdiArea
 */
void CMainWindow::setActiveSubWindow(QWidget *window)
{
	if (!window)
		return;

	QMdiSubWindow *mdiWindow = qobject_cast<QMdiSubWindow *>(window);
	if (mdiWindow != 0)
		_ui.mdiArea->setActiveSubWindow(mdiWindow);



}

/**
 * Update the menu Windows
 * Get the active subwindows and list them in Windows menu with
 * options to switch from them.
 * This function is used when the Windows menu is clicked.
 */
void CMainWindow::updateWindowsList()
{
	if(_ui.mdiArea->activeSubWindow())
	{
		windowMenu->clear();
		QMdiSubWindow *current_window = _ui.mdiArea->activeSubWindow();
		QList<QMdiSubWindow *> subWindows = _ui.mdiArea->subWindowList();

		updateToolbar(current_window);

		for(int i = 0; i < subWindows.size(); ++i)
		{
			QString window_file = QFileInfo(subWindows.at(i)->windowFilePath()).fileName();
			QString action_text;
			if (i < 9)
			{
				action_text = QString("&%1 %2").arg(i + 1).arg(window_file);
			}
			else
			{
				action_text = QString("%1 %2").arg(i + 1).arg(window_file);
			}
			QAction *action = new QAction(action_text, this);
			action->setCheckable(true);
			action->setChecked(subWindows.at(i) == current_window);
			connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
			windowMenu->addAction(action);
			windowMapper->setMapping(action, subWindows.at(i));
		}
	}
	else
	{
		windowMenu->clear();
	}
}

/**
 * Update the Windows menu with options for a Worksheet editor
 * This function is called by the updateWindowsList()
 */
void CMainWindow::updateToolbar(QMdiSubWindow *window)
{
	if(_ui.mdiArea->subWindowList().size() > 0)
		if(QString(window->widget()->metaObject()->className()) == "QTableWidget") // Sheet Editor
		{
			QAction *insertRowAct = new QAction(tr("Insert new row"), this);
			connect(insertRowAct, SIGNAL(triggered()), window, SLOT(insertRow()));
			windowMenu->addAction(insertRowAct);
			QAction *deleteRowAct = new QAction(tr("Delete row"), this);
			connect(deleteRowAct, SIGNAL(triggered()), window, SLOT(deleteRow()));
			windowMenu->addAction(deleteRowAct);
		}
}

/**
 * Open function
 * Open a file using the correct editor
 */
void CMainWindow::open()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup("translationmanager");
	QString lastOpenLocation = settings->value("lastOpenLocation").toString();
	QString file_name = QFileDialog::getOpenFileName(this, tr("Open translation file"), lastOpenLocation, tr("Translation files (*txt)"));
	QFileInfo *file_info = new QFileInfo(file_name);
	settings->setValue("lastOpenLocation", file_info->absolutePath());
	settings->endGroup();

	if(!file_name.isEmpty())
	{
		CEditor *editor = editorWindow->getEditorWindowByFileName(file_name, _ui.mdiArea->subWindowList());
		if(editor != NULL)
		{
			editor->activateWindow();
			return;
		} 
		QApplication::setOverrideCursor(Qt::WaitCursor);

		// worksheet editor
		if(editorWindow->getEditorType(file_name) == Constants::ED_WORKSHEET)
		{
			CEditorWorksheet *new_window = new CEditorWorksheet(_ui.mdiArea);
			new_window->open(file_name);
			new_window->activateWindow();
		}
		// phrase editor
		if(editorWindow->getEditorType(file_name) == Constants::ED_PHRASE)
		{
			CEditorPhrase *new_window = new CEditorPhrase(_ui.mdiArea);
			new_window->open(file_name);
			new_window->activateWindow();
		}

		QApplication::restoreOverrideCursor();
	}
}

/**
 * Open work file
 * In the settings dialog there is a option to set the directory
 * for work files.
 * This function is used by the extraction options. 
 * If there is no file in the editor we use a work a file.
 */
CEditorWorksheet* CMainWindow::openWorkFile(QString fileName)
{
	QFileInfo *filePath = new QFileInfo(QString("%1/%2").arg(work_path).arg(fileName));
	if(filePath->exists())
	{
		if(editorWindow->getEditorType(filePath->filePath()) == Constants::ED_WORKSHEET)
		{
			CEditorWorksheet *editor = new CEditorWorksheet(_ui.mdiArea);
			editor->open(filePath->filePath());
			editor->activateWindow();
			return editor;
		}
	}
	else
	{
		QErrorMessage error;
		error.showMessage(tr("The %1 file don't exists.").arg(filePath->fileName()));
		error.exec();
	}

	return NULL;
}

/**
 * Save function
 * Save the modification made in the editor in the coresponding file
 */
void CMainWindow::save()
{
	if(_ui.mdiArea->subWindowList().size() > 0)
	{
		CEditor *current_window = qobject_cast<CEditor *>(_ui.mdiArea->currentSubWindow());
		QApplication::setOverrideCursor(Qt::WaitCursor);
		current_window->save();
		QApplication::restoreOverrideCursor();
	}
}

/**
 * Save as function
 * Save the modification made in the editor in a custom file
 */
void CMainWindow::saveAs()
{
	QString file_name;
	if (_ui.mdiArea->isActiveWindow())
	{
		file_name = QFileDialog::getSaveFileName(this);
	}
	if (!file_name.isEmpty())
	{
		CEditor *current_window = qobject_cast<CEditor *>(_ui.mdiArea->currentSubWindow());
		QApplication::setOverrideCursor(Qt::WaitCursor);
		current_window->saveAs(file_name);
		QApplication::restoreOverrideCursor();
	}
}

/**
 * Initialize settings
 * This function is needed by extraction.
 */
void CMainWindow::initializeSettings(bool georges = false)
{
	if(georges == true && initialize_settings["georges"] == false)
	{
		NLMISC::CPath::addSearchPath(level_design_path.toStdString() + "/DFN", true, false);
		NLMISC::CPath::addSearchPath(level_design_path.toStdString() + "/Game_elem/Creature", true, false);
		initialize_settings["georges"] = true;
	}

	if(initialize_settings["ligo"] == false)
	{
		try
		{
			// Search path of file world_editor_classes.xml
			std::string ligoPath = NLMISC::CPath::lookup("world_editor_classes.xml");
			// Init LIGO
			ligoConfig.readPrimitiveClass(ligoPath.c_str(), true);
			NLLIGO::Register();
			NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig = &ligoConfig;
			initialize_settings["ligo"] = true;
		}
		catch (NLMISC::Exception &e)
		{
			nlerror("We haven't find the path to world_editor_classes.xml");
		}
	}
}

/**
 * Extract menu
 * Extraction options for the worksheet editor
 */
void CMainWindow::extractMenu(QString workSheetType)
{
	if(verifySettings())
	{

		CEditorWorksheet *worksheetEditor = editorWindow->getEditorWindowByWorksheetType(workSheetType, _ui.mdiArea->subWindowList());		
		if(worksheetEditor == NULL)
		{
			worksheetEditor = openWorkFile(workSheetType);
		}

		worksheetEditor->activateWindow();
		QString fileName = worksheetEditor->windowFilePath();

		if(workSheetType.toAscii() == Constants::WK_BOTNAMES)
		{
			extractBotNames(worksheetEditor);
		} 
		else
		{
			extractWords(worksheetEditor, fileName);
		}
	}
}

/**
 * Extract words
 * Extract words : sitem, creature, sbrick, sphrase, placeId
 */
void CMainWindow::extractWords(CEditorWorksheet *editor, QString fileName)
{
		QString column_name;
		// Sheet extraction
		CSheetWordListBuilder	builderS;
		// Primitives extraction
		CRegionPrimWordListBuilder builderP;
		bool isSheet = false;
		if(fileName.toAscii() == Constants::WK_ITEM)
		{
			column_name = "item ID";
			builderS.SheetExt = "sitem";
			builderS.SheetPath = level_design_path.append("/game_element/sitem").toStdString();
			isSheet = true;
		}
		else if(fileName.toAscii() == Constants::WK_CREATURE)
		{
			column_name = "creature ID";
			builderS.SheetExt = "creature";
			builderS.SheetPath = level_design_path.append("/Game_elem/Creature/fauna").toStdString();
			isSheet = true;
		}
		else if(fileName.toAscii() == Constants::WK_SBRICK)
		{
			column_name = "sbrick ID";
			builderS.SheetExt = "sbrick";
			builderS.SheetPath = level_design_path.append("/game_element/sbrick").toStdString();
			isSheet = true;
		}
		else if(fileName.toAscii() == Constants::WK_SPHRASE)
		{
			column_name = "sphrase ID";
			builderS.SheetExt = "sphrase";
			builderS.SheetPath = level_design_path.append("/game_element/sphrase").toStdString();
			isSheet = true;
		}
		else if(fileName.toAscii() == Constants::WK_PLACE)
		{
			column_name = "placeId";
			builderP.PrimPath = primitives_path.toStdString();
			builderP.PrimFilter.push_back("region_*.primitive");
			builderP.PrimFilter.push_back("indoors_*.primitive");
			isSheet = false;
		}

		QApplication::setOverrideCursor(Qt::WaitCursor);
		if(isSheet)
		{
			editor->extractWords(editor->windowFilePath(), column_name, builderS);
		}
		else
		{
			initializeSettings(false);
			editor->extractWords(editor->windowFilePath(), column_name, builderP);
		}
		QApplication::restoreOverrideCursor();
	
}

/**
 * Extract bot names
 * Extract bot names from primitives
 */
void CMainWindow::extractBotNames(CEditorWorksheet *editor)
{
		QApplication::setOverrideCursor(Qt::WaitCursor);
		initializeSettings(true);

		std::list<std::string> stdFilters;
		Q_FOREACH(QString filter, filters)
		{
			stdFilters.push_back(filter.toStdString());
		}

		editor->extractBotNames(stdFilters, level_design_path.toStdString(), ligoConfig);
		QApplication::restoreOverrideCursor();
}

// Merge the content for 2 worksheet files
void CMainWindow::mergeSingleFile()
{
	CEditor *editor_window = qobject_cast<CEditor *>(_ui.mdiArea->currentSubWindow());
	CSourceDialog *dialog = new CSourceDialog(this);
	CFtpSelection *ftp_dialog;
	map<QListWidgetItem *, int> methods;
	QString file_name;

	if (_ui.mdiArea->subWindowList().size() == 0)
	{
		QErrorMessage error;
		error.showMessage(tr("Open a work file in editor for merge operation."));
		error.exec();
		return;
	}

	if(editor_window->eType() != Constants::ED_WORKSHEET) // Sheet Editor
	{
		QErrorMessage error;
		error.showMessage(tr("Please open or activate the window with a sheet file."));
		error.exec();
		return;
	}

	// create items
	QListWidgetItem *local_item = new QListWidgetItem();
	local_item->setText("Local directory");
	methods[local_item] = 0;
	QListWidgetItem *ftp_item = new QListWidgetItem();
	ftp_item->setText("From a FTP server");
	methods[ftp_item] = 1;

	dialog->setSourceOptions(methods);
	dialog->show();
	dialog->exec();
	// get the file for merge
	if(dialog->selected_item == local_item) // Local directory
	{
		file_name = QFileDialog::getOpenFileName(this);
	}
	else if(dialog->selected_item == ftp_item) // Ftp directory
	{
		CFtpSelection *ftp_dialog = new CFtpSelection(this);
		ftp_dialog->show();

		if(ftp_dialog->exec() && ftp_dialog->status == true)
			file_name = ftp_dialog->file->fileName();

		delete ftp_dialog;
	}
	else
		return;

	// Make sure we retrieved a file name
	if(file_name.isEmpty())
		return;

	editor_window->activateWindow();
	CEditorWorksheet *current_window = qobject_cast<CEditorWorksheet *>(editor_window);
	if(current_window->windowFilePath() == file_name)
		return;
	if(current_window->compareWorksheetFile(file_name))
	{
		current_window->mergeWorksheetFile(file_name);
	}
	else
	{
		QErrorMessage error;
		error.showMessage(tr("The file: %1 has different columns from the current file in editor.").arg(file_name));
		error.exec();
	}
	if(dialog->selected_item == ftp_item)
	{
		/*
		// TODO: uninit ftp_dialog?????
		if(!ftp_dialog->file->remove())
		{
			QErrorMessage error;
			error.showMessage(tr("Please remove the file from ftp server manually. The file is located on the same directory with OVQT application."));
			error.exec();
		}
		*/
	}
}

/**
 * Return the current Undo Stack
 * Used by the context manager
 */
QUndoStack* CMainWindow::getCurrentUndoStack() 
{
	if(_ui.mdiArea->activeSubWindow() == 0)
	{
		return undoStack;
	} else {
		CEditor *editor = qobject_cast<CEditor *>(_ui.mdiArea->activeSubWindow());
		return editor->getUndoStack();
	}	
}

// Read the settings from QSettings
void CMainWindow::readSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	// translation manager settings
	settings->beginGroup("translationmanager");
	filters = settings->value("filters").toStringList();
	languages = settings->value("trlanguages").toStringList();
	translation_path = settings->value("translation").toString();
	work_path = settings->value("work").toString();
	settings->endGroup();
	// core settings
	settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
	level_design_path = settings->value(Core::Constants::LEVELDESIGN_PATH).toString();
	primitives_path = QString(Core::Constants::PRIMITIVES_PATH); //TODO
	settings->endGroup();
}

// Verify the settings
bool CMainWindow::verifySettings()
{
	bool count_errors = false;

	if(level_design_path.isNull() || primitives_path.isNull() || work_path.isNull())
	{
		QErrorMessage error_settings;
		error_settings.showMessage(tr("Please write all the paths on the settings dialog."));
		error_settings.exec();
		count_errors = true;
	}

	return !count_errors;
}

bool CCoreListener::closeMainWindow() const
{
	bool okToClose = true;
	Q_FOREACH(QMdiSubWindow *subWindow, m_MainWindow->_ui.mdiArea->subWindowList())
	{
		CEditor *currentEditor = qobject_cast<CEditor *>(subWindow);
		if(subWindow->isWindowModified())
		{
			QMessageBox msgBox;
			msgBox.setIcon(QMessageBox::Question);
			msgBox.setText(tr("The document has been modified (%1).").arg(currentEditor->windowFilePath()));
			msgBox.setInformativeText(tr("Do you want to save your changes?"));
			msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Save);
			int ret = msgBox.exec();
			if(ret == QMessageBox::Save)
			{
				currentEditor->save();
				currentEditor->removeUndoStack();
			}
			else if(ret == QMessageBox::Cancel)
			{
				okToClose = false;
				break;
			} else {
				currentEditor->removeUndoStack();
			}
		}
	}
	return okToClose;
}

} /* namespace TranslationManager */


