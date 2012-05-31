#include <algorithm>

// to get rid of you_must_not_use_assert___use_nl_assert___read_debug_h_file messages
#include <cassert>
#ifdef assert
	#undef assert
#endif

// Warning: cannot use namespace std,    when using luabind
#ifdef NL_OS_WINDOWS
#  ifndef NL_EXTENDED_FOR_SCOPE
#    undef for
#  endif
#endif

#ifdef NL_DEBUG
#	define assert(x) nlassert(x)
#else
#	define assert(x)
#endif

#include <luabind/luabind.hpp>
// in luabind > 0.6, LUABIND_MAX_ARITY is set to 10
#if LUABIND_MAX_ARITY == 10
#	include <luabind/operator.hpp>
// only luabind > 0.7 have version.hpp (file checked with build system)
#	ifdef HAVE_LUABIND_VERSION
#		include <luabind/version.hpp>
#	endif
#	ifndef LUABIND_VERSION
// luabind 0.7 doesn't define LUABIND_VERSION
#		define LUABIND_VERSION 700
#	endif
// luabind 0.6 doesn't define LUABIND_VERSION but LUABIND_MAX_ARITY is set to 5
#elif LUABIND_MAX_ARITY == 5
#	define LUABIND_VERSION 600
#else
#	pragma error("luabind version not recognized")
#endif

#include "lua_ihm_ryzom.h"
#include "interface_manager.h"
#include "nel/gui/lua_helper.h"
#include "nel/gui/lua_object.h"

#include "nel/gui/lua_ihm.h"
#include "nel/gui/reflect.h"
#include "action_handler.h"
#include "action_handler_tools.h"
#include "interface_manager.h"
#include "interface_group.h"
#include "view_text.h"
#include "game_share/people_pd.h"
#include "group_tree.h"
#include "interface_link.h"
#include "nel/gui/interface_expr.h"
#include "people_interraction.h"
#include "nel/misc/algo.h"
#include "nel/misc/file.h"
#include "nel/misc/i18n.h"
#include "nel/misc/time_nl.h"
#include "skill_manager.h"
#include "group_html.h"
#include "../net_manager.h"
#include "../user_entity.h"
#include "sphrase_manager.h"
#include "guild_manager.h"
#include "../client_cfg.h"
#include "../sheet_manager.h"
#include "nel/gui/lua_object.h"
#include "game_share/emote_list_parser.h"
#include "game_share/pvp_clan.h"
#include "../weather.h"
#include "../continent_manager.h"
#include "../zone_util.h"
#include "../motion/user_controls.h"
#include "group_html_cs.h"
#include "bonus_malus.h"
#include "group_editbox.h"
#include "../entities.h"
#include "../sheet_manager.h"				// for emotes
#include "../global.h"						// for emotes
#include "../entity_animation_manager.h"	// for emotes
#include "../net_manager.h"				// for emotes
#include "../client_chat_manager.h"		// for emotes
#include "../login.h"
#include "nel/gui/lua_object.h"
#include "../actions.h"
#include "../bg_downloader_access.h"
#include "../connection.h"
#include "../login_patch.h"

#include "bot_chat_page_all.h"
#include "bot_chat_page_ring_sessions.h"
#include "nel/georges/u_form_loader.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/misc/polygon.h"
#include "game_share/scenario_entry_points.h"
#include "game_share/bg_downloader_msg.h"
#include "game_share/constants.h"
#include "game_share/visual_slot_manager.h"
#include "nel/gui/lua_manager.h"

#ifdef LUA_NEVRAX_VERSION
	#include "lua_ide_dll_nevrax/include/lua_ide_dll/ide_interface.h" // external debugger
#endif


#ifdef LUA_NEVRAX_VERSION
	extern ILuaIDEInterface *LuaDebuggerIDE;
#endif

using namespace NLMISC;
using namespace NLGUI;
using namespace R2;

extern NLMISC::CLog	g_log;
extern CContinentManager ContinentMngr;
extern CClientChatManager		ChatMngr;

// ***************************************************************************
class CHandlerLUA : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller,    const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// For getUI() LUA function,    push the UI caller
		if(pCaller)
			_UICallerStack.push_back(pCaller);

		// execute a small script. NB: use a small script here because
		// most often action handlers are called from xml files => lot of redundant script
		pIM->executeLuaScript(sParams,   true);

		// pop UI caller
		if(pCaller)
			_UICallerStack.pop_back();
	}

	// get the top of stack Caller to this LUA script
	static CCtrlBase	*getUICaller();

private:
	static	std::deque<CRefPtr<CCtrlBase> >		_UICallerStack;
};
REGISTER_ACTION_HANDLER( CHandlerLUA,    "lua");
std::deque<CRefPtr<CCtrlBase> >		CHandlerLUA::_UICallerStack;

// ***************************************************************************
// Allow also to call script from expression
static DECLARE_INTERFACE_USER_FCT(lua)
{
	if(args.size()!=1 || !args[0].toString())
	{
		nlwarning("<lua> requires 1 arg (string=script)");
		return false;
	}

	// Retrieve lua state
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CLuaState	*state= CLuaManager::getInstance().getLuaState();
	if(!state)
		return false;
	CLuaState	&ls= *state;

	// *** clear return value
	const	std::string		retId= "__ui_internal_ret_";
	CLuaStackChecker	lsc(&ls);
	ls.push(retId);
	ls.pushNil();
	ls.setTable(LUA_GLOBALSINDEX);


	// *** execute script
	std::string	script= args[0].getString();
	// assign return value in retId.
	script= retId + "= " + script;
	// execute a small script here,   because most often exprs are called from xml files => lot of redundant script
	pIM->executeLuaScript(script,   true);


	// *** retrieve and convert return value
	ls.push(retId);
	ls.getTable(LUA_GLOBALSINDEX);
	bool	ok= false;
	sint	type= ls.type();
	if (type==LUA_TBOOLEAN)
	{
		// get and pop
		bool	val= ls.toBoolean();
		ls.pop();
		// set result
		result.setBool(val);
		ok= true;
	}
	else if(type==LUA_TNUMBER)
	{
		// get and pop
		double	val= ls.toNumber();
		ls.pop();
		// set double or integer?
		if(val==floor(val))
			result.setInteger(sint64(floor(val)));
		else
			result.setDouble(val);
		ok= true;
	}
	else if(type==LUA_TSTRING)
	{
		// get and pop
		std::string	val;
		ls.toString(-1,    val);
		ls.pop();
		// set result
		result.setString(val);
		ok= true;
	}
	else if(type==LUA_TUSERDATA)
	{
		// NB: the value is poped in obj.set() (no need to do ls.pop());

		// try with ucstring
		ucstring ucstrVal;
		if (CLuaIHM::pop(ls, ucstrVal))
		{
			result.setUCString(ucstrVal);
			ok= true;
		}

		// try with RGBA
		if(!ok)
		{
			NLMISC::CRGBA rgbaVal;
			if (CLuaIHM::pop(ls, rgbaVal))
			{
				result.setRGBA(rgbaVal);
				ok= true;
			}
		}
	}
	else
	{
		// error (nil for instance)
		ls.pop();
	}

	return ok;
}
REGISTER_INTERFACE_USER_FCT("lua",    lua)


CCtrlBase	*CHandlerLUA::getUICaller()
{
	if(_UICallerStack.empty())
		return NULL;
	else
		return _UICallerStack.back();
}

#define LUABIND_ENUM(__enum__, __name__, __num__, __toStringFunc__) \
	createLuaEnumTable(ls, __name__); \
	for (uint e=0 ; e<__num__ ; e++) \
	{ \
		std::string str = __toStringFunc__((__enum__)e); \
		std::string temp = __name__ + toString(".") + __toStringFunc__((__enum__)e) + " = " + toString("%d;", e); \
		ls.executeScript(temp); \
	} \



#define LUABIND_FUNC(__func__) luabind::def(#__func__,    &__func__)

// ***************************************************************************
int CLuaIHMRyzom::luaClientCfgIndex(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_luaClientCfgIndex)
	CConfigFile::CVar *v = ClientCfg.ConfigFile.getVarPtr(ls.toString(2));
	if (!v) return 0;
	if (v->size() != 1)
	{
		// arrays not implemented (would require a second metatable)....
		throw ELuaWrappedFunctionException(&ls, "Access to array inside client.cfg not supported.");
	}
	switch(v->Type)
	{
		case CConfigFile::CVar::T_REAL:
			ls.push((double) v->asDouble());
			return 1;
		break;
		case CConfigFile::CVar::T_STRING:
			ls.push(v->asString());
			return 1;
		break;
		default: // handle both T_INT && T_BOOL
		case CConfigFile::CVar::T_INT:
			ls.push((double) v->asInt());
			return 1;
		break;
	}
	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::luaClientCfgNewIndex(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_luaClientCfgNewIndex)
	throw ELuaWrappedFunctionException(&ls, "Can't write into config file from lua.");
}

// ***************************************************************************
CInterfaceElement *CLuaIHMRyzom::getUIRelative(CInterfaceElement *pIE,    const std::string &propName)
{
	//H_AUTO(Lua_CLuaIHM_getUIRelative)
	if (pIE == NULL) return NULL;
	// If the prop is "parent",    then return the parent of the ui
	if(propName=="parent")
	{
		return pIE->getParent();
	}
	// else try to get a child (if group/exist)
	else
	{
		CInterfaceGroup		*group= dynamic_cast<CInterfaceGroup*>(pIE);
		if(group)
		{
			return group->getElement(group->getId()+":"+propName);
		}
	}

	return NULL;
}

static CLuaString lstr_Env("Env");
static CLuaString lstr_isNil("isNil");

// ***************************************************************************
int CLuaIHMRyzom::luaUIIndex(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_luaUIIndex)
	nlassert(ls.getTop()==2);
	// get the userdata and key
	CReflectableLuaRef *pRefElm = (CReflectableLuaRef *) ls.toUserData(1);

	const char *propName = ls.toString(2);
	CReflectableRefPtrTarget	*pRPT= (CReflectableRefPtrTarget*)(pRefElm->Ptr);
	// ** try to get the Env Table (interface group only)
	if(propName==lstr_isNil)
	{
		ls.push(pRPT==NULL);
		return 1;
	}

	// Check the object is not NULL or freed
	if(pRPT==NULL)
	{
		return 0;
	}

	// ** try to get the Env Table (interface group only)
	if(propName==lstr_Env)
	{
		// Env can be bound to a CInterfaceGroup only
		CInterfaceGroup		*group= dynamic_cast<CInterfaceGroup*>(pRPT);
		if(group==NULL)
		{
			ls.pushNil();
			return 1;
		}
		else
		{
			group->pushLUAEnvTable();
			return 1;
		}
	}

	// ** try to get the property
	const CReflectedProperty *prop = pRefElm->getProp(propName);
	if (prop)
	{
		CLuaIHM::luaValueFromReflectedProperty(ls, *pRPT, *prop);
		return 1;
	}

	// ** try to get a UI relative
	CInterfaceElement	*uiRelative= getUIRelative(dynamic_cast<CInterfaceElement *>(pRPT),    propName);
	if(uiRelative)
	{
		// push the UI onto the stack
		pushUIOnStack(ls,    uiRelative);
		return 1;
	}


	// Fail to find any Attributes or elements
	// Yoyo: don't write any message or warning because this may be a feature (if user want to test that something exit in the ui)
	ls.pushNil();
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::luaUINewIndex(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_luaUINewIndex)
	nlassert(ls.getTop()==3);
	// get the userdata and key
	CReflectableLuaRef	*pRefElm = (CReflectableLuaRef *) ls.toUserData(1);
	nlassert(pRefElm);
	CReflectableRefPtrTarget	*pRPT= (CReflectableRefPtrTarget*)(pRefElm->Ptr);
	// Check the UI is not NULL or freed
	if(pRPT == NULL)
	{
		return 0;
	}

	const char *propName = ls.toString(2);
	// ** try to set the Env Table (interface group only)
	if(propName == lstr_Env)
	{
		CInterfaceElement *pIE = dynamic_cast<CInterfaceElement *>(pRPT);
		std::string name ;
		if (pIE)
		{
			name = pIE->getId();
		}
		else
		{
			name = "<reflectable element>";
		}
		// Exception!!! not allowed
		throw ELuaIHMException("You cannot change the Env Table of '%s'",    name.c_str());
	}


	// ** try to set the property
	const CReflectedProperty *prop = pRefElm->getProp(propName);
	if (prop)
	{
		CLuaIHM::luaValueToReflectedProperty(ls, 3, *pRPT, *prop);
		return 0;
	}

	CInterfaceElement	*pIE = dynamic_cast<CInterfaceElement *>(pRPT);
	// ** try to get another UI (child or parent)
	CInterfaceElement	*uiRelative= getUIRelative(pIE,    propName);
	if(uiRelative)
	{
		// Exception!!! not allowed
		throw ELuaIHMException("You cannot write into the UI '%s' of '%s'",    propName,    pIE->getId().c_str());
	}

	// ** Prop Not Found
	throw ELuaIHMException("Property '%s' not found in '%s' of type %s",    propName,    pIE ? pIE->getId().c_str() : "<reflectable element>", typeid(*pRPT).name());

	// Fail to find any Attributes or elements
	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::luaUIEq(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_luaUIEq)
	nlassert(ls.getTop() == 2);
	// read lhs & rhs
	// get the userdata and key
	CReflectableLuaRef	*lhs = (CReflectableLuaRef *) ls.toUserData(1);
	CReflectableLuaRef	*rhs = (CReflectableLuaRef *) ls.toUserData(2);
	nlassert(lhs);
	nlassert(rhs);
	ls.push(lhs->Ptr == rhs->Ptr);
	return 1;
}


// ***************************************************************************
int CLuaIHMRyzom::luaUIDtor(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_luaUIDtor)
	nlassert(ls.getTop()==1);
	// get the userdata
	CReflectableLuaRef	*pRefElm = (CReflectableLuaRef *) ls.toUserData(1);
	nlassert(pRefElm);

	// call dtor
	pRefElm->~CReflectableLuaRef();

	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::luaUINext(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_luaUINext)
	// Code below allow enumeration of properties of a reflectable object
	// From lua standpoint, the object is seen as a table with (key, value) pairs
	// If object is a CInterfaceGroup, iteration is also done on sons (groups, controls & view).

	if (ls.getTop() != 2)
	{
		CLuaIHM::fails(ls, "__next metamethod require 2 arguments (table & key)");
	}
	CLuaIHM::check(ls, CLuaIHM::isReflectableOnStack(ls, 1), "__next :  require ui element as first arg");
	CReflectableRefPtrTarget *reflectedObject = CLuaIHM::getReflectableOnStack(ls, 1);
	// To traverse all properties / field of the object, we must be able to determine the next key from a previous key
	// (keys are ordered)
	// We use the 'TValueType' enum to know which kind of property we are traversing, and an index in this group of properties
	// The key which uniquely identify an element / property in the reflectable object
	struct CKey
	{
		enum TValueType
		{
			VTGroup = 0, // children groups    (If the object is a CInterfaceGroup)
			VTView,      // children views	   (If the object is a CInterfaceView)
			VTCtrl, 	 // children controls  (If the object is a CInterfaceCtrl)
			VTProp       // List of exported proeprties (For all relfectable objects)
		};
		TValueType		  ValueType;
		sint			  Index;
		const CClassInfo  *ClassInfo; // if ValueType is "VTProp" -> give the class for which property are currently enumerated
		//
		static int tostring(CLuaState &ls) // '__print' metamathod
		{
			CLuaIHM::checkArgCount(ls, "reflected object metatable:__print", 1);
			CKey key;
			key.pop(ls);
			switch(key.ValueType)
			{
				case VTGroup: ls.push(toString("_Group %d", key.Index)); break;
				case VTView:  ls.push(toString("_View %d", key.Index)); break;
				case VTCtrl:  ls.push(toString("_Ctrl %d", key.Index)); break;
				case VTProp:  ls.push(key.ClassInfo->Properties[key.Index].Name); break;
			}
			return 1;
		}
		// push the key on the lua stack
		void push(CLuaState &ls)
		{
			void *ud = ls.newUserData(sizeof(*this));
			*(CKey *) ud = *this;
			getMetaTable(ls).push();
			ls.setMetaTable(-2);
		}
		// pop the key from the lua stack
		void pop(CLuaState &ls)
		{
			CLuaStackChecker lsc(&ls, -1);
			if (!ls.isUserData(-1))
			{
				CLuaIHM::fails(ls, "Can't pop object, not a user data");
			}
			// check that metatable is good (it is share between all keys)
			ls.getMetaTable(-1);
			getMetaTable(ls).push();
			if (!ls.rawEqual(-1, -2))
			{
				CLuaIHM::fails(ls, "Bad metatable for reflectable object key");
			}
			ls.pop(2);
			// retrieve key
			*this = *(CKey *) ls.toUserData(-1);
			ls.pop();
		}
		// get the metatable for a CKey
		CLuaObject &getMetaTable(CLuaState &ls)
		{
			static CLuaObject metatable;
			if (!metatable.isValid())
			{
				// first build
				CLuaStackChecker lsc(&ls);
				ls.newTable();
				ls.push("__tostring");
				ls.push(CKey::tostring);
				ls.setTable(-3);
				metatable.pop(ls);
			}
			return metatable;
		}
	};
	// Pop the current key to continue enumeration
	CKey key;
	if (ls.isNil(2))
	{
		// no key -> start of table
		key.ValueType = CKey::VTGroup;
		key.Index = -1;
	}
	else
	{
		key.pop(ls);
	}
	//
	CInterfaceGroup *group = dynamic_cast<CInterfaceGroup *>(reflectedObject);
	bool enumerate = true;
	while (enumerate)
	{
		switch(key.ValueType)
		{
			case CKey::VTGroup:
				if (!group || (key.Index + 1) == (sint) group->getGroups().size())
				{
					key.Index     = -1;
					key.ValueType = CKey::VTView; // continue enumeration with views
				}
				else
				{
					++ key.Index;
					key.push(ls);
					pushUIOnStack(ls, group->getGroups()[key.Index]);
					return 2;
				}
			break;
			case CKey::VTView:
				if (!group || (key.Index + 1) == (sint) group->getViews().size())
				{
					key.Index     = -1;
					key.ValueType = CKey::VTCtrl; // continue enumeration with controls
				}
				else
				{
					++ key.Index;
					key.push(ls);
					pushUIOnStack(ls, group->getViews()[key.Index]);
					return 2;
				}
			break;
			case CKey::VTCtrl:
				if (!group || (key.Index + 1) == (sint) group->getControls().size())
				{
					key.Index     = -1;
					key.ValueType = CKey::VTProp; // continue enumeration with properties
					key.ClassInfo = reflectedObject->getClassInfo();
				}
				else
				{
					++ key.Index;
					key.push(ls);
					pushUIOnStack(ls, group->getControls()[key.Index]);
					return 2;
				}
			break;
			case CKey::VTProp:
				if (!key.ClassInfo)
				{
					enumerate = false;
					break;
				}
				if ((sint) key.ClassInfo->Properties.size() == (key.Index + 1))
				{
					key.ClassInfo = key.ClassInfo->ParentClass; // continue enumeration in parent class
					key.Index = -1;
				}
				else
				{
					++ key.Index;
					key.push(ls);
					CLuaIHM::luaValueFromReflectedProperty(ls, *reflectedObject, key.ClassInfo->Properties[key.Index]);
					return 2;
				}
			break;
			default:
				nlassert(0);
			break;
		}
	}
	ls.pushNil();
	return 0;
}

// ***************************************************************************
void	CLuaIHMRyzom::pushUIOnStack(CLuaState &ls,    class CInterfaceElement *pIE)
{
	//H_AUTO(Lua_CLuaIHM_pushUIOnStack)
	CLuaIHM::pushReflectableOnStack(ls,    pIE);
}

// ***************************************************************************
bool	CLuaIHMRyzom::isUIOnStack(CLuaState &ls,    sint index)
{
	//H_AUTO(Lua_CLuaIHM_isUIOnStack)
	return getUIOnStack(ls,    index) != NULL;
}

// ***************************************************************************
CInterfaceElement	*CLuaIHMRyzom::getUIOnStack(CLuaState &ls,    sint index)
{
	//H_AUTO(Lua_CLuaIHM_getUIOnStack)
	return dynamic_cast<CInterfaceElement *>(CLuaIHM::getReflectableOnStack(ls,    index));
}

// ***************************************************************************
void CLuaIHMRyzom::checkArgTypeUIElement(CLuaState &ls, const char *funcName, uint index)
{
	//H_AUTO(Lua_CLuaIHM_checkArgTypeUIElement)
	nlassert(index > 0);
	if (ls.getTop() < (int) index)
	{
		CLuaIHM::fails(ls, "%s : argument %d of expected type ui element was not defined",   funcName,   index);
	}
	if (!isUIOnStack(ls, index))
	{
		CLuaIHM::fails(ls, "%s : argument %d of expected type ui element has bad type : %s",   funcName,   index, ls.getTypename(ls.type(index)),   ls.type(index));
	}
}



// ***************************************************************************
void CLuaIHMRyzom::createLuaEnumTable(CLuaState &ls, const std::string &str)
{
	//H_AUTO(Lua_CLuaIHM_createLuaEnumTable)
	std::string path = "", script, p;
	CSString s = str;
	// Create table recursively (ex: 'game.TPVPClan' will check/create the table 'game' and 'game.TPVPClan')
	p = s.splitTo('.', true);
	while (p.size() > 0)
	{
		if (path == "")
			path = p;
		else
			path += "." + p;
		script = "if (" + path + " == nil) then " + path + " = {}; end";
		ls.executeScript(script);
		p = s.splitTo('.', true);
	}
}

void CLuaIHMRyzom::RegisterRyzomFunctions( NLGUI::CLuaState &ls )
{
	CLuaStackChecker lsc( &ls );

	// MISC ui ctors
	struct CUICtor
	{
		// CGroupTree::SNode
		static int SNode(CLuaState &ls)
		{
			CLuaIHM::checkArgCount(ls,    "SNode",    0);
			CLuaIHM::pushReflectableOnStack(ls,    new CGroupTree::SNode);
			return 1;
		}
	};

	ls.registerFunc("SNode",    CUICtor::SNode);

	// *** Register the metatable for access to client.cfg (nb nico this may be more general later -> access to any config file ...)
	ls.pushValue(LUA_GLOBALSINDEX);
	CLuaObject globals(ls);
	CLuaObject clientCfg = globals.newTable("config");
	CLuaObject mt = globals.newTable("__cfmt");
	nlverify(clientCfg.setMetaTable(mt));
	mt.setValue("__index", luaClientCfgIndex);
	mt.setValue("__newindex", luaClientCfgNewIndex);
	globals.setNil("__cfmt"); // remove temp metatable

	// *** Register the MetaTable for UI userdata
	ls.push(IHM_LUA_METATABLE);			// "__ui_metatable"
	ls.newTable();						// "__ui_metatable"  {}
	// set the '__index' method
	ls.push("__index");
	ls.push(luaUIIndex);
	nlassert(ls.isCFunction());
	ls.setTable(-3);					// "__ui_metatable"  {"__index"= CFunc_luaUIIndex}
	// set the '__newindex' method
	ls.push("__newindex");
	ls.push(luaUINewIndex);
	nlassert(ls.isCFunction());
	ls.setTable(-3);
	// set the '__newindex' method
	ls.push("__gc");
	ls.push(luaUIDtor);
	nlassert(ls.isCFunction());
	ls.setTable(-3);
	// set the '__eq' method
	ls.push("__eq");
	ls.push(luaUIEq);
	nlassert(ls.isCFunction());
	ls.setTable(-3);
	// set the custom '__next' method
	ls.push("__next");
	ls.push(luaUINext);
	nlassert(ls.isCFunction());
	ls.setTable(-3);
	// set registry
	ls.setTable(LUA_REGISTRYINDEX);


	ls.registerFunc( "getUI", getUI );
	ls.registerFunc("setOnDraw",    setOnDraw);
	ls.registerFunc("setCaptureKeyboard", setCaptureKeyboard);
	ls.registerFunc("resetCaptureKeyboard", resetCaptureKeyboard);
	ls.registerFunc("validMessageBox",    validMessageBox);
	ls.registerFunc("setTopWindow", setTopWindow);
	ls.registerFunc("concatUCString", concatUCString);
	ls.registerFunc("concatString", concatString);
	ls.registerFunc("tableToString", tableToString);
	ls.registerFunc("addOnDbChange",    addOnDbChange);
	ls.registerFunc("removeOnDbChange",    removeOnDbChange);
	ls.registerFunc("getUICaller",    getUICaller);
	ls.registerFunc("getCurrentWindowUnder", getCurrentWindowUnder);
	ls.registerFunc("getUI",    getUI);
	ls.registerFunc("getIndexInDB", getIndexInDB);
	ls.registerFunc("getUIId",    getUIId);
	ls.registerFunc("createGroupInstance", createGroupInstance);
	ls.registerFunc("createRootGroupInstance", createRootGroupInstance);
	ls.registerFunc("createUIElement", createUIElement);
	ls.registerFunc("launchContextMenuInGame",    launchContextMenuInGame);
	ls.registerFunc("parseInterfaceFromString",    parseInterfaceFromString);
	ls.registerFunc("updateAllLocalisedElements",    updateAllLocalisedElements);
	ls.registerFunc("runAH",    runAH);
	ls.registerFunc("runExpr",    runExpr);
	ls.registerFunc("runFct",    runFct);
	ls.registerFunc("runCommand",    runCommand);
	ls.registerFunc("formatUI",    formatUI);
	ls.registerFunc("formatDB",    formatDB);
	ls.registerFunc("deleteUI",    deleteUI);
	ls.registerFunc("deleteReflectable",    deleteReflectable);
	ls.registerFunc("dumpUI",    dumpUI);
	ls.registerFunc("setKeyboardContext",    setKeyboardContext);
	ls.registerFunc("breakPoint",    breakPoint);
	ls.registerFunc("getWindowSize",    getWindowSize);
	ls.registerFunc("setTextFormatTaged",    setTextFormatTaged);
	ls.registerFunc("initEmotesMenu", initEmotesMenu);
	ls.registerFunc("isUCString", isUCString);
	ls.registerFunc("hideAllWindows", hideAllWindows);
	ls.registerFunc("hideAllNonSavableWindows", hideAllNonSavableWindows);
	ls.registerFunc("getDesktopIndex", getDesktopIndex);
	ls.registerFunc("setLuaBreakPoint", setLuaBreakPoint);
	ls.registerFunc("getMainPageURL", getMainPageURL);
	ls.registerFunc("getCharSlot", getCharSlot);
	ls.registerFunc("getPathContent", getPathContent);
	ls.registerFunc("getServerSeason", getServerSeason);
	ls.registerFunc("computeCurrSeason", computeCurrSeason);
	ls.registerFunc("getAutoSeason", getAutoSeason);
	ls.registerFunc("getTextureSize", getTextureSize);
	ls.registerFunc("enableModalWindow", enableModalWindow);
	ls.registerFunc("disableModalWindow", disableModalWindow);
	ls.registerFunc("getPlayerPos", getPlayerPos);
	ls.registerFunc("getPlayerFront", getPlayerFront);
	ls.registerFunc("getPlayerDirection", getPlayerDirection);
	ls.registerFunc("getPlayerGender", getPlayerGender);
	ls.registerFunc("getPlayerName", getPlayerName);
	ls.registerFunc("getPlayerTitleRaw", getPlayerTitleRaw);
	ls.registerFunc("getPlayerTitle", getPlayerTitle);
	ls.registerFunc("getTargetPos", getTargetPos);
	ls.registerFunc("getTargetFront", getTargetFront);
	ls.registerFunc("getTargetDirection", getTargetDirection);
	ls.registerFunc("getTargetGender", getTargetGender);
	ls.registerFunc("getTargetName", getTargetName);
	ls.registerFunc("getTargetTitleRaw", getTargetTitleRaw);
	ls.registerFunc("getTargetTitle", getTargetTitle);
	ls.registerFunc("addSearchPathUser", addSearchPathUser);
	ls.registerFunc("displaySystemInfo", displaySystemInfo);
	ls.registerFunc("disableContextHelpForControl", disableContextHelpForControl);
	ls.registerFunc("disableContextHelp", disableContextHelp);
	ls.registerFunc("setWeatherValue", setWeatherValue);
	ls.registerFunc("getWeatherValue", getWeatherValue);
	ls.registerFunc("getCompleteIslands", getCompleteIslands);
	ls.registerFunc("displayBubble", displayBubble);
	ls.registerFunc("getIslandId", getIslandId);
	ls.registerFunc("getClientCfgVar", getClientCfgVar);
	ls.registerFunc("isPlayerFreeTrial", isPlayerFreeTrial);
	ls.registerFunc("isPlayerNewbie", isPlayerNewbie);
	ls.registerFunc("isInRingMode", isInRingMode);
	ls.registerFunc("getUserRace",  getUserRace);
	ls.registerFunc("getSheet2idx",  getSheet2idx);
	ls.registerFunc("getTargetSlot",  getTargetSlot);
	ls.registerFunc("getSlotDataSetId",  getSlotDataSetId);
	
	
	lua_State	*L= ls.getStatePointer();

	LUABIND_ENUM(PVP_CLAN::TPVPClan, "game.TPVPClan", PVP_CLAN::NbClans, PVP_CLAN::toString);
	LUABIND_ENUM(BONUS_MALUS::TBonusMalusSpecialTT, "game.TBonusMalusSpecialTT", BONUS_MALUS::NbSpecialTT, BONUS_MALUS::toString);

	luabind::module(L)
	[
		LUABIND_FUNC(getDbProp),
		LUABIND_FUNC(setDbProp),
		LUABIND_FUNC(addDbProp),
		LUABIND_FUNC(delDbProp),
		LUABIND_FUNC(debugInfo),
		LUABIND_FUNC(rawDebugInfo),
		LUABIND_FUNC(dumpCallStack),
		LUABIND_FUNC(getDefine),
		LUABIND_FUNC(setContextHelpText),
		luabind::def("messageBox",    (void(*)(const ucstring &)) &messageBox),
		luabind::def("messageBox",    (void(*)(const ucstring &, const std::string &)) &messageBox),
		luabind::def("messageBox",    (void(*)(const ucstring &, const std::string &, int caseMode)) &messageBox),
		luabind::def("messageBox",    (void(*)(const std::string &)) &messageBox),
		luabind::def("messageBoxWithHelp",    (void(*)(const ucstring &)) &messageBoxWithHelp),
		luabind::def("messageBoxWithHelp",    (void(*)(const ucstring &, const std::string &)) &messageBoxWithHelp),
		luabind::def("messageBoxWithHelp",    (void(*)(const ucstring &, const std::string &, int caseMode)) &messageBoxWithHelp),
		luabind::def("messageBoxWithHelp",    (void(*)(const std::string &)) &messageBoxWithHelp),
		LUABIND_FUNC(replacePvpEffectParam),
		LUABIND_FUNC(secondsSince1970ToHour),
		LUABIND_FUNC(pauseBGDownloader),
		LUABIND_FUNC(unpauseBGDownloader),
		LUABIND_FUNC(requestBGDownloaderPriority),
		LUABIND_FUNC(getBGDownloaderPriority),
		LUABIND_FUNC(getPatchLastErrorMessage),
		LUABIND_FUNC(getPlayerSelectedSlot),
		LUABIND_FUNC(isInGame),
		LUABIND_FUNC(isPlayerSlotNewbieLand),
		LUABIND_FUNC(getSkillIdFromName),
		LUABIND_FUNC(getSkillLocalizedName),
		LUABIND_FUNC(getMaxSkillValue),
		LUABIND_FUNC(getBaseSkillValueMaxChildren),
		LUABIND_FUNC(getMagicResistChance),
		LUABIND_FUNC(getDodgeParryChance),
		LUABIND_FUNC(browseNpcWebPage),
		LUABIND_FUNC(clearHtmlUndoRedo),
		LUABIND_FUNC(getDynString),
		LUABIND_FUNC(isDynStringAvailable),
		LUABIND_FUNC(isFullyPatched),
		LUABIND_FUNC(getSheetType),
		LUABIND_FUNC(getSheetName),
		LUABIND_FUNC(getFameIndex),
		LUABIND_FUNC(getFameName),
		LUABIND_FUNC(getFameDBIndex),
		LUABIND_FUNC(getFirstTribeFameIndex),
		LUABIND_FUNC(getNbTribeFameIndex),
		LUABIND_FUNC(getClientCfg),
		LUABIND_FUNC(fileExists),
		LUABIND_FUNC(sendMsgToServer),
		LUABIND_FUNC(sendMsgToServerPvpTag),
		LUABIND_FUNC(isGuildQuitAvailable),
		LUABIND_FUNC(sortGuildMembers),
		LUABIND_FUNC(getNbGuildMembers),
		LUABIND_FUNC(getGuildMemberName),
		LUABIND_FUNC(getGuildMemberGrade),
		LUABIND_FUNC(isR2Player),
		LUABIND_FUNC(getR2PlayerRace),
		LUABIND_FUNC(isR2PlayerMale),
		LUABIND_FUNC(getCharacterSheetSkel),
		LUABIND_FUNC(getSheetId),
		LUABIND_FUNC(getCharacterSheetRegionForce),
		LUABIND_FUNC(getCharacterSheetRegionLevel),
		LUABIND_FUNC(getRegionByAlias),
		LUABIND_FUNC(tell),
		LUABIND_FUNC(isRingAccessPointInReach),
		LUABIND_FUNC(updateTooltipCoords),
		LUABIND_FUNC(isCtrlKeyDown),
		LUABIND_FUNC(encodeURLUnicodeParam),
		LUABIND_FUNC(getPlayerLevel),
		LUABIND_FUNC(getPlayerVpa),
		LUABIND_FUNC(getPlayerVpb),
		LUABIND_FUNC(getPlayerVpc),
		LUABIND_FUNC(getTargetLevel),
		LUABIND_FUNC(getTargetForceRegion),
		LUABIND_FUNC(getTargetLevelForce),
		LUABIND_FUNC(getTargetSheet),
		LUABIND_FUNC(getTargetVpa),
		LUABIND_FUNC(getTargetVpb),
		LUABIND_FUNC(getTargetVpc),
		LUABIND_FUNC(isTargetNPC),
		LUABIND_FUNC(isTargetPlayer),
		LUABIND_FUNC(isTargetUser),
		LUABIND_FUNC(isPlayerInPVPMode),
		LUABIND_FUNC(isTargetInPVPMode)
	];
	
}

// ***************************************************************************
static sint32 getTargetSlotNr()
{
	const char *dbPath = "UI:VARIABLES:TARGET:SLOT";
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp(dbPath, false);
	if (!node) return 0;
	if ((uint8) node->getValue32() == (uint8) CLFECOMMON::INVALID_SLOT)
	{
		return 0;
	}
	return node->getValue32();
}

static CEntityCL *getTargetEntity()
{
	const char *dbPath = "UI:VARIABLES:TARGET:SLOT";
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp(dbPath, false);
	if (!node) return NULL;
	if ((uint8) node->getValue32() == (uint8) CLFECOMMON::INVALID_SLOT)
	{
		return NULL;
	}
	return EntitiesMngr.entity((uint) node->getValue32());
}


static CEntityCL *getSlotEntity(uint slot)
{
	return EntitiesMngr.entity(slot);
}


int	CLuaIHMRyzom::getUI(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getUI)
	// params: "ui:interface:...".
	// return: CInterfaceElement*  (nil if error)
	const char *funcName = "getUI";
	CLuaIHM::check(ls,  ls.getTop() == 1 || ls.getTop() == 2, funcName);
	CLuaIHM::checkArgType(ls,   funcName, 1, LUA_TSTRING);
	bool verbose = true;
	if (ls.getTop() > 1)
	{
		CLuaIHM::checkArgType(ls,   funcName, 2, LUA_TBOOLEAN);
		verbose = ls.toBoolean(2);
	}

	// get the string
	std::string	eltStr;
	ls.toString(1,    eltStr);

	// return the element
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CInterfaceElement	*pIE= CWidgetManager::getInstance()->getElementFromId(eltStr);
	if(!pIE)
	{
		ls.pushNil();
		if (verbose)
		{
			std::string stackContext;
			ls.getStackContext(stackContext,   1);
			debugInfo( NLMISC::toString("%s : getUI(): '%s' not found",    stackContext.c_str(),   eltStr.c_str()));
		}
	}
	else
	{
		pushUIOnStack(ls,    pIE);
	}
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::setCaptureKeyboard(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setCaptureKeyboard)
	const char *funcName = "setCaptureKeyboard";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	checkArgTypeUIElement(ls, funcName, 1);
	CCtrlBase *ctrl = dynamic_cast<CCtrlBase *>( getUIOnStack(ls, 1));
	if (!ctrl)
	{
		CLuaIHM::fails(ls, "%s waits a ui control as arg 1", funcName);
	}
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->setCaptureKeyboard(ctrl);
	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::resetCaptureKeyboard(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_resetCaptureKeyboard)
	const char *funcName = "resetCaptureKeyboard";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->resetCaptureKeyboard();
	return 0;
}

// ***************************************************************************
int	CLuaIHMRyzom::setOnDraw(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setOnDraw)
	CLuaStackChecker lsc(&ls,    0);

	// params: CInterfaceGroup*,    "script".
	// return: none
	CLuaIHM::checkArgCount(ls,    "setOnDraw",    2);
	CLuaIHM::check(ls,   isUIOnStack(ls,    1),    "setOnDraw() requires a UI object in param 1");
	CLuaIHM::check(ls,   ls.isString(2),    "setOnDraw() requires a string in param 2");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
	std::string			script;
	ls.toString(2,    script);

	// must be a group
	CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>(pIE);
	if(!group)
		throw ELuaIHMException("setOnDraw(): '%s' is not a group",    pIE->getId().c_str());
	// Set the script to be executed at each draw
	group->setLuaScriptOnDraw(script);

	return 0;
}

// ***************************************************************************
int	CLuaIHMRyzom::addOnDbChange(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_addOnDbChange)
	CLuaStackChecker lsc(&ls,    0);

	// params: CInterfaceGroup*,    "dblist",    "script".
	// return: none
	CLuaIHM::checkArgCount(ls,    "addOnDbChange",    3);
	CLuaIHM::check(ls,   isUIOnStack(ls,    1),    "addOnDbChange() requires a UI object in param 1");
	CLuaIHM::check(ls,   ls.isString(2),    "addOnDbChange() requires a string in param 2");
	CLuaIHM::check(ls,   ls.isString(3),    "addOnDbChange() requires a string in param 3");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
	std::string			dbList,    script;
	ls.toString(2,    dbList);
	ls.toString(3,    script);

	// must be a group
	CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>(pIE);
	if(!group)
		throw ELuaIHMException("addOnDbChange(): '%s' is not a group",    pIE->getId().c_str());
	// Set the script to be executed when the given DB change
	group->addLuaScriptOnDBChange(dbList,    script);

	return 0;
}


// ***************************************************************************
int	CLuaIHMRyzom::removeOnDbChange(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_removeOnDbChange)
	CLuaStackChecker lsc(&ls,    0);

	// params: CInterfaceGroup*,    "dbList"
	// return: none
	CLuaIHM::checkArgCount(ls,    "removeOnDbChange",    2);
	CLuaIHM::check(ls,   isUIOnStack(ls,    1),    "removeOnDbChange() requires a UI object in param 1");
	CLuaIHM::check(ls,   ls.isString(2),    "removeOnDbChange() requires a string in param 2");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
	std::string			dbList;
	ls.toString(2,    dbList);

	// must be a group
	CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>(pIE);
	if(!group)
		throw ELuaIHMException("removeOnDbChange(): '%s' is not a group",    pIE->getId().c_str());
	// Remove the script to be executed when the given DB change
	group->removeLuaScriptOnDBChange(dbList);

	return 0;
}


// ***************************************************************************
int	CLuaIHMRyzom::runAH(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_runAH)
	CLuaStackChecker lsc(&ls,    0);

	// params: CInterfaceElement *,    "ah",    "params".
	// return: none
	CLuaIHM::checkArgCount(ls,    "runAH",    3);
	CLuaIHM::check(ls,   isUIOnStack(ls,    1) || ls.isNil(1),    "runAH() requires a UI object in param 1 (or Nil)");
	CLuaIHM::check(ls,   ls.isString(2),    "runAH() requires a string in param 2");
	CLuaIHM::check(ls,   ls.isString(3),    "runAH() requires a string in param 3");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
	std::string			ah,    params;
	ls.toString(2,    ah);
	ls.toString(3,    params);

	// run AH
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	// The element must be ctrl (or NULL)
	CCtrlBase	*ctrl= NULL;
	if(pIE)
	{
		ctrl= dynamic_cast<CCtrlBase*>(pIE);
		if(!ctrl)
			throw ELuaIHMException("runAH(): '%s' is not a ctrl",    pIE->getId().c_str());
	}
	CAHManager::getInstance()->runActionHandler(ah,    ctrl,    params);

	return 0;
}

// ***************************************************************************
int	CLuaIHMRyzom::runExpr(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_runExpr)
	CLuaStackChecker lsc(&ls,    1);

	// params: "expr".
	// return: any of: nil,   bool,   string,   number,    RGBA,    UCString
	CLuaIHM::checkArgCount(ls,    "runExpr",    1);
	CLuaIHM::check(ls,   ls.isString(1),    "runExpr() requires a string in param 1");

	// retrieve args
	std::string expr;
	ls.toString(1,    expr);

	// run expression and push result
	return runExprAndPushResult(ls,    expr);
}

// ***************************************************************************
int		CLuaIHMRyzom::runFct(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_runFct)
	CLuaStackChecker lsc(&ls,    1);

	// params: "expr",    param1,    param2...
	// return: any of: nil,   bool,   string,   number,    RGBA,    UCString
	CLuaIHM::checkArgMin(ls,    "runFct",    1);
	CLuaIHM::check(ls,   ls.isString(1),    "runExpr() requires a string in param 1");

	// retrieve fct
	std::string expr;
	ls.toString(1,    expr);
	expr+= "(";

	// retrieve params
	uint	top= ls.getTop();
	for(uint i=2;i<=top;i++)
	{
		if(i>2)
			expr+= ",   ";

		// If it is a number
		if(ls.type(i)==LUA_TNUMBER)
		{
			std::string	paramValue;
			ls.toString(i,    paramValue);		// nb: transformed to a string in the stack
			expr+= paramValue;
		}
		// else suppose a string
		else
		{
			// must enclose with "'"
			std::string	paramValue;
			ls.toString(i,    paramValue);
			expr+= std::string("'") + paramValue + std::string("'") ;
		}
	}

	// end fct call
	expr+= ")";


	// run expression and push result
	return runExprAndPushResult(ls,    expr);
}

// ***************************************************************************
int CLuaIHMRyzom::runCommand(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_runCommand)
	CLuaStackChecker lsc(&ls,    1);
	if (ls.empty())
	{
		nlwarning("'runCommand' : Command name expected");
		ls.push(false);
		return 1;
	}
	const char *commandName = ls.toString(1);
	if (!commandName)
	{
		nlwarning("'runCommand' : Bad command name");
		ls.push(false);
		return 1;
	}
	if (!NLMISC::ICommand::LocalCommands || !NLMISC::ICommand::LocalCommands->count(ls.toString(1)))
	{
		nlwarning("'runCommand' : Command %s not found",    ls.toString(1));
		ls.push(false);
		return 1;
	}
	std::string rawCommandString = ls.toString(1);
	NLMISC::ICommand *command = (*NLMISC::ICommand::LocalCommands)[ls.toString(1)];
	nlassert(command);
	std::vector<std::string> args(ls.getTop() - 1);
	for(uint k = 2; k <= (uint) ls.getTop(); ++k)
	{
		if (ls.toString(k))
		{
			args[k - 2] = ls.toString(k);
			rawCommandString += " " + std::string(ls.toString(k));
		}
	}

	ls.push(command->execute(rawCommandString,   args,    g_log,    false,    true));
	return 1;
}

// ***************************************************************************
int		CLuaIHMRyzom::formatUI(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_formatUI)
	CLuaStackChecker lsc(&ls,    1);

	// params: "expr",    param1,    param2....
	// return: string with # and % parsed
	CLuaIHM::checkArgMin(ls,    "formatUI",    1);
	CLuaIHM::check(ls,   ls.isString(1),    "formatUI() require a string in param1");

	// get the string to format
	std::string	propVal;
	ls.toString(1,    propVal);

	// *** format with %
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	std::string	newPropVal,    defError;
	if(!pIM->solveDefine(propVal,    newPropVal,    defError))
	{
		throw ELuaIHMException("formatUI(): Can't find define: '%s'",    defError.c_str());
	}

	// *** format with any additional parameter and #1,    #2,    #3 etc...
	// search backward,    starting from bigger param to replace (thus avoid to replace #1 before #13 for instance...)
	sint	stackIndex= ls.getTop();
	while(stackIndex>1)
	{
		std::string	paramValue;
		ls.toString(stackIndex,    paramValue);

		// For stack param 4,    the param index is 3 (because stack param 2 is the param No 1)
		sint	paramIndex= stackIndex-1;
		while(NLMISC::strFindReplace(newPropVal,    NLMISC::toString("#%d",    paramIndex),    paramValue));

		// next
		stackIndex--;
	}

	// return result
	ls.push(newPropVal);
	return 1;
}

// ***************************************************************************
int		CLuaIHMRyzom::formatDB(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_formatDB)
	CLuaStackChecker lsc(&ls,    1);

	// params: param1,    param2....
	// return: string with @ and ,    added
	CLuaIHM::checkArgMin(ls,    "formatDB",    1);
	uint	top= ls.getTop();

	std::string	dbRes;
	for(uint i=1;i<=top;i++)
	{
		if(i==1)
			dbRes= "@";
		else
			dbRes+= ",   @";

		std::string	paramValue;
		ls.toString(i,    paramValue);
		dbRes+= paramValue;
	}

	// return result
	ls.push(dbRes);
	return 1;
}

// ***************************************************************************
int	CLuaIHMRyzom::deleteUI(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_deleteUI)
	CLuaStackChecker lsc(&ls,    0);

	// params: CInterfaceElement *
	// return: none
	CLuaIHM::checkArgCount(ls,    "deleteUI",    1);
	CLuaIHM::check(ls,   isUIOnStack(ls,    1),    "deleteUI() requires a UI object in param 1");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
	if(!pIE)
		return 0;

	// has a parent?
	CInterfaceGroup	*parent= pIE->getParent();
	if(parent)
	{
		// correctly remove from parent
		parent->delElement(pIE);
	}
	else
	{
		// just delete
		delete pIE;
	}

	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::deleteReflectable(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_deleteReflectable)
	CLuaStackChecker lsc(&ls,    0);

	// params: CInterfaceElement *
	// return: none
	CLuaIHM::checkArgCount(ls,    "deleteReflectable",    1);
	CLuaIHM::check(ls,   CLuaIHM::isReflectableOnStack(ls,    1),    "deleteReflectable() requires a reflectable C++ object in param 1");

	// retrieve args
	CReflectableRefPtrTarget	*pRPT= CLuaIHM::getReflectableOnStack(ls,    1);
	if(!pRPT)
		return 0;


	CInterfaceElement *pIE = dynamic_cast<CInterfaceElement *>(pRPT);

	if (pIE)
	{
		// has a parent?
		CInterfaceGroup	*parent= pIE->getParent();
		if(parent)
		{
			// correctly remove from parent
			parent->delElement(pIE);
		}
	}

	// just delete
	delete pIE;

	return 0;
}

// ***************************************************************************
int	CLuaIHMRyzom::dumpUI(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_dumpUI)
	CLuaStackChecker lsc(&ls,    0);

	// params: CInterfaceElement *
	// return: none
	CLuaIHM::checkArgCount(ls,    "dumpUI",    1);
	CLuaIHM::check(ls,   isUIOnStack(ls,    1),    "dumpUI() requires a UI object in param 1");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
	if(!pIE)
		debugInfo("UI: NULL");
	else
	{
		// Display also Information on RefPtr (warning: don't modify pinfo!!!)
		nlassert(pIE->pinfo);
		debugInfo(NLMISC::toString("UI: %x. %s. RefPtrCount: %d",    pIE,    pIE->getId().c_str(),
			pIE->pinfo->IsNullPtrInfo?0:pIE->pinfo->RefCount));
	}

	return 0;
}

// ***************************************************************************
int	CLuaIHMRyzom::setKeyboardContext(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setKeyboardContext)
	const char *funcName = "setKeyboardContext";
	CLuaIHM::checkArgMin(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);

	ActionsContext.setContext(ls.toString(1));

	return 0;
}


// ***************************************************************************
int CLuaIHMRyzom::validMessageBox(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_validMessageBox)
	const char *funcName = "validMessageBox";
	CLuaIHM::checkArgCount(ls, funcName, 6);
	ucstring msg;
	ls.pushValue(1); // copy ucstring at the end of stack to pop it
	CLuaIHM::check(ls, CLuaIHM::pop(ls, msg), "validMessageBox : ucstring wanted as first parameter");
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 4, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 5, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 6, LUA_TSTRING);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->validMessageBox(CInterfaceManager::QuestionIconMsg, msg, ls.toString(2), ls.toString(3), ls.toString(4), ls.toString(5), ls.toString(6));
	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::setTopWindow(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setTopWindow)
	const char *funcName = "setTopWindow";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CInterfaceGroup *wnd = dynamic_cast<CInterfaceGroup *>( getUIOnStack(ls, 1));
	if (!wnd)
	{
		CLuaIHM::fails(ls, "%s : interface group expected as arg 1", funcName);
	}
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CWidgetManager::getInstance()->setTopWindow(wnd);
	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::concatUCString(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_concatUCString)
	const char *funcName = "concatUCString";
	ucstring result;
	for (uint k = 1; k <= (uint) ls.getTop(); ++k)
	{
		//nlwarning("arg %d = %s", k, ls.getTypename(ls.type(k)));
		ucstring part;
		if (ls.isString(k))
		{
			part.fromUtf8(ls.toString(k));
		}
		else
		{
			CLuaIHM::checkArgTypeUCString(ls, funcName, k);
			nlverify(CLuaIHM::getUCStringOnStack(ls, k, part));
		}
		result += part;
	}
	CLuaIHM::push(ls, result);
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::concatString(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_concatUCString)
	const char *funcName = "concatString";
	std::string result;
	uint stackSize = ls.getTop();
	for (uint k = 1; k <= stackSize; ++k)
	{
		CLuaIHM::checkArgType(ls, funcName, k, LUA_TSTRING);
		result += ls.toString(k);
	}
	ls.push(result);
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::tableToString(CLuaState &ls)
{
	const char *funcName = "tableToString";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TTABLE);
	uint length = 0;
	// compute size
	ls.pushNil();
	while (ls.next(-2))
	{
		ls.toString(-1);
		length += (uint)ls.strlen(-1);
		ls.pop(2);
	}
	std::string result;
	result.resize(length);
	char *dest = &result[0];
	// concatenate
	ls.pushNil();
	while (ls.next(-2))
	{
		uint length = (uint)ls.strlen(-1);
		if (length)
		{
			memcpy(dest, ls.toString(-1), length);
		}
		dest += length;
		ls.pop(2);
	}
	ls.push(result);
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::breakPoint(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_breakPoint)
	std::string reason;
	ls.getStackContext(reason,   1);		// 1 because 0 is the current C function => return 1 for script called
	LuaHelperStuff::formatLuaStackContext(reason);
	NLMISC::InfoLog->displayRawNL(reason.c_str());
	static volatile bool doAssert = true;
	if (doAssert) // breakPoint can be discarded in case of looping assert
	{
		NLMISC_BREAKPOINT;
	}
	return 0;
}



// ***************************************************************************
int CLuaIHMRyzom::getWindowSize(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getWindowSize)
	CLuaIHM::checkArgCount(ls,   "getWindowSize",   0);
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	uint32 w,   h;
	pIM->getViewRenderer().getScreenSize(w,   h);
	ls.push((double) w);
	ls.push((double) h);
	return 2;
}


// ***************************************************************************
int	CLuaIHMRyzom::setTextFormatTaged(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setTextFormatTaged)
	// params: CViewText*,    "text" (or ucstring)
	// return: none
	CLuaIHM::checkArgCount(ls,    "setTextFormatTaged",    2);

	// *** check and retrieve param 1
	CLuaIHM::check(ls,   isUIOnStack(ls,    1),    "setTextFormatTaged() requires a UI object in param 1");
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);

	// *** check and retrieve param 2. must be a string or a ucstring
	ucstring	text;
	if(ls.isString(2))
	{
		std::string			str;
		ls.toString(2,    str);
		text= str;
	}
	else
	{
		// try to pop a ucstring from the stack
		// fail?
		if(!CLuaIHM::pop(ls, text))
		{
			CLuaIHM::check(ls,   false,    "setTextFormatTaged() requires a string or a ucstring in param 2");
		}
	}

	// must be a view text
	CViewText	*vt= dynamic_cast<CViewText*>(pIE);
	if(!vt)
		throw ELuaIHMException("setTextFormatTaged(): '%s' is not a CViewText",    pIE->getId().c_str());

	// Set the text as format
	vt->setTextFormatTaged(text);

	return 0;
}


struct CEmoteStruct
{
	string EmoteId;
	string Path;
	string Anim;
	bool   UsableFromClientUI;

	bool operator< (const CEmoteStruct & entry) const
	{
		string path1 = Path;
		string path2 = entry.Path;

		for(;;)
		{
			string::size_type pos1 = path1.find('|');
			string::size_type pos2 = path2.find('|');

			ucstring s1 = toUpper(CI18N::get(path1.substr(0, pos1)));
			ucstring s2 = toUpper(CI18N::get(path2.substr(0, pos2)));

			sint result = s1.compare(s2);
			if (result != 0)
				return (result < 0);

			if (pos1 == string::npos)
				return (pos2 != string::npos);
			if (pos2 == string::npos)
				return false;

			path1 = path1.substr(pos1 + 1);
			path2 = path2.substr(pos2 + 1);
		}
		return false;
	}
};

// ***************************************************************************
int CLuaIHMRyzom::initEmotesMenu(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_initEmotesMenu)
	CLuaIHM::checkArgCount(ls, "initEmotesMenu", 2);
	CLuaIHM::checkArgType(ls, "initEmotesMenu", 2, LUA_TSTRING);
	const std::string & emoteMenu = ls.toString(1);
	const std::string & luaParams = ls.toString(2);

	ls.newTable();
	CLuaObject result(ls);
	std::map<std::string, std::string> emoteList;
	uint maxVisibleLine=10;

	CTextEmotListSheet *pTELS = dynamic_cast<CTextEmotListSheet*>(SheetMngr.get(CSheetId("list.text_emotes")));
	if (pTELS == NULL)
		return 0;

	std::list<CEmoteStruct> entries;
	if (entries.empty())
	{
		for (uint i = 0; i < pTELS->TextEmotList.size(); i++)
		{
			CEmoteStruct entry;
			entry.EmoteId = pTELS->TextEmotList[i].EmoteId;
			entry.Path = pTELS->TextEmotList[i].Path;
			entry.Anim = pTELS->TextEmotList[i].Anim;
			entry.UsableFromClientUI = pTELS->TextEmotList[i].UsableFromClientUI;
			entries.push_back(entry);
		}
		entries.sort();
	}

	// The list of behaviour missnames emotList
	CEmotListSheet *pEmotList = dynamic_cast<CEmotListSheet*>(SheetMngr.get(CSheetId("list.emot")));
	nlassert (pEmotList != NULL);
	nlassert (pEmotList->Emots.size() <= 255);

	// Get the focus beta tester flag
	bool betaTester = false;

	CInterfaceManager	*pIM = CInterfaceManager::getInstance();
	CSkillManager		*pSM = CSkillManager::getInstance();

	betaTester = pSM->isTitleUnblocked(CHARACTER_TITLE::FBT);

	CGroupMenu *pInitRootMenu = dynamic_cast<CGroupMenu*>(CWidgetManager::getInstance()->getElementFromId(emoteMenu));
	pInitRootMenu->reset();

	for (std::list<CEmoteStruct>::const_iterator it = entries.begin(); it != entries.end(); it++)
	{
		std::string sEmoteId = (*it).EmoteId;
		std::string sState = (*it).Anim;
		std::string sName = (*it).Path;

		// Check that the emote can be added to UI
		// ---------------------------------------
		if( (*it).UsableFromClientUI == false )
		{
			continue;
		}

		// Check the emote reserved for FBT (hardcoded)
		// --------------------------------------------
		if (sState == "FBT" && !betaTester)
			continue;

		uint32 i, j;
		// Add to the game context menu
		// ----------------------------
		uint32 nbToken = 1;
		for (i = 0; i < sName.size(); ++i)
			if (sName[i] == '|')
				nbToken++;

		CGroupMenu *pRootMenu = dynamic_cast<CGroupMenu*>(CWidgetManager::getInstance()->getElementFromId(emoteMenu));
		CGroupSubMenu *pMenu = pRootMenu->getRootMenu();

		for (i = 0; i < nbToken; ++i)
		{
			if(i==0)
			{
				sName = sName.substr(sName.find('|')+1,sName.size());
			}
			else
			{
				string sTmp;
				if (i != (nbToken-1))
					sTmp = sName.substr(0,sName.find('|'));
				else
					sTmp = sName;



				// Look if this part of the path is already present
				bool bFound = false;
				for (j = 0; j < pMenu->getNumLine(); ++j)
				{
					if (sTmp == pMenu->getLineId(j))
					{
						bFound = true;
						break;
					}
				}

				if (!bFound) // Create it
				{
					if (i != (nbToken-1))
					{
						pMenu->addLine (CI18N::get(sTmp), "", "", sTmp);
						// Create a sub menu
						CGroupSubMenu *pNewSubMenu = new CGroupSubMenu(CViewBase::TCtorParam());
						pMenu->setSubMenu(j, pNewSubMenu);
					}
					else
					{
						// Create a line
						pMenu->addLine (CI18N::get(sTmp), "lua",
							luaParams+"('"+sEmoteId+"', '"+toString(CI18N::get(sTmp))+"')", sTmp);
						emoteList[sEmoteId] = (toLower(CI18N::get(sTmp))).toUtf8();
					}
				}

				// Jump to sub menu
				if (i != (nbToken-1))
				{
					pMenu = pMenu->getSubMenu(j);
					sName = sName.substr(sName.find('|')+1,sName.size());
				}
			}
		}
		pMenu->setMaxVisibleLine(maxVisibleLine);
	}
	pInitRootMenu->setMaxVisibleLine(maxVisibleLine);

	std::map<std::string, std::string>::iterator it;
	for(it=emoteList.begin(); it!=emoteList.end(); it++)
	{
		result.setValue(it->first, it->second);
	}
	result.push();

	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::isUCString(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_isUCString)
	const char *funcName = "isUCString";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	ls.push(CLuaIHM::isUCStringOnStack(ls, 1));
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::hideAllWindows(CLuaState &/* ls */)
{
	//H_AUTO(Lua_CLuaIHM_hideAllWindows)
	CInterfaceManager::getInstance()->hideAllWindows();
	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::hideAllNonSavableWindows(CLuaState &/* ls */)
{
	//H_AUTO(Lua_CLuaIHM_hideAllNonSavableWindows)
	CInterfaceManager::getInstance()->hideAllNonSavableWindows();
	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::getDesktopIndex(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getDesktopIndex)
	ls.push((double) CInterfaceManager::getInstance()->getMode());
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::setLuaBreakPoint(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setLuaBreakPoint)
	const char *funcName = "setLuaBreakPoint";
	CLuaIHM::checkArgCount(ls, funcName, 2);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);


	#ifdef LUA_NEVRAX_VERSION
		if (LuaDebuggerIDE)
		{
			LuaDebuggerIDE->setBreakPoint(ls.toString(1), (int) ls.toNumber(2));
		}
	#endif

	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::getMainPageURL(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getMainPageURL)
	const char *funcName = "getMainPageURL";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	ls.push(RingMainURL);
	return 1;
}

// ***************************************************************************
int	CLuaIHMRyzom::getCharSlot(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getCharSlot)
	const char *funcName = "getCharSlot";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	ls.push(double(PlayerSelectedSlot));
	return 1;
}


int CLuaIHMRyzom::getPathContent(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getPathContent)
	const char *funcName = "getPathContent";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	std::vector<std::string> files;
	NLMISC::CPath::getPathContent(ls.toString(1), false, false, true, files);
	ls.newTable();
	for(uint k = 0; k < files.size(); ++k)
	{
		ls.push((double) k);
		ls.push(files[k]);
		ls.setTable(-3);
	}
	return 1;
}

int CLuaIHMRyzom::getServerSeason(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getServerSeason)
	const char *funcName = "getServerSeason";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	extern uint8 ServerSeasonValue;
	ls.push((double) ServerSeasonValue);
	return 1;
}

int CLuaIHMRyzom::computeCurrSeason(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_computeCurrSeason)
	const char *funcName = "computeCurrSeason";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	ls.push((double) (::computeCurrSeason() + 1));
	return 1;
}

int CLuaIHMRyzom::getAutoSeason(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getAutoSeason)
	const char *funcName = "getAutoSeason";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	ls.push((double) (StartupSeason + 1));
	return 1;
}



int CLuaIHMRyzom::getTextureSize(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getTextureSize)
	const char *funcName = "getTextureSize";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	std::string textureName = ls.toString(1);

	CBitmap bitmap;
	CIFile fs(CPath::lookup(textureName).c_str());
	bitmap.load(fs);

	ls.push((double) bitmap.getWidth());
	ls.push((double) bitmap.getHeight());

	return 2;
}


int CLuaIHMRyzom::enableModalWindow(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_enableModalWindow)
	const char *funcName = "enableModalWindow";
	CLuaIHM::checkArgCount(ls, funcName, 2);

	CLuaIHM::check(ls,   isUIOnStack(ls, 1), "enableModalWindow() requires a UI object in param 1");
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);

	CInterfaceElement	*pIE= getUIOnStack(ls, 1);
	std::string modalId = ls.toString(2);

	// convert to id
	if(pIE)
	{
		CCtrlBase * ctrl = dynamic_cast<CCtrlBase*>(pIE);
		if(ctrl)
		{
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();
			CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(modalId) );
			if(group)
			{
				UserControls.stopFreeLook();

				// enable the modal
				CWidgetManager::getInstance()->enableModalWindow(ctrl, group);
			}
			else
			{
				nlwarning("<CLuaIHMRyzom::enableModalWindow> Couldn't find group %s", modalId.c_str());
			}

		}
	}

	return 0;
}

// ***************************************************************************
int		CLuaIHMRyzom::disableModalWindow(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_disableModalWindow)
	CLuaIHM::checkArgCount(ls, "disableModalWindow", 0);
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CWidgetManager::getInstance()->disableModalWindow();
	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::getPlayerPos(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getPlayerPos)
	CLuaIHM::checkArgCount(ls, "getPlayerPos", 0);
	ls.push(UserEntity->pos().x);
	ls.push(UserEntity->pos().y);
	ls.push(UserEntity->pos().z);
	return 3;
}

// ***************************************************************************
int CLuaIHMRyzom::getPlayerFront(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getPlayerFront", 0);
	ls.push(atan2(UserEntity->front().y, UserEntity->front().x));
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getPlayerDirection(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getPlayerDirection", 0);
	ls.push(atan2(UserEntity->dir().y, UserEntity->dir().x));
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getPlayerGender(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getPlayerGender", 0);
	ls.push((lua_Number)(UserEntity->getGender()));
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getPlayerName(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getPlayerName", 0);
	ls.push(UserEntity->getEntityName().toUtf8());
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getPlayerTitleRaw(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getPlayerTitleRaw", 0);
	ls.push(UserEntity->getTitleRaw().toUtf8());
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getPlayerTitle(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getPlayerTitle", 0);
	ls.push(UserEntity->getTitle().toUtf8());
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getTargetPos(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getTargetPos", 0);
	CEntityCL *target = getTargetEntity();
	if (!target) return 0;
	ls.push(target->pos().x);
	ls.push(target->pos().y);
	ls.push(target->pos().z);
	return 3;
}

// ***************************************************************************
int CLuaIHMRyzom::getTargetFront(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getTargetFront", 0);
	CEntityCL *target = getTargetEntity();
	if (!target) return 0;
	ls.push(atan2(target->front().y, target->front().x));
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getTargetDirection(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getTargetDirection", 0);
	CEntityCL *target = getTargetEntity();
	if (!target) return 0;
	ls.push(atan2(target->dir().y, target->dir().x));
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getTargetGender(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getTargetGender", 0);
	CCharacterCL* target = (CCharacterCL*)getTargetEntity();
	if (!target) return (int)GSGENDER::unknown;
	ls.push((lua_Number)(target->getGender()));
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getTargetName(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getTargetName", 0);
	CEntityCL *target = getTargetEntity();
	if (!target) return 0;
	ls.push(target->getEntityName().toUtf8());
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getTargetTitleRaw(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getTargetTitleRaw", 0);
	CEntityCL *target = getTargetEntity();
	if (!target) return 0;
	ls.push(target->getTitleRaw().toUtf8());
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getTargetTitle(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getTargetTitle", 0);
	CEntityCL *target = getTargetEntity();
	if (!target) return 0;
	ls.push(target->getTitle().toUtf8());
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::addSearchPathUser(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_addSearchPathUser)
	bool memoryCompressed = CPath::isMemoryCompressed();
	if (memoryCompressed)
	{
		CPath::memoryUncompress();
	}
	CPath::addSearchPath("user/", true, false, NULL);
	if (memoryCompressed)
	{
		CPath::memoryCompress();
	}
	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::isPlayerFreeTrial(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "isPlayerFreeTrial", 0);
	ls.push(FreeTrial);
	return 1;
}



// ***************************************************************************
int CLuaIHMRyzom::disableContextHelp(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_disableContextHelp)
	CLuaStackChecker lsc(&ls,    0);
	CLuaIHM::checkArgCount(ls,    "disableContextHelp",    0);
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->disableContextHelp();
	return 0;
}

// ***************************************************************************
int			CLuaIHMRyzom::disableContextHelpForControl(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_disableContextHelpForControl)
	CLuaStackChecker lsc(&ls,    0);

	// params: CCtrlBase*
	// return: none
	CLuaIHM::checkArgCount(ls,    "disableContextHelpForControl",    1);
	CLuaIHM::check(ls,   isUIOnStack(ls,   1),    "disableContextHelpForControl() requires a UI object in param 1");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);

	// go
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->disableContextHelpForControl(dynamic_cast<CCtrlBase*>(pIE));

	return 0;
}


// ***************************************************************************
int CLuaIHMRyzom::isPlayerNewbie(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "isPlayerNewbie", 0);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	ls.push(NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:IS_NEWBIE")->getValueBool());
	return 1;
}


// ***************************************************************************
int CLuaIHMRyzom::isInRingMode(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "isInRingMode", 0);
	extern bool IsInRingMode();
	ls.push(IsInRingMode());
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getUserRace(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getUserRace", 0);
	if (!UserEntity || !UserEntity->playerSheet())
	{
		ls.push("Unknwown");
	}
	else
	{
		ls.push(EGSPD::CPeople::toString(UserEntity->playerSheet()->People));
	}
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getSheet2idx(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getSheet2idx", 2);
	CLuaIHM::checkArgType(ls, "getSheet2idx", 1, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, "getSheet2idx", 2, LUA_TNUMBER);

	const std::string & sheedtName = ls.toString(1);
	uint32 slotId = (uint32)ls.toNumber(2);

	NLMISC::CSheetId sheetId;

	if (sheetId.buildSheetId(sheedtName))
	{
		uint32 idx = CVisualSlotManager::getInstance()->sheet2Index(sheetId, (SLOTTYPE::EVisualSlot)slotId);
		ls.push((lua_Number)idx);
	}
	else
		return 0;
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getTargetSlot(CLuaState &ls)
{
	uint32 slot = (uint32)getTargetSlotNr();
	ls.push((lua_Number)slot);
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getSlotDataSetId(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getSlotDataSetId", 1);
	CLuaIHM::checkArgType(ls, "getSlotDataSetId", 1, LUA_TNUMBER);

	uint32 slot = (uint32)ls.toNumber(1);
	CEntityCL *e = getSlotEntity(slot);
	string id = toString(e->dataSetId());
	ls.push(id);
	return 1;
}

int CLuaIHMRyzom::getClientCfgVar(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getClientCfgVar)
	const char *funcName = "getClientCfgVar";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	std::string varName = ls.toString(1);

	CConfigFile::CVar *v = ClientCfg.ConfigFile.getVarPtr(varName);
	if (!v) return 0;
	if(v->size()==1)
	{
		switch(v->Type)
		{
			case CConfigFile::CVar::T_REAL:
				ls.push((double) v->asDouble());
				return 1;
			break;
			case CConfigFile::CVar::T_STRING:
				ls.push(v->asString());
				return 1;
			break;
			default: // handle both T_INT && T_BOOL
			case CConfigFile::CVar::T_INT:
				ls.push((double) v->asInt());
				return 1;
			break;
		}
	}
	else
	{
		ls.newTable();
		CLuaObject result(ls);
		uint count = 0;
		for(uint i = 0; i<v->StrValues.size(); i++)
		{
			result.setValue(toString(count).c_str(), v->StrValues[i]);
			count++;
		}
		for(uint i = 0; i<v->IntValues.size(); i++)
		{
			result.setValue(toString(count).c_str(), (double)v->IntValues[i]);
			count++;
		}
		for(uint i = 0; i<v->RealValues.size(); i++)
		{
			result.setValue(toString(count).c_str(), (double)v->RealValues[i]);
			count++;
		}
		result.push();
		return 1;
	}

	return 0;
}

int CLuaIHMRyzom::displaySystemInfo(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_displaySystemInfo)
	const char *funcName = "displaySystemInfo";
	CLuaIHM::checkArgCount(ls, funcName, 2);
	CLuaIHM::checkArgTypeUCString(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	ucstring msg;
	nlverify(CLuaIHM::getUCStringOnStack(ls, 1, msg));
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->displaySystemInfo(msg, ls.toString(2));
	return 0;
}

int CLuaIHMRyzom::setWeatherValue(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setWeatherValue)
	const char *funcName = "setWeatherValue";
	CLuaIHM::checkArgMin(ls, funcName, 1);
	CLuaIHM::checkArgMax(ls, funcName, 2);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TBOOLEAN);
//	bool autoWeather = ls.toBoolean(1);
	ClientCfg.ManualWeatherSetup = !ls.toBoolean(1);
	if (ls.getTop() == 2)
	{
		CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);
		ManualWeatherValue = (float) ls.toNumber(2);
	}
	return 0;
}

int CLuaIHMRyzom::getWeatherValue(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getWeatherValue)
	const char *funcName = "getWeatherValue";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	uint64 currDay = RT.getRyzomDay();
	float currHour = (float) RT.getRyzomTime();
	ls.push(::getBlendedWeather(currDay, currHour, *WeatherFunctionParams, ContinentMngr.cur()->WeatherFunction));
	return 1;
}

int	CLuaIHMRyzom::getUICaller(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getUICaller)
	CLuaStackChecker lsc(&ls,    1);

	// params: none.
	// return: CInterfaceElement*  (nil if error)
	CInterfaceElement	*pIE= CHandlerLUA::getUICaller();
	if(!pIE)
	{
		ls.pushNil();
		debugInfo(toString("getUICaller(): No UICaller found. return Nil"));
	}
	else
	{
		pushUIOnStack(ls,    pIE);
	}
	return 1;
}

int CLuaIHMRyzom::getCurrentWindowUnder(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getCurrentWindowUnder)
	CLuaStackChecker lsc(&ls,    1);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CInterfaceElement	*pIE= im->getCurrentWindowUnder();
	if(!pIE)
	{
		ls.pushNil();
		debugInfo(toString("getCurrentWindowUnder(): No UICaller found. return Nil"));
	}
	else
	{
		pushUIOnStack(ls,    pIE);
	}
	return 1;
}

// ***************************************************************************
int	CLuaIHMRyzom::getUIId(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getUIId)
	CLuaStackChecker lsc(&ls,    1);

	// params: CInterfaceElement*
	// return: "ui:interface:...". (empty if error)
	CLuaIHM::checkArgCount(ls,    "getUIId",    1);
	CLuaIHM::check(ls,   isUIOnStack(ls,   1),    "getUIId() requires a UI object in param 1");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);

	// convert to id
	if(pIE)
		ls.push(pIE->getId());
	else
		ls.push("");

	return 1;
}

// ***************************************************************************
int	CLuaIHMRyzom::getIndexInDB(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getIndexInDB)
	CLuaStackChecker lsc(&ls,    1);

	// params: CDBCtrlSheet*
	// return: index in DB of a dbctrlsheet  (empty if error)
	CLuaIHM::checkArgCount(ls,    "getIndexInDB",    1);
	CLuaIHM::check(ls,   isUIOnStack(ls,   1),    "getIndexInDB() requires a UI object in param 1");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
	CDBCtrlSheet		*pCS= dynamic_cast<CDBCtrlSheet*>(pIE);

	// get the index in db
	if(pCS)
		ls.push((double)pCS->getIndexInDB());
	else
		ls.push(0.0);

	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::createGroupInstance(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_createGroupInstance)
	const char *funcName = "createGroupInstance";
	CLuaIHM::checkArgCount(ls, funcName, 3);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TTABLE);
	std::vector<std::pair<std::string, std::string> > templateParams;
	CLuaObject params;
	params.pop(ls);
	ENUM_LUA_TABLE(params, it)
	{
		if (!it.nextKey().isString())
		{
			nlwarning("%s : bad key encountered with type %s, string expected.", funcName, it.nextKey().getTypename());
			continue;
		}
		if (!it.nextValue().isString())
		{
			nlwarning("%s : bad value encountered with type %s for key %s, string expected.", funcName, it.nextValue().getTypename(), it.nextKey().toString().c_str());
			continue;
		}
		templateParams.push_back(std::pair<std::string, std::string>(it.nextKey().toString(), it.nextValue().toString())); // strange compilation bug here when I use std::make_pair ... :(
	}
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CInterfaceGroup *result = im->createGroupInstance(ls.toString(1), ls.toString(2), templateParams);
	if (!result)
	{
		ls.pushNil();
	}
	else
	{
		pushUIOnStack(ls, result);
	}
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::createRootGroupInstance(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_createGroupInstance)
	const char *funcName = "createRootGroupInstance";
	CLuaIHM::checkArgCount(ls, funcName, 3);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TTABLE);
	std::vector<std::pair<std::string, std::string> > templateParams;
	CLuaObject params;
	params.pop(ls);
	ENUM_LUA_TABLE(params, it)
	{
		if (!it.nextKey().isString())
		{
			nlwarning("%s : bad key encountered with type %s, string expected.", funcName, it.nextKey().getTypename());
			continue;
		}
		if (!it.nextValue().isString())
		{
			nlwarning("%s : bad value encountered with type %s for key %s, string expected.", funcName, it.nextValue().getTypename(), it.nextKey().toString().c_str());
			continue;
		}
		templateParams.push_back(std::pair<std::string, std::string>(it.nextKey().toString(), it.nextValue().toString())); // strange compilation bug here when I use std::make_pair ... :(
	}
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CInterfaceGroup *result = im->createGroupInstance(ls.toString(1), "ui:interface:"+string(ls.toString(2)), templateParams);
	if (!result)
	{
		ls.pushNil();
	}
	else
	{
		result->setId("ui:interface:"+string(ls.toString(2)));
		result->updateCoords();
		CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", result);
		CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
		result->setParent(pRoot);
		if (pRoot)
			pRoot->addGroup(result);
		result->setActive(true);
		pushUIOnStack(ls, result);
	}
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::createUIElement(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_createUIElement)
	const char *funcName = "addUIElement";
	CLuaIHM::checkArgCount(ls, funcName, 3);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TTABLE);
	std::vector<std::pair<std::string, std::string> > templateParams;
	CLuaObject params;
	params.pop(ls);
	ENUM_LUA_TABLE(params, it)
	{
		if (!it.nextKey().isString())
		{
			nlwarning("%s : bad key encountered with type %s, string expected.", funcName, it.nextKey().getTypename());
			continue;
		}
		if (!it.nextValue().isString())
		{
			nlwarning("%s : bad value encountered with type %s for key %s, string expected.", funcName, it.nextValue().getTypename(), it.nextKey().toString().c_str());
			continue;
		}
		templateParams.push_back(std::pair<std::string, std::string>(it.nextKey().toString(), it.nextValue().toString())); // strange compilation bug here when I use std::make_pair ... :(
	}
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CInterfaceElement *result = im->createUIElement(ls.toString(1), ls.toString(2), templateParams);
	if (!result)
	{
		ls.pushNil();
	}
	else
	{
		pushUIOnStack(ls, result);
	}
	return 1;
}


// ***************************************************************************
int CLuaIHMRyzom::displayBubble(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_createUIElement)
	const char *funcName = "displayBubble";
	CLuaIHM::checkArgCount(ls, funcName, 3);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TTABLE);
	std::vector<std::string> strs;
	std::vector<std::string> links;
	CLuaObject params;
	params.pop(ls);
	ENUM_LUA_TABLE(params, it)
	{
		if (!it.nextKey().isString())
		{
			nlwarning("%s : bad key encountered with type %s, string expected.", funcName, it.nextKey().getTypename());
			continue;
		}
		if (!it.nextValue().isString())
		{
			nlwarning("%s : bad value encountered with type %s for key %s, string expected.", funcName, it.nextValue().getTypename(), it.nextKey().toString().c_str());
			continue;
		}
		links.push_back(it.nextValue().toString());
		strs.push_back(it.nextKey().toString());
	}
	
	InSceneBubbleManager.webIgChatOpen((uint32)ls.toNumber(1), ls.toString(2), strs, links);
	
	return 1;
}

int CLuaIHMRyzom::launchContextMenuInGame(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_launchContextMenuInGame)
	CLuaStackChecker lsc(&ls);
	CLuaIHM::checkArgCount(ls,    "launchContextMenuInGame",    1);
	CLuaIHM::check(ls,   ls.isString(1),    "launchContextMenuInGame() requires a string in param 1");
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->launchContextMenuInGame(ls.toString(1));
	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::parseInterfaceFromString(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_parseInterfaceFromString)
	CLuaStackChecker lsc(&ls,    1);
	CLuaIHM::checkArgCount(ls,    "parseInterfaceFromString",    1);
	CLuaIHM::check(ls,   ls.isString(1),    "parseInterfaceFromString() requires a string in param 1");
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	std::vector<std::string> script(1);
	script[0] = ls.toString(1);
	ls.push(pIM->parseInterface(script,    true,    false));
	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::updateAllLocalisedElements(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_updateAllLocalisedElements)
	TTime startTime = CTime::getLocalTime();
	//
	CLuaStackChecker lsc(&ls);
	CLuaIHM::checkArgCount(ls,    "updateAllLocalisedElements",    0);
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->updateAllLocalisedElements();
	//
	TTime endTime = CTime::getLocalTime();
	if (ClientCfg.R2EDVerboseParseTime)
	{
		nlinfo("%.2f seconds for 'updateAllLocalisedElements'", (endTime - startTime) / 1000.f);
	}
	return 0;
}

// ***************************************************************************
int CLuaIHMRyzom::getCompleteIslands(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getCompleteIslands)
	const char *funcName = "getCompleteIslands";
	CLuaIHM::checkArgCount(ls, funcName, 0);

	ls.newTable();
	CLuaObject result(ls);

	// load entryPoints
	CScenarioEntryPoints scenarioEntryPoints = CScenarioEntryPoints::getInstance();
	const CScenarioEntryPoints::TCompleteIslands& islands =  scenarioEntryPoints.getCompleteIslands();

	CScenarioEntryPoints::TCompleteIslands::const_iterator island(islands.begin()), lastIsland(islands.end());
	for( ; island != lastIsland ; ++island)
	{
		ls.newTable();
		CLuaObject islandTable(ls);
		islandTable.setValue("continent", island->Continent);
		islandTable.setValue("xmin", (double)island->XMin);
		islandTable.setValue("ymin", (double)island->YMin);
		islandTable.setValue("xmax", (double)island->XMax);
		islandTable.setValue("ymax", (double)island->YMax);

		ls.newTable();
		CLuaObject entrypointsTable(ls);

		for(uint e=0; e<island->EntryPoints.size(); e++)
		{
			const CScenarioEntryPoints::CShortEntryPoint & entryPoint = island->EntryPoints[e];
			ls.newTable();
			CLuaObject entrypointTable(ls);
			entrypointTable.setValue("x", (double)entryPoint.X);
			entrypointTable.setValue("y", (double)entryPoint.Y);

			entrypointsTable.setValue(entryPoint.Location, entrypointTable);
		}
		islandTable.setValue("entrypoints", entrypointsTable);

		result.setValue(island->Island, islandTable);
	}

	result.push();

	return 1;
}

// ***************************************************************************
int CLuaIHMRyzom::getIslandId(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getIslandId)
	const char *funcName = "getIslandId";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::check(ls,   ls.isString(1),    "getIslandId() requires a string in param 1");


	CScenarioEntryPoints scenarioEntryPoints = CScenarioEntryPoints::getInstance();
	uint32 id = scenarioEntryPoints.getIslandId(ls.toString(1));
	ls.push((double)id);

	return 1;
}

////////////////////////////////////////// Standard Lua stuff ends here //////////////////////////////////////

// ***************************************************************************
sint32	CLuaIHMRyzom::getDbProp(const std::string &dbProp)
{
	//H_AUTO(Lua_CLuaIHM_getDbProp)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(dbProp,    false);
	if(node)
		return node->getValue32();
	else
	{
		debugInfo(toString("getDbProp(): '%s' dbProp Not found",    dbProp.c_str()));
		return 0;
	}
}

void	CLuaIHMRyzom::setDbProp(const std::string &dbProp,    sint32 value)
{
	//H_AUTO(Lua_CLuaIHM_setDbProp)
	// Do not allow Write on SERVER: or LOCAL:
	static const std::string	dbServer= "SERVER:";
	static const std::string	dbLocal= "LOCAL:";
	static const std::string	dbLocalR2= "LOCAL:R2";
	if( (0==dbProp.compare(0,    dbServer.size(),    dbServer)) ||
		(0==dbProp.compare(0,    dbLocal.size(),    dbLocal))
		)
	{
		if (0!=dbProp.compare(0,    dbLocalR2.size(),    dbLocalR2))
		{
			nlstop;
			throw ELuaIHMException("setDbProp(): You are not allowed to write on 'SERVER:...' or 'LOCAL:...' database");
		}
	}

	// Write to the DB if found
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(dbProp,    false);

	if(node)
		node->setValue32(value);
	else
		debugInfo(toString("setDbProp(): '%s' dbProp Not found",    dbProp.c_str()));
}

void	CLuaIHMRyzom::delDbProp(const string &dbProp)
{
	//H_AUTO(Lua_CLuaIHM_setDbProp)
	// Do not allow Write on SERVER: or LOCAL:
	static const string	dbServer= "SERVER:";
	static const string	dbLocal= "LOCAL:";
	static const string	dbLocalR2= "LOCAL:R2";
	if( (0==dbProp.compare(0,    dbServer.size(),    dbServer)) ||
		(0==dbProp.compare(0,    dbLocal.size(),    dbLocal))
		)
	{
		if (0!=dbProp.compare(0,    dbLocalR2.size(),    dbLocalR2))
		{
			nlstop;
			throw ELuaIHMException("setDbProp(): You are not allowed to write on 'SERVER:...' or 'LOCAL:...' database");
		}
	}

	// Write to the DB if found
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->delDbProp(dbProp);
}

void	CLuaIHMRyzom::addDbProp(const std::string &dbProp,    sint32 value)
{
	//H_AUTO(Lua_CLuaIHM_setDbProp)
	// Do not allow Write on SERVER: or LOCAL:
	static const std::string	dbServer= "SERVER:";
	static const std::string	dbLocal= "LOCAL:";
	static const std::string	dbLocalR2= "LOCAL:R2";
	if( (0==dbProp.compare(0,    dbServer.size(),    dbServer)) ||
		(0==dbProp.compare(0,    dbLocal.size(),    dbLocal))
		)
	{
		if (0!=dbProp.compare(0,    dbLocalR2.size(),    dbLocalR2))
		{
			nlstop;
			throw ELuaIHMException("setDbProp(): You are not allowed to write on 'SERVER:...' or 'LOCAL:...' database");
		}
	}

	// Write to the DB if found
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(dbProp, true);
	if(node)
		node->setValue32(value);
}

// ***************************************************************************
void		CLuaIHMRyzom::debugInfo(const std::string &cstDbg)
{
	//H_AUTO(Lua_CLuaIHM_debugInfo)
	if(ClientCfg.DisplayLuaDebugInfo)
	{
		std::string dbg = cstDbg;
		if (ClientCfg.LuaDebugInfoGotoButtonEnabled)
		{
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();
			lua_State *ls = CLuaManager::getInstance().getLuaState()->getStatePointer();
			lua_Debug	luaDbg;
			if(lua_getstack (ls,     1,     &luaDbg))
			{
				if(lua_getinfo(ls,     "lS",     &luaDbg))
				{
					// add a command button to jump to the wanted file
					dbg = createGotoFileButtonTag(luaDbg.short_src, luaDbg.currentline) + dbg;
				}
			}
		}
		rawDebugInfo(dbg);
	}
}

// ***************************************************************************
void CLuaIHMRyzom::rawDebugInfo(const std::string &dbg)
{
	//H_AUTO(Lua_CLuaIHM_rawDebugInfo)
	if(ClientCfg.DisplayLuaDebugInfo)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		if (!dbg.empty() && dbg[0] == '@')
		{
			// if color is already given use the message as it
			NLMISC::InfoLog->displayRawNL(dbg.c_str());
		}
		else
		{
			NLMISC::InfoLog->displayRawNL(LuaHelperStuff::formatLuaErrorSysInfo(dbg).c_str());
		}
		#ifdef LUA_NEVRAX_VERSION
			if (LuaDebuggerIDE)
			{
				LuaDebuggerIDE->debugInfo(dbg.c_str());
			}
		#endif
		pIM->displaySystemInfo( LuaHelperStuff::formatLuaErrorSysInfo(dbg));
	}
}


void CLuaIHMRyzom::dumpCallStack(int startStackLevel)
{
	//H_AUTO(Lua_CLuaIHM_dumpCallStack)
	if(ClientCfg.DisplayLuaDebugInfo)
	{
		lua_Debug	dbg;
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		lua_State *ls = CLuaManager::getInstance().getLuaState()->getStatePointer();
		int stackLevel = startStackLevel;
		rawDebugInfo("Call stack : ");
		rawDebugInfo("-------------");
		while (lua_getstack (ls,   stackLevel,   &dbg))
		{
			if(lua_getinfo(ls,   "lS",   &dbg))
			{
				std::string result = createGotoFileButtonTag(dbg.short_src,   dbg.currentline) + NLMISC::toString("%s:%d:",   dbg.short_src,   dbg.currentline);
				rawDebugInfo(result);
			}
			++ stackLevel;
		}
	}
}

// ***************************************************************************
void CLuaIHMRyzom::getCallStackAsString(int startStackLevel /*=0*/,std::string &result)
{
	//H_AUTO(Lua_CLuaIHM_getCallStackAsString)
	result.clear();
	lua_Debug	dbg;
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	lua_State *ls = CLuaManager::getInstance().getLuaState()->getStatePointer();
	int stackLevel = startStackLevel;
	result += "Call stack : \n";
	result += "-------------";
	while (lua_getstack (ls,   stackLevel,   &dbg))
	{
		if(lua_getinfo(ls,   "lS",   &dbg))
		{
			result += NLMISC::toString("%s:%d:",   dbg.short_src,   dbg.currentline);
		}
		++ stackLevel;
	}
}

// ***************************************************************************
std::string	CLuaIHMRyzom::getDefine(const std::string &def)
{
	//H_AUTO(Lua_CLuaIHM_getDefine)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	if(ClientCfg.DisplayLuaDebugInfo && !pIM->isDefineExist(def))
		debugInfo(toString("getDefine(): '%s' not found",    def.c_str()));
	return pIM->getDefine(def);
}

// ***************************************************************************
void		CLuaIHMRyzom::setContextHelpText(const ucstring &text)
{
	//H_AUTO(Lua_CLuaIHM_setContextHelpText)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->setContextHelpText(text);
}

// ***************************************************************************
void		CLuaIHMRyzom::messageBox(const ucstring &text)
{
	//H_AUTO(Lua_CLuaIHM_messageBox)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBox(text);
}

// ***************************************************************************
void		CLuaIHMRyzom::messageBox(const ucstring &text, const std::string &masterGroup)
{
	//H_AUTO(Lua_CLuaIHM_messageBox)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBox(text, masterGroup);
}

// ***************************************************************************
void		CLuaIHMRyzom::messageBox(const ucstring &text, const std::string &masterGroup, int caseMode)
{
	if (caseMode < 0 || caseMode >= CaseCount)
	{
		throw ELuaIHMException("messageBox: case mode value is invalid.");
	}
	//H_AUTO(Lua_CLuaIHM_messageBox)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBox(text, masterGroup, (TCaseMode) caseMode);
}

// ***************************************************************************
void		CLuaIHMRyzom::messageBox(const std::string &text)
{
	//H_AUTO(Lua_CLuaIHM_messageBox)
	static volatile bool dumpCallStack = false;
	if (dumpCallStack)
	{
		CLuaIHMRyzom::dumpCallStack(0);
	}
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBox(text);
}

// ***************************************************************************
void		CLuaIHMRyzom::messageBoxWithHelp(const ucstring &text)
{
	//H_AUTO(Lua_CLuaIHM_messageBox)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBoxWithHelp(text);
}

// ***************************************************************************
void		CLuaIHMRyzom::messageBoxWithHelp(const ucstring &text, const std::string &masterGroup)
{
	//H_AUTO(Lua_CLuaIHM_messageBox)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBoxWithHelp(text, masterGroup);
}

// ***************************************************************************
void		CLuaIHMRyzom::messageBoxWithHelp(const ucstring &text, const std::string &masterGroup, int caseMode)
{
	if (caseMode < 0 || caseMode >= CaseCount)
	{
		throw ELuaIHMException("messageBoxWithHelp: case mode value is invalid.");
	}
	//H_AUTO(Lua_CLuaIHM_messageBox)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBoxWithHelp(text, masterGroup, "" ,"", (TCaseMode) caseMode);
}

// ***************************************************************************
void		CLuaIHMRyzom::messageBoxWithHelp(const std::string &text)
{
	//H_AUTO(Lua_CLuaIHM_messageBox)
	static volatile bool dumpCallStack = false;
	if (dumpCallStack)
	{
		CLuaIHMRyzom::dumpCallStack(0);
	}
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBoxWithHelp(text);
}

// ***************************************************************************
bool CLuaIHMRyzom::executeFunctionOnStack(CLuaState &ls,   int numArgs,   int numRet)
{
	//H_AUTO(Lua_CLuaIHM_executeFunctionOnStack)
	static volatile bool dumpFunction = false;
	if (dumpFunction)
	{
		CLuaStackRestorer lsr(&ls, ls.getTop());
		lua_Debug ar;
		ls.pushValue(-1 - numArgs);
		lua_getinfo (ls.getStatePointer(), ">lS", &ar);
		nlwarning((std::string(ar.what) + ", at line " + toString(ar.linedefined) + " in " + std::string(ar.source)).c_str());
	}
	int result = ls.pcall(numArgs,   numRet);
	switch (result)
	{
		case LUA_ERRRUN:
		case LUA_ERRMEM:
		case LUA_ERRERR:
		{
			debugInfo(ls.toString(-1));
			ls.pop();
			return false;
		}
		break;
		case 0:
			return true;
		break;
		default:
			nlassert(0);
		break;
	}
	return false;
}

// ***************************************************************************
ucstring CLuaIHMRyzom::replacePvpEffectParam(const ucstring &str, sint32 parameter)
{
	//H_AUTO(Lua_CLuaIHM_replacePvpEffectParam)
	ucstring result = str;
	CSString s = str.toString();
	std::string p, paramString;

	// Locate parameter and store it
	p = s.splitTo('%', true);
	while (p.size() > 0 && s.size() > 0)
	{
		if (s[0] == 'p' || s[0] == 'n' || s[0] == 'r')
		{
			paramString = "%";
			paramString += s[0];
			break;
		}
		p = s.splitTo('%', true);
	}

	// Return original string if param isn't found
	if (paramString.size() < 2)
		return str;

	// Replace parameter based on its type
	switch (paramString[1])
	{
	case 'p':
		p = toString("%.1f %%", parameter/100.0);
		break;
	case 'n':
		p = toString(parameter);
		break;
	case 'r':
		p = toString("%.1f", parameter/100.0);
		break;
	default:
		debugInfo("Bad arguments in " + str.toString() + " : " + paramString);
	}

	strFindReplace(result, paramString.c_str(), p);

	return result;
}

// ***************************************************************************
sint32 CLuaIHMRyzom::secondsSince1970ToHour(sint32 seconds)
{
	//H_AUTO(Lua_CLuaIHM_secondsSince1970ToHour)
	// convert to readable form
	struct tm	*tstruct;
	time_t		tval= seconds;
	tstruct= gmtime(&tval);
	if(!tstruct)
	{
		debugInfo(toString("Bad Date Received: %d", seconds));
		return 0;
	}

	return tstruct->tm_hour;	// 0-23
}

// ***************************************************************************
void CLuaIHMRyzom::pauseBGDownloader()
{
	::pauseBGDownloader();
}

// ***************************************************************************
void CLuaIHMRyzom::unpauseBGDownloader()
{
	::unpauseBGDownloader();
}

// ***************************************************************************
void CLuaIHMRyzom::requestBGDownloaderPriority(uint priority)
{
	if (priority >= BGDownloader::ThreadPriority_Count)
	{
		throw NLMISC::Exception("requestBGDownloaderPriority() : invalid priority");
	}
	CBGDownloaderAccess::getInstance().requestDownloadThreadPriority((BGDownloader::TThreadPriority) priority, false);
}

// ***************************************************************************
sint CLuaIHMRyzom::getBGDownloaderPriority()
{
	return CBGDownloaderAccess::getInstance().getDownloadThreadPriority();
}

// ***************************************************************************
ucstring CLuaIHMRyzom::getPatchLastErrorMessage()
{
	if (isBGDownloadEnabled())
	{
		return CBGDownloaderAccess::getInstance().getLastErrorMessage();
	}
	else
	{
		CPatchManager *pPM = CPatchManager::getInstance();
		return pPM->getLastErrorMessage();
	}
}

// ***************************************************************************
bool CLuaIHMRyzom::isInGame()
{
	CInterfaceManager	*pIM = CInterfaceManager::getInstance();
	return pIM->isInGame();
}

// ***************************************************************************
uint32 CLuaIHMRyzom::getPlayerSelectedSlot()
{
	return (uint32) PlayerSelectedSlot;
}

// ***************************************************************************
bool CLuaIHMRyzom::isPlayerSlotNewbieLand(uint32 slot)
{
	if (slot > CharacterSummaries.size())
	{
		throw ELuaIHMException("isPlayerSlotNewbieLand(): Invalid slot %d",  (int) slot);
	}
	return CharacterSummaries[slot].InNewbieland;
}

// ***************************************************************************
sint32	CLuaIHMRyzom::getSkillIdFromName(const std::string &def)
{
	//H_AUTO(Lua_CLuaIHM_getSkillIdFromName)
	SKILLS::ESkills	e= SKILLS::toSkill(def);
	// Avoid any bug,    return SF if not found
	if(e>=SKILLS::unknown)
		e= SKILLS::SF;
	return e;
}

// ***************************************************************************
ucstring	CLuaIHMRyzom::getSkillLocalizedName(sint32 skillId)
{
	//H_AUTO(Lua_CLuaIHM_getSkillLocalizedName)
	return ucstring(STRING_MANAGER::CStringManagerClient::getSkillLocalizedName((SKILLS::ESkills)skillId));
}

// ***************************************************************************
sint32	CLuaIHMRyzom::getMaxSkillValue(sint32 skillId)
{
	//H_AUTO(Lua_CLuaIHM_getMaxSkillValue)
	CSkillManager	*pSM= CSkillManager::getInstance();
	return pSM->getMaxSkillValue((SKILLS::ESkills)skillId);
}

// ***************************************************************************
sint32	CLuaIHMRyzom::getBaseSkillValueMaxChildren(sint32 skillId)
{
	//H_AUTO(Lua_CLuaIHM_getBaseSkillValueMaxChildren)
	CSkillManager	*pSM= CSkillManager::getInstance();
	return pSM->getBaseSkillValueMaxChildren((SKILLS::ESkills)skillId);
}

// ***************************************************************************
sint32	CLuaIHMRyzom::getMagicResistChance(bool elementalSpell,   sint32 casterSpellLvl,   sint32 victimResistLvl)
{
	//H_AUTO(Lua_CLuaIHM_getMagicResistChance)
	CSPhraseManager	*pPM= CSPhraseManager::getInstance();
	casterSpellLvl= std::max(casterSpellLvl,   sint32(0));
	victimResistLvl= std::max(victimResistLvl,   sint32(0));
	/*  The success rate in the table is actually the "Casting Success Chance".
		Thus,   the relativeLevel is casterSpellLvl - victimResistLvl
		Moreover,   must take the "PartialSuccessMaxDraw" line because the spell is not resisted if success>0
	*/
	sint32	chanceToHit= pPM->getSuccessRate(elementalSpell?CSPhraseManager::STResistMagic:CSPhraseManager::STResistMagicLink,
		casterSpellLvl-victimResistLvl,   true);
	clamp(chanceToHit,   0,   100);

	// Thus,   the resist chance is 100 - hit chance.
	return 100 - chanceToHit;
}

// ***************************************************************************
sint32	CLuaIHMRyzom::getDodgeParryChance(sint32 attLvl, sint32 defLvl)
{
	//H_AUTO(Lua_CLuaIHM_getDodgeParryChance)
	CSPhraseManager	*pPM = CSPhraseManager::getInstance();
	attLvl= std::max(attLvl, sint32(0));
	defLvl= std::max(defLvl, sint32(0));

	sint32 chance = pPM->getSuccessRate(CSPhraseManager::STDodgeParry, defLvl-attLvl, false);
	clamp(chance, 0, 100);

	return chance;
}

// ***************************************************************************
void	CLuaIHMRyzom::browseNpcWebPage(const std::string &htmlId, const std::string &urlIn, bool addParameters, double timeout)
{
	//H_AUTO(Lua_CLuaIHM_browseNpcWebPage)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CGroupHTML	*groupHtml= dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId(htmlId));
	if(groupHtml)
	{
		// if true, it means that we want to display a web page that use webig auth
		bool webig = urlIn.find("http://") == 0;

		string	url;
		// append the WebServer to the url
		if (urlIn.find("ring_access_point=1") != std::string::npos)
		{
			url = RingMainURL + "?" + urlIn;
		}
		else if(webig)
		{
			url = urlIn;
		}
		else
		{
			url = WebServer + urlIn;
		}

		if (addParameters && !webig)
		{
			// append shardid, playername and language code
			string userName;
			string guildName;
			if(UserEntity)
			{
				userName = UserEntity->getDisplayName ().toString();
				STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
				ucstring ucsTmp;
				pSMC->getString (UserEntity->getGuildNameID(), ucsTmp);
				guildName = ucsTmp.toString();

				while (guildName.find(' ') != string::npos)
				{
					guildName[guildName.find(' ')] = '_';
				}
			}

			url += ((url.find('?') != string::npos) ? "&" : "?") +
					string("shard=") + toString(ShardId) +
					string("&user_login=") + userName +
					string("&lang=") + ClientCfg.getHtmlLanguageCode() +
					string("&guild_name=") + guildName;
		}
/* Already added by GroupHtml
		if(webig)
		{
			// append special webig auth params
			addWebIGParams(url);
		}
*/
		// set the wanted timeout
		groupHtml->setTimeout((float)std::max(0.0, timeout));

		// Browse the url
		groupHtml->clean();
		groupHtml->browse(url.c_str());
		// Set top of the page
		CCtrlScroll *pScroll = groupHtml->getScrollBar();
		if (pScroll != NULL)
			pScroll->moveTrackY(10000);
	}
}


// ***************************************************************************
void		CLuaIHMRyzom::clearHtmlUndoRedo(const std::string &htmlId)
{
	//H_AUTO(Lua_CLuaIHM_clearHtmlUndoRedo)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CGroupHTML	*groupHtml= dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId(htmlId));
	if(groupHtml)
		groupHtml->clearUndoRedo();
}

// ***************************************************************************
ucstring	CLuaIHMRyzom::getDynString(sint32 dynStringId)
{
	//H_AUTO(Lua_CLuaIHM_getDynString)
	ucstring result;
	STRING_MANAGER::CStringManagerClient::instance()->getDynString(dynStringId,   result);
	return result;
}

// ***************************************************************************
bool		CLuaIHMRyzom::isDynStringAvailable(sint32 dynStringId)
{
	//H_AUTO(Lua_CLuaIHM_isDynStringAvailable)
	ucstring result;
	bool res = STRING_MANAGER::CStringManagerClient::instance()->getDynString(dynStringId,   result);
	return res;
}

// ***************************************************************************
bool CLuaIHMRyzom::isFullyPatched()
{
	return AvailablePatchs == 0;
}

// ***************************************************************************
std::string CLuaIHMRyzom::getSheetType(const std::string &sheet)
{
	//H_AUTO(Lua_CLuaIHM_getSheetType)
	const CEntitySheet *sheetPtr = SheetMngr.get(CSheetId(sheet));
	if (!sheetPtr) return "";
	return CEntitySheet::typeToString(sheetPtr->Type);
}


// ***************************************************************************
std::string CLuaIHMRyzom::getSheetName(uint32 sheetId)
{
	return CSheetId(sheetId).toString();
}

// ***************************************************************************
sint32	CLuaIHMRyzom::getFameIndex(const std::string &factionName)
{
	//H_AUTO(Lua_CLuaIHM_getFameIndex)
	return CStaticFames::getInstance().getFactionIndex(factionName);
}

// ***************************************************************************
std::string	CLuaIHMRyzom::getFameName(sint32 fameIndex)
{
	//H_AUTO(Lua_CLuaIHM_getFameName)
	return CStaticFames::getInstance().getFactionName(fameIndex);
}

// ***************************************************************************
sint32	CLuaIHMRyzom::getFameDBIndex(sint32 fameIndex)
{
	//H_AUTO(Lua_CLuaIHM_getFameDBIndex)
	// Yoyo: avoid crash if fames not initialized
	if(CStaticFames::getInstance().getNbFame()==0)
		return 0;
	else
		return CStaticFames::getInstance().getDatabaseIndex(fameIndex);
}

// ***************************************************************************
sint32	CLuaIHMRyzom::getFirstTribeFameIndex()
{
	//H_AUTO(Lua_CLuaIHM_getFirstTribeFameIndex)
	return CStaticFames::getInstance().getFirstTribeFameIndex();
}

// ***************************************************************************
sint32	CLuaIHMRyzom::getNbTribeFameIndex()
{
	//H_AUTO(Lua_CLuaIHM_getNbTribeFameIndex)
	// Yoyo: avoid crash if fames not initialized. at leasst one tribe
	return std::max(1U, CStaticFames::getInstance().getNbTribeFameIndex());
}

// ***************************************************************************
string CLuaIHMRyzom::getClientCfg(const string &varName)
{
	//H_AUTO(Lua_CLuaIHM_getClientCfg)
	return ClientCfg.readString(varName);
}

// ***************************************************************************
bool CLuaIHMRyzom::fileExists(const string &fileName)
{
	//H_AUTO(Lua_CLuaIHM_fileExists)
	return CPath::exists(fileName);
}

// ***************************************************************************
void CLuaIHMRyzom::sendMsgToServer(const std::string &sMsg)
{
	//H_AUTO(Lua_CLuaIHM_sendMsgToServer)
	::sendMsgToServer(sMsg);
}

// ***************************************************************************
void CLuaIHMRyzom::sendMsgToServerPvpTag(bool pvpTag)
{
	//H_AUTO(Lua_CLuaIHM_sendMsgToServerPvpTag)
	uint8 tag = (uint8)pvpTag;
	::sendMsgToServer("PVP:PVP_TAG", tag);
}

// ***************************************************************************
bool CLuaIHMRyzom::isGuildQuitAvailable()
{
	//H_AUTO(Lua_CLuaIHM_isGuildQuitAvailable)
	return CGuildManager::getInstance()->getGuild().QuitGuildAvailable;
}

// ***************************************************************************
void CLuaIHMRyzom::sortGuildMembers()
{
	//H_AUTO(Lua_CLuaIHM_sortGuildMembers)
	CGuildManager::getInstance()->sortGuildMembers();
}

// ***************************************************************************
sint32 CLuaIHMRyzom::getNbGuildMembers()
{
	//H_AUTO(Lua_CLuaIHM_getNbGuildMembers)
	return (sint32)CGuildManager::getInstance()->getGuildMembers().size();
}

// ***************************************************************************
string CLuaIHMRyzom::getGuildMemberName(sint32 nMemberId)
{
	//H_AUTO(Lua_CLuaIHM_getGuildMemberName)
	if ((nMemberId < 0) || (nMemberId >= getNbGuildMembers()))
		return "";
	return CGuildManager::getInstance()->getGuildMembers()[nMemberId].Name.toString();
}

// ***************************************************************************
string CLuaIHMRyzom::getGuildMemberGrade(sint32 nMemberId)
{
	//H_AUTO(Lua_CLuaIHM_getGuildMemberGrade)
	if ((nMemberId < 0) || (nMemberId >= getNbGuildMembers()))
		return "";
	return EGSPD::CGuildGrade::toString(CGuildManager::getInstance()->getGuildMembers()[nMemberId].Grade);
}

// ***************************************************************************
bool CLuaIHMRyzom::isR2Player(const std::string &sheet)
{
	//H_AUTO(Lua_CLuaIHM_isR2Player)
	const CEntitySheet *entitySheet = SheetMngr.get(CSheetId(sheet));
	if (!entitySheet) return false;
	const CCharacterSheet *chSheet = dynamic_cast<const CCharacterSheet *>(entitySheet);
	if(!chSheet) return false;
	return chSheet->R2Npc;
}

// ***************************************************************************
std::string CLuaIHMRyzom::getR2PlayerRace(const std::string &sheet)
{
	//H_AUTO(Lua_CLuaIHM_getR2PlayerRace)
	const CEntitySheet *entitySheet = SheetMngr.get(CSheetId(sheet));
	if (!entitySheet) return "";
	const CCharacterSheet *chSheet = dynamic_cast<const CCharacterSheet *>(entitySheet);
	if(!chSheet) return "";
	return EGSPD::CPeople::toString(chSheet->Race);
}

// ***************************************************************************
bool CLuaIHMRyzom::isR2PlayerMale(const std::string &sheet)
{
	//H_AUTO(Lua_CLuaIHM_isR2PlayerMale)
	const CEntitySheet *entitySheet = SheetMngr.get(CSheetId(sheet));
	if (!entitySheet) return true;
	const CCharacterSheet *chSheet = dynamic_cast<const CCharacterSheet *>(entitySheet);
	if(!chSheet) return true;

	return (chSheet->Gender == GSGENDER::male);
}

// ***************************************************************************
std::string CLuaIHMRyzom::getCharacterSheetSkel(const std::string &sheet, bool isMale)
{
	//H_AUTO(Lua_CLuaIHM_getCharacterSheetSkel)
	const CEntitySheet *sheetPtr = SheetMngr.get(CSheetId(sheet));
	const CCharacterSheet *charSheet = dynamic_cast<const CCharacterSheet *>(sheetPtr);
	if (charSheet) return charSheet->getSkelFilename();
	const CRaceStatsSheet *raceStatSheet = dynamic_cast<const CRaceStatsSheet *>(sheetPtr);
	if (raceStatSheet) return raceStatSheet->GenderInfos[isMale ? 0 : 1].Skelfilename;
	return "";
}

// ***************************************************************************
sint32	CLuaIHMRyzom::getSheetId(const std::string &itemName)
{
	//H_AUTO(Lua_CLuaIHM_getSheetId)
	return (sint32)CSheetId(itemName).asInt();
}

// ***************************************************************************
sint CLuaIHMRyzom::getCharacterSheetRegionForce(const std::string &sheet)
{
	//H_AUTO(Lua_CLuaIHM_getCharacterSheetRegionForce)
	const CCharacterSheet *charSheet = dynamic_cast<const CCharacterSheet *>(SheetMngr.get(CSheetId(sheet)));
	if (!charSheet) return 0;
	return charSheet->RegionForce;
}

// ***************************************************************************
sint CLuaIHMRyzom::getCharacterSheetRegionLevel(const std::string &sheet)
{
	//H_AUTO(Lua_CLuaIHM_getCharacterSheetRegionLevel)
	const CCharacterSheet *charSheet = dynamic_cast<const CCharacterSheet *>(SheetMngr.get(CSheetId(sheet)));
	if (!charSheet) return 0;
	return charSheet->RegionForce;
}

// ***************************************************************************
string CLuaIHMRyzom::getRegionByAlias(uint32 alias)
{
	//H_AUTO(Lua_CLuaIHM_getRegionByAlias)
	return ContinentMngr.getRegionNameByAlias(alias);
}

// ***************************************************************************
void CLuaIHMRyzom::tell(const ucstring &player, const ucstring &msg)
{
	//H_AUTO(Lua_CLuaIHM_tell)
	// display a /tell command in the main chat
	if (!player.empty())
	{
		if (!msg.empty())
		{
			// Parse any tokens in the message.
			ucstring msg_modified = msg;
			// Parse any tokens in the text
			if ( ! CInterfaceManager::parseTokens(msg_modified))
			{
				return;
			}
			ChatMngr.tell(player.toUtf8(), msg_modified);
		}
		else
		{
			CChatWindow *w = PeopleInterraction.ChatGroup.Window;
			if (w)
			{
				CInterfaceManager *im = CInterfaceManager::getInstance();
				w->setKeyboardFocus();
				w->enableBlink(1);
				w->setCommand(ucstring("tell ") + CEntityCL::removeTitleFromName(player) + ucstring(" "), false);
				CGroupEditBox *eb = w->getEditBox();
				if (eb != NULL)
				{
					eb->bypassNextKey();
				}
				if (w->getContainer())
				{
					w->getContainer()->setActive(true);
					CWidgetManager::getInstance()->setTopWindow(w->getContainer());
				}
			}
		}
	}
}

// ***************************************************************************
bool CLuaIHMRyzom::isRingAccessPointInReach()
{
	//H_AUTO(Lua_CLuaIHM_isRingAccessPointInReach)
	if (BotChatPageAll->RingSessions->RingAccessPointPos == CVector::Null) return false;
	const CVectorD &vect1 = BotChatPageAll->RingSessions->RingAccessPointPos;
	CVectorD vect2 = UserEntity->pos();
	double distanceSquare = pow(vect1.x-vect2.x,2) + pow(vect1.y-vect2.y,2);
	return distanceSquare <= MaxTalkingDistSquare;
}

// ***************************************************************************
void CLuaIHMRyzom::updateTooltipCoords()
{
	CInterfaceManager::getInstance()->updateTooltipCoords();
}

// ***************************************************************************
bool CLuaIHMRyzom::isCtrlKeyDown()
{
	//H_AUTO(Lua_CLuaIHM_isCtrlKeyDown)
	bool ctrlDown = Driver->AsyncListener.isKeyDown(KeyLCONTROL) ||
					Driver->AsyncListener.isKeyDown(KeyRCONTROL);
	if (ctrlDown) nlwarning("ctrl down");
	else nlwarning("ctrl up");
	return ctrlDown;
}

// ***************************************************************************
std::string CLuaIHMRyzom::encodeURLUnicodeParam(const ucstring &text)
{
	//H_AUTO(Lua_CLuaIHM_encodeURLUnicodeParam)
	return convertToHTML(text.toUtf8());
}

// ***************************************************************************
sint32 CLuaIHMRyzom::getPlayerLevel()
{
	if (!UserEntity) return -1;
	CSkillManager	*pSM= CSkillManager::getInstance();
	uint32 maxskill = pSM->getBestSkillValue(SKILLS::SC);
	maxskill = std::max(maxskill, pSM->getBestSkillValue(SKILLS::SF));
	maxskill = std::max(maxskill, pSM->getBestSkillValue(SKILLS::SH));
	maxskill = std::max(maxskill, pSM->getBestSkillValue(SKILLS::SM));
	return sint32(maxskill);
}

// ***************************************************************************
sint64 CLuaIHMRyzom::getPlayerVpa()
{
	sint64 prop = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E0:P"+toString("%d", CLFECOMMON::PROPERTY_VPA))->getValue64();
	return prop;
}

// ***************************************************************************
sint64 CLuaIHMRyzom::getPlayerVpb()
{
	sint64 prop = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E0:P"+toString("%d", CLFECOMMON::PROPERTY_VPB))->getValue64();
	return prop;
}

// ***************************************************************************
sint64 CLuaIHMRyzom::getPlayerVpc()
{
	sint64 prop = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E0:P"+toString("%d", CLFECOMMON::PROPERTY_VPB))->getValue64();
	return prop;
}

// ***************************************************************************
sint32 CLuaIHMRyzom::getTargetLevel()
{
	CEntityCL *target = getTargetEntity();
	if (!target) return -1;
	if ( target->isPlayer() )
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CCDBNodeLeaf *pDbPlayerLevel = NLGUI::CDBManager::getInstance()->getDbProp( pIM->getDefine("target_player_level") );
		return pDbPlayerLevel ? pDbPlayerLevel->getValue32() : -1;
	}
	else
	{
		CCharacterSheet *pCS = dynamic_cast<CCharacterSheet*>(SheetMngr.get(target->sheetId()));
		if(!pCS) return -1;
		// only display the consider if the target is attackable #523
		if(!pCS->Attackable) return -1;
		if(!target->properties().attackable()) return -1;
		return sint32(pCS->Level);
	}
	return -1;
}

// ***************************************************************************
ucstring CLuaIHMRyzom::getTargetSheet()
{
	CEntityCL *target = getTargetEntity();
	if (!target) return ucstring();

	return target->sheetId().toString();
}

// ***************************************************************************
sint64 CLuaIHMRyzom::getTargetVpa()
{
	CEntityCL *target = getTargetEntity();
	if (!target) return 0;

	sint64 prop = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", getTargetSlotNr())+":P"+toString("%d", CLFECOMMON::PROPERTY_VPA))->getValue64();

	return prop;
}

// ***************************************************************************
sint64 CLuaIHMRyzom::getTargetVpb()
{
	CEntityCL *target = getTargetEntity();
	if (!target) return 0;

	sint64 prop = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", getTargetSlotNr())+":P"+toString("%d", CLFECOMMON::PROPERTY_VPB))->getValue64();

	return prop;
}

// ***************************************************************************
sint64 CLuaIHMRyzom::getTargetVpc()
{
	CEntityCL *target = getTargetEntity();
	if (!target) return 0;

	sint64 prop = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", getTargetSlotNr())+":P"+toString("%d", CLFECOMMON::PROPERTY_VPB))->getValue64();

	return prop;
}

// ***************************************************************************
sint32 CLuaIHMRyzom::getTargetForceRegion()
{	
	CEntityCL *target = getTargetEntity();
	if (!target) return -1;
	if ( target->isPlayer() )
	{			
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CCDBNodeLeaf *pDbPlayerLevel = NLGUI::CDBManager::getInstance()->getDbProp( pIM->getDefine("target_player_level") );			
		if (!pDbPlayerLevel) return -1;
		sint nLevel = pDbPlayerLevel->getValue32();
		if ( nLevel < 250 )
		{				
			return (sint32) ((nLevel < 20) ? 1 : (nLevel / 50) + 2);
		}
		else
		{				
			return 8;
		}
	}
	else
	{			
		CCharacterSheet *pCS = dynamic_cast<CCharacterSheet*>(SheetMngr.get(target->sheetId()));
		return pCS ? (sint32) pCS->RegionForce : -1;
	}		
	return 0;
}

// ***************************************************************************
sint32 CLuaIHMRyzom::getTargetLevelForce()
{
	CEntityCL *target = getTargetEntity();
	if (!target) return -1;
	if ( target->isPlayer() )
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CCDBNodeLeaf *pDbPlayerLevel = NLGUI::CDBManager::getInstance()->getDbProp( pIM->getDefine("target_player_level") );
		if (!pDbPlayerLevel) return -1;
		sint nLevel = pDbPlayerLevel->getValue32();
		if ( nLevel < 250 )
		{
			return (sint32) (((nLevel % 50) * 5 / 50) + 1);
		}
		else
		{
			return 6;
		}
	}
	else
	{
		CCharacterSheet *pCS = dynamic_cast<CCharacterSheet*>(SheetMngr.get(target->sheetId()));
		return pCS ? (sint32) pCS->ForceLevel : -1;
	}
	return 0;
}

// ***************************************************************************
bool CLuaIHMRyzom::isTargetNPC()
{
	CEntityCL *target = getTargetEntity();
	if (!target) return false;
	return target->isNPC();
}

// ***************************************************************************
bool CLuaIHMRyzom::isTargetPlayer()
{
	CEntityCL *target = getTargetEntity();
	if (!target) return false;
	return target->isPlayer();
}


// ***************************************************************************
bool CLuaIHMRyzom::isTargetUser()
{
	CEntityCL *target = getTargetEntity();
	if (!target) return false;
	return target->isUser();
}

// ***************************************************************************
bool CLuaIHMRyzom::isPlayerInPVPMode()
{
	if (!UserEntity) return false;
	return (UserEntity->getPvpMode() & PVP_MODE::PvpFaction || UserEntity->getPvpMode() & PVP_MODE::PvpFactionFlagged || UserEntity->getPvpMode() & PVP_MODE::PvpZoneFaction)  != 0;
}

// ***************************************************************************
bool CLuaIHMRyzom::isTargetInPVPMode()
{
	CEntityCL *target = getTargetEntity();
	if (!target) return false;
	return (target->getPvpMode() & PVP_MODE::PvpFaction || target->getPvpMode() & PVP_MODE::PvpFactionFlagged || target->getPvpMode() & PVP_MODE::PvpZoneFaction)  != 0;
}

// ***************************************************************************
std::string	CLuaIHMRyzom::createGotoFileButtonTag(const char *fileName, uint line)
{
	//H_AUTO(Lua_CLuaIHM_createGotoFileButtonTag)
	if (ClientCfg.LuaDebugInfoGotoButtonEnabled)
	{
		// TODO nico : put this in the interface
		// add a command button to jump to the wanted file
		return toString("/$$%s|%s|lua|%s('%s',   %d)$$/",
					   ClientCfg.LuaDebugInfoGotoButtonTemplate.c_str(),
					   ClientCfg.LuaDebugInfoGotoButtonCaption.c_str(),
					   ClientCfg.LuaDebugInfoGotoButtonFunction.c_str(),
					   fileName,
					   line
					  );
	}
	return "";
}

// ***************************************************************************
int	CLuaIHMRyzom::runExprAndPushResult(CLuaState &ls,    const std::string &expr)
{
	//H_AUTO(Lua_CLuaIHM_runExprAndPushResult)
	// Execute expression
	CInterfaceExprValue value;
	if (CInterfaceExpr::eval(expr,    value,    NULL))
	{
		switch(value.getType())
		{
		case CInterfaceExprValue::Boolean:
			ls.push(value.getBool());
			break;
		case CInterfaceExprValue::Integer:
			ls.push((double)value.getInteger());
			break;
		case CInterfaceExprValue::Double:
			ls.push(value.getDouble());
			break;
		case CInterfaceExprValue::String:
			{
				ucstring	ucstr= value.getUCString();
				// Yoyo: dynamically decide whether must return a string or a ucstring
				bool	mustUseUCString= false;
				for (uint i = 0; i < ucstr.size (); i++)
				{
					if (ucstr[i] > 255)
					{
						mustUseUCString= true;
						break;
					}
				}
				// push a ucstring?
				if(mustUseUCString)
				{
#if LUABIND_VERSION > 600
					luabind::detail::push(ls.getStatePointer(), ucstr);
#else
					luabind::object obj(ls.getStatePointer(), ucstr);
					obj.pushvalue();
#endif
				}
				else
				{
					ls.push(ucstr.toString());
				}
				break;
			}
		case CInterfaceExprValue::RGBA:
			{
				CRGBA color = value.getRGBA();
#if LUABIND_VERSION > 600
				luabind::detail::push(ls.getStatePointer(), color);
#else
				luabind::object obj(ls.getStatePointer(), color);
				obj.pushvalue();
#endif
				break;
			}
			break;
		case CInterfaceExprValue::UserType: // Yoyo: don't care UserType...
		default:
			ls.pushNil();
			break;
		}
	}
	else
		ls.pushNil();

	return 1;
}
