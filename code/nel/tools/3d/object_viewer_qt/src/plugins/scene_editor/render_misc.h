// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2012  Dzmitry Kamiahin <dzmitry.kamiahin@gmail.com>
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

#ifndef RENDER_MISC_H
#define RENDER_MISC_H

#include <nel/3d/u_driver.h>
#include <nel/3d/u_material.h>

namespace SceneEditor
{

namespace RenderMisc
{

void init(NL3D::UDriver *driver);
void release();

void drawArrow(const NLMISC::CVector &start, const NLMISC::CVector &dir, const NLMISC::CRGBA &color, float size);
void drawBox(const NLMISC::CVector &vMin, const NLMISC::CVector &vMax, const NLMISC::CRGBA &color);

}

} /* namespace SceneEditor */

#endif // RENDER_MISC_H
