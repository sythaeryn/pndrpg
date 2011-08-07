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

#ifndef EDITOR_PHRASE_H
#define EDITOR_PHRASE_H

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QWidget>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QUndoCommand>
#include <QtGui/QUndoStack>
#include <QtGui/QTextEdit>

// Project includes
#include "translation_manager_editor.h"

namespace Plugin {

class CEditorPhrase : public CEditor
{
	Q_OBJECT
private:
	QTextEdit *text_edit;
public:
    CEditorPhrase(QMdiArea* parent) : CEditor(parent) {}
    CEditorPhrase() : CEditor() {}
    void open(QString filename);
    void save();
    void saveAs(QString filename);
    void activateWindow();
	void closeEvent(QCloseEvent *event);
};

}

#endif	/* EDITOR_PHRASE_H */