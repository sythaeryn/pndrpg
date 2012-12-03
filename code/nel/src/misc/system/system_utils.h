// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_SYSTEM_UTILS_H
#define NL_SYSTEM_UTILS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/ucstring.h"

namespace NLMISC
{

/*
 * Operating system miscellaneous functions (all methods and variables should be static)
 * \author Kervala
 * \date 2010
 */
class CSystemUtils
{
	static nlWindow s_window;
public:

	/// Initialize data which needs it before using them.
	static bool init();

	/// Uninitialize data when they won't be used anymore.
	static bool uninit();

	/// Set the window which will be used by some functions.
	static void setWindow(nlWindow window);

	/// Check if system supports unicode.
	static bool supportUnicode();

	/// Check if keyboard layout is AZERTY.
	static bool isAzertyKeyboard();

	/// Check if screensaver is enabled.
	static bool isScreensaverEnabled();

	/// Enable or disable screeensaver.
	static bool enableScreensaver(bool screensaver);

	/// Get the ROOT registry key used by getRegKey and setRegKey.
	static std::string getRootKey();

	/// Set the ROOT registry key used by getRegKey and setRegKey.
	static void setRootKey(const std::string &root);

	/// Read a value from registry.
	static std::string getRegKey(const std::string &Entry);

	/// Write a value to registry.
	static bool setRegKey(const std::string &ValueName, const std::string &Value);

	/// Get desktop current color depth without using UDriver.
	static uint getCurrentColorDepth();
};

} // NLMISC

#endif // NL_SYSTEM_UTILS_H
