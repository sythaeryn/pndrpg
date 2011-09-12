// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef BUILD_GAMEDATA_VIEW_H
#define BUILD_GAMEDATA_VIEW_H

#include "ui_build_gamedata_view.h"

#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"

#include <QtGui/QDialog>

class BuildGamedataView : public QDockWidget
{
	Q_OBJECT

public:
	explicit BuildGamedataView(QWidget *parent = 0);
	~BuildGamedataView();

private:
	Ui::BuildGamedataView m_ui;
};

#endif // BUILD_GAMEDATA_VIEW_H
