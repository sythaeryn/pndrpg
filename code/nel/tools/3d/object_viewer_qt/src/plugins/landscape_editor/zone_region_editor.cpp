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
#include "zone_region_editor.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/o_xml.h>

// Qt includes
#include <QtGui/QMessageBox>

namespace LandscapeEditor
{

LigoData::LigoData()
{
	posX = 0;
	posY = 0;
	zoneName = "";
	rot = 0;
	flip = 0;
	sharingMatNames[0] = "";
	sharingMatNames[1] = "";
	sharingMatNames[2] = "";
	sharingMatNames[3] = "";
	sharingCutEdges[0] = 0;
	sharingCutEdges[1] = 0;
	sharingCutEdges[2] = 0;
	sharingCutEdges[3] = 0;
}

bool LigoData::operator!= (const LigoData& other) const
{
	return (posX != other.posX) ||
		   (posY != other.posY) ||
		   (rot != other.rot) ||
		   (flip != other.flip) ||
		   (zoneName != other.zoneName) ||
		   (sharingMatNames[0] != other.sharingMatNames[0]) ||
		   (sharingMatNames[1] != other.sharingMatNames[1]) ||
		   (sharingMatNames[2] != other.sharingMatNames[2]) ||
		   (sharingMatNames[3] != other.sharingMatNames[3]) ||
		   (sharingCutEdges[0] != other.sharingCutEdges[0]) ||
		   (sharingCutEdges[1] != other.sharingCutEdges[1]) ||
		   (sharingCutEdges[2] != other.sharingCutEdges[2]) ||
		   (sharingCutEdges[3] != other.sharingCutEdges[3]);
}

ZoneRegionObject::ZoneRegionObject()
{
	m_fileName = "";
}

ZoneRegionObject::~ZoneRegionObject()
{
}

bool ZoneRegionObject::load(const std::string &fileName)
{
	bool result = true;
	try
	{
		// Open it
		NLMISC::CIFile fileIn;
		if (fileIn.open(fileName))
		{
			NLMISC::CIXml xml(true);
			xml.init(fileIn);
			m_zoneRegion.serial(xml);
		}
		else
		{
			nlwarning("Can't open file %s for reading", fileName.c_str());
			result = false;
		}
	}
	catch (NLMISC::Exception& e)
	{
		nlwarning("Error reading file %s : %s", fileName.c_str(), e.what ());
		result = false;
	}
	if (result)
		m_fileName = fileName;
	return result;
}

bool ZoneRegionObject::save()
{
	if (m_fileName.empty())
		return false;

	bool result = true;
	// Save the landscape
	try
	{

		// Open file for writing
		NLMISC::COFile fileOut;
		if (fileOut.open(m_fileName, false, false, true))
		{
			// Be careful with the flushing of the COXml object
			{
				NLMISC::COXml xmlOut;
				xmlOut.init(&fileOut);
				m_zoneRegion.serial(xmlOut);
				// Done
				m_modified = false;
			}
			fileOut.close();
		}
		else
		{
			nlwarning("Can't open file %s for writing.", m_fileName.c_str());
			result = false;
		}
	}
	catch (NLMISC::Exception& e)
	{
		nlwarning("Error writing file %s : %s", m_fileName.c_str(), e.what());
		result = false;
	}
	return result;
}

std::string ZoneRegionObject::fileName() const
{
	return m_fileName;
}

void ZoneRegionObject::setFileName(const std::string &fileName)
{
	m_fileName = fileName;
}

void ZoneRegionObject::ligoData(LigoData &data, const sint32 x, const sint32 y)
{
	/*
		nlassert((x >= m_zoneRegion.getMinX()) &&
				 (x <= m_zoneRegion.getMaxX()) &&
				 (y >= m_zoneRegion.getMinY()) &&
				 (y <= m_zoneRegion.getMaxY()));
	*/
	data.posX = m_zoneRegion.getPosX(x, y);
	data.posY = m_zoneRegion.getPosY(x, y);
	data.zoneName = m_zoneRegion.getName(x, y);
	data.rot = m_zoneRegion.getRot(x, y);
	data.flip = m_zoneRegion.getFlip(x, y);
	data.sharingMatNames[0] = m_zoneRegion.getSharingMatNames(x, y, 0);
	data.sharingMatNames[1] = m_zoneRegion.getSharingMatNames(x, y, 1);
	data.sharingMatNames[2] = m_zoneRegion.getSharingMatNames(x, y, 2);
	data.sharingMatNames[3] = m_zoneRegion.getSharingMatNames(x, y, 3);
	data.sharingCutEdges[0] = m_zoneRegion.getSharingCutEdges(x, y, 0);
	data.sharingCutEdges[1] = m_zoneRegion.getSharingCutEdges(x, y, 1);
	data.sharingCutEdges[2] = m_zoneRegion.getSharingCutEdges(x, y, 2);
	data.sharingCutEdges[3] = m_zoneRegion.getSharingCutEdges(x, y, 3);
}

void ZoneRegionObject::setLigoData(const LigoData &data, const sint32 x, const sint32 y)
{
	/*
		nlassert((x >= m_zoneRegion.getMinX()) &&
				 (x <= m_zoneRegion.getMaxX()) &&
				 (y >= m_zoneRegion.getMinY()) &&
				 (y <= m_zoneRegion.getMaxY()));
	*/
	m_zoneRegion.setPosX(x, y, data.posX);
	m_zoneRegion.setPosY(x, y, data.posY);
	m_zoneRegion.setName(x, y, data.zoneName);
	m_zoneRegion.setRot(x, y, data.rot);
	m_zoneRegion.setFlip(x, y, data.flip);
	m_zoneRegion.setSharingMatNames(x, y, 0, data.sharingMatNames[0]);
	m_zoneRegion.setSharingMatNames(x, y, 1, data.sharingMatNames[1]);
	m_zoneRegion.setSharingMatNames(x, y, 2, data.sharingMatNames[2]);
	m_zoneRegion.setSharingMatNames(x, y, 3, data.sharingMatNames[3]);
	m_zoneRegion.setSharingCutEdges(x, y, 0, data.sharingCutEdges[0]);
	m_zoneRegion.setSharingCutEdges(x, y, 1, data.sharingCutEdges[1]);
	m_zoneRegion.setSharingCutEdges(x, y, 2, data.sharingCutEdges[2]);
	m_zoneRegion.setSharingCutEdges(x, y, 3, data.sharingCutEdges[3]);
}

NLLIGO::CZoneRegion &ZoneRegionObject::zoneRegion()
{
	return m_zoneRegion;
}

void ZoneRegionObject::setZoneRegion(const NLLIGO::CZoneRegion &zoneRegion)
{
	m_zoneRegion = zoneRegion;
}

bool ZoneRegionObject::checkPos(const sint32 x, const sint32 y)
{
	return ((x >= m_zoneRegion.getMinX()) &&
			(x <= m_zoneRegion.getMaxX()) &&
			(y >= m_zoneRegion.getMinY()) &&
			(y <= m_zoneRegion.getMaxY()));
}

} /* namespace LandscapeEditor */
