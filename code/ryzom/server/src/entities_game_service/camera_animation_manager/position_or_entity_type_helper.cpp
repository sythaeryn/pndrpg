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
// 



#include "camera_animation_manager/position_or_entity_type_helper.h"
#include "game_share/mirror.h"
#include "game_share/mirrored_data_set.h"
#include "game_share/base_types.h"
#include "mission_manager/ai_alias_translator.h"
#include "mission_manager/mission_parser.h"
#include "egs_mirror.h"

const TPositionOrEntity CPositionOrEntityHelper::Invalid = TPositionOrEntity();

TPositionOrEntity CPositionOrEntityHelper::fromString(const std::string& s)
{
	std::string str = s;
	CMissionParser::removeBlanks(str);

	std::vector<std::string> resS;
	NLMISC::splitString(str, ";", resS);
	// If we don't have 3 components, it's an entity
	if (resS.size() != 3)
	{
		std::vector<TAIAlias> res;
		CAIAliasTranslator::getInstance()->getNPCAliasesFromName(str, res);
		if (res.size() != 1)
		{
			nlerror("TPositionOrentityHelper : no alias for entity name %s", str.c_str());
			return TPositionOrEntity();
		}
		TAIAlias alias = res[0];
		if (alias == CAIAliasTranslator::Invalid)
		{
			nlerror("TPositionOrentityHelper : invalid alias for entity name %s", str.c_str());
			return TPositionOrEntity();
		}
		NLMISC::CEntityId eid = CAIAliasTranslator::getInstance()->getEntityId(alias);
		if (eid == NLMISC::CEntityId::Unknown)
		{
			nlerror("TPositionOrentityHelper : invalid entity id from alias %d", alias);
			return TPositionOrEntity();
		}
		TDataSetIndex compressedId = TheDataset.getDataSetRow(eid).getCompressedIndex();

		return TPositionOrEntity(compressedId);
	}
	else
	{
		// It's a position
		std::string xStr = resS[0];
		std::string yStr = resS[1];
		std::string zStr = resS[2];

		CMissionParser::removeBlanks(xStr);
		CMissionParser::removeBlanks(yStr);
		CMissionParser::removeBlanks(zStr);

		float x = 0.f;
		float y = 0.f;
		float z = 0.f;

		if (!NLMISC::fromString(xStr, x))
		{
			nlerror("TPositionOrentityHelper : invalid x component from string %s", xStr.c_str());
			return TPositionOrEntity();
		}
		if (!NLMISC::fromString(yStr, y))
		{
			nlerror("TPositionOrentityHelper : invalid y component from string %s", yStr.c_str());
			return TPositionOrEntity();
		}
		if (!NLMISC::fromString(yStr, x))
		{
			nlerror("TPositionOrentityHelper : invalid z component from string %s", zStr.c_str());
			return TPositionOrEntity();
		}

		return TPositionOrEntity(NLMISC::CVector(x, y, z));
	}

	return TPositionOrEntity();
}