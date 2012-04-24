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

#ifndef SCENE_MANAGER_BASE_H
#define SCENE_MANAGER_BASE_H

#include "scene_node.h"

#include <nel/3d/u_driver.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_light.h>
#include <nel/3d/u_3d_mouse_listener.h>

namespace SceneEditor
{

class SceneManagerBase
{
public:
	SceneManagerBase(NL3D::UDriver *driver);
	~SceneManagerBase();

	TransformNode *addInstance(const QString &fileName);
	TransformNode *addCharacter(const QStringList &listNames, const QString &skelName);

	NL3D::UScene *scene() const;
	NL3D::U3dMouseListener *getMouseListener() const;

private:
	NL3D::UDriver *m_driver;
	NL3D::UScene *m_scene;
	NL3D::ULight *m_light;
	NL3D::U3dMouseListener *m_mouseListener;

	TransformNode *m_rootNode;
};

} /* namespace SceneEditor */

#endif // SCENE_MANAGER_BASE_H
