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

#include "scene_manager_base.h"

#include <nel/misc/debug.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/path.h>

namespace SceneEditor
{

SceneManagerBase::SceneManagerBase(NL3D::UDriver *driver)
	: m_driver(driver),
	m_scene(0),
	m_light(0),
	m_mouseListener(0),
	m_rootNode(0)
{
	nlassert(m_driver);

	// Create a scene
	m_scene = m_driver->createScene(false);
	nlassert(m_scene);

	m_scene->setPolygonBalancingMode(NL3D::UScene::PolygonBalancingClamp);

	// replace on LightSceneNode
	m_light = NL3D::ULight::createLight();

	m_light->setMode(NL3D::ULight::DirectionalLight);
	m_light->setPosition(NLMISC::CVector(-20.f, 30.f, 10.f));

	// white light
	m_light->setAmbiant(NLMISC::CRGBA(255, 255, 255));

	// set and enable the light
	m_driver->setLight(0, *m_light);
	m_driver->enableLight(0);
	m_scene->enableLightingSystem(true);

	m_mouseListener = m_driver->create3dMouseListener();
	m_mouseListener->setMouseMode(NL3D::U3dMouseListener::firstPerson);

	m_rootNode = new TransformNode(m_scene);
}

SceneManagerBase::~SceneManagerBase()
{
	delete m_rootNode;
	delete m_light;

	m_driver->delete3dMouseListener(m_mouseListener);
	m_driver->deleteScene(m_scene);
}

TransformNode *SceneManagerBase::addInstance(const QString &fileName)
{
	return 0;
}

TransformNode *SceneManagerBase::addCharacter(const QStringList &listNames, const QString &skelName)
{
	return 0;
}

NL3D::UScene *SceneManagerBase::scene() const
{
	return m_scene;
}

NL3D::U3dMouseListener *SceneManagerBase::getMouseListener() const
{
	return m_mouseListener;
}

}