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

#ifndef EDITOR_BASE_H
#define EDITOR_BASE_H

// Project includes
#include "editor_worksheet.h"
#include "editor_phrase.h"

namespace TranslationManager
{

class CEditorBase
{
public:
	CEditorBase();
	int getEditorType(QString fileName);
	CEditor* getEditorWindowByFileName(const QString &fileName,  QList<QMdiSubWindow*> subWindows);
	CEditorWorksheet* getEditorWindowByWorksheetType(const QString &workSheetType,  QList<QMdiSubWindow*> subWindows);
};


}



#endif	/* EDITOR_BASE_H */