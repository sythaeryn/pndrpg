/**
 * \file database_status.h
 * \brief CDatabaseStatus
 * \date 2012-02-18 18:11GMT
 * \author Jan Boon (Kaetemi)
 * CDatabaseStatus
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with RYZOM CORE PIPELINE; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_DATABASE_STATUS_H
#define PIPELINE_DATABASE_STATUS_H
#include <nel/misc/types_nl.h>

// STL includes
#include <vector>

// NeL includes
#include <nel/misc/mutex.h>
#include <nel/misc/stream.h>

// Project includes
#include "callback.h"

namespace PIPELINE {

#define PIPELINE_DATABASE_STATUS_SUBDIR "database.status/"
#define PIPELINE_DATABASE_ERRORS_SUBDIR "database.errors/"

struct CFileError
{
public:
	uint32 Time; // The time when this error occured.
	std::string Project;
	std::string Process;
	std::string Message;

	void serial(NLMISC::IStream &stream);
};

/// Errors set by a process when the file causes a build failure.
typedef std::vector<CFileError> CFileErrors;

struct CFileStatus
{
public:
	uint32 FirstSeen;
	uint32 LastChanged; // The modification date value read when the CRC32 was calculated.
	uint32 LastUpdate; // The start time when the CRC32 was calculated.
	uint32 CRC32;

	void serial(NLMISC::IStream &stream);
};

typedef CCallback<void, const std::string &/*filePath*/, const CFileStatus &/*fileStatus*/> TFileStatusCallback;

/**
 * \brief CDatabaseStatus
 * \date 2012-02-18 18:11GMT
 * \author Jan Boon (Kaetemi)
 * CDatabaseStatus
 */
class CDatabaseStatus
{
protected:
	NLMISC::CMutex m_StatusMutex;
	NLMISC::CMutex m_ErrorMutex;
	
public:
	CDatabaseStatus();
	virtual ~CDatabaseStatus();
	
	/// Tries to read the last file status. Return false if the status is invalid. Call updateFileStatus if the result is false to update asynchronously.
	bool getFileStatus(CFileStatus &fileStatus, const std::string &filePath) const;
	/// Updates the file status asynchronously. The new file status is broadcast to clients and slaves afterwards.
	void updateFileStatus(const TFileStatusCallback &callback, const std::string &filePath);
	/// Forces an update of the complete database status.
	void updateDatabaseStatus(const CCallback<void> &callback);
	
	void getFileErrors(CFileErrors &fileErrors, const std::string &filePath, uint32 newerThan = 0) const;
	void addFileError(const std::string &filePath, const CFileError &fileError);
	
}; /* class CDatabaseStatus */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_DATABASE_STATUS_H */

/* end of file */
