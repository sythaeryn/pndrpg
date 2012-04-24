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

#include "scene_node.h"

#include "scene_editor_constants.h"

namespace SceneEditor
{

SceneNode::SceneNode()
	: m_parent(0)
{
}

SceneNode::~SceneNode()
{
	if (m_parent)
		m_parent->removeChildNode(this);

	deleteAllChilds();
}

void SceneNode::prependChildNode(SceneNode *node)
{
	// Node is already a child
	nlassert(!m_children.contains(node));

	// Node already has a parent
	nlassert(!m_children.contains(node));

	m_children.prepend(node);
	node->m_parent = this;
}

void SceneNode::appendChildNode(SceneNode *node)
{
	// Node is already a child
	nlassert(!m_children.contains(node));

	// Node already has a parent
	nlassert(!m_children.contains(node));

	m_children.append(node);
	node->m_parent = this;
}

void SceneNode::insertChildNodeBefore(SceneNode *node, SceneNode *before)
{
	// Node is already a child
	nlassert(!m_children.contains(node));

	// Node already has a parent
	nlassert(!m_children.contains(node));

	int idx = before ? m_children.indexOf(before) : -1;
	if (idx == -1)
		m_children.append(node);
	else
		m_children.insert(idx, node);
	node->m_parent = this;
}

void SceneNode::insertChildNodeAfter(SceneNode *node, SceneNode *after)
{
	// Node is already a child
	nlassert(!m_children.contains(node));

	// Node already has a parent
	nlassert(!m_children.contains(node));

	int idx = after ? m_children.indexOf(after) : -1;
	if (idx == -1)
		m_children.append(node);
	else
		m_children.insert(idx + 1, node);
	node->m_parent = this;
}

void SceneNode::insertChildNode(int pos, SceneNode *node)
{
	// Node is already a child
	nlassert(!m_children.contains(node));

	// Node already has a parent
	nlassert(!m_children.contains(node));

	m_children.insert(pos, node);
	node->m_parent = this;
}

void SceneNode::removeChildNode(SceneNode *node)
{
	nlassert(m_children.contains(node));
	nlassert(node->parent() == this);

	m_children.removeOne(node);

	node->m_parent = 0;
}

SceneNode *SceneNode::child(int row)
{
	return m_children.at(row);
}

int SceneNode::childCount() const
{
	return m_children.count();
}

void SceneNode::deleteAllChilds()
{
	qDeleteAll(m_children);
	m_children.clear();
}

QVariant SceneNode::data(int key) const
{
	return m_data[key];
}

void SceneNode::setData(int key, const QVariant &data)
{
	m_data[key] = data;
}

SceneNode *SceneNode::parent()
{
	return m_parent;
}

int SceneNode::row() const
{
	if (m_parent)
		return m_parent->m_children.indexOf(const_cast<SceneNode *>(this));

	return 0;
}

SceneNode::NodeType SceneNode::type() const
{
	return BasicNodeType;
}

/// Transform scene node
TransformNode::TransformNode(NL3D::UScene *scene)
	: SceneNode(),
	m_scene(scene)
{
	nlassert(m_scene);
	m_transform = m_scene->createTransform();
}

TransformNode::TransformNode(NL3D::UTransform transform)
	: m_scene(0)
{
	m_transform = transform;
}

TransformNode::TransformNode()
	: m_scene(0)
{
}

TransformNode::~TransformNode()
{
	if (m_scene != 0)
		m_scene->deleteTransform(m_transform);
}

SceneNode::NodeType TransformNode::type() const
{
	return TransformNodeType;
}

void TransformNode::bind(NL3D::UTransform transform)
{
	transform.parent(transform);
}

void TransformNode::setTransform(NL3D::UTransform transform)
{
	m_transform = transform;
}

NL3D::UTransform TransformNode::transform() const
{
	return m_transform;
}

/// Instance scene node
InstanceNode::InstanceNode(NL3D::UScene *scene, const QString &fileName)
	: TransformNode(),
	m_scene(scene)
{
	nlassert(m_scene);
	m_instance = m_scene->createInstance(fileName.toStdString());
	if  (!m_instance.empty())
	{
		setTransform(m_instance);
		m_instance.setTransformMode(NL3D::UTransformable::RotEuler);
		m_instance.start();
	}
	else
	{
		nlerror("createInstance %s returned 0 -> no shape.", fileName.toStdString().c_str());
	}
	setData(Constants::NODE_FILE_NAME, fileName);
}

InstanceNode::InstanceNode()
	: TransformNode(),
	m_scene(0)
{
}

InstanceNode::InstanceNode(NL3D::UInstance instance)
	: TransformNode(),
	m_scene(0)
{
	m_instance = instance;
	setTransform(m_instance);
}

InstanceNode::~InstanceNode()
{
	if ((m_scene != 0) && (!m_instance.empty()))
		m_scene->deleteInstance(m_instance);
}

SceneNode::NodeType InstanceNode::type() const
{
	return InstanceNodeType;
}

void InstanceNode::setInstance(NL3D::UInstance instance)
{
	m_instance = instance;
	setTransform(instance);
}

NL3D::UInstance InstanceNode::instance() const
{
	return m_instance;
}

/// Skeleton scene node
SkeletonNode::SkeletonNode(NL3D::UScene *scene, const QString &fileName)
	: TransformNode(),
	m_scene(0)
{
	nlassert(m_scene);
	m_skeleton = m_scene->createSkeleton(fileName.toStdString());
	m_skelTransform = m_scene->createTransform();

	if (!m_skeleton.empty())
	{
		m_skeleton.parent(m_skelTransform);
	}
	else
		nlerror("createSkeleton %s returned 0 -> no skeleton.", fileName.toStdString().c_str());

	setData(Constants::NODE_FILE_NAME, fileName);
}

SkeletonNode::~SkeletonNode()
{
	if (!m_skeleton.empty())
	{
		m_scene->deleteSkeleton( m_skeleton );
		m_scene->deleteTransform(m_skelTransform);
	}
}

SceneNode::NodeType SkeletonNode::type() const
{
	return SkeletonNodeType;
}

NL3D::USkeleton SkeletonNode::skeleton() const
{
	return m_skeleton;
}

void SkeletonNode::bind(NL3D::UTransform transform)
{
	NL3D::UInstance instance;
	instance.cast(transform);

	// TODO: strong check
	m_skeleton.bindSkin(instance);
}

/// Particle system scene node
ParticleSystemNode::ParticleSystemNode(NL3D::UScene *scene, const QString &fileName)
	: InstanceNode(),
	m_scene(scene)
{
	nlassert(m_scene);

	NL3D::UInstance instance = m_scene->createInstance(fileName.toStdString());
	if  (!instance.empty())
	{
		setInstance(instance);
		m_particleSystem.cast(instance);
	}
	else
	{
		nlerror("createInstance %s returned 0 -> no particle system.", fileName.toStdString().c_str());
	}
	setData(Constants::NODE_FILE_NAME, fileName);
}

ParticleSystemNode::~ParticleSystemNode()
{
	if (!m_particleSystem.empty())
		m_scene->deleteInstance(m_particleSystem);
}

SceneNode::NodeType ParticleSystemNode::type() const
{
	return ParticleSystemNodeType;
}

} /* namespace SceneEditor */