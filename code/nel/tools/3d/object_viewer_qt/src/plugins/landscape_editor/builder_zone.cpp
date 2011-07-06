// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// Project includes
#include "builder_zone.h"
#include "list_zones_widget.h"
#include "landscape_actions.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtCore/QDir>
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtGui/QProgressDialog>

namespace LandscapeEditor
{

ZoneBuilder::ZoneBuilder(LandscapeScene *landscapeScene, ListZonesWidget *listZonesWidget, QUndoStack *undoStack)
	: m_currentZoneRegion(-1),
	  m_pixmapDatabase(0),
	  m_listZonesWidget(listZonesWidget),
	  m_landscapeScene(landscapeScene),
	  m_undoStack(undoStack)
{
	nlassert(m_landscapeScene);
	m_pixmapDatabase = new PixmapDatabase();
	m_lastPathName = "";
}

ZoneBuilder::~ZoneBuilder()
{
	delete m_pixmapDatabase;
}

bool ZoneBuilder::init(const QString &pathName, bool makeAZone)
{
	bool bRet = true;
	if (pathName != m_lastPathName)
	{
		m_lastPathName = pathName;
		QString zoneBankPath = pathName;
		zoneBankPath += "/zoneligos/";

		// Init the ZoneBank
		m_zoneBank.reset ();
		if (!initZoneBank (zoneBankPath))
		{
			m_zoneBank.reset ();
			return false;
		}
		// Construct the DataBase from the ZoneBank
		QString zoneBitmapPath = pathName;
		zoneBitmapPath += "/zonebitmaps/";
		m_pixmapDatabase->reset();
		if (!m_pixmapDatabase->loadPixmaps(zoneBitmapPath, m_zoneBank))
		{
			m_zoneBank.reset();
			return false;
		}
	}
	if ((makeAZone) && (bRet))
		createZoneRegion();
	return bRet;
}

void ZoneBuilder::actionLigoTile(const LigoData &data, const ZonePosition &zonePos)
{
	if (m_undoStack == 0)
		return;

	checkBeginMacro();
	nlinfo(QString("%1 %2 %3 (%4 %5)").arg(data.zoneName.c_str()).arg(zonePos.x).arg(zonePos.y).arg(data.posX).arg(data.posY).toStdString().c_str());
	m_zonePositionList.push_back(zonePos);
	m_undoStack->push(new LigoTileCommand(data, zonePos, this, m_landscapeScene));
}

void ZoneBuilder::actionLigoMove(uint index, sint32 deltaX, sint32 deltaY)
{
	if (m_undoStack == 0)
		return;

	checkBeginMacro();
	nlinfo("ligoMove");
	//m_undoStack->push(new LigoMoveCommand(index, deltaX, deltaY, this));
}

void ZoneBuilder::actionLigoResize(uint index, sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY)
{
	if (m_undoStack == 0)
		return;

	checkBeginMacro();
	nlinfo(QString("minX=%1 maxX=%2 minY=%3 maxY=%4").arg(newMinX).arg(newMaxX).arg(newMinY).arg(newMaxY).toStdString().c_str());
	m_undoStack->push(new LigoResizeCommand(index, newMinX, newMaxX, newMinY, newMaxY, this));
}

void ZoneBuilder::addZone(sint32 posX, sint32 posY)
{
	// Read-only mode
	if ((m_listZonesWidget == 0) || (m_undoStack == 0))
		return;

	if (m_landscapeItems.empty())
		return;

	// Check zone name
	std::string zoneName = m_listZonesWidget->currentZoneName().toStdString();
	if (zoneName.empty())
		return;

	BuilderZoneRegion *builderZoneRegion = m_landscapeItems.at(m_currentZoneRegion).builderZoneRegion;
	builderZoneRegion->init(this);

	uint8 rot = uint8(m_listZonesWidget->currentRot());
	uint8 flip = uint8(m_listZonesWidget->currentFlip());

	NLLIGO::CZoneBankElement *zoneBankElement = getZoneBank().getElementByZoneName(zoneName);

	m_titleAction = QString("Add zone %1,%2").arg(posX).arg(posY);
	m_createdAction = false;
	m_zonePositionList.clear();
	if (m_listZonesWidget->isForce())
	{
		builderZoneRegion->addForce(posX, posY, rot, flip, zoneBankElement);
	}
	else
	{
		if (m_listZonesWidget->isNotPropogate())
			builderZoneRegion->addNotPropagate(posX, posY, rot, flip, zoneBankElement);
		else
			builderZoneRegion->add(posX, posY, rot, flip, zoneBankElement);
	}
	checkEndMacro();
}

void ZoneBuilder::addTransition(const sint32 posX, const sint32 posY)
{
	if ((m_listZonesWidget == 0) || (m_undoStack == 0))
		return;
}

void ZoneBuilder::delZone(const sint32 posX, const sint32 posY)
{
	if ((m_listZonesWidget == 0) || (m_undoStack == 0))
		return;

	if (m_landscapeItems.empty())
		return;

	m_titleAction = QString("Del zone %1,%2").arg(posX).arg(posY);
	m_createdAction = false;

	BuilderZoneRegion *builderZoneRegion = m_landscapeItems.at(m_currentZoneRegion).builderZoneRegion;

	builderZoneRegion->init(this);
	builderZoneRegion->del(posX, posY);
	checkEndMacro();
}

int ZoneBuilder::createZoneRegion()
{
	int newId = m_landscapeItems.size();
	LandscapeItem landItem;
	landItem.zoneRegionObject = new ZoneRegionObject();
	landItem.builderZoneRegion = new BuilderZoneRegion(newId);
	landItem.builderZoneRegion->init(this);
	landItem.rectItem = 0;

	newZone();
	m_landscapeItems.push_back(landItem);
	if (m_currentZoneRegion == -1)
		setCurrentZoneRegion(newId);

	return newId;
}

int ZoneBuilder::createZoneRegion(const QString &fileName)
{
	int newId = m_landscapeItems.size();
	LandscapeItem landItem;
	landItem.zoneRegionObject = new ZoneRegionObject();
	landItem.zoneRegionObject->load(fileName.toStdString());

	if (!checkOverlaps(landItem.zoneRegionObject->ligoZoneRegion()))
	{
		delete landItem.zoneRegionObject;
		return -1;
	}
	landItem.builderZoneRegion = new BuilderZoneRegion(newId);
	landItem.builderZoneRegion->init(this);

	newZone();
	m_landscapeItems.push_back(landItem);

	m_landscapeScene->addZoneRegion(landItem.zoneRegionObject->ligoZoneRegion());
	m_landscapeItems.at(newId).rectItem = m_landscapeScene->createLayerBlackout(landItem.zoneRegionObject->ligoZoneRegion());

	if (m_currentZoneRegion == -1)
		setCurrentZoneRegion(newId);

	return newId;
}

void ZoneBuilder::deleteZoneRegion(int id)
{
	if ((0 <= id) && (id < int(m_landscapeItems.size())))
	{
		if (m_landscapeItems.at(id).rectItem != 0)
			delete m_landscapeItems.at(id).rectItem;
		m_landscapeScene->delZoneRegion(m_landscapeItems.at(id).zoneRegionObject->ligoZoneRegion());
		delete m_landscapeItems.at(id).zoneRegionObject;
		delete m_landscapeItems.at(id).builderZoneRegion;
		m_landscapeItems.erase(m_landscapeItems.begin() + id);
		calcMask();
	}
}

void ZoneBuilder::setCurrentZoneRegion(int id)
{
	if ((0 <= id) && (id < int(m_landscapeItems.size())))
	{
		if (currentIdZoneRegion() != -1)
		{
			NLLIGO::CZoneRegion &ligoRegion = m_landscapeItems.at(m_currentZoneRegion).zoneRegionObject->ligoZoneRegion();
			m_landscapeItems.at(m_currentZoneRegion).rectItem = m_landscapeScene->createLayerBlackout(ligoRegion);
		}
		delete m_landscapeItems.at(id).rectItem;
		m_landscapeItems.at(id).rectItem = 0;
		m_currentZoneRegion = id;
	}
}

int ZoneBuilder::currentIdZoneRegion() const
{
	return m_currentZoneRegion;
}

ZoneRegionObject *ZoneBuilder::currentZoneRegion() const
{
	return m_landscapeItems.at(m_currentZoneRegion).zoneRegionObject;
}

int ZoneBuilder::countZoneRegion() const
{
	return m_landscapeItems.size();
}

ZoneRegionObject *ZoneBuilder::zoneRegion(int id) const
{
	return m_landscapeItems.at(id).zoneRegionObject;
}

void ZoneBuilder::ligoData(LigoData &data, const ZonePosition &zonePos)
{
	m_landscapeItems.at(zonePos.region).zoneRegionObject->ligoData(data, zonePos.x, zonePos.y);
}

void ZoneBuilder::setLigoData(LigoData &data, const ZonePosition &zonePos)
{
	m_landscapeItems.at(zonePos.region).zoneRegionObject->setLigoData(data, zonePos.x, zonePos.y);
}

bool ZoneBuilder::initZoneBank (const QString &pathName)
{
	QDir *dir = new QDir(pathName);
	QStringList filters;
	filters << "*.ligozone";

	// Find all ligozone files in dir
	QStringList listFiles = dir->entryList(filters, QDir::Files);

	std::string error;
	Q_FOREACH(QString file, listFiles)
	{
		//nlinfo(file.toStdString().c_str());
		if (!m_zoneBank.addElement((pathName + file).toStdString(), error))
			QMessageBox::critical(0, QObject::tr("Landscape editor"), QString(error.c_str()), QMessageBox::Ok);
	}
	delete dir;
	return true;
}

PixmapDatabase *ZoneBuilder::pixmapDatabase() const
{
	return m_pixmapDatabase;
}

QString ZoneBuilder::dataPath() const
{
	return m_lastPathName;
}

void ZoneBuilder::newZone()
{
	// Select starting point for the moment 0,0
	sint32 x = 0, y = 0;

	// If there are some zone already present increase x until free
	for (size_t i = 0; i < m_landscapeItems.size(); ++i)
	{
		const NLLIGO::CZoneRegion &zoneRegion = m_landscapeItems.at(i).zoneRegionObject->ligoZoneRegion();
		const std::string &zoneName = zoneRegion.getName (x, y);
		if ((zoneName != STRING_OUT_OF_BOUND) && (zoneName != STRING_UNUSED))
		{
			++x;
			i = -1;
		}
	}
	calcMask();
}

bool ZoneBuilder::getZoneMask(sint32 x, sint32 y)
{
	if ((x < m_minX) || (x > m_maxX) ||
			(y < m_minY) || (y > m_maxY))
	{
		return true;
	}
	else
	{
		return m_zoneMask[(x - m_minX) + (y - m_minY) * (1 + m_maxX - m_minX)];
	}
}

void ZoneBuilder::calcMask()
{
	sint32 x, y;

	m_minY = m_minX = 1000000;
	m_maxY = m_maxX = -1000000;

	if (m_landscapeItems.size() == 0)
		return;

	for (size_t i = 0; i < m_landscapeItems.size(); ++i)
	{
		const NLLIGO::CZoneRegion &region = m_landscapeItems.at(i).zoneRegionObject->ligoZoneRegion();

		if (m_minX > region.getMinX())
			m_minX = region.getMinX();
		if (m_minY > region.getMinY())
			m_minY = region.getMinY();
		if (m_maxX < region.getMaxX())
			m_maxX = region.getMaxX();
		if (m_maxY < region.getMaxY())
			m_maxY = region.getMaxY();
	}

	m_zoneMask.resize ((1 + m_maxX - m_minX) * (1 + m_maxY - m_minY));
	sint32 stride = (1 + m_maxX - m_minX);
	for (y = m_minY; y <= m_maxY; ++y)
		for (x = m_minX; x <= m_maxX; ++x)
		{
			m_zoneMask[x - m_minX + (y - m_minY) * stride] = true;

			for (size_t i = 0; i < m_landscapeItems.size(); ++i)
				if (int(i) != m_currentZoneRegion)
				{
					const NLLIGO::CZoneRegion &region = zoneRegion(i)->ligoZoneRegion();

					const std::string &rSZone = region.getName (x, y);
					if ((rSZone != STRING_OUT_OF_BOUND) && (rSZone != STRING_UNUSED))
					{
						m_zoneMask[x - m_minX + (y - m_minY) * stride] = false;
					}
				}
		}
}

bool ZoneBuilder::getZoneAmongRegions(ZonePosition &zonePos, BuilderZoneRegion *builderZoneRegionFrom, sint32 x, sint32 y)
{
	for (size_t i = 0; i < m_landscapeItems.size(); ++i)
	{
		const NLLIGO::CZoneRegion &region = m_landscapeItems.at(i).zoneRegionObject->ligoZoneRegion();
		if ((x < region.getMinX()) || (x > region.getMaxX()) ||
				(y < region.getMinY()) || (y > region.getMaxY()))
			continue;
		if (region.getName(x, y) != STRING_UNUSED)
		{
			builderZoneRegionFrom = m_landscapeItems.at(i).builderZoneRegion;
			zonePos = ZonePosition(x, y, i);
			return true;
		}
	}

	// The zone is not present in other region so it is an empty or oob zone of the current region
	const NLLIGO::CZoneRegion &region = zoneRegion(builderZoneRegionFrom->getRegionId())->ligoZoneRegion();
	if ((x < region.getMinX()) || (x > region.getMaxX()) ||
			(y < region.getMinY()) || (y > region.getMaxY()))
		return false; // Out Of Bound

	zonePos = ZonePosition(x, y, builderZoneRegionFrom->getRegionId());
	return true;
}

void ZoneBuilder::checkBeginMacro()
{
	if (!m_createdAction)
	{
		m_createdAction = true;
		m_undoStack->beginMacro(m_titleAction);
		m_undoScanRegionCommand = new UndoScanRegionCommand(this, m_landscapeScene);
		m_undoStack->push(m_undoScanRegionCommand);
	}
}

void ZoneBuilder::checkEndMacro()
{
	if (m_createdAction)
	{
		RedoScanRegionCommand *redoScanRegionCommand = new RedoScanRegionCommand(this, m_landscapeScene);
		m_undoScanRegionCommand->setScanList(m_zonePositionList);
		redoScanRegionCommand->setScanList(m_zonePositionList);
		m_undoStack->push(redoScanRegionCommand);
		m_undoStack->endMacro();
	}
}

bool ZoneBuilder::checkOverlaps(const NLLIGO::CZoneRegion &newZoneRegion)
{
	for (size_t j = 0; j < m_landscapeItems.size(); ++j)
	{
		const NLLIGO::CZoneRegion &zoneRegion = m_landscapeItems.at(j).zoneRegionObject->ligoZoneRegion();
		for (sint32 y = zoneRegion.getMinY(); y <= zoneRegion.getMaxY(); ++y)
			for (sint32 x = zoneRegion.getMinX(); x <= zoneRegion.getMaxX(); ++x)
			{
				const std::string &refZoneName = zoneRegion.getName(x, y);
				if (refZoneName != STRING_UNUSED)
				{
					const std::string &zoneName = newZoneRegion.getName(x, y);
					if ((zoneName != STRING_UNUSED) && (zoneName != STRING_OUT_OF_BOUND))
						return false;
				}
			}
	}
	return true;
}

} /* namespace LandscapeEditor */
