// Object Viewer Qt - Build Gamedata Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef BUILD_GAMEDATA_CONSTANTS_H
#define BUILD_GAMEDATA_CONSTANTS_H

namespace BuildGamedata
{
namespace Constants
{
//settings
const char * const BUILD_GAMEDATA_SECTION = "BuildGamedataPlugin";
const char * const SETTING_PYTHON_EXE_PATH = "PythonExePath";
const char * const SETTING_PYTHON_EXE_PATH_DEFAULT = "c:/Python27/python.exe";

const char * const SETTING_SCRIPT_PATH = "ScriptPath";
const char * const SETTING_SCRIPT_PATH_DEFAULT = "R:/code/nel/tools/build_gamedata";

const char * const SETTING_WORKSPACE_PATH = "WorkspacePath";
const char * const SETTING_WORKSPACE_PATH_DEFAULT = "R:/code/ryzom/tools/build_gamedata/workspace";

const char * const SETTING_DATABASE_PATH = "DatabasePath";
const char * const SETTING_DATABASE_PATH_DEFAULT = "W:/database";

const char * const SETTING_LEVELDESIGN_PATH = "LevelDesignPath";
const char * const SETTING_LEVELDESIGN_PATH_DEFAULT = "L:/leveldesign";

const char * const SETTING_LEVELDESIGN_DFN_PATH = "LevelDesignDfnPath";
const char * const SETTING_LEVELDESIGN_DFN_PATH_DEFAULT = "L:/leveldesign/dfn";

const char * const SETTING_LEVELDESIGN_WORLD_PATH = "LevelDesignWorldPath";
const char * const SETTING_LEVELDESIGN_WORLD_PATH_DEFAULT = "L:/leveldesign/world";

const char * const SETTING_PRIMITIVES_PATH = "PrimitivesPath";
const char * const SETTING_PRIMITIVES_PATH_DEFAULT = "L:/primitives";

const char * const SETTING_GAMEDEV_PATH = "GamedevPath";
const char * const SETTING_GAMEDEV_PATH_DEFAULT = "R:/code/ryzom/client/data/gamedev";

const char * const SETTING_DATA_COMMON_PATH = "DataCommonPath";
const char * const SETTING_DATA_COMMON_PATH_DEFAULT = "R:/code/ryzom/common/data_common";

const char * const SETTING_EXPORT_PATH = "ExportPath";
const char * const SETTING_EXPORT_PATH_DEFAULT = "W:/export";

const char * const SETTING_INSTALL_PATH = "InstallPath";
const char * const SETTING_INSTALL_PATH_DEFAULT = "W:/install";

const char * const SETTING_DATA_SHARD_PATH = "DataShardPath";
const char * const SETTING_DATA_SHARD_PATH_DEFAULT = "R:/code/ryzom/server/data_shard";

const char * const SETTING_CLIENT_DEV_PATH = "ClientDevPath";
const char * const SETTING_CLIENT_DEV_PATH_DEFAULT = "W:/client_dev";

const char * const SETTING_CLIENT_PATCH_PATH = "ClientPatchPath";
const char * const SETTING_CLIENT_PATCH_PATH_DEFAULT = "W:/client_patch";

const char * const SETTING_CLIENT_INSTALL_PATH = "ClientInstallPath";
const char * const SETTING_CLIENT_INSTALL_PATH_DEFAULT = "W:/client_install";

const char * const SETTING_TOOL_SUFFIX = "ToolSuffix";
const char * const SETTING_TOOL_SUFFIX_DEFAULT = ".exe";

const char * const SETTING_TOOL_DIRECTORIES = "ToolDirectories";
const char * const SETTING_TOOL_DIRECTORIES_DEFAULT_1 = "R:/build/dev/bin/Release";
const char * const SETTING_TOOL_DIRECTORIES_DEFAULT_2 = "D:/libraries/external/bin";

const char * const SETTING_EXE_DIRECTORIES = "ExeDllCfgDirectories";
const char * const SETTING_EXE_DIRECTORIES_DEFAULT_1 = "C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC/redist/x86";
const char * const SETTING_EXE_DIRECTORIES_DEFAULT_2 = "D:/libraries/external/bin";
const char * const SETTING_EXE_DIRECTORIES_DEFAULT_3 = "R:/build/dev/bin/Release";
const char * const SETTING_EXE_DIRECTORIES_DEFAULT_4 = "R:/code/ryzom/client";
const char * const SETTING_EXE_DIRECTORIES_DEFAULT_5 = "R:/code/nel/lib";
const char * const SETTING_EXE_DIRECTORIES_DEFAULT_6 = "R:/code/ryzom/bin";
const char * const SETTING_EXE_DIRECTORIES_DEFAULT_7 = "R:/code/ryzom/tools/client/client_config/bin";

} // namespace Constants
} // namespace BuildGamedata

#endif // BUILD_GAMEDATA_CONSTANTS_H
