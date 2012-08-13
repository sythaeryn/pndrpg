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



// ----------------------------------------------------------------------------
#include "nel/gui/group_wheel.h"


NLMISC_REGISTER_OBJECT(CViewBase, CInterfaceGroupWheel, std::string, "group_wheel");

namespace NLGUI
{

	// *****************************************************************************************************************
	CInterfaceGroupWheel::CInterfaceGroupWheel(const TCtorParam &param) : CInterfaceGroup(param)
	{
		_AHWheelUp = NULL;
		_AHWheelDown = NULL;
	}

	std::string CInterfaceGroupWheel::getProperty( const std::string &name ) const
	{
		if( name == "on_wheel_up" )
		{
			return CAHManager::getInstance()->getActionHandlerName( _AHWheelUp );
		}
		else
		if( name == "on_wheel_up_params" )
		{
			return _AHWheelUpParams;
		}
		else
		if( name == "on_wheel_down" )
		{
			return CAHManager::getInstance()->getActionHandlerName( _AHWheelDown );
		}
		else
		if( name == "on_wheel_down_params" )
		{
			return _AHWheelDownParams;
		}
		else
			return CInterfaceGroup::getProperty( name );
	}

	void CInterfaceGroupWheel::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "on_wheel_up" )
		{
			_AHWheelUp = CAHManager::getInstance()->getAH( value, std::string() );
			return;
		}
		else
		if( name == "on_wheel_up_params" )
		{
			_AHWheelUpParams = value;
			return;
		}
		else
		if( name == "on_wheel_down" )
		{
			_AHWheelDown = CAHManager::getInstance()->getAH( value, std::string() );
			return;
		}
		else
		if( name == "on_wheel_down_params" )
		{
			_AHWheelDownParams = value;
			return;
		}
		else
			CInterfaceGroup::setProperty( name, value );
	}


	xmlNodePtr CInterfaceGroupWheel::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CInterfaceGroup::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "group_wheel" );

		xmlSetProp( node, BAD_CAST "on_wheel_up",
			BAD_CAST CAHManager::getInstance()->getActionHandlerName( _AHWheelUp ).c_str() );

		xmlSetProp( node, BAD_CAST "on_wheel_up_params", BAD_CAST _AHWheelUpParams.toString().c_str() );
		
		xmlSetProp( node, BAD_CAST "on_wheel_down",
			BAD_CAST CAHManager::getInstance()->getActionHandlerName( _AHWheelDown ).c_str() );

		xmlSetProp( node, BAD_CAST "on_wheel_down_params", BAD_CAST _AHWheelDownParams.toString().c_str() );

		return node;
	}

	// *****************************************************************************************************************
	bool CInterfaceGroupWheel::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
	{
		if (!CInterfaceGroup::parse(cur, parentGroup)) return false;
		CAHManager::getInstance()->parseAH(cur, "on_wheel_up", "on_wheel_up_params", _AHWheelUp, _AHWheelUpParams);
		CAHManager::getInstance()->parseAH(cur, "on_wheel_down", "on_wheel_down_params", _AHWheelDown, _AHWheelDownParams);
		return true;
	}

	// *****************************************************************************************************************
	bool CInterfaceGroupWheel::handleEvent(const NLGUI::CEventDescriptor &event)
	{
		if (CInterfaceGroup::handleEvent(event)) return true;
		if (event.getType() == NLGUI::CEventDescriptor::mouse)
		{
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousewheel)
			{
				if (eventDesc.getWheel() > 0 && _AHWheelUp)
				{
					CAHManager::getInstance()->runActionHandler (_AHWheelUp, this, _AHWheelUpParams);
					return true;
				}
				else if (_AHWheelDown)
				{
					CAHManager::getInstance()->runActionHandler (_AHWheelDown, this, _AHWheelDownParams);
					return true;
				}
			}
		}
		return false;
	}

}

