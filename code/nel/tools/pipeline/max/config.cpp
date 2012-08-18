/**
 * \file config.cpp
 * \brief CConfig
 * \date 2012-08-18 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CConfig
 */

/*
 * Copyright (C) 2012  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include "config.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/vector.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfig::CConfig()
{

}

CConfig::~CConfig()
{

}

std::string CConfig::getClassName()
{
	return "Config";
}

void CConfig::toString(std::ostream &ostream, const std::string &pad)
{
	CStorageContainer::toString(ostream, pad);
}

IStorageObject *CConfig::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x2180: // CConfigScript
			return new CConfigScript();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

void CConfig::serialized(TStorageObjectContainer::iterator soit, bool container)
{
	CStorageContainer::serialized(soit, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfigScript::CConfigScript()
{

}

CConfigScript::~CConfigScript()
{

}

std::string CConfigScript::getClassName()
{
	return "ConfigScript";
}

void CConfigScript::toString(std::ostream &ostream, const std::string &pad)
{
	CStorageContainer::toString(ostream, pad);
}

IStorageObject *CConfigScript::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x0040: // ConfigScriptEntry
			return new CConfigScriptEntry();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

void CConfigScript::serialized(TStorageObjectContainer::iterator soit, bool container)
{
	CStorageContainer::serialized(soit, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfigScriptEntry::CConfigScriptEntry()
{

}

CConfigScriptEntry::~CConfigScriptEntry()
{

}

std::string CConfigScriptEntry::getClassName()
{
	return "ConfigScriptEntry";
}

void CConfigScriptEntry::toString(std::ostream &ostream, const std::string &pad)
{
	CStorageContainer::toString(ostream, pad);
}

IStorageObject *CConfigScriptEntry::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x0007: // ConfigScriptMetaContainer
			return new CConfigScriptMetaContainer();
		}
	}
	else
	{
		switch (id)
		{
		case 0x0050: // ConfigScriptHeader
			return new CConfigScriptHeader();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

void CConfigScriptEntry::serialized(TStorageObjectContainer::iterator soit, bool container)
{
	CStorageContainer::serialized(soit, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfigScriptHeader::CConfigScriptHeader()
{

}

CConfigScriptHeader::~CConfigScriptHeader()
{

}

std::string CConfigScriptHeader::getClassName()
{
	return "ConfigScriptHeader";
}

void CConfigScriptHeader::serial(NLMISC::IStream &stream)
{
	stream.serial(SuperClassID);
	stream.serial(ClassID);
}

void CConfigScriptHeader::toString(std::ostream &ostream, const std::string &pad)
{
	ostream << "(" << getClassName() << ") { ";
	ostream << "\n" << pad << "SuperClassID: " << SuperClassID;
	ostream << "\n" << pad << "ClassID: " << NLMISC::toString(ClassID);
	ostream << " } ";
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfigScriptMetaContainer::CConfigScriptMetaContainer()
{

}

CConfigScriptMetaContainer::~CConfigScriptMetaContainer()
{

}

std::string CConfigScriptMetaContainer::getClassName()
{
	return "ConfigScriptMetaContainer";
}

void CConfigScriptMetaContainer::toString(std::ostream &ostream, const std::string &pad)
{
	CStorageContainer::toString(ostream, pad);
}

IStorageObject *CConfigScriptMetaContainer::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x0007: // ConfigScriptMetaContainer
			return new CConfigScriptMetaContainer();
		}
	}
	else
	{
		switch (id)
		{
		case 0x0001: // type: boolean (stored in four bytes, yes)
			return new CStorageValue<uint32>();
		case 0x0002: // ???
			nlerror("0x0002 found, please implement");
			break;
		case 0x0003: // type: integer (not sure if signed)
			return new CStorageValue<sint32>();
		case 0x0004: // type: float
			return new CStorageValue<float>();
		case 0x0005: // type: string (same format as the meta string)
		case 0x0006: // ConfigScriptMetaString
			return new CConfigScriptMetaString();
		case 0x0008: // type: color (stored as 3 float vector)
			return new CStorageValue<NLMISC::CVector>;
		case 0x0060: // ConfigScriptMetaContainerHeader (contains the number of entries, amusingly)
			return new CStorageValue<uint32>();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

void CConfigScriptMetaContainer::serialized(TStorageObjectContainer::iterator soit, bool container)
{
	CStorageContainer::serialized(soit, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfigScriptMetaString::CConfigScriptMetaString()
{

}

CConfigScriptMetaString::~CConfigScriptMetaString()
{

}

std::string CConfigScriptMetaString::getClassName()
{
	return "ConfigScriptMetaString";
}

void CConfigScriptMetaString::serial(NLMISC::IStream &stream)
{
	if (stream.isReading())
	{
		uint32 size;
		stream.serial(size);
		Value.resize(size - 1);
	}
	else
	{
		uint32 size = Value.size() + 1;
		stream.serial(size);
	}
	stream.serialBuffer(static_cast<uint8 *>(static_cast<void *>(&Value[0])), Value.size());
	uint8 endByte = 0;
	stream.serial(endByte);
	nlassert(endByte == 0);
}

void CConfigScriptMetaString::toString(std::ostream &ostream, const std::string &pad)
{
	ostream << "(" << getClassName() << ") { " << Value << " } ";
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
