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

#ifndef GRAPHICS_WIDGET_H
#define GRAPHICS_WIDGET_H

// Project includes
#include "nel_widget.h"

namespace SceneViewer
{

/**
@class GraphicsWidget
@brief
*/
class GraphicsWidget : public NLQT::QNLWidget
{
	Q_OBJECT

public:
	GraphicsWidget(QWidget *parent);
	virtual ~GraphicsWidget();

protected:
	virtual void resizeEvent(QResizeEvent *resizeEvent);
	virtual void updateData();
	virtual void updateRender();

private:
	Q_DISABLE_COPY(GraphicsWidget)

}; /* class GraphicsWidget */

} /* namespace SceneViewer */


#endif // GRAPHICS_VIEWPORT_H
