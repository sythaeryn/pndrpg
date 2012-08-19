/**
 * \file dll_directory.cpp
 * \brief CDllDirectory
 * \date 2012-08-18 09:01GMT
 * \author Jan Boon (Kaetemi)
 * CDllDirectory
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
#include "dll_directory.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

CDllDirectory::CDllDirectory()
{

}

CDllDirectory::~CDllDirectory()
{
	// TODO: Delete m_ChunkCache and m_Entries when !ChunksOwnsPointers
}

std::string CDllDirectory::getClassName()
{
	return "DllDirectory";
}

void CDllDirectory::toString(std::ostream &ostream, const std::string &pad)
{
	if (ChunksOwnsPointers)
	{
		CStorageContainer::toString(ostream, pad);
	}
	else
	{
		ostream << "(" << getClassName() << ") [" << Chunks.size() << "] PARSED { ";
		std::string padpad = pad + "\t";
		sint i = 0;
		for (TStorageObjectContainer::const_iterator it = m_ChunkCache.begin(), end = m_ChunkCache.end(); it != end; ++it)
		{
			uint16 id = it->first;
			switch (id)
			{
			case 0x2038:
				{
					uint subi = 0;
					for (std::vector<CDllEntry *>::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
					{
						ostream << "\n" << pad << "Entries[" << subi << "]: ";
						(*subit)->toString(ostream, padpad);
						++subi;
					}
				}
				break;
			default:
				std::stringstream ss;
				ss << std::hex << std::setfill('0');
				ss << std::setw(4) << it->first;
				ostream << "\n" << pad << "0x" << ss.str() << ": ";
				it->second->toString(ostream, padpad);
				++i;
				break;
			}
		}
		ostream << "} ";
	}
}

void CDllDirectory::parse(uint16 version, TParseLevel level)
{
	if (level & PARSE_INTERNAL)
	{
		// Ensure not yet parsed
		nlassert(m_ChunkCache.empty());
		nlassert(m_Entries.empty());

		// Parse entries first
		CStorageContainer::parse(version, level);

		// Initialize
		m_ParseVersion = version;
		uint16 lastCached = 0xFFFF;
		bool parsedDllEntry = false;

		// Parse chunks
		for (TStorageObjectContainer::iterator it = Chunks.begin(), end = Chunks.end(); it != end; ++it)
		{
			uint16 id = it->first;
			switch (id)
			{
			case 0x2038: // DllEntry
				if (parsedDllEntry && (lastCached != id))
					throw EStorageParse(); // There were chunks inbetween
				if (!parsedDllEntry)
				{
					m_ChunkCache.push_back(TStorageObjectWithId(id, NULL)); // Dummy entry to know the location
					lastCached = id;
					parsedDllEntry = true;
				}
				m_Entries.push_back(static_cast<CDllEntry *>(it->second));
				break;
			default:
				m_ChunkCache.push_back(*it); // Dummy entry to know the location
				lastCached = id;
				break;
			}
		}

		// Now ownership of the pointers lies in m_ChunkCache and m_Entries
		ChunksOwnsPointers = false;
	}
}

void CDllDirectory::build(uint16 version)
{
	// TODO: Set up the Chunks list, when (CDllEntry::ID, NULL) is found write out all of the entries.
	// Build the entries last
	CStorageContainer::build(version);

	// NOTE: Ownership remains with m_ChunkCache and m_Entries
}

const CDllEntry *CDllDirectory::get(std::vector<CDllEntry *>::size_type idx) const
{
	return m_Entries[idx];
}

IStorageObject *CDllDirectory::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x2038: // DllEntry
			return new CDllEntry();
		}
	}
	else
	{
		switch (id)
		{
		case 0x21C0: // DllDirectoryHeader
			return new CStorageValue<uint32>();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

CDllEntry::CDllEntry() : m_DllDescription(NULL), m_DllFilename(NULL)
{

}

CDllEntry::~CDllEntry()
{

}

std::string CDllEntry::getClassName()
{
	return "DllEntry";
}

void CDllEntry::toString(std::ostream &ostream, const std::string &pad)
{
	if (m_DllDescription && m_DllFilename)
	{
		ostream << "(" << getClassName() << ") [" << Chunks.size() << "] PARSED { ";
		std::string padpad = pad + "\t";
		ostream << "\n" << pad << "DllDescription: " << m_DllDescription->Value.toUtf8();
		ostream << "\n" << pad << "DllFilename: " << m_DllFilename->Value.toUtf8();
		ostream << "} ";
	}
	else
	{
		CStorageContainer::toString(ostream, pad);
	}
}

void CDllEntry::parse(uint16 version, TParseLevel level)
{
	// CStorageContainer::parse(version, level);
	nlassert(Chunks.size() == 2);
	TStorageObjectContainer::iterator it = Chunks.begin();
	nlassert(it->first == 0x2039); // DllDescription
	m_DllDescription = static_cast<CStorageValue<ucstring> *>(it->second);
	++it;
	nlassert(it->first == 0x2037); // DllFilename
	m_DllFilename = static_cast<CStorageValue<ucstring> *>(it->second);
	// ++it;
}

void CDllEntry::build(uint16 version)
{
	// Nothing to do here!
	// CStorageContainer::build(version);
}

IStorageObject *CDllEntry::createChunkById(uint16 id, bool container)
{
	if (!container)
	{
		switch (id)
		{
		case 0x2039: // DllDescription
		case 0x2037: // DllFilename
			return new CStorageValue<ucstring>();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
