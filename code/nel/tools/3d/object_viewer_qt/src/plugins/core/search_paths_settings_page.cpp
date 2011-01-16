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

#include "search_paths_settings_page.h"

// Qt includes
#include <QtGui/QWidget>

// NeL includes

// Project includes

namespace Core
{

CSearchPathsSettingsPage::CSearchPathsSettingsPage(QObject *parent)
	: QObject(parent),
	  _currentPage(NULL)
{
}

QString CSearchPathsSettingsPage::id() const
{
	return QLatin1String("SearchPaths");
}

QString CSearchPathsSettingsPage::trName() const
{
	return tr("Search Paths");
}

QString CSearchPathsSettingsPage::category() const
{
	return QLatin1String("General");
}

QString CSearchPathsSettingsPage::trCategory() const
{
	return tr("General");
}

QWidget *CSearchPathsSettingsPage::createPage(QWidget *parent)
{
	_currentPage = new QWidget(parent);
	_ui.setupUi(_currentPage);
	return _currentPage;
}

void CSearchPathsSettingsPage::apply()
{
}

} /* namespace Core */