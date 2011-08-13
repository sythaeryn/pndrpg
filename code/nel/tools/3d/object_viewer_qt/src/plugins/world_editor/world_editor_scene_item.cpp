// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// Project includes
#include "world_editor_scene_item.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QPainter>
#include <QtCore/QRectF>
#include <QPolygonF>
#include <QTransform>
#include <QStyleOptionGraphicsItem>
#include <QPropertyAnimation>

namespace WorldEditor
{

static QPainterPath qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen)
{
	// We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
	// if we pass a value of 0.0 to QPainterPathStroker::setWidth()
	const qreal penWidthZero = qreal(0.00000001);

	if (path == QPainterPath())
		return path;
	QPainterPathStroker ps;
	ps.setCapStyle(pen.capStyle());
	if (pen.widthF() <= 0.0)
		ps.setWidth(penWidthZero);
	else
		ps.setWidth(pen.widthF());
	ps.setJoinStyle(pen.joinStyle());
	ps.setMiterLimit(pen.miterLimit());
	QPainterPath p = ps.createStroke(path);
	p.addPath(path);
	return p;
}
/*
GraphicsItemNode::GraphicsItemNode(GraphicsItemZone *itemZone, QGraphicsItem *parent)
	: QGraphicsObject(parent)
{
	m_itemZone = itemZone;
	m_color = QColor(12, 150, 215);
	//setFlag(ItemIgnoresTransformations, true);
	//setFlag(ItemClipsToShape);

	QPropertyAnimation *animation = new QPropertyAnimation(this, "colorNode");
	animation->setDuration(3000);
	animation->setStartValue(QColor(10, 0, 50));
	animation->setKeyValueAt(0.5, QColor(155, 255, 0));
	animation->setEndValue(QColor(10, 0, 50));
	animation->setLoopCount(2000);
	animation->setEasingCurve(QEasingCurve::OutInExpo);
	animation->start();

	setFlag(ItemIsSelectable);
	setFlag(ItemIsMovable);
	setFlag(ItemSendsScenePositionChanges);
	m_type = EdgeType;

	setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
	setZValue(10000000);
}

GraphicsItemNode::~GraphicsItemNode()
{
}

void GraphicsItemNode::setColorNode(const QColor &color)
{
	m_color = color;
	update();
}

void GraphicsItemNode::setNodeType(NodeType nodeType)
{
	m_type = nodeType;
	if (m_type == EdgeType)
	{
		setFlag(ItemIsSelectable);
		setFlag(ItemIsMovable);
		setFlag(ItemSendsScenePositionChanges);
		setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
		setZValue(10000);
	}
	else if (m_type == MiddleType)
	{
		setFlag(ItemIsSelectable, false);
		setFlag(ItemIsMovable, false);
		setFlag(ItemSendsScenePositionChanges, false);
		setAcceptedMouseButtons(Qt::LeftButton);
		setZValue(10001);
	}
	update();
}

QRectF GraphicsItemNode::boundingRect() const
{
	return QRectF(QPointF(0, 0), QSizeF(20, 20));
}

void GraphicsItemNode::paint(QPainter *painter,
							 const QStyleOptionGraphicsItem *option,
							 QWidget *)
{
	// Here comes the magic:
	//painter->setClipRect(option->exposedRect);

	painter->setPen(Qt::NoPen);

	if (m_type == EdgeType)
	{
		painter->setBrush(QColor(255, 0, 0));
	}
	else if (m_type == MiddleType)
	{
		painter->setBrush(QColor(0, 0, 255));
	}
	if (option->state & QStyle::State_Selected)
	{
		painter->setBrush(QColor(0, 255, 0));
	}

	painter->drawRect(2, 2, 18, 18);
}

QVariant GraphicsItemNode::itemChange(GraphicsItemChange change,
									  const QVariant &value)
{
	if (change == ItemPositionHasChanged)
	{
		m_itemZone->updateZone();
	}
	return QGraphicsItem::itemChange(change, value);
}

void GraphicsItemNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if ((m_type == MiddleType) && (event->button() == Qt::LeftButton))
	{
		m_itemZone->updateMiddleNode(this);
		setNodeType(EdgeType);
	}
	else if ((m_type == EdgeType) && (event->button() == Qt::RightButton))
	{
		if (m_itemZone->deleteEdgePoint(this))
			setNodeType(MiddleType);
	}
}

GraphicsItemZone::GraphicsItemZone(QGraphicsScene *scene, QGraphicsItem *parent)
	: QGraphicsPolygonItem(parent)
{
	m_scene = scene;

	m_color = QColor(12, 150, 215);

	setBrush(QBrush(QColor(100, 100, 255, 128)));
	updateZone();
	setZValue(100);
	//setFlag(ItemClipsToShape);
	//setFlag(ItemIsSelectable, true);
	//setFlag(ItemIsMovable, true);
	//setFlag(ItemHasNoContents, true);
}

GraphicsItemZone::~GraphicsItemZone()
{
}

void GraphicsItemZone::updateMiddleNode(GraphicsItemNode *node)
{
	for (int i = 0; i < m_listLines.size(); ++i)
	{
		if (node == m_listLines.at(i).itemPoint)
		{
			LineItem oldLineItem = m_listLines[i];

			GraphicsItemNode *newNode1 = new GraphicsItemNode(this);
			newNode1->setPos((oldLineItem.lineNode.first->pos() + node->pos()) / 2);
			newNode1->setNodeType(GraphicsItemNode::MiddleType);
			m_scene->addItem(newNode1);

			GraphicsItemNode *newNode2 = new GraphicsItemNode(this);
			newNode2->setPos((oldLineItem.lineNode.second->pos() + node->pos()) / 2);
			newNode2->setNodeType(GraphicsItemNode::MiddleType);
			m_scene->addItem(newNode2);

			LineItem newLineItem1;
			newLineItem1.itemPoint = newNode1;
			newLineItem1.lineNode = LineNode(oldLineItem.lineNode.first, node);
			m_listLines.push_back(newLineItem1);

			LineItem newLineItem2;
			newLineItem2.itemPoint = newNode2;
			newLineItem2.lineNode = LineNode(node, oldLineItem.lineNode.second);
			m_listLines.push_back(newLineItem2);

			m_listLines.removeAt(i);

			int pos = m_listItems.indexOf(oldLineItem.lineNode.second);
			m_listItems.insert(pos, node);

			break;
		}
	}
}

bool GraphicsItemZone::deleteEdgePoint(GraphicsItemNode *node)
{
	if (m_listItems.size() < 4)
		return false;

	int pos = m_listItems.indexOf(node);
	m_listItems.takeAt(pos);

	LineItem newLineItem;

	newLineItem.itemPoint = node;

	for (int i = 0; i < m_listLines.size(); ++i)
	{
		if (node == m_listLines.at(i).lineNode.first)
		{
			// Saving second point for new line
			newLineItem.lineNode.second = m_listLines.at(i).lineNode.second;
			delete m_listLines.at(i).itemPoint;
			m_listLines.removeAt(i);
			break;
		}
	}

	for (int i = 0; i < m_listLines.size(); ++i)
	{
		if (node == m_listLines.at(i).lineNode.second)
		{
			newLineItem.lineNode.first = m_listLines.at(i).lineNode.first;
			delete m_listLines.at(i).itemPoint;
			m_listLines.removeAt(i);
			break;
		}
	}
	node->setPos((newLineItem.lineNode.first->pos() + newLineItem.lineNode.second->pos()) / 2);
	m_listLines.push_back(newLineItem);

	return true;
}

void GraphicsItemZone::scanPolygon(const QPolygonF &polygon)
{
	GraphicsItemNode *node1;
	node1 = new GraphicsItemNode(this);
	node1->setPos(*polygon.begin());
	m_listItems.push_back(node1);
	m_scene->addItem(node1);
	for (int i = 1; i < polygon.count(); ++i)
	{
		GraphicsItemNode *node2 = new GraphicsItemNode(this);
		node2->setPos(polygon.at(i));
		m_listItems.push_back(node2);

		GraphicsItemNode *node3 = new GraphicsItemNode(this);
		node3->setPos((node1->pos() + node2->pos()) / 2);
		node3->setNodeType(GraphicsItemNode::MiddleType);
		m_scene->addItem(node3);

		LineItem newLineItem;
		newLineItem.itemPoint = node3;
		newLineItem.lineNode = LineNode(node1, node2);
		m_listLines.push_back(newLineItem);

		node1 = node2;
		m_scene->addItem(node1);
	}
	setPolygon(polygon);
}

void GraphicsItemZone::updateZone()
{
	QPolygonF polygon;
	Q_FOREACH(GraphicsItemNode *node, m_listItems)
	{
		polygon << node->pos();
	}

	for (int i = 0; i < m_listLines.size(); ++i)
	{
		m_listLines.at(i).itemPoint->setPos((m_listLines.at(i).lineNode.first->pos() + m_listLines.at(i).lineNode.second->pos()) / 2);
	}

	setPolygon(polygon);
}
*/

AbstractWorldItem::AbstractWorldItem(QGraphicsItem *parent)
	: QGraphicsItem(parent)
{
}

AbstractWorldItem::~AbstractWorldItem()
{
}

int AbstractWorldItem::type() const
{
	return Type;
}

WorldItemPoint::WorldItemPoint(const QPointF &point, const qreal angle, const qreal radius,
							   bool showArrow, QGraphicsItem *parent)
	: AbstractWorldItem(parent),
	  m_angle((2 * NLMISC::Pi - angle) * 180 / NLMISC::Pi),
	  m_radius(radius),
	  m_showArrow(showArrow)
{
	setZValue(WORLD_POINT_LAYER);

	//setFlag(ItemIgnoresTransformations);

	setPos(point);

	m_rect.setCoords(-SIZE_POINT, -SIZE_POINT, SIZE_POINT, SIZE_POINT);

	m_pen.setColor(QColor(255, 100, 10));
	//m_pen.setWidth(0);

	m_selectedPen.setColor(Qt::white);
	//m_selectedPen.setWidth(0);

	m_brush.setColor(QColor(255, 100, 10));
	m_brush.setStyle(Qt::SolidPattern);

	m_selectedBrush.setColor(Qt::white);
	m_selectedBrush.setStyle(Qt::SolidPattern);

	createCircle();

	// Create arrow
	if (showArrow)
	{
		m_arrow.push_back(QLine(0, 0, SIZE_ARROW, 0));
		m_arrow.push_back(QLine(SIZE_ARROW - 2, -2, SIZE_ARROW, 0));
		m_arrow.push_back(QLine(SIZE_ARROW - 2, 2, SIZE_ARROW, 0));
	}

	updateBoundingRect();
	//setFlag(ItemIsSelectable);
}

WorldItemPoint::~WorldItemPoint()
{
}

void WorldItemPoint::rotateOn(const QPointF &pivot, const qreal deltaAngle)
{
	prepareGeometryChange();

	QPolygonF rotatedPolygon(m_rect);

	// TODO
	rotatedPolygon.translate(pos() - pivot);
	//rotatedPolygon.translate(-pivot);

	QTransform trans;
	trans = trans.rotate(deltaAngle);
	rotatedPolygon = trans.map(rotatedPolygon);
	rotatedPolygon.translate(pivot);

	setPos(rotatedPolygon.boundingRect().center());
}

void WorldItemPoint::scaleOn(const QPointF &pivot, const QPointF &factor)
{
	prepareGeometryChange();

	QPolygonF scaledPolygon(m_rect);

	// TODO
	scaledPolygon.translate(pos() - pivot);
	//scaledPolygon.translate(-pivot);

	QTransform trans;
	trans = trans.scale(factor.x(), factor.y());
	scaledPolygon = trans.map(scaledPolygon);
	scaledPolygon.translate(pivot);

	setPos(scaledPolygon.boundingRect().center());
}

void WorldItemPoint::turnOn(const qreal angle)
{
	m_angle += angle;
	update();
}

void WorldItemPoint::radiusOn(const qreal radius)
{
	if (m_radius == 0)
		return;
}

void WorldItemPoint::setColor(const QColor &color)
{
	m_pen.setColor(color);
	m_brush.setColor(color);
}

void WorldItemPoint::createCircle()
{
	if (m_radius != 0)
	{
		// Create circle
		int segmentCount = 30;
		QPointF circlePoint(m_radius, 0);
		m_circle << circlePoint;
		for (int i = 1; i < segmentCount + 1; ++i)
		{
			qreal angle = i * (2 * NLMISC::Pi / segmentCount);
			circlePoint.setX(cos(angle) * m_radius);
			circlePoint.setY(sin(angle) * m_radius);
			m_circle << circlePoint;
		}
	}
}

void WorldItemPoint::updateBoundingRect()
{
	m_boundingRect.setCoords(-SIZE_POINT, -SIZE_POINT, SIZE_POINT, SIZE_POINT);
	QRectF circleBoundingRect;
	circleBoundingRect.setCoords(-m_radius, -m_radius, m_radius, m_radius);
	m_boundingRect = m_boundingRect.united(circleBoundingRect);
}

QPainterPath WorldItemPoint::shape() const
{
	QPainterPath path;

	path.addRect(m_boundingRect);
	return qt_graphicsItem_shapeFromPath(path, m_pen);
}

QRectF WorldItemPoint::boundingRect() const
{
	return m_boundingRect;
}

void WorldItemPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
	// Here comes the magic:
	//painter->setClipRect(option->exposedRect);

	painter->setPen(m_pen);

	// Draw circle
	// Draws artefacts with using opengl painter
	// painter->drawEllipse(-m_radius / 2, -m_radius / 2, m_radius, m_radius);
	painter->drawPolygon(m_circle);

	painter->rotate(m_angle);

	// Draw arrow
	painter->drawLines(m_arrow);

	painter->setPen(Qt::NoPen);
	if (option->state & QStyle::State_Selected)
		painter->setBrush(m_selectedBrush);
	else
		painter->setBrush(m_brush);

	// Draw point
	painter->drawRect(m_rect);
}

QVariant WorldItemPoint::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemPositionHasChanged)
	{
	}
	return QGraphicsItem::itemChange(change, value);
}

WorldItemPath::WorldItemPath(const QPolygonF &polygon, QGraphicsItem *parent)
	: AbstractWorldItem(parent),
	  m_polygon(polygon)
{
	//setFlag(ItemIsSelectable);

	setZValue(WORLD_PATH_LAYER);

	m_pen.setColor(Qt::black);
	m_pen.setWidth(3);
	m_pen.setJoinStyle(Qt::MiterJoin);

	m_selectedPen.setColor(Qt::white);
	m_selectedPen.setWidth(3);
	m_selectedPen.setJoinStyle(Qt::MiterJoin);

	QPointF center = m_polygon.boundingRect().center();
	m_polygon.translate(-center);
	setPos(center);
}

WorldItemPath::~WorldItemPath()
{
}

void WorldItemPath::rotateOn(const QPointF &pivot, const qreal deltaAngle)
{
	prepareGeometryChange();

	QPolygonF rotatedPolygon(m_polygon);
	rotatedPolygon.translate(pos() - pivot);

	QTransform trans;
	trans = trans.rotate(deltaAngle);
	m_polygon = trans.map(rotatedPolygon);

	m_polygon.translate(pivot - pos());
}

void WorldItemPath::scaleOn(const QPointF &pivot, const QPointF &factor)
{
	prepareGeometryChange();

	QPolygonF scaledPolygon(m_polygon);
	scaledPolygon.translate(pos() - pivot);

	QTransform trans;
	trans = trans.scale(factor.x(), factor.y());
	m_polygon = trans.map(scaledPolygon);

	m_polygon.translate(pivot - pos());
}

void WorldItemPath::turnOn(const qreal angle)
{
}

void WorldItemPath::radiusOn(const qreal radius)
{
}

void WorldItemPath::setColor(const QColor &color)
{
	m_pen.setColor(color);
}

QPainterPath WorldItemPath::shape() const
{
	QPainterPath path;

	path.moveTo(m_polygon.first());
	for (int i = 1; i < m_polygon.count(); ++i)
		path.lineTo(m_polygon.at(i));

	return qt_graphicsItem_shapeFromPath(path, m_pen);
}

QRectF WorldItemPath::boundingRect() const
{
	return m_polygon.boundingRect();
}

void WorldItemPath::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
	// Here comes the magic:
	//painter->setClipRect(option->exposedRect);

	if (option->state & QStyle::State_Selected)
		painter->setPen(m_selectedPen);
	else
		painter->setPen(m_pen);

	painter->drawPolyline(m_polygon);
}

QVariant WorldItemPath::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemPositionHasChanged)
	{
	}
	return QGraphicsItem::itemChange(change, value);
}

WorldItemZone::WorldItemZone(const QPolygonF &polygon, QGraphicsItem *parent)
	: AbstractWorldItem(parent),
	  m_polygon(polygon)
{
	//setFlag(ItemIsSelectable);

	setZValue(WORLD_ZONE_LAYER);

	m_pen.setColor(QColor(20, 100, 255));
	m_pen.setWidth(0);

	m_selectedPen.setColor(Qt::white);
	m_selectedPen.setWidth(0);

	m_brush.setColor(QColor(20, 100, 255, TRANSPARENCY));
	m_brush.setStyle(Qt::SolidPattern);

	m_selectedBrush.setColor(QColor(255, 255, 255, 100));
	m_selectedBrush.setStyle(Qt::SolidPattern);

	QPointF center = m_polygon.boundingRect().center();
	m_polygon.translate(-center);
	setPos(center);
}

WorldItemZone::~WorldItemZone()
{
}

void WorldItemZone::rotateOn(const QPointF &pivot, const qreal deltaAngle)
{
	prepareGeometryChange();

	QPolygonF rotatedPolygon(m_polygon);
	rotatedPolygon.translate(pos() - pivot);

	QTransform trans;
	trans = trans.rotate(deltaAngle);
	m_polygon = trans.map(rotatedPolygon);

	m_polygon.translate(pivot - pos());
}

void WorldItemZone::scaleOn(const QPointF &pivot, const QPointF &factor)
{
	prepareGeometryChange();

	QPolygonF scaledPolygon(m_polygon);

	scaledPolygon.translate(pos() - pivot);

	QTransform trans;
	trans = trans.scale(factor.x(), factor.y());
	m_polygon = trans.map(scaledPolygon);

	m_polygon.translate(pivot - pos());
}

void WorldItemZone::turnOn(const qreal angle)
{
}

void WorldItemZone::radiusOn(const qreal radius)
{
}

void WorldItemZone::setColor(const QColor &color)
{
	m_pen.setColor(color);

	QColor brushColor(color);
	brushColor.setAlpha(TRANSPARENCY);

	m_brush.setColor(brushColor);
}

QRectF WorldItemZone::boundingRect() const
{
	return m_polygon.boundingRect();
}

QPainterPath WorldItemZone::shape() const
{
	QPainterPath path;
	path.addPolygon(m_polygon);
	return qt_graphicsItem_shapeFromPath(path, m_pen);
}

void WorldItemZone::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
	if (option->state & QStyle::State_Selected)
	{
		painter->setPen(m_selectedPen);
		painter->setBrush(m_selectedBrush);
	}
	else
	{
		painter->setPen(m_pen);
		painter->setBrush(m_brush);
	}

	painter->drawPolygon(m_polygon);
}

QVariant WorldItemZone::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemPositionHasChanged)
	{
	}
	return QGraphicsItem::itemChange(change, value);
}

} /* namespace WorldEditor */
