/**
 * \file storage_array.h
 * \brief CStorageArray
 * \date 2012-08-21 11:33GMT
 * \author Jan Boon (Kaetemi)
 * CStorageArray
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

#ifndef PIPELINE_STORAGE_ARRAY_H
#define PIPELINE_STORAGE_ARRAY_H
#include <nel/misc/types_nl.h>

// STL includes
#include <vector>

// NeL includes
#include <nel/misc/ucstring.h>
#include <nel/misc/string_common.h>

// Project includes
#include "storage_object.h"

namespace PIPELINE {
namespace MAX {

/**
 * \brief CStorageArray
 * \date 2012-08-21 11:33GMT
 * \author Jan Boon (Kaetemi)
 * WARNING: sizeof(TType) should match the serialized size,
 * otherwise you must specialize the getSize and setSize functions!
 */
template <typename T>
class CStorageArray : public IStorageObject
{
public:
	// public data
	typedef T TType;
	typedef std::vector<TType> TTypeArray;
	TTypeArray Value;

	// inherited
	virtual std::string className() const;
	virtual void serial(NLMISC::IStream &stream);
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;

public: // should be protected but that doesn't compile, nice c++!
	// Sets size when reading
	virtual void setSize(sint32 size);
	// Gets the size when writing, return false if unknown
	virtual bool getSize(sint32 &size) const;
}; /* class CStorageArray */

template <typename T>
std::string CStorageArray<T>::className() const
{
	return "StorageArray";
}

template <typename T>
void CStorageArray<T>::serial(NLMISC::IStream &stream)
{
	for (typename TTypeArray::const_iterator it = Value.begin(), end = Value.end(); it != end; ++it)
	{
		stream.serial(*it);
	}
}

template <typename T>
void CStorageArray<T>::toString(std::ostream &ostream, const std::string &pad) const
{
	ostream << "(" << className() << ") { "; // << s << " } ";
	uint i = 0;
	for (typename TTypeArray::const_iterator it = Value.begin(), end = Value.end(); it != end; ++it)
	{
		std::string s = NLMISC::toString(*it);
		ostream << "\n" << pad << i << ": " << s;
		++i;
	}
	ostream << " } ";
}

template <typename T>
void CStorageArray<T>::setSize(sint32 size)
{
	if ((sizeof(TType) % size) != 0)
		nlerror("Size does not match value type");
	Value.resize(size / sizeof(TType));
}

template <typename T>
bool CStorageArray<T>::getSize(sint32 &size) const
{
	size = Value.size() * sizeof(TType);
	return true;
}

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_STORAGE_ARRAY_H */

/* end of file */
