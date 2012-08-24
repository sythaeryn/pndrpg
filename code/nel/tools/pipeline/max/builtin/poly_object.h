/**
 * \file poly_object.h
 * \brief CPolyObject
 * \date 2012-08-22 08:58GMT
 * \author Jan Boon (Kaetemi)
 * CPolyObject
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

#ifndef PIPELINE_POLY_OBJECT_H
#define PIPELINE_POLY_OBJECT_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "object.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CPolyObject
 * \date 2012-08-22 08:58GMT
 * \author Jan Boon (Kaetemi)
 * CPolyObject
 */
class CPolyObject : public CObject
{
public:
	CPolyObject(CScene *scene);
	virtual ~CPolyObject();

}; /* class CPolyObject */

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_POLY_OBJECT_H */

/* end of file */
