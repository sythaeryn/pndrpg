// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
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

// Project includes
#include "editor_base.h"
#include "translation_manager_constants.h"

// STL includes
#include <vector>

// Nel includes
#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"
#include "nel/ligo/ligo_config.h"

namespace TranslationManager
{

CEditorBase::CEditorBase()
{

}

int CEditorBase::getEditorType(QString fileName)
{
	STRING_MANAGER::TWorksheet worksheetFile;
	if((loadExcelSheet(fileName.toStdString(), worksheetFile, true) == true) && 
		(worksheetFile.ColCount > 1))
	{
		return Constants::ED_WORKSHEET;		
	}
		
	std::vector<STRING_MANAGER::TPhrase> phrases;
	if(readPhraseFile(fileName.toStdString(), phrases, false))
	{
		return Constants::ED_PHRASE;
	}

	return 0;	
}

CEditor* CEditorBase::getEditorWindowByFileName(const QString &fileName, QList<QMdiSubWindow*> subWindows)
{
	Q_FOREACH(QMdiSubWindow *subWindow, subWindows)
	{
		CEditor *currentEditor = qobject_cast<CEditor *>(subWindow);
		if(currentEditor->subWindowFilePath() == fileName)
		{
			return currentEditor;
		}
			
	}

	return NULL;		
}

CEditorWorksheet* CEditorBase::getEditorWindowByWorksheetType(const QString &workSheetType, QList<QMdiSubWindow*> subWindows)
{
	Q_FOREACH(QMdiSubWindow *subWindow, subWindows)
	{
		CEditor *currentEditor = qobject_cast<CEditor *>(subWindow);
		if(currentEditor->eType() == Constants::ED_WORKSHEET)
		{
			CEditorWorksheet *workSheetEditor = qobject_cast<CEditorWorksheet *>(currentEditor);
			if(workSheetType != Constants::WK_BOTNAMES)
			{
				if(workSheetEditor->isSheetTable(workSheetType))
				{
					return workSheetEditor;
				}
			}
			else
			{
				if(workSheetEditor->isBotNamesTable())
				{
					return workSheetEditor;
				}
			}
		}
	}

	return NULL;
}

} /* namespace TranslationManager */