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

#ifndef RY_CAMERAANIMATIONPLAYER_H
#define RY_CAMERAANIMATIONPLAYER_H


#include <string>
#include "nel\misc\bit_mem_stream.h"
#include "camera_animation_manager/camera_animation_step_player_factory.h"


/************************************************************************/
/* Class that manages the camera animations. (singleton).
 * It's responsible of playing camera animations
 * 
 * \author Fabien Henon
 * \date 2012
 */
/************************************************************************/
class CCameraAnimationPlayer
{
public:
	/// Gets the current instance of the manager
	static CCameraAnimationPlayer* getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CCameraAnimationPlayer();
		return _Instance;
	}
	/// Releases the instance
	static void release()
	{
		if (_Instance != NULL)
		{
			delete _Instance;
			_Instance = NULL;
		}
	}

	/// Starts playing an animation
	void start();

	/// Stops an animation
	void stop();

	/// Loads and play the specified step
	void playStep(const std::string& stepName, NLMISC::CBitMemStream& impulse);

	/// Checks if an animation is being played
	bool isPlaying();

private:
	/// Constructor
	CCameraAnimationPlayer();
	/// Destructor
	~CCameraAnimationPlayer();

	/// Instance of the manager
	static CCameraAnimationPlayer* _Instance;


	bool _IsPlaying;

	std::vector<ICameraAnimationStepPlayer*> _Steps;
};


#endif /* RY_CAMERAANIMATIONPLAYER_H */
