// Object Viewer Qt - Scene Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2012  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef SCENE_EDITOR_CONSTANTS_H
#define SCENE_EDITOR_CONSTANTS_H

namespace SceneEditor
{
namespace Constants
{
const char * const SCENE_EDITOR_PLUGIN = "SceneEditor";

const int USER_TYPE = 65536;
const int NODE_IS_MODIFIED = USER_TYPE + 1;
const int NODE_FILE_IS_CREATED = USER_TYPE + 2;
const int NODE_IS_VISIBLE = USER_TYPE + 3;
const int NODE_IS_ENABLD = USER_TYPE + 4;
const int NODE_FILE_NAME = USER_TYPE + 5;
const int NODE_NON_REMOVABLE = USER_TYPE + 6;

//settings
const char * const SCENE_EDITOR_SECTION = "SceneEditor";

//resources
const char * const ICON_LANDSCAPE_ITEM = ":/icons/ic_nel_landscape_item.png";
const char * const ICON_LIGHT_ITEM = ":/icons/ic_nel_light_item.png";
const char * const ICON_IG_ITEM = ":/icons/ic_nel_ig_item.png";
const char * const ICON_PARTICLES_ITEM = ":/icons/ic_nel_particles.png";
const char * const ICON_SHAPE_ITEM = ":/icons/ic_nel_shape_item.png";
const char * const ICON_SKEL_ITEM = ":/icons/ic_nel_skel_item.png";
const char * const ICON_WATER_ITEM = ":/icons/ic_nel_water_shape.png";
const char * const ICON_WIND = ":/icons/ic_nel_wind.png";

} // namespace Constants
} // namespace SceneEditor

#endif // SCENE_EDITOR_CONSTANTS_H
