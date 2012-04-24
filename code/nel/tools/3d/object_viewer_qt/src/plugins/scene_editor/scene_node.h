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

#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include <nel/misc/types_nl.h>
#include <nel/3d/animation_time.h>
#include <nel/3d/u_transform.h>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVariant>

namespace SceneEditor
{

/*
@class SceneNode
@brief
@details
*/
class SceneNode
{
public:

	enum NodeType
	{
		BasicNodeType,
		RootNodeType,
		TransformNodeType,
		InstanceNodeType,
		SkeletonNodeType,
		ParticleSystemNodeType,
		InstanceGroupNodeType,
		UserNodeType = 1024
	};

	SceneNode();
	virtual ~SceneNode();

	/// Remove child node from the child list.
	void removeChildNode(SceneNode *node);

	/// Insert node at the beginning of the list.
	void prependChildNode(SceneNode *node);

	/// Insert node at the end of the list.
	void appendChildNode(SceneNode *node);

	/// Insert node in front of the node pointed to by the pointer before.
	void insertChildNodeBefore(SceneNode *node, SceneNode *before);

	/// Insert node in back of the node pointed to by the pointer after.
	void insertChildNodeAfter(SceneNode *node, SceneNode *after);

	/// Insert node in pos
	void insertChildNode(int pos, SceneNode *node);

	/// Return the node at index position row in the child list.
	SceneNode *child(int row);

	/// Return the number of nodes in the list.
	int childCount() const;

	/// Return a row index this node.
	int row() const;

	/// Return a pointer to this node's parent item. If this node does not have a parent, 0 is returned.
	SceneNode *parent();

	/// Set this node's custom data for the key key to value.
	void setData(int key, const QVariant &data);

	/// Return this node's custom data for the key key as a QVariant.
	QVariant data(int key) const;

	/// Return a type this node.
	virtual NodeType type() const;

private:
	Q_DISABLE_COPY(SceneNode)

	SceneNode *m_parent;
	QList<SceneNode *> m_children;
	QHash<int, QVariant> m_data;
};

} /* namespace SceneEditor */

#endif // SCENE_NODE_H
