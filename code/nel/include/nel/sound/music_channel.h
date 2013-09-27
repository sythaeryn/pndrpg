/**
 * \file source_music_channel.h
 * \brief CMusicChannel
 * \date 2012-04-11 16:08GMT
 * \author Jan Boon (Kaetemi)
 * CMusicChannel
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE.
 * RYZOM CORE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * RYZOM CORE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef NLSOUND_SOURCE_MUSIC_CHANNEL_H
#define NLSOUND_SOURCE_MUSIC_CHANNEL_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/sound/stream_file_sound.h>

// Project includes

namespace NLSOUND {
	class CStreamFileSource;

/**
 * \brief CMusicChannel
 * \date 2012-04-11 16:08GMT
 * \author Jan Boon (Kaetemi)
 * CMusicChannel
 * Background music channel
 */
class CMusicChannel
{
public:
	CMusicChannel();
	~CMusicChannel();

	/** Play some music (.ogg only)
	 *	NB: if an old music was played, it is first stop with stopMusic()
	 *	\param filepath file path, CPath::lookup is done here
	 *  \param async stream music from hard disk, preload in memory if false
	 *	\param loop must be true to play the music in loop. 
	 */
	bool play(const std::string &filepath, bool async, bool loop); 
	
	/// Stop the music previously loaded and played (the Memory is also freed)
	void stop();

	/// Makes sure any resources are freed, but keeps available for next play call
	void reset();
	
	/// Pause the music previously loaded and played (the Memory is not freed)
	void pause();
	
	/// Resume the music previously paused
	void resume();
	
	/// Return true if a song is finished.
	bool isEnded();
	
	/// Return true if the song is still loading asynchronously and hasn't started playing yet (false if not async), used to delay fading
	bool isLoadingAsync();
	
	/// Return the total length (in second) of the music currently played
	float getLength();
	
	/** Set the music volume (if any music played). (volume value inside [0 , 1]) (default: 1)
	 *	NB: the volume of music is NOT affected by IListener::setGain()
	 */
	void setVolume(float gain);
	
private:
	CStreamFileSound m_Sound;
	CStreamFileSource *m_Source;
	float m_Gain;

}; /* class CMusicChannel */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_SOURCE_MUSIC_CHANNEL_H */

/* end of file */
