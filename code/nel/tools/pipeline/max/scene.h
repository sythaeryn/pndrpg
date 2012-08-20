/**
 * \file scene.h
 * \brief CScene
 * \date 2012-08-18 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CScene
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

#ifndef PIPELINE_SCENE_H
#define PIPELINE_SCENE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/class_id.h>

// Project includes
#include "storage_object.h"
#include "storage_value.h"
#include "scene_class.h"

namespace PIPELINE {
namespace MAX {

// Registry containing available scene classes
class CSceneClassRegistry;

// Storage
class CDllDirectory;
class CClassDirectory3;
class CSceneClassContainer;

/**
 * \brief CScene
 * \date 2012-08-19 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CScene
 */
class CScene : public CStorageContainer
{
public:
	CScene(const CSceneClassRegistry *sceneClassRegistry, CDllDirectory *dllDirectory, CClassDirectory3 *classDirectory3);
	virtual ~CScene();

	// inherited
	virtual std::string getClassName();
	virtual void toString(std::ostream &ostream, const std::string &pad = "");
	virtual void parse(uint16 version, TParseLevel level);
	virtual void clean();
	virtual void build(uint16 version);
	virtual void disown();

	// public
	uint16 version();
	CSceneClassContainer *container();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	const CSceneClassRegistry *m_SceneClassRegistry;
	CDllDirectory *m_DllDirectory;
	CClassDirectory3 *m_ClassDirectory3;

}; /* class CScene */

/**
 * \brief CSceneClassContainer
 * \date 2012-08-19 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClassContainer
 */
class CSceneClassContainer : public CStorageContainer
{
public:
	CSceneClassContainer(const CSceneClassRegistry *sceneClassRegistry, CDllDirectory *dllDirectory, CClassDirectory3 *classDirectory3);
	virtual ~CSceneClassContainer();

	// inherited
	virtual std::string getClassName();
	virtual void toString(std::ostream &ostream, const std::string &pad = "");
	virtual void parse(uint16 version, TParseLevel level);
	virtual void clean();
	virtual void build(uint16 version);
	virtual void disown();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	const CSceneClassRegistry *m_SceneClassRegistry;
	CDllDirectory *m_DllDirectory;
	CClassDirectory3 *m_ClassDirectory3;

}; /* class CSceneClassContainer */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_SCENE_H */

/* end of file */
