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



#ifndef RY_STUN_EFFECT_H
#define RY_STUN_EFFECT_H

#include "phrase_manager/s_effect.h"
#include "entity_manager/entity_base.h"


/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CStunEffect : public CSTimedEffect
{
public:
	NLMISC_DECLARE_CLASS(CStunEffect)
	///\ctor
	CStunEffect()
	{}

	CStunEffect( const TDataSetRow & creatorRowId, const TDataSetRow & targetRowId, EFFECT_FAMILIES::TEffectFamily family, sint32 effectValue, uint32 endDate)
		:CSTimedEffect(creatorRowId, targetRowId, family, false, effectValue,0,endDate)
	{}

	/**
	 * apply the effects of the... effect
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);

	/// callback called when the effect is actually removed
	virtual void removed();
};


#endif // RY_STUN_EFFECT_H

/* End of stun_effect.h */
