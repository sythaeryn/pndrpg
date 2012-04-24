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

	qDeleteAll(m_children);
	nlassert(m_children.isEmpty());
	m_data.clear();
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

} /* namespace SceneEditor */