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

#include "graphics_widget.h"

// STL includes

// Qt includes
#include <QtGui/QResizeEvent>

// NeL includes
#include <nel/misc/rgba.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/driver_user.h>

namespace SceneViewer
{

GraphicsWidget::GraphicsWidget(QWidget *parent)
	: NLQT::QNLWidget(parent)
{
}

GraphicsWidget::~GraphicsWidget()
{

}

void GraphicsWidget::updateData()
{
}

void GraphicsWidget::updateRender()
{
}

void GraphicsWidget::resizeEvent(QResizeEvent *resizeEvent)
{
	QWidget::resizeEvent(resizeEvent);
}

} /* namespace SceneViewer */

