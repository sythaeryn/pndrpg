/**
 * \file pipeline_interface.cpp
 * \brief IPipelineInterface
 * \date 2012-02-25 12:10GMT
 * \author Jan Boon (Kaetemi)
 * IPipelineInterface
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
#include "pipeline_interface.h"

// STL includes

// NeL includes
#include <nel/misc/app_context.h>
#include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

void fksjdlfkjdskfljdsklfjdslkfjdsf() { }

IPipelineInterface *IPipelineInterface::getInstance()
{
	nlassert(NLMISC::INelContext::isContextInitialised());
	return static_cast<IPipelineInterface *>(NLMISC::INelContext::getInstance().getSingletonPointer("IPipelineInterface"));
}

} /* namespace PIPELINE */

/* end of file */
