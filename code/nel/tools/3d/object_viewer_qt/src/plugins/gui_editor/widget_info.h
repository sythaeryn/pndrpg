// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#ifndef WIDGET_INFO_H
#define WIDGET_INFO_H

#include <string>
#include <vector>

namespace GUIEditor
{
	struct SPropEntry
	{
		std::string propName;
		std::string propType;
		std::string propDefault;
		
		static SPropEntry create( const char *propname, const char *proptype, const char *propdefault )
		{
			SPropEntry entry;
			entry.propName = propname;
			entry.propType = proptype;
			entry.propDefault = propdefault;
			return entry;
		}
	};

	struct SWidgetInfo
	{
		std::string name;
		std::string GUIName;
		std::string description;
		bool isAbstract;
		std::string icon;

		std::vector< SPropEntry > props;
	};
}

#endif
