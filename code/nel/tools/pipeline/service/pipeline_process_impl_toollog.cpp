/**
 * \file pipeline_process_impl.cpp
 * \brief CPipelineProcessImpl
 * \date 2012-08-03 18:22GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineProcessImpl
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

#include <nel/misc/types_nl.h>
#include "pipeline_process_impl.h"

// STL includes
#include <sstream>

// NeL includes
#include <nel/misc/time_nl.h>
#include <nel/misc/app_context.h>
#include <nel/misc/debug.h>

// Project includes
#include "pipeline_service.h"
#include "pipeline_project.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

void CPipelineProcessImpl::parseToolLog(const std::string &dependLogFile, const std::string &errorLogFile, bool writeOutputMeta)
{
	m_SubTaskErrorMessage = "Log parsing not implemented, goodbye";
	m_SubTaskResult = FINISH_ERROR;
}

} /* namespace PIPELINE */

/* end of file */
