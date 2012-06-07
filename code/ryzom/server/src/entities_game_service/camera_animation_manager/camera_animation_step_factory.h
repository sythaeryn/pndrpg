// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef RY_CAMERAANIMATIONSTEPFACTORY_H
#define RY_CAMERAANIMATIONSTEPFACTORY_H

#include "nel/ligo/primitive.h"
#include <string>

/************************************************************************/
/* Interface for camera animation steps.
 * It has to be able to parse the step from the primitive.
 * And also to generate a small script to send to the client
 */
/************************************************************************/
class ICameraAnimationStep
{
public:
	/// This function is called when it's time to parse the primitive to load the camera animation step
	virtual bool parseStep(const NLLIGO::IPrimitive* prim, const std::string& filename) = 0;
};

/************************************************************************/
/* Factory class that can instanciate the correct camera animation step handler.
 * 
 * \author Fabien Henon
 * \date 2012
 */
/************************************************************************/
class ICameraAnimationStepFactory
{
public:
	/// Function that will instanciate the correct camera animation step
	static ICameraAnimationStep* parseStep(const NLLIGO::IPrimitive* prim, const std::string& filename, const std::string& name);	
protected:

	/// Functions used to be able to create the camera animation steps
	static void init();
	virtual ICameraAnimationStep * instanciate() = 0;
	static std::vector<std::pair<std::string, ICameraAnimationStepFactory*> >* Entries;
};

// Define used to register the different types of camera animation steps
#define MISSION_REGISTER_ACTION(_class_,_name_) \
class _class_##CameraAnimationStepFactory : public ICameraAnimationStepFactory \
{\
public:\
	_class_##CameraAnimationStepFactory()\
{\
	init();\
	std::string str = std::string(_name_); \
	for (uint i = 0; i < (*Entries).size(); i++ ) \
{\
	if ( (*Entries)[i].first == str || (*Entries)[i].second == this )nlstop;\
}\
	(*Entries).push_back( std::make_pair( str, this ) );\
}\
	ICameraAnimationStep * instanciate()\
{ \
	return new _class_;\
} \
};\
	static _class_##ActionFactory* _class_##ActionFactoryInstance = new _class_##ActionFactory;

#endif /* RY_CAMERAANIMATIONSTEPFACTORY_H */
