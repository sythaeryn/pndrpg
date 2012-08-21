/**
 * \file app_data.cpp
 * \brief CAppData
 * \date 2012-08-21 11:47GMT
 * \author Jan Boon (Kaetemi)
 * CAppData
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
#include "app_data.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

// Elevate warnings to errors in this file for stricter reading
#undef nlwarning
#define nlwarning nlerror

// Elevate debug to error in this file for debugging
// #undef nldebug
// #define nldebug nlerror

// Chunk identifiers
#define NLMAXFILE_APP_DATA_HEADER_CHUNK_ID 0x0100
#define NLMAXFILE_APP_DATA_ENTRY_CHUNK_ID 0x0110
#define NLMAXFILE_APP_DATA_ENTRY_KEY_CHUNK_ID 0x0120
#define NLMAXFILE_APP_DATA_ENTRY_VALUE_CHUNK_ID 0x0130

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CAppData::TKey::TKey(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId) : ClassId(classId), SuperClassId(superClassId), SubId(subId)
{

}

bool CAppData::TKey::operator<(const CAppData::TKey &right) const
{
	if (ClassId < right.ClassId)
		return true;
	if (ClassId > right.ClassId)
		return false;
	if (SuperClassId < right.SuperClassId)
		return true;
	if (SuperClassId > right.SuperClassId)
		return false;
	if (SubId < right.SubId)
		return true;
	if (SubId > right.SubId)
		return false;
	return false;
}

bool CAppData::TKey::operator>(const CAppData::TKey &right) const
{
	if (ClassId > right.ClassId)
		return true;
	if (ClassId < right.ClassId)
		return false;
	if (SuperClassId > right.SuperClassId)
		return true;
	if (SuperClassId < right.SuperClassId)
		return false;
	if (SubId > right.SubId)
		return true;
	if (SubId < right.SubId)
		return false;
	return false;
}

bool CAppData::TKey::operator==(const CAppData::TKey &right) const
{
	return ClassId == right.ClassId && SuperClassId == right.SuperClassId && SubId == right.SubId;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CAppData::CAppData()
{

}

CAppData::~CAppData()
{

}

std::string CAppData::getClassName()
{
	return "AppData";
}

void CAppData::toString(std::ostream &ostream, const std::string &pad)
{
	if (ChunksOwnsPointers)
	{
		CStorageContainer::toString(ostream, pad);
	}
	else
	{
		ostream << "(" << getClassName() << ") [" << m_Entries.size() << "] PARSED { ";
		std::string padpad = pad + "\t";
		uint subi = 0;
		for (TMap::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
		{
			ostream << "\n" << pad << "Entries[" << subi << "]: ";
			subit->second->toString(ostream, padpad);
			++subi;
		}
		ostream << "} ";
	}
}

void CAppData::parse(uint16 version, TParseLevel level)
{
	if (level & PARSE_BUILTIN)
	{
		// First parse all the child nodes
		CStorageContainer::parse(version, level);

		// Verify
		if (Chunks.size() < 2) { nlwarning("Bad container size %i", Chunks.size()); disown(); return; }

		// Header
		TStorageObjectContainer::iterator it = Chunks.begin();
		if (it->first != NLMAXFILE_APP_DATA_HEADER_CHUNK_ID) { nlwarning("Bad id %x, expected %x", (uint32)it->first, NLMAXFILE_APP_DATA_HEADER_CHUNK_ID); disown();  return; }
		uint32 headerSize = static_cast<CStorageValue<uint32> *>(it->second)->Value;
		++it;

		// Entries
		for (TStorageObjectContainer::iterator end = Chunks.end(); it != end; ++it)
		{
			if (it->first != NLMAXFILE_APP_DATA_ENTRY_CHUNK_ID) { nlwarning("Bad id %x, expected %x", (uint32)it->first, NLMAXFILE_APP_DATA_ENTRY_CHUNK_ID); disown(); return; }
			CAppDataEntry *entry = static_cast<CAppDataEntry *>(it->second);
			TKey key(entry->key()->ClassId, entry->key()->SuperClassId, entry->key()->SubId);
			if (m_Entries.find(key) != m_Entries.end()) { nlwarning("Duplicate entry"); disown(); return; }
			m_Entries[key] = entry;
		}

		// Verify or fail
		if (m_Entries.size() != headerSize) { nlwarning("Entry count %i does not match header %i", m_Entries.size(), headerSize); disown(); return; }

		// Take local ownership
		ChunksOwnsPointers = false;
	}
}

void CAppData::clean()
{
	if (ChunksOwnsPointers) { nldebug("Not parsed"); return; } // Must have local ownership
	if (Chunks.size() == 0) { nlwarning("Bad container size"); return; } // Already cleaned
	if (Chunks.begin()->first != NLMAXFILE_APP_DATA_HEADER_CHUNK_ID) { nlerror("Bad id %x, expected %x", (uint32)Chunks.begin()->first, NLMAXFILE_APP_DATA_HEADER_CHUNK_ID); return; } // Cannot happen, because we won't have local ownership if parsing failed
	delete Chunks.begin()->second; // Delete the header chunk, since we own it
	Chunks.clear(); // Clear the remaining chunks
}

void CAppData::build(uint16 version)
{
	// Must be clean first
	if (!ChunksOwnsPointers && Chunks.size() != 0) { nlerror("Not cleaned"); return; }
	if (Chunks.size() != 0) { nldebug("Not parsed"); return; }

	// Set up the header in the chunks container
	CStorageValue<uint32> *headerSize = new CStorageValue<uint32>(); // Owned locally, not by Chunks
	headerSize->Value = m_Entries.size();
	Chunks.push_back(TStorageObjectWithId(NLMAXFILE_APP_DATA_HEADER_CHUNK_ID, headerSize));

	// Set up the entries
	for (TMap::iterator it = m_Entries.begin(), end = m_Entries.end(); it != end; ++it)
		Chunks.push_back(TStorageObjectWithId(NLMAXFILE_APP_DATA_ENTRY_CHUNK_ID, it->second));

	// Build all the child chunks
	CStorageContainer::build(version);
}

void CAppData::disown()
{
	if (ChunksOwnsPointers) { nldebug("Not parsed"); }
	if (!ChunksOwnsPointers && (Chunks.size() != (m_Entries.size() + 1))) { nlerror("Not built"); return; } // If chunks is not the owner, built chunks must match the parsed data
	// NOTE: Chunks must be valid at this point!
	// Disown all the child chunks
	CStorageContainer::disown();
	// Disown locally
	m_Entries.clear();
	// Give ownership back
	ChunksOwnsPointers = true;
}

const uint8 *CAppData::read(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint32 &size) const
{
	if (ChunksOwnsPointers) { nlwarning("Not parsed"); return NULL; }
	TKey key(classId, superClassId, subId);
	TMap::const_iterator it = m_Entries.find(key);
	if (it == m_Entries.end()) return NULL;
	size = it->second->value()->Value.size();
	return &it->second->value()->Value[0];
}

uint8 *CAppData::lock(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint32 capacity)
{
	if (ChunksOwnsPointers) { nlwarning("Not parsed"); return NULL; }
	TKey key(classId, superClassId, subId);
}

void CAppData::unlock(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint32 size)
{
	if (ChunksOwnsPointers) { nlwarning("Not parsed"); return; }
	TKey key(classId, superClassId, subId);
}

void CAppData::fill(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint8 *buffer, uint32 size)
{
	if (ChunksOwnsPointers) { nlwarning("Not parsed"); return; }
	TKey key(classId, superClassId, subId);
}

void CAppData::erase(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId)
{
	if (ChunksOwnsPointers) { nlwarning("Not parsed"); return; }
	TKey key(classId, superClassId, subId);
}

IStorageObject *CAppData::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case NLMAXFILE_APP_DATA_HEADER_CHUNK_ID:
		nlassert(!container);
		return new CStorageValue<uint32>();
		break;
	case NLMAXFILE_APP_DATA_ENTRY_CHUNK_ID:
		nlassert(container);
		return new CAppDataEntry();
		break;
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CAppDataEntryKey::CAppDataEntryKey()
{

}

CAppDataEntryKey::~CAppDataEntryKey()
{

}

std::string CAppDataEntryKey::getClassName()
{
	return "AppDataEntryKey";
}

void CAppDataEntryKey::serial(NLMISC::IStream &stream)
{
	stream.serial(ClassId);
	stream.serial(SuperClassId);
	stream.serial(SubId);
	stream.serial(Size);
}

void CAppDataEntryKey::toString(std::ostream &ostream, const std::string &pad)
{
	ostream << "(" << getClassName() << ") { ";
	ostream << "\n" << pad << "ClassId: " << NLMISC::toString(ClassId);
	ostream << "\n" << pad << "SuperClassId: " << SuperClassId;
	ostream << "\n" << pad << "SubId: " << SubId;
	ostream << "\n" << pad << "Size: " << Size;
	ostream << " } ";
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CAppDataEntry::CAppDataEntry() : m_Key(NULL), m_Value(NULL)
{

}

CAppDataEntry::~CAppDataEntry()
{

}

std::string CAppDataEntry::getClassName()
{
	return "AppDataEntry";
}

void CAppDataEntry::toString(std::ostream &ostream, const std::string &pad)
{
	if (m_Key && m_Value)
	{
		ostream << "(" << getClassName() << ") [" << Chunks.size() << "] PARSED { ";
		std::string padpad = pad + "\t";
		ostream << "\n" << pad << "Key: ";
		m_Key->toString(ostream, padpad);
		ostream << "\n" << pad << "Value: ";
		m_Value->toString(ostream, padpad);
		ostream << "} ";
	}
	else
	{
		CStorageContainer::toString(ostream, pad);
	}
}

void CAppDataEntry::parse(uint16 version, TParseLevel level)
{
	// CStorageContainer::parse(version, level);
	// if (!ChunksOwnsPointers) { nlwarning("Already parsed"); return; }
	if (Chunks.size() != 2) { nlwarning("Bad container size"); disown(); return; }

	TStorageObjectContainer::iterator it = Chunks.begin();
	if (it->first != NLMAXFILE_APP_DATA_ENTRY_KEY_CHUNK_ID) { nlwarning("Bad id %x, expected %x", (uint32)it->first, NLMAXFILE_APP_DATA_ENTRY_KEY_CHUNK_ID); disown();  return; }
	m_Key = static_cast<CAppDataEntryKey *>(it->second);

	++it;
	if (it->first != NLMAXFILE_APP_DATA_ENTRY_VALUE_CHUNK_ID) { nlwarning("Bad id %x, expected %x", (uint32)it->first, NLMAXFILE_APP_DATA_ENTRY_VALUE_CHUNK_ID); disown(); return; }
	m_Value = static_cast<CStorageRaw *>(it->second);

	// ChunksOwnsPointers = false;
	nlassert(ChunksOwnsPointers); // Never set false here
}

void CAppDataEntry::clean()
{
	nlassert(false);
	// CStorageContainer::clean();
	// if (ChunksOwnsPointers) { nlwarning("Not parsed"); return; }
	// Nothing to do here!
}

void CAppDataEntry::build(uint16 version)
{
	nlassert(false);
	// if (ChunksOwnsPointers) { nlwarning("Not parsed"); return; }
	// Nothing to do here!
	// CStorageContainer::build(version);
}

void CAppDataEntry::disown()
{
	// CStorageContainer::disown();
	if (Chunks.size() != 2) { nlerror("Not built"); return; } // Built chunks must match the parsed data
	m_Key = NULL;
	m_Value = NULL;
}

void CAppDataEntry::init()
{
	nlassert(Chunks.size() == 0);
	m_Key = new CAppDataEntryKey();
	Chunks.push_back(TStorageObjectWithId(NLMAXFILE_APP_DATA_ENTRY_KEY_CHUNK_ID, m_Key));
	m_Value = new CStorageRaw();
	Chunks.push_back(TStorageObjectWithId(NLMAXFILE_APP_DATA_ENTRY_VALUE_CHUNK_ID, m_Value));
}

CAppDataEntryKey *CAppDataEntry::key()
{
	return m_Key;
}

CStorageRaw *CAppDataEntry::value()
{
	return m_Value;
}

IStorageObject *CAppDataEntry::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case NLMAXFILE_APP_DATA_ENTRY_KEY_CHUNK_ID:
		nlassert(!container);
		return new CAppDataEntryKey();
	case NLMAXFILE_APP_DATA_ENTRY_VALUE_CHUNK_ID:
		nlassert(!container);
		return new CStorageRaw();
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
