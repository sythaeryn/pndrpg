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

#include "interface_parser.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/file.h"
#include "nel/misc/algo.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/factory.h"
#include "nel/misc/big_file.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/interface_options.h"
#include "nel/gui/interface_anim.h"
#include "nel/gui/interface_expr.h"
#include "nel/gui/view_pointer.h"
#include "nel/gui/group_modal.h"
#include "nel/gui/group_list.h"
#include "nel/gui/group_container.h"
#include "nel/gui/interface_link.h"
#include "nel/gui/lua_helper.h"
#include "nel/gui/lua_ihm.h"
#include "nel/gui/lua_manager.h"

#include "interface_options_ryzom.h"
#include "interface_3d_scene.h"
#include "lua_ihm_ryzom.h"
#include "interface_ddx.h"
#include "macrocmd_manager.h"
#include "../commands.h"

#ifdef LUA_NEVRAX_VERSION
	#include "lua_ide_dll_nevrax/include/lua_ide_dll/ide_interface.h" // external debugger
#endif
const uint32 UI_CACHE_SERIAL_CHECK = (uint32) 'IUG_';

using namespace NLMISC;
using namespace NLGUI;

void saveXMLTree(COFile &f, xmlNodePtr node)
{
	// save node name
	std::string name = (const char *) node->name;
	f.serial(name);
	// save properties
	uint32 numProp = 0;
	xmlAttrPtr currProp = node->properties;
	while (currProp)
	{
		++ numProp;
		currProp = currProp->next;
	}
	f.serial(numProp);
	currProp = node->properties;
	while (currProp)
	{
		std::string name = (const char *) currProp->name;
		f.serial(name);
		CXMLAutoPtr ptr(xmlGetProp(node, currProp->name));
		std::string value = (const char *) ptr;
		f.serial(value);
		currProp = currProp->next;
	}
	uint32 numChildren = 0;
	xmlNodePtr currChild = node->children;
	while (currChild)
	{
		++ numChildren;
		currChild = currChild->next;
	}
	f.serial(numChildren);
	currChild = node->children;
	while (currChild)
	{
		saveXMLTree(f, currChild);
		currChild = currChild->next;
	}
}

xmlNodePtr buildTree(CIFile &f)
{
	// load node name
	std::string name;
	f.serial(name);
	xmlNodePtr node = xmlNewNode(NULL, (const xmlChar *) name.c_str());
	// slod properties
	uint32 numProp;
	f.serial(numProp);
	for(uint k = 0; k < numProp; ++k)
	{
		std::string name, value;
		f.serial(name, value);
		xmlSetProp(node, (const xmlChar *) name.c_str(), (const xmlChar *) value.c_str());
	}
	uint32 numChildren;
	f.serial(numChildren);
	for(uint k = 0; k < numChildren; ++k)
	{
		xmlAddChild(node, buildTree(f));
	}
	return node;
}



// ----------------------------------------------------------------------------

extern CActionsManager Actions;	// Actions Manager.
extern CActionsManager EditActions;	// Actions Manager.
extern CActionsContext ActionsContext;	// Actions context.

// ----------------------------------------------------------------------------

using namespace NLMISC;
using namespace std;

// ----------------------------------------------------------------------------
// CRootGroup
// ----------------------------------------------------------------------------

class CRootGroup : public CInterfaceGroup
{
public:
	CRootGroup(const TCtorParam &param)
		: CInterfaceGroup(param)
	{ }

	/// Destructor
	virtual ~CRootGroup() { }

	virtual CInterfaceElement* getElement (const std::string &id)
	{
		if (_Id == id)
		return this;

		if (id.substr(0, _Id.size()) != _Id)
			return NULL;

		vector<CViewBase*>::const_iterator itv;
		for (itv = _Views.begin(); itv != _Views.end(); itv++)
		{
			CViewBase *pVB = *itv;
			if (pVB->getId() == id)
				return pVB;
		}

		vector<CCtrlBase*>::const_iterator itc;
		for (itc = _Controls.begin(); itc != _Controls.end(); itc++)
		{
			CCtrlBase* ctrl = *itc;
			if (ctrl->getId() == id)
				return ctrl;
		}

		// Accelerate
		string sTmp = id;
		sTmp = sTmp.substr(_Id.size()+1,sTmp.size());
		string::size_type pos = sTmp.find(':');
		if (pos != string::npos)
			sTmp = sTmp.substr(0,pos);

		map<string,CInterfaceGroup*>::iterator it = _Accel.find(sTmp);
		if (it != _Accel.end())
		{
			CInterfaceGroup *pIG = it->second;
			return pIG->getElement(id);
		}
		return NULL;
	}

	virtual void addGroup (CInterfaceGroup *child, sint eltOrder = -1)
	{
		string sTmp = child->getId();
		sTmp = sTmp.substr(_Id.size()+1,sTmp.size());
		_Accel.insert(pair<string,CInterfaceGroup*>(sTmp, child));
		CInterfaceGroup::addGroup(child,eltOrder);
	}

	virtual bool delGroup (CInterfaceGroup *child, bool dontDelete = false)
	{
		string sTmp = child->getId();
		sTmp = sTmp.substr(_Id.size()+1,sTmp.size());
		map<string,CInterfaceGroup*>::iterator it = _Accel.find(sTmp);
		if (it != _Accel.end())
		{
			_Accel.erase(it);
		}
		return CInterfaceGroup::delGroup(child,dontDelete);
	}

private:
	map<string,CInterfaceGroup*> _Accel;
};

// ----------------------------------------------------------------------------
// CInterfaceParser
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
CInterfaceParser::CInterfaceParser()
{
	// LUA
	_LuaState= NULL;
}

CInterfaceParser::~CInterfaceParser()
{
	_LuaState = NULL;
}
/** Convert a string into a memstream
  */
static void interfaceScriptAsMemStream(const std::string &script, CMemStream &destStream)
{
	NLMISC::contReset(destStream);
	if (destStream.isReading()) // we must be sure that we are reading the stream
	{
		destStream.invert();
	}
	destStream.seek(0, NLMISC::IStream::begin);
	if (script.empty()) return;
	destStream.serialBuffer(const_cast<uint8 *>((const uint8 *) &script[0]), (uint)script.size());
	destStream.invert();
	destStream.seek(0, NLMISC::IStream::begin);
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseInterface (const std::vector<std::string> & strings, bool reload, bool isFilename, bool checkInData)
{
	bool	ok;

	bool needCheck = false;

#if !FINAL_VERSION
	needCheck = false;
#endif

	// TestYoyo. UnHide For Parsing Profile
	/*
	NLMISC::CHTimer::startBench();
	{

	H_AUTO(parseInterface);
	*/

	//ignore the content of tags containing only white space
	xmlKeepBlanksDefault(0);
	//parse all interface files and build a single xml document
	xmlNodePtr globalEnclosing;
	nlassert (strings.size());
	CIXml read;
	string nextFileName;
	static const char *SCRIPT_AS_STRING = "<script as string>";
	try
	{
		CIFile file;
		CMemStream scriptStream;
		string firstFileName;
		vector<string>::const_iterator it = strings.begin();
		if (isFilename)
		{
			//get the first file document pointer
			firstFileName = *it;
			string filename = CPath::lookup(firstFileName);
			bool isInData = false;
			string::size_type pos = filename.find ("@");
			if (pos != string::npos)
			{
				vector<string> bigFilePaths;
				CBigFile::getInstance().getBigFilePaths(bigFilePaths);
				if (CBigFile::getInstance().getBigFileName(filename.substr(0, pos)) != "data/"+filename.substr(0, pos))
					isInData = false;
				else
					isInData = true;
			}

			if ((needCheck && !isInData) || !file.open (CPath::lookup(firstFileName)))
			{
				// todo hulud interface syntax error
				nlwarning ("could not open file %s, skipping xml parsing",firstFileName.c_str());
				return false;
			}
			read.init (file);
		}
		else
		{
			firstFileName = SCRIPT_AS_STRING; // for error msg
			interfaceScriptAsMemStream(*it, scriptStream);
			read.init(scriptStream);
		}
		//get the enclosing element (<interface config>)
		globalEnclosing = read.getRootNode();
		if (!globalEnclosing)
		{
			// todo hulud interface syntax error
			nlwarning ("no root element in xml file %s, skipping xml parsing",firstFileName.c_str());
			return false;
		}
		if (strcmp( (char*)globalEnclosing->name,"interface_config") )
		{
			// todo hulud interface syntax error
			nlwarning ("wrong root element in xml file %s, skipping xml parsing",firstFileName.c_str());
			return false;
		}
		if (isFilename)
		{
			file.close();
		}

		// Get all other xml files, and add their nodes to the first xml document
		it++;
		uint32 i = 0;
		for (; it != strings.end(); it++)
		{
			//nlwarning("Parsing interface file : %s", it->c_str());
			nextFileName = *it;
			CIXml nextRead;
			xmlNodePtr cur = NULL;
			bool saveParseResult = false;
			bool readFromUncompressedXML = true;
			if( false /* isFilename && ClientCfg.CacheUIParsing */ )
			{
				saveParseResult = true;
				std::string archive = CPath::lookup(nextFileName + "_compressed", false, false);
				std::string current = CPath::lookup(nextFileName, false, false);
				if (!archive.empty() && !current.empty())
				{
					if (CFile::getFileModificationDate(current) <= CFile::getFileModificationDate(archive))
					{
						CIFile input;
						input.open(archive);
						input.serialCheck(UI_CACHE_SERIAL_CHECK);
						input.serialVersion(0);
						cur = buildTree(input);
						input.serialCheck(UI_CACHE_SERIAL_CHECK);
						readFromUncompressedXML = false;
						saveParseResult = false;
					}
				}
			}
			if (!cur)
			{
				if (isFilename)
				{
					if (!file.open(CPath::lookup(nextFileName, false, false)))
					{
						// todo hulud interface syntax error
						nlwarning ("could not open file %s, skipping xml parsing",nextFileName.c_str());
						return false;
					}
					nextRead.init (file);
				}
				else
				{
					interfaceScriptAsMemStream(nextFileName, scriptStream);
					nextFileName = SCRIPT_AS_STRING; // for error MSG
					read.init(scriptStream);
				}
				cur = nextRead.getRootNode();
				if (!cur)
				{
					// todo hulud interface syntax error
					nlwarning ("no root element in  xml file %s, skipping xml parsing", it->c_str() );
					return false;
				}
			}
			if (saveParseResult)
			{
				nlassert(isFilename);
				std::string outputFilename = CPath::standardizePath("data") + CFile::getFilename(nextFileName) + std::string("_compressed");
				COFile f;
				f.open(outputFilename);
				f.serialCheck(UI_CACHE_SERIAL_CHECK);
				f.serialVersion(0);
				saveXMLTree(f, cur);
				f.serialCheck(UI_CACHE_SERIAL_CHECK);
			}
			if (strcmp( (char*)cur->name,"interface_config") )
			{
				// todo hulud interface syntax error
				nlwarning ("wrong root element in xml file %s. should be interface config, skipping xml parsing", nextFileName.c_str());
				return false;
			}
			xmlNodePtr curSon = cur->children;
			while (curSon)
			{
				xmlNodePtr bufNode = xmlCopyNode (curSon, 1);
				xmlAddChild (globalEnclosing,bufNode);
				curSon = curSon->next;
			}
			if (!readFromUncompressedXML)
			{
				freeXMLNodeAndSibblings(cur);
			}
			if (isFilename)
			{
				file.close();
			}
			i++;
		}
	}
	catch (const Exception &e)
	{
		// Output error
		// todo hulud interface syntax error
		nlwarning ("CInterfaceParser: Error while loading the xml interface file %s, skipping xml parsing : %s", nextFileName.c_str(), e.what());
		if (testWildCard(nextFileName, "save/keys_?*.xml"))
		{
			// if file matches 'save/keys_?*.xml', move this file as a backup
			string	backup = nextFileName+".backup";
			if (CFile::fileExists(backup))
				CFile::deleteFile(backup);
			CFile::moveFile(backup.c_str(), nextFileName.c_str());
		}
		return false;
	}
	//parse the built doc
	ok = parseXMLDocument(globalEnclosing, reload);
//	freeXMLNodeAndSibblings(globalEnclosing); // Done by the ~CIXml


	// TestYoyo. UnHide for Parsing Profile
	/*
	}
	NLMISC::CHTimer::endBench();
	// Display and save profile to a File.
	CLog	log;
	CFileDisplayer	fileDisplayer(NLMISC::CFile::findNewFile(getLogDirectory() + "profile_parseInterface.log"));
	log.addDisplayer(&fileDisplayer);
	// diplay
	NLMISC::CHTimer::displayHierarchicalByExecutionPathSorted(&log, CHTimer::TotalTime, true, 48, 2);
	NLMISC::CHTimer::display(&log, CHTimer::TotalTime);
	*/


	return ok;
}


// ----------------------------------------------------------------------------
bool CInterfaceParser::parseXMLDocument(xmlNodePtr root, bool reload)
{
	H_AUTO(parseXMLDocument);

	CWidgetManager::SMasterGroup *curRoot = NULL;
	CInterfaceGroup *rootGroup = NULL;
	//parse templates
	xmlNodePtr curNode = root->children;

	// Resize action category array
	uint actionCategoryCount = CIXml::countChildren(curNode, "action_category");
	Actions.reserveCategories((uint)Actions.getCategories ().size()+actionCategoryCount);
	EditActions.reserveCategories(1);

	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();

	while (curNode)
	{
		// first solve define for the xml node and his sons
		if(!solveDefine(curNode))
		{
			// todo hulud interface syntax error
			nlwarning ("could not read all define");
		}
		// then try to solve Style for the xml node and his sons
		else if (!solveStyle(curNode))
		{
			// todo hulud interface syntax error
			nlwarning ("could not read all styles");
		}
		// If define and style oks, try to parse "1st pass" objets (define, options....).
		else if ( !strcmp((char*)curNode->name,"template") )
		{
			// Check there is a valid name for this template
			CXMLAutoPtr ptr((const char*) xmlGetProp( curNode, (xmlChar*)"name" ));
			if (ptr)
			{
				// remove any template with the same name in the list (useful when using 'loadui' command)
				for(uint k = 0; k < _Templates.size(); ++k)
				{
					CXMLAutoPtr otherTemplName((const char*) xmlGetProp( _Templates[k], (xmlChar*)"name" ));
					if (strcmp((const char *) otherTemplName, (const char *) ptr) == 0)
					{
						nlwarning("Replacing template %s with new version", (const char *) ptr);
						xmlFreeNode(_Templates[k]);
						_Templates[k] = NULL;
					}
				}
				_Templates.erase(std::remove(_Templates.begin(), _Templates.end(), (xmlNodePtr) NULL), _Templates.end());
				_Templates.push_back(curNode);
			}
			else
				// todo hulud interface syntax error
				nlwarning ("no name in a template node");
		}
		else if ( !strcmp((char*)curNode->name,"options") )
		{
			if (!parseOptions(curNode,rootGroup))
				// todo hulud interface syntax error
				nlwarning ("could not parse options");
		}
		else if ( !strcmp((char*)curNode->name,"define") )
		{
			if (!parseDefine(curNode))
				// todo hulud interface syntax error
				nlwarning ("could not parse define");
		}
		else if ( !strcmp((char*)curNode->name,"action_category") )
		{
			if (!parseActionCategory(curNode))
				// todo hulud interface syntax error
				nlwarning ("could not parse action_category");
		}
		else if ( !strcmp((char*)curNode->name,"key") )
		{
			parseKey(curNode);
		}
		else if ( !strcmp((char*)curNode->name,"macro") )
		{
			parseMacro(curNode);
		}
		else if ( !strcmp((char*)curNode->name,"command") )
		{
			parseCommand(curNode);
		}
		else if ( !strcmp((char*)curNode->name,"style") )
		{
			if (!parseStyle(curNode))
				// todo hulud interface syntax error
				nlwarning ("could not parse 'style'");
		}

		curNode = curNode->next;
	}


	if (!reload) // TMP : crahs when doing the setup twice (old pointer on the text manager ...)
	{
		setupOptions();
	}


	//parse filters , groups and eventually instances
	// vector that are on top of the xml hierarchy
	root = root->children;
	while (root)
	{
		if ( !strcmp((char*)root->name,"root") )
		{
			CXMLAutoPtr ptr((const char*)xmlGetProp (root, (xmlChar*)"id"));
			if (ptr)
			{
				rootGroup = CWidgetManager::getInstance()->getMasterGroupFromId (string("ui:") + (const char*)ptr);
				if (rootGroup == NULL)
				{
					rootGroup = (CInterfaceGroup*)(new CRootGroup(CViewBase::TCtorParam()));
					rootGroup->parse (root, NULL);
					CWidgetManager::SMasterGroup mg;
					mg.Group = rootGroup;
					_MasterGroups.push_back (mg);
				}
				for (uint32 i = 0; i < _MasterGroups.size(); ++i)
					if (_MasterGroups[i].Group == rootGroup)
						curRoot = &_MasterGroups[i];
			}
			else
			{
				// todo hulud interface syntax error
				nlwarning ("could not parse root");
			}
		}
		else if (!strcmp((char*)root->name,"group"))
		{
			if (!parseGroup(root,rootGroup, reload))
				// todo hulud interface syntax error
				nlwarning ("could not parse group");
		}
		else if (!strcmp((char*)root->name,"instance"))
		{
			if (!parseInstance(root))
				// todo hulud interface syntax error
				nlwarning ("could not parse instance");
		}
		else if (!strcmp((char*)root->name,"view"))
		{
			if (!parseView(root,rootGroup, reload))
				// todo hulud interface syntax error
				nlwarning ("could not parse view");
		}
		else if (!strcmp((char*)root->name,"ctrl"))
		{
			if (!parseControl(root,rootGroup, reload))
				// todo hulud interface syntax error
				nlwarning ("could not parse control");
		}
		else if ( !strcmp((char*)root->name,"vector") )
		{
			if (!parseVector(root))
				// todo hulud interface syntax error
				nlwarning ("could not parse vector");
		}
		else if ( !strcmp((char*)root->name,"link") )
		{
			if (!parseLink(root,rootGroup))
				// todo hulud interface syntax error
				nlwarning ("could not parse link");
		}
		else if ( !strcmp((char*)root->name,"variable") )
		{
			if (!parseVariable(root,rootGroup))
				// todo hulud interface syntax error
				nlwarning ("could not parse variable");
		}
		else if ( !strcmp((char*)root->name,"tree") )
		{
			if (!parseTree(root,curRoot))
				// todo hulud interface syntax error
				nlwarning ("could not parse tree");
			/* if (!setupTree(root,curRoot))
				nlwarning ("could not setup tree"); */
		}
		else if ( !strcmp((char*)root->name,"proc") )
		{
			if (!parseProcedure(root, reload))
				// todo hulud interface syntax error
				nlwarning ("could not parse procedure");
		}
		else if ( !strcmp((char*)root->name,"sheet_selection") )
		{
			if (!parseSheetSelection(root))
				// todo hulud interface syntax error
				nlwarning ("could not parse sheet selection");
		}
		else if ( !strcmp((char*)root->name,"anim") )
		{
			if (!parseAnim(root,rootGroup))
				// todo hulud interface syntax error
				nlwarning ("could not parse 'anim'");
		}
		else if ( !strcmp((char*)root->name,"scene3d") )
		{
			if (!parseScene3D(root,rootGroup))
				// todo hulud interface syntax error
				nlwarning ("could not parse 'scene3d'");
		}
		// Special BrickViewer
		else if ( !strcmp((char*)root->name,"brick_career_generator") )
		{
			if (!parseBrickCareerGenerator(root))
				// todo hulud interface syntax error
				nlwarning ("could not parse 'brick_career_generator'");
		}
		else if ( !strcmp((char*)root->name,"brick_suffix_generator") )
		{
			if (!parseBrickSuffixGenerator(root))
				// todo hulud interface syntax error
				nlwarning ("could not parse 'brick_suffix_generator'");
		}
		else if ( !strcmp((char*)root->name,"ddx") )
		{
			if (!parseDDX(root,rootGroup))
				// todo hulud interface syntax error
				nlwarning ("could not parse 'ddx'");
		}
		else if ( !strcmp((char*)root->name,"lua") )
		{
			if(!parseLUAScript(root))
			{
				nlwarning ("could not parse 'lua'");
				exit( EXIT_FAILURE );
			}
		}

		root = root->next;
	}

	// add all modals group to the window list
	for (uint32 i = 0; i < _MasterGroups.size(); ++i)
	{
		H_AUTO(addWindowToMasterGroup)

		CWidgetManager::SMasterGroup &rMG = _MasterGroups[i];
		// insert all modals
		for (uint32 j = 0; j < rMG.Group->getGroups().size(); ++j)
		{
			CGroupModal *pIG = dynamic_cast<CGroupModal*>(rMG.Group->getGroups()[j]);
			// if it is a modal group
			if(pIG)
			{
				// add to the window list
				CWidgetManager::getInstance()->addWindowToMasterGroup(rMG.Group->getId(), pIG);
			}
		}
	}

	// init all the elements coords, and Lua Script
	if (!initCoordsAndLuaScript())
		return false;

	std::vector<xmlNodePtr> keptTemplates;

	// keep template that have the "keep" flag if we are not reloading
	// if reloading, always keep the template
	{
		H_AUTO(keep_template)

			CXMLAutoPtr ptr;
		for(uint k = 0; k < _Templates.size(); ++k)
		{
			CXMLAutoPtr ptr(xmlGetProp(_Templates[k], (const xmlChar *) "keep"));
			if (reload || (ptr && nlstricmp((const char *) ptr, "true") == 0))
			{
				// xmlUnsetProp(_Templates[k], (const xmlChar *) "keep");
				xmlNodePtr	copy = xmlCopyNode(_Templates[k], 1);
				if (copy)
					keptTemplates.push_back(copy);
			}

			// free the original node, whether we kept a copy or not
			xmlUnlinkNode( _Templates[k] );
			xmlFreeNode( _Templates[k] );
			_Templates[k] = NULL;
		}
		// keep new list
		_Templates.swap(keptTemplates);
	}
	return true;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseTemplateNode(xmlNodePtr node,xmlNodePtr instance,xmlNodePtr templ)
{
	CXMLAutoPtr ptr;
	//get the node properties
	xmlAttrPtr props = node->properties;
	while (props)
	{
		//get the property value
		ptr = (char*)xmlGetProp( node, props->name);
		nlassert(ptr);
		//if it begins with a #, it is a reference in the instance attribute
		if (strchr(ptr, '#') != NULL)
		{
			string LastProp = ptr;
			string NewProp ="";
			string RepProp;

			while (LastProp.size() > 0)
			{
				string::size_type diesPos = LastProp.find("#");
				if (diesPos != string::npos)
				{
					if (diesPos > 0)
					{
						NewProp += LastProp.substr(0, LastProp.find("#"));
						LastProp = LastProp.substr(LastProp.find("#"),LastProp.size());
					}

					CXMLAutoPtr instanceProp;
					for (uint32 i = 0; i < LastProp.size(); ++i)
					{
						RepProp = LastProp.substr(0, LastProp.size()-i);
						instanceProp = xmlGetProp (instance , (const xmlChar*)(RepProp.c_str() + 1));
						if (instanceProp)
							break;
						instanceProp = xmlGetProp (templ , (const xmlChar*)(RepProp.c_str() + 1));
						if (instanceProp)
							break;
					}
					if (!instanceProp)
					{
						CXMLAutoPtr ptr2((const char*)xmlGetProp( instance, (xmlChar*)"id"));
						string sTmp;
						if (ptr2.getDatas() != NULL)
							sTmp = string("cannot parse template node property: ") + ((const char *) ptr + 1) + string(" in instance : ") + string((const char*)ptr2);
						else
							sTmp = string("cannot parse template node property: ") + ((const char *) ptr + 1) + string(" in instance : NULL");
						// todo hulud interface syntax error
						nlinfo(sTmp.c_str());

						return false;
					}
					NewProp += string((const char*)instanceProp);
					LastProp = LastProp.substr (RepProp.size(), LastProp.size());
				}
				else
				{
					NewProp += LastProp;
					LastProp = "";
				}
			}
			xmlSetProp(node,props->name, (const xmlChar*)NewProp.c_str());
		}
		props = props->next;
	}
	//parse the node children
	node = node->children;
	while (node)
	{
		if (!parseTemplateNode(node,instance,templ))
			return false;
		node = node->next;
	}
	return true;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseInstance(xmlNodePtr cur)
{
	H_AUTO(parseInstance)

	CXMLAutoPtr ptr;

	//try to find the instance template in our template vector. If the template doesn't exist, return false
	CXMLAutoPtr templ((const char*) xmlGetProp( cur, (xmlChar*)"template" ));
	if (!templ)
	{
		// todo hulud interface syntax error
		nlinfo("parse error : no referenced template in an instance");
		return false;
	}
	vector<xmlNodePtr>::const_iterator it;
	for (it = _Templates.begin(); it != _Templates.end();it++)
	{
		ptr = (char*) xmlGetProp( *it, (xmlChar*)"name" );
		if (!ptr)
		{
			// todo hulud interface syntax error
			nlinfo("no name in a template node");
			return false;
		}
		if ( !strcmp(templ,ptr) )
		{
			break;
		}
	}
	if ( it == _Templates.end() )
	{
		// todo hulud interface syntax error
		nlinfo("the template %s was not found", (const char*)templ);
		return false;
	}

	xmlNodePtr templNode = *it;
	//for each child of the template, create the appropriate node
	xmlNodePtr child = (*it)->children;
	xmlNodePtr nextSibling=cur;
	while (child)
	{
		//copy the template child node
		xmlNodePtr node = xmlCopyNode (child, 1);

		//add it to our tree
		//node = xmlAddChild(cur->parent,node);
		node = xmlAddNextSibling (nextSibling, node);
		nextSibling = nextSibling->next;

		//parse the child
		if (!parseTemplateNode(node, cur, templNode))
			return false;
		child = child->next;
	}
	return true;
}

// ----------------------------------------------------------------------------
/*bool CInterfaceParser::parseDynamicList(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	CGroupListDynamic* li = new CGroupListDynamic;
	if (!li->parse(cur,parentGroup))
	{
		nlinfo("failed to parse a dynamic list");
		delete li;
		return false;
	}

	//copy the templates used by the instance of the list, otherwise it will be scratched after init
	xmlNodePtr listChild = cur->children;
	//listChild should exist here otherwise li->parse would have returned false
	nlassert(listChild);
	CXMLAutoPtr buf = (char*) xmlGetProp( listChild, (xmlChar*)"template" );
	if (!buf)
	{
		nlinfo(" dynamic list : the child instance has no template attribute");
		return false;
	}

	for (vector<xmlNodePtr>::const_iterator it = _Templates.begin(); it != _Templates.end();it++)
	{
		CXMLAutoPtr ptr = (char*) xmlGetProp( *it, (xmlChar*)"name" );
		if (!ptr)
		{
			nlinfo("no name in a template node");
			return false;
		}
		if ( !strcmp(buf,ptr) )
		{
			break;
		}
	}
	xmlNodePtr node = xmlCopyNode(*it,1);
	_KeptTemplates.push_back(node);

	CXMLAutoPtr dependencies = (char*) xmlGetProp( cur, (xmlChar*)"dependencies" );
	if (dependencies)
	{
		char *seekPtr = dependencies.getDatas();
		seekPtr = strtok(seekPtr," ,\t");
		while (seekPtr)
		{
			for (vector<xmlNodePtr>::const_iterator it = _Templates.begin(); it != _Templates.end();it++)
			{
				CXMLAutoPtr ptr = (char*) xmlGetProp( *it, (xmlChar*)"name" );
				if (!ptr)
				{
					nlinfo("no name in a template node");
					return false;
				}
				if ( !strcmp(seekPtr, ptr) )
				{
					break;
				}
			}
			xmlNodePtr node = xmlCopyNode(*it,1);
			_KeptTemplates.push_back(node);
			seekPtr = strtok(NULL," ,\t");
		}
	}

	//add the list in the tree
	if (parentGroup)
	{
		parentGroup->addGroup(li);
	}
	return true;
}*/

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseVector(xmlNodePtr cur)
{
	H_AUTO(parseVector)

	//get the number of elements
	CXMLAutoPtr cSize((const char*) xmlGetProp( cur, (xmlChar*)"_size" ));
	if (!cSize)
	{
		// todo hulud interface syntax error
		nlinfo("no _size in a vector");
		return false;
	}
	sint32 size;
	fromString(cSize, size);
	if (size <= 0)
	{
		// todo hulud interface syntax error
		nlinfo("size<0");
		return false;
	}

	// Vector of groups
	bool bGroupVector = true;

	//get the first position reference
	CXMLAutoPtr firstpos((const char*) xmlGetProp( cur, (xmlChar*)"_firstpos" ));
	if (!firstpos)
	{
		bGroupVector = false;
	}
	//get the next position reference
	CXMLAutoPtr nextpos((const char*) xmlGetProp( cur, (xmlChar*)"_nextpos" ));
	if (!nextpos)
	{
		bGroupVector = false;
	}

	sint32 index = 0;
	//get the first index value
	CXMLAutoPtr indexChar((const char*) xmlGetProp( cur, (xmlChar*)"_firstindex" ));
	if (indexChar)
	{
		fromString((const char*)indexChar, index);
	}

	sint32 step = 1;
	//get the step for the indices
	CXMLAutoPtr stepChar((const char*) xmlGetProp( cur, (xmlChar*)"_step" ));
	if (stepChar)
	{
		fromString((const char*)stepChar, step);
	}


	//get the x and y of the first element
	CXMLAutoPtr xfirst((const char*) xmlGetProp( cur, (xmlChar*)"_xfirst" ));
	CXMLAutoPtr yfirst((const char*) xmlGetProp( cur, (xmlChar*)"_yfirst" ));

	xmlNodePtr node;
	CXMLAutoPtr id;
	xmlNodePtr nextSibling= cur;
	//now we can add all the following elements
	sint32 i;
	for (i = index;; i += step)
	{
		if (step > 0)
		{
			if (i >= index + size) break;
		}
		else
		{
			if (i <= index - size) break;
		}
		//copy the node
		node = xmlCopyNode(cur,1);
		//set the name and posref

		if (bGroupVector)
		{
			if (i==index)
			{
				xmlSetProp(node,(xmlChar*)"posref",(const xmlChar*)firstpos);
				if (xfirst)
				{
					xmlSetProp(node,(xmlChar*)"x",(const xmlChar*)xfirst);
				}
				if (yfirst)
				{
					xmlSetProp(node,(xmlChar*)"y",(const xmlChar*)yfirst);
				}
			}
			else
			{
				xmlSetProp(node,(xmlChar*)"posref",(const xmlChar*)nextpos);
				xmlSetProp(node,(xmlChar*)"posparent",(const xmlChar*)id);
			}
		}

		xmlNodeSetName(node,(xmlChar*)"instance");

		//replace all the $i
		xmlAttrPtr attr = node->properties;
		while(attr)
		{
			CXMLAutoPtr prop((const char*) xmlGetProp( node, attr->name ));
			string str((const char*)prop);
			string::size_type pos = str.find("$i");
			while (pos != string::npos)
			{
				str.replace(pos,2,toString(i));
				pos = str.find("$i",pos);
			}
			xmlSetProp(node,attr->name,(xmlChar*)str.c_str());
			attr = attr->next;

		}
		//add the new instance to our tree, just near us.
		xmlAddNextSibling (nextSibling, node);
		nextSibling= node;

		//get the node id, used in the next iteration
		if (bGroupVector)
		{
			id = (char*) xmlGetProp( node, (xmlChar*)"id" );
			if (!id)
			{
				// todo hulud interface syntax error
				nlinfo("no id in a vector");
				break;
			}
		}
	}

	return step > 0 ? (i == index + size) : (i == index - size);
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseLink(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	H_AUTO(parseLink)

	CXMLAutoPtr ptr((const char*) xmlGetProp (cur, (xmlChar*)"expr"));
	if (!ptr)
	{
		// todo hulud interface syntax error
		nlwarning("<CInterfaceParser::parseLink> Can't read  the expression for a link node");
		return false;
	}
	std::string expr = ptr;


	std::vector<CInterfaceLink::CTargetInfo> targets;

	ptr = (char*) xmlGetProp (cur, (xmlChar*)"target");
	if (ptr)
	{
		CInterfaceLink::splitLinkTargets(std::string((const char*)ptr), parentGroup, targets);
	}
	// optional action handler
	std::string action;
	std::string params;
	std::string cond;
	ptr = (char*) xmlGetProp (cur, (xmlChar*)"action");
	if (ptr) action = (const char *) ptr;
	ptr = (char*) xmlGetProp (cur, (xmlChar*)"params");
	if (ptr) params = (const char *) ptr;
	ptr = (char*) xmlGetProp (cur, (xmlChar*)"cond");
	if (ptr) cond = (const char *) ptr;

	// create the link
	CInterfaceLink *il = new CInterfaceLink;
	il->init(targets, expr, action, params, cond, parentGroup); // init will add 'il' in the list of link present in 'elm'
	return true;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseVariable (xmlNodePtr cur, CInterfaceGroup * /* parentGroup */)
{
	H_AUTO(parseVariable )

	//get the args
	CXMLAutoPtr ptr((const char*) xmlGetProp (cur, (xmlChar*)"entry"));
	if (!ptr)
	{
		// todo hulud interface syntax error
		nlinfo ("no entry in a variable tag");
		return false;
	}
	string entry((const char*)ptr);

	ptr = (char*) xmlGetProp (cur, (xmlChar*)"type");
	if (!ptr)
	{
		// todo hulud interface syntax error
		nlinfo ("no type in a variable tag");
		return false;
	}
	string type((const char*)ptr);

	string value;
	ptr = (char*) xmlGetProp (cur, (xmlChar*)"value");
	if (!ptr)
	{
		//if no value is specified, try to get the db entry directly
		value = entry;
	}
	else
		value = string((const char*)ptr);

	// Array definition
	sint	size= 1;
	bool	ArrayMode= false;
	string	entryPrefix;
	string	entrySuffix;
	ptr = (char*) xmlGetProp (cur, (xmlChar*)"size");
	if (ptr)
	{
		ArrayMode= true;
		fromString((const char*)ptr, size);
		string::size_type pos= entry.find("$i");
		if( pos==string::npos )
		{
			// todo hulud interface syntax error
			nlinfo ("no $i found in a 'variable' tag with 'size' defined ");
			return false;
		}
		else
		{
			entryPrefix= entry.substr(0, pos);
			entrySuffix= entry.substr(pos+2);
		}
	}

	// loop all variables
	for(sint index= 0;index<size;index++)
	{
		// If array variable, build the variable name
		if(ArrayMode)
		{
			entry= entryPrefix + toString(index) + entrySuffix;
		}

		// access the database
		CInterfaceProperty prop;

		if (type == "sint64")
			prop.readSInt64(value.c_str(),entry);
		else if (type == "sint32")
			prop.readSInt32(value.c_str(),entry);
		else if (type == "float" || type == "double")
			prop.readDouble(value.c_str(),entry);
		else if (type == "bool")
			prop.readBool(value.c_str(),entry);
		else if (type == "rgba")
			prop.readRGBA(value.c_str(),entry);
		else if (type == "hotspot")
			prop.readHotSpot(value.c_str(),entry);
		else if (type == "text")
		{
			/*uint textId = addText(value);
			prop.readSInt32(toString(textId),entry);*/
		}
	}

	return true;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseOptions (xmlNodePtr cur, CInterfaceGroup * /* parentGroup */)
{
	H_AUTO(parseOptions )

	// build the options from type
	CInterfaceOptions *options;
	CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"type" ));
	if (ptr)
	{
		if (nlstricmp(ptr.getDatas(), "layer") == 0)
			options = new COptionsLayer;
		else if (nlstricmp(ptr.getDatas(), "container_insertion_opt") == 0)
			options = new COptionsContainerInsertion;
		else if (nlstricmp(ptr.getDatas(), "container_move_opt") == 0)
			options = new COptionsContainerMove;
		else if (nlstricmp(ptr.getDatas(), "list") == 0)
			options = new COptionsList;
		else if (nlstricmp(ptr.getDatas(), "mission_icons") == 0)
			options = new CMissionIconList;
		else if (nlstricmp(ptr.getDatas(), "animation_set") == 0)
			options = new COptionsAnimationSet;
		else
			options = new CInterfaceOptions;
	}
	else
	{
		options = new CInterfaceOptions;
	}

	CWidgetManager *wm = CWidgetManager::getInstance();

	// get the name
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"name" );
	if (!ptr)
	{
		// todo hulud interface syntax error
		nlinfo ("options has no name");
		return false;
	}
	string optionsName = ptr;

	// herit if possible
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"herit" );
	if (ptr)
	{
		string optionsParentName = ptr;
		CInterfaceOptions *io = wm->getOptions( optionsParentName );
		if( io != NULL )
			options->copyBasicMap( *io );
	}

	// parse parameters
	if (options->parse (cur))
	{
		// Remove old one
		wm->removeOptions( optionsName );
		wm->addOptions( optionsName, options );
	}
	else
	{
		delete options;
		return false;
	}
	return true;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseGroup (xmlNodePtr cur, CInterfaceGroup * parentGroup, bool reload)
{
	H_AUTO(parseGroup )

	CInterfaceGroup * group;
	CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"type" ));
	if (ptr)
	{
		group = dynamic_cast<CInterfaceGroup*>(NLMISC_GET_FACTORY(CViewBase, std::string).createObject(string((const char*)ptr), CViewBase::TCtorParam()));
		if (group == NULL)
		{
			group = dynamic_cast<CInterfaceGroup*>(NLMISC_GET_FACTORY(CViewBase, std::string).createObject("interface_group", CViewBase::TCtorParam()));
		}

	}
	else
		group = dynamic_cast<CInterfaceGroup*>(NLMISC_GET_FACTORY(CViewBase, std::string).createObject("interface_group", CViewBase::TCtorParam()));

	// parse the group attributes
	if (!group->parse(cur,parentGroup))
	{
		delete group;
		// todo hulud interface syntax error
		nlwarning ("cannot parse group attributes");
		return false;
	}

	if (parentGroup)
	{
		CGroupList *pList = dynamic_cast<CGroupList*>(parentGroup);
		if (parentGroup->getElement(group->getId()) != NULL)
		{
			// Remove old groupe and replace
			if (reload)
			{
				// Remove from the parent
				parentGroup->delElement (group->getId(), true);
			}
			else
			{
				// todo hulud interface syntax error
				nlwarning ("id already exists for %s in %s", group->getId().c_str(), parentGroup->getId().c_str());
				delete group;
				return false;
			}
		}

		if (pList != NULL)
			pList->addChild (group);
		else
			parentGroup->addGroup (group);
	}
	else
	{
		// todo hulud interface syntax error
		nlinfo ("no parent for %s", group->getId().c_str());
		delete group;
		return false;
	}

	//parse the children
	bool ok = parseGroupChildren(cur, group, reload);

	if (!ok)
	{
		string tmp = "cannot parse group "+group->getId();
		// todo hulud interface syntax error
		nlinfo (tmp.c_str());
	}
	return ok;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseGroupChildren(xmlNodePtr cur, CInterfaceGroup * parentGroup, bool reload)
{
	cur = cur->children;
	bool ok = true;
	while (cur)
	{
		if ( !strcmp((char*)cur->name,"view") )
			ok = ok && parseView(cur,parentGroup, reload);
		else if ( !strcmp((char*)cur->name,"ctrl") )
			ok = ok && parseControl(cur,parentGroup, reload);
		else if ( !strcmp((char*)cur->name,"group") )
			ok = ok && parseGroup(cur,parentGroup, reload);
		else if ( !strcmp((char*)cur->name,"instance") )
			ok = ok && parseInstance(cur);
		else if ( !strcmp((char*)cur->name,"vector") )
			ok = ok && parseVector(cur);
		else if ( !strcmp((char*)cur->name,"link") )
			ok = ok && parseLink(cur,parentGroup);
		else if ( !strcmp((char*)cur->name,"scene3d") )
			ok = ok && parseScene3D(cur,parentGroup);
		else if ( !strcmp((char*)cur->name,"ddx") )
			ok = ok && parseDDX(cur,parentGroup);


		cur = cur->next;
	}
	return ok;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseControl (xmlNodePtr cur, CInterfaceGroup * parentGroup, bool reload)
{
	H_AUTO(parseControl )

	CCtrlBase* ctrl = NULL;
	CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"type" ));
	if (!ptr)
	{
		// todo hulud interface syntax error
		nlinfo ("no type in a control tag");
		return false;
	}

	ctrl = dynamic_cast<CCtrlBase*>(NLMISC_GET_FACTORY(CViewBase, std::string).createObject(string((const char*)ptr), CViewBase::TCtorParam()));

	if (ctrl)
	{
		if (!ctrl->parse(cur,parentGroup))
		{
			delete ctrl;
			return false;
		}
		if (parentGroup->getElement(ctrl->getId()) != NULL)
		{
			// Remove old groupe and replace
			if (reload)
				parentGroup->delElement (ctrl->getId());
			else
			{
				// todo hulud interface syntax error
				nlwarning ("id already exists for %s in %s", ctrl->getId().c_str(), parentGroup->getId().c_str());
				delete ctrl;
				return false;
			}
		}
		// Add the ctrl to the parent group
		parentGroup->addCtrl(ctrl);
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseView(xmlNodePtr cur, CInterfaceGroup * parentGroup, bool reload)
{
	H_AUTO(parseView)

	CViewBase * view=NULL;
	CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"type" ));
	if (!ptr)
	{
		// todo hulud interface syntax error
		nlinfo("no type in a view");
		return false;
	}

	view = NLMISC_GET_FACTORY(CViewBase, std::string).createObject(string((const char*)ptr), CViewBase::TCtorParam());

	if ( !strcmp(ptr,"pointer"))
	{
		CWidgetManager::getInstance()->setPointer( dynamic_cast<CViewPointer*>(view) );
	}

	//nlinfo("view type %s mem : %d",ptr,view->getMemory());
	if (view)
	{
		if (!view->parse(cur,parentGroup))
		{
			delete view;
			return false;
		}
		if (parentGroup->getElement(view->getId()) != NULL)
		{
			// Remove old groupe and replace
			if (reload)
				parentGroup->delElement (view->getId());
			else
			{
				// todo hulud interface syntax error
				nlwarning ("id already exists for %s in %s", view->getId().c_str(), parentGroup->getId().c_str());
				delete view;
				return false;
			}
		}

		//add the view to the parent group
		CGroupList *pList = dynamic_cast<CGroupList*>(parentGroup);
		if (pList != NULL)
		{
			pList->addChild (view);
		}
		else
		{
			parentGroup->addView(view);
		}
		return true;
	}
	// todo hulud interface syntax error
	nlinfo("unknown view type %s", (const char*)ptr);
	return false;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseTreeNode (xmlNodePtr cur, CGroupContainer *parentGroup)
{
	CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"node" ));
	if (!ptr) return false;
	CInterfaceElement *pEltFound = NULL;
	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();
	for (uint32 i = 0; i < _MasterGroups.size(); ++i)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[i];
		for (uint32 j = 0; j < rMG.Group->getGroups().size(); ++j)
		{
			CInterfaceGroup *pIG = rMG.Group->getGroups()[j];
			string stmp = strlwr(pIG->getId().substr(pIG->getId().rfind(':')+1,pIG->getId().size()));
			string stmp2 = strlwr(string((const char*)ptr));
			if (stmp == stmp2)
			{
				pEltFound = pIG;
				break;
			}
		}
		if (pEltFound != NULL)
			break;
	}
	if (pEltFound == NULL)
	{
		string stmp = string("element not found for tree : ") + string((const char*)ptr);
		// todo hulud interface syntax error
		nlinfo(stmp.c_str());
		return false;
	}
	CGroupContainer *pIC = dynamic_cast<CGroupContainer*>(pEltFound);
	if (pIC == NULL)
	{
		string stmp = string("not a container : ") + pEltFound->getId();
		// todo hulud interface syntax error
		nlinfo(stmp.c_str());
		return false;
	}
	parentGroup->attachContainer (pIC);
	cur = cur->children;
	while (cur)
	{
		parseTreeNode(cur, pIC);
		cur = cur->next;
	}
	return true;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::setupTreeNode (xmlNodePtr cur, CGroupContainer * /* parentGroup */)
{
	CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"node" ));
	if (!ptr) return false;
	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();
	CInterfaceElement *pEltFound = NULL;
	for (uint32 i = 0; i < _MasterGroups.size(); ++i)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[i];
		for (uint32 j = 0; j < rMG.Group->getGroups().size(); ++j)
		{
			CInterfaceGroup *pIG = rMG.Group->getGroups()[j];
			string stmp = strlwr(pIG->getId().substr(pIG->getId().rfind(':')+1,pIG->getId().size()));
			string stmp2 = strlwr(string((const char*)ptr));
			if (stmp == stmp2)
			{
				pEltFound = pIG;
				break;
			}
		}
		if (pEltFound != NULL)
			break;
	}
	if (pEltFound == NULL)
	{
		string stmp = string("element not found for tree : ") + string((const char*)ptr);
		// todo hulud interface syntax error
		nlinfo(stmp.c_str());
		return false;
	}
	CGroupContainer *pIC = dynamic_cast<CGroupContainer*>(pEltFound);
	if (pIC == NULL)
	{
		string stmp = string("not a container : ") + pEltFound->getId();
		// todo hulud interface syntax error
		nlinfo(stmp.c_str());
		return false;
	}
	// See if the group should be docked.
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"docked" );
	if (nlstricmp((const char *) ptr, "true") == 0)
	{
		// dock the container
		pIC->popupCurrentPos();
		// compute position on screen
		sint32 x = 0, y = 0, w = 200, h = 100;
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"x" );
		if (ptr)
		{
			sint32 value;
			if (fromString((const char*)ptr, value))
			{
				x = value;
			}
		}
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"y" );
		if (ptr)
		{
			sint32 value;
			if (fromString((const char*)ptr, value))
			{
				y = value;
			}
		}
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"w" );
		if (ptr)
		{
			sint32 value;
			if (fromString((const char*)ptr, value))
			{
				w = value;
			}
		}
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"h" );
		if (ptr)
		{
			sint32 value;
			if (fromString((const char*)ptr, value))
			{
				h = value;
			}
		}
		pIC->setX(x);
		pIC->setY(y);
		pIC->setW(w);
		pIC->setH(h);
		pIC->invalidateCoords();
	}
	//
	while (cur)
	{
		setupTreeNode(cur, pIC);
		cur = cur->next;
	}
	return true;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::setupTree (xmlNodePtr cur, CWidgetManager::SMasterGroup * /* parentGroup */)
{
	CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"node" ));
	if (!ptr) return false;
	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();
	CInterfaceElement *pEltFound = NULL;
	for (uint32 i = 0; i < _MasterGroups.size(); ++i)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[i];
		for (uint32 j = 0; j < rMG.Group->getGroups().size(); ++j)
		{
			CInterfaceGroup *pIG = rMG.Group->getGroups()[j];
			string stmp = strlwr(pIG->getId().substr(pIG->getId().rfind(':')+1,pIG->getId().size()));
			string stmp2 = strlwr(string((const char*)ptr));
			if (stmp == stmp2)
			{
				pEltFound = pIG;
				break;
			}
		}
		if (pEltFound != NULL)
			break;
	}

	if (pEltFound == NULL)
	{
		string stmp = string("no group found for ") + string((const char*)ptr);
		// todo hulud interface syntax error
		nlinfo(stmp.c_str());
		return false;
	}

	// the element must be a group
	CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(pEltFound);
	if (pIG == NULL)
	{
		string stmp = string("not a group !") + pEltFound->getId();
		// todo hulud interface syntax error
		nlinfo(stmp.c_str());
		return false;
	}

	// but must not be a group modal
	if (dynamic_cast<CGroupModal*>(pIG))
	{
		string stmp = string("tree can't have modal group !") + pEltFound->getId();
		// todo hulud interface syntax error
		nlinfo(stmp.c_str());
		return false;
	}


	CGroupContainer *pIC = dynamic_cast<CGroupContainer*>(pEltFound);
	if (pIC != NULL)
	{
		cur = cur->children;
		while (cur)
		{
			setupTreeNode(cur, pIC);
			cur = cur->next;
		}
	}
	return true;
}



// ----------------------------------------------------------------------------
bool CInterfaceParser::parseTree (xmlNodePtr cur, CWidgetManager::SMasterGroup *parentGroup)
{
	H_AUTO(parseTree )

	CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"node" ));
	if (!ptr) return false;
	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();
	CInterfaceElement *pEltFound = NULL;
	for (uint32 i = 0; i < _MasterGroups.size(); ++i)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[i];
		for (uint32 j = 0; j < rMG.Group->getGroups().size(); ++j)
		{
			CInterfaceGroup *pIG = rMG.Group->getGroups()[j];
			string stmp = NLMISC::toLower(pIG->getId().substr(pIG->getId().rfind(':')+1,pIG->getId().size()));
			string stmp2 = NLMISC::toLower(string((const char*)ptr));
			if (stmp == stmp2)
			{
				pEltFound = pIG;
				break;
			}
		}
		if (pEltFound != NULL)
			break;
	}

	if (pEltFound == NULL)
	{
		string stmp = string("no group found for ") + string((const char*)ptr);
		// todo hulud interface syntax error
		nlinfo(stmp.c_str());
		return false;
	}

	// the element must be a group
	CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(pEltFound);
	if (pIG == NULL)
	{
		string stmp = string("not a group !") + pEltFound->getId();
		// todo hulud interface syntax error
		nlinfo(stmp.c_str());
		return false;
	}

	// but must not be a group modal
	if (dynamic_cast<CGroupModal*>(pIG))
	{
		string stmp = string("tree can't have modal group !") + pEltFound->getId();
		// todo hulud interface syntax error
		nlinfo(stmp.c_str());
		return false;
	}

	// Ok add it.
	CWidgetManager::getInstance()->addWindowToMasterGroup(parentGroup->Group->getId(), pIG);

	CGroupContainer *pIC = dynamic_cast<CGroupContainer*>(pEltFound);
	if (pIC != NULL)
	{
		cur = cur->children;
		while (cur)
		{
			parseTreeNode(cur, pIC);
			cur = cur->next;
		}
	}
	return true;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseDefine(xmlNodePtr cur)
{
	H_AUTO(parseDefine)

	CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"id" ));
	if (!ptr || *ptr==0)
	{
		// todo hulud interface syntax error
		nlinfo ("no id in a define");
		return false;
	}
	CXMLAutoPtr ptrVal2;
	CXMLAutoPtr ptrVal((const char*) xmlGetProp( cur, (xmlChar*)"value" ));
	if (!ptrVal)
	{
		ptrVal2 = (char*) xmlGetProp( cur, (xmlChar*)"value_from_code" );
		if (!ptrVal2)
		{
			// todo hulud interface syntax error
			nlwarning ("<parseDefine> : no value nor value_from_code in a define");
			return false;
		}
	}

	// verify id.
	string	id= (const char*)ptr;
	for(uint i=0;i<id.size();i++)
	{
		if(!validDefineChar(id[i]))
		{
			// todo hulud interface syntax error
			nlwarning ("<parseDefine> : bad id in a define. Bad char found: %c", id[i]);
			return false;
		}
	}

	// Check if we have to execute some code
	if (ptrVal2)
	{
		CInterfaceExprValue res;

		if (CInterfaceExpr::eval(ptrVal2, res))
		{
			if (!res.toString())
			{
				// todo hulud interface syntax error
				nlwarning ("<parseDefine> : cant eval to string value_from_code : %s", (const char*)ptrVal2);
				return false;
			}
			setDefine(id, res.getString().c_str());
		}
		else
		{
			// todo hulud interface syntax error
			nlwarning ("<parseDefine> : cant eval value_from_code : %s", (const char*)ptrVal2);
			return false;
		}
	}
	else
	{
		// Assign var
		setDefine (id, (const char*)ptrVal);
	}

	return true;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseProcedure(xmlNodePtr cur, bool reload)
{
	H_AUTO(parseProcedure)

	CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"id" ));
	if (!ptr || *ptr==0)
	{
		// todo hulud interface syntax error
		nlwarning ("no id in a procedure");
		return false;
	}
	string	procId= ptr;

	if (_ProcedureMap.find(procId) != _ProcedureMap.end())
	{
		// If reloading the interface, remove the old proc
		if (reload)
		{
			_ProcedureMap.erase (procId);
		}
		else
		{
			// todo hulud interface syntax error
			nlwarning ("id already exists for procedure %s", procId.c_str());
			return false;
		}
	}

	// build the procedure
	CProcedure	newProc;

	// Look for sons.
	cur = cur->children;
	while (cur)
	{
		if (stricmp((char*)cur->name,"action") == 0)
		{
			CXMLAutoPtr name((const char*) xmlGetProp (cur, (xmlChar*)"handler"));
			CXMLAutoPtr params((const char*) xmlGetProp (cur, (xmlChar*)"params"));
			CXMLAutoPtr cond((const char*) xmlGetProp (cur, (xmlChar*)"cond"));
			CAction	action;
			if(!name)
			{
				// todo hulud interface syntax error
				nlinfo("no action name in a action of procedure %s", procId.c_str());
				return false;
			}
			else
				action.Action= (const char*)name;
			if(params)
				action.buildParamBlock((const char*)params);
			if(cond)
				action.buildCondBlock ((const char*)cond);
			newProc.Actions.push_back(action);
		}
		else if (!strcmp((char*)cur->name,"instance"))
		{
			if (!parseInstance(cur))
				// todo hulud interface syntax error
				nlwarning ("could not parse instance");
		}
		else if (!strcmp((char*)cur->name,"vector"))
		{
			if (!parseVector(cur))
				// todo hulud interface syntax error
				nlwarning ("could not parse vector");
		}
		cur = cur->next;
	}

	// add/replace the procedure
	_ProcedureMap[procId]= newProc;

	return true;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::initCoordsAndLuaScript()
{
	H_AUTO(initCoordsAndLuaScript)

	// set all position associations
	for (map<CInterfaceElement*,string>::const_iterator it = _ParentPositionsMap.begin(); it != _ParentPositionsMap.end();it++)
	{
		CInterfaceElement *pIEL = it->first;
		string EltName = it->second;

		CInterfaceGroup *parent = pIEL->getParent();
		CInterfaceElement *parentpos;
		//if the element has a parent check the parent's children
		if (parent)
			parentpos = parent->getElement(EltName);
		//if the element has no parent, check the windows
		else
			parentpos = CWidgetManager::getInstance()->getWindowFromId(EltName);

		if (parentpos == NULL)
		{
			// todo hulud interface syntax error
			nlinfo(" the element %s was not found as %s position reference ", EltName.c_str(), pIEL->getId().c_str());
		}
		else
		{
			pIEL->setParentPos (parentpos);
		}
	}
	// Same for size
	for (map<CInterfaceElement*,string>::const_iterator it2 = _ParentSizesMap.begin(); it2 != _ParentSizesMap.end(); it2++)
	{
		CInterfaceElement *pIEL = it2->first;
		string EltName = it2->second;

		CInterfaceGroup *parent = pIEL->getParent();
		CInterfaceElement *parentsize;
		if (EltName == "parent")
		{
			parentsize = pIEL->getParent();
		}
		else
		{
			//if the element has a parent check the parent's children
			if (parent)
				parentsize = parent->getElement(EltName);
			//if the element has no parent, check the windows
			else
				parentsize = CWidgetManager::getInstance()->getWindowFromId(EltName);
		}

		if (parentsize == NULL)
		{
			// todo hulud interface syntax error
			nlinfo(" the element %s was not found as %s size reference ", EltName.c_str(), pIEL->getId().c_str());
		}
		else
		{
			pIEL->setParentSize (parentsize);
		}
	}
	// Same for size max
	for (map<CInterfaceElement*,string>::const_iterator it3 = _ParentSizesMaxMap.begin(); it3 != _ParentSizesMaxMap.end(); it3++)
	{
		CInterfaceGroup *pIEL = dynamic_cast<CInterfaceGroup*>(it3->first);
		if (pIEL == NULL) continue;
		string EltName = it3->second;

		CInterfaceGroup *parent = pIEL->getParent();
		CInterfaceElement *parentsizemax;
		if (EltName == "parent")
		{
			parentsizemax = parent;
		}
		else
		{
			//if the element has a parent check the parent's children
			if (parent)
				parentsizemax = parent->getElement(EltName);
			//if the element has no parent, check the windows
			else
				parentsizemax = CWidgetManager::getInstance()->getWindowFromId(EltName);
		}

		if (parentsizemax == NULL)
		{
			// todo hulud interface syntax error
			nlinfo(" the element %s was not found as %s sizemax reference ", EltName.c_str(), pIEL->getId().c_str());
		}
		else
		{
			pIEL->setParentSizeMax (parentsizemax);
		}
	}

	// Same For LUA Class association
	for (map<CInterfaceGroup*,string>::const_iterator itLua = _LuaClassAssociation.begin(); itLua != _LuaClassAssociation.end(); itLua++)
	{
		// execute the script on this group
		CAHManager::getInstance()->runActionHandler("lua", itLua->first, itLua->second);
	}


	// Clear all structures used only for init
	NLMISC::contReset (_ParentPositionsMap);
	NLMISC::contReset (_ParentSizesMap);
	NLMISC::contReset (_ParentSizesMaxMap);
	NLMISC::contReset (_LuaClassAssociation);
	return true;
}

// ----------------------------------------------------------------------------
void CInterfaceParser::addParentPositionAssociation(CInterfaceElement* element, const std::string& parent)
{
	_ParentPositionsMap.insert (std::map<CInterfaceElement*,std::string>::value_type(element, parent));
}

// ----------------------------------------------------------------------------
void CInterfaceParser::addParentSizeAssociation(CInterfaceElement* element, const std::string& parent)
{
	_ParentSizesMap.insert (std::map<CInterfaceElement*,std::string>::value_type(element, parent));
}

// ----------------------------------------------------------------------------
void CInterfaceParser::addParentSizeMaxAssociation (CInterfaceElement *element, const std::string &parent)
{
	_ParentSizesMaxMap.insert (std::map<CInterfaceElement*,std::string>::value_type(element, parent));
}

// ----------------------------------------------------------------------------
void CInterfaceParser::addLuaClassAssociation (CInterfaceGroup *group, const std::string &luaScript)
{
	_LuaClassAssociation.insert (std::map<CInterfaceGroup*,std::string>::value_type(group, luaScript));
}

// ***************************************************************************
const std::string	&CInterfaceParser::getDefine(const std::string &id) const
{
	static	string NullStr;
	CstItVarMap	it= _DefineMap.find(id);
	if(it==_DefineMap.end())
		return NullStr;
	else
		return it->second;
}

// ***************************************************************************
bool				CInterfaceParser::isDefineExist(const std::string &id) const
{
	CstItVarMap	it= _DefineMap.find(id);
	return it!=_DefineMap.end();
}


// ***************************************************************************
void				CInterfaceParser::setDefine(const std::string &id, const std::string &value)
{
	_DefineMap[id]= value;
}


// ***************************************************************************
bool				CInterfaceParser::validDefineChar(char c) const
{
	if(c>='A' && c<='Z')
		return true;
	if(c>='a' && c<='z')
		return true;
	if(c>='0' && c<='9')
		return true;
	if(c=='_')
		return true;

	return false;
}

#define	DEFINE_IDENT	'%'

// ***************************************************************************
bool				CInterfaceParser::solveDefine(const std::string &propVal, std::string &newPropVal, std::string &defError)
{
	string::size_type	curPos= 0;
	string::size_type	lastPos= 0;

	//if it has some % then solve define value
	while( (curPos=propVal.find(DEFINE_IDENT, curPos)) != string::npos)
	{
		// If it is end of line
		if(curPos==propVal.size()-1)
		{
			// then skip
			curPos= propVal.size();
		}
		// If it is a double %%
		else if(propVal[curPos+1]==DEFINE_IDENT)
		{
			// copy from last to cur included
			curPos++;
			newPropVal+= propVal.substr(lastPos, curPos-lastPos);
			// both % are skipped
			curPos++;
			lastPos= curPos;
		}
		// else parse define value
		else
		{
			// copy the last not define sub string.
			newPropVal+= propVal.substr(lastPos, curPos-lastPos);

			// skip the %
			curPos++;

			// get the id pos
			uint	startIdPos= (uint)curPos;
			while( curPos<propVal.size() && validDefineChar(propVal[curPos]) )
				curPos++;
			// get the id
			string	defineId= propVal.substr(startIdPos, curPos-startIdPos);
			if(!isDefineExist(defineId))
			{
				defError= defineId;
				return false;
			}
			// Add the define value to the string
			newPropVal+= getDefine(defineId);

			// valid pos is current pos
			lastPos= curPos;
		}
	}
	// concat last part
	newPropVal+= propVal.substr(lastPos, propVal.size()-lastPos);

	return true;
}

// ***************************************************************************
bool				CInterfaceParser::solveDefine(xmlNodePtr cur)
{
	H_AUTO(solveDefine)

	CXMLAutoPtr ptr;

	//get the node properties
	xmlAttrPtr props = cur->properties;
	while (props)
	{
		//get the property value
		ptr = (char*)xmlGetProp( cur, props->name);
		nlassert(ptr);
		string	propVal= ptr;
		string	newPropVal;

		// solve define of this prop
		string	defError;
		if(!solveDefine(propVal, newPropVal, defError))
		{
			// todo hulud interface syntax error
			nlinfo("can't find define: %s", defError.c_str());
			return false;
		}

		// change value
		xmlSetProp(cur, props->name, (const xmlChar*)newPropVal.c_str());

		// next property
		props = props->next;
	}

	// recurs to node children
	cur= cur->children;
	while(cur)
	{
		if(!solveDefine(cur))
			return false;
		cur= cur->next;
	}

	return true;
}


// ***************************************************************************
// CInterfaceParser::CAction
// ***************************************************************************

#define	PROC_PARAM_IDENT	'@'

// ***************************************************************************
void CInterfaceParser::CAction::buildParamBlock(const std::string &params)
{
	buildBlocks (params, ParamBlocks);
}

void CInterfaceParser::CAction::buildParams(const std::vector<string> &paramList, std::string &params) const
{
	eval (paramList, ParamBlocks, params);
}

void CInterfaceParser::CAction::buildCondBlock(const std::string &params)
{
	buildBlocks (params, CondBlocks);
}

void CInterfaceParser::CAction::buildCond(const std::vector<string> &paramList, std::string &params) const
{
	eval (paramList, CondBlocks, params);
}

// ***************************************************************************
void		CInterfaceParser::CAction::buildBlocks (const std::string &in, std::vector<CParamBlock> &out)
{
	out.clear();

	if(in.empty())
		return;

	string	lastString;
	string::size_type	curPos= 0;
	string::size_type	lastPos= 0;

	//if it has some @ then solve proc value
	while( (curPos=in.find(PROC_PARAM_IDENT, curPos)) != string::npos)
	{
		// If it is end of line
		if(curPos==in.size()-1)
		{
			// then skip
			curPos= in.size();
		}
		else
		{
			// Skip all @
			uint countNbIdent = 0;
			while (curPos<in.size() && in[curPos]==PROC_PARAM_IDENT)
			{
				curPos++;
				countNbIdent++;
			}

			// get the id pos
			uint countNbDigit = 0;
			uint startIdPos= (uint)curPos;
			while (curPos<in.size() && in[curPos]>='0' && in[curPos]<='9')
			{
				curPos++;
				countNbDigit++;
			}

			if (curPos == startIdPos)
			{
				// No digit so it is a normal db entry
				lastString+= in.substr (lastPos, curPos-(countNbIdent-1)-lastPos);
				// all @ are skipped
			}
			else
			{
				// There is some digit it is an argument

				// copy the last not param sub string.
				sint nbToCopy = (sint)(curPos-countNbIdent-countNbDigit-lastPos);
				if (nbToCopy > 0)
					lastString += in.substr(lastPos, nbToCopy);

				// if not empty, add to the param block
				if (!lastString.empty())
				{
					CParamBlock pb;
					pb.String = lastString;
					out.push_back(pb);
					// clear it
					lastString.clear();
				}

				// get the param id
				sint paramId;
				fromString(in.substr(startIdPos, curPos-startIdPos), paramId);
				// Add it to the param block
				CParamBlock	 pb;
				pb.NumParam = paramId;
				out.push_back(pb);
			}

			// valid pos is current pos
			lastPos= curPos;
		}
	}
	// concat last part
	lastString+= in.substr(lastPos, in.size()-lastPos);
	if(!lastString.empty())
	{
		CParamBlock pb;
		pb.String = lastString;
		out.push_back(pb);
	}
}

// ***************************************************************************
//void		CInterfaceParser::CAction::buildParams(const std::vector<string> &paramList, std::string &params) const
void CInterfaceParser::CAction::eval (const std::vector<string> &inArgs, const std::vector<CParamBlock> &inBlocks, std::string &out)
{
	// clear the ret string
	out.clear();

	// for all block
	for (uint i=0; i < inBlocks.size(); i++)
	{
		const CParamBlock &pb = inBlocks[i];
		// if the block is a raw string
		if (pb.NumParam < 0)
		{
			// concat with the block
			out += pb.String;
		}
		// else get from paramList
		else
		{
			// add 1, because paramList[0] is the name of the procedure
			sint idInList = pb.NumParam+1;
			// if param exist
			if (idInList < (sint)inArgs.size())
				// concat with the params
				out += inArgs[idInList];
			// else skip (should fail)
		}
	}
}

// ***************************************************************************
bool CInterfaceParser::parseSheetSelection(xmlNodePtr cur)
{
	H_AUTO(parseSheetSelection)

	CXMLAutoPtr prop;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"name" );
	if (!prop)
	{
		// todo hulud interface syntax error
		nlwarning("<CInterfaceParser::parseSheetSelection> can't get name of a selection");
		return false;
	}
	std::string groupName = (const char *) prop;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"texture" );
	if (!prop)
	{
		// todo hulud interface syntax error
		nlwarning("<CInterfaceParser::parseSheetSelection> can't get texture name for selection %s", groupName.c_str());
		return false;
	}
	std::string texName = (const char *) prop;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"color" );
	CRGBA color = CRGBA::White;
	if (prop)
	{
		color = CInterfaceElement::convertColor(prop);
	}
	bool globalColor = true;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"global_color" );
	if (prop) globalColor = CInterfaceElement::convertBool(prop);
	sint groupIndex = _CtrlSheetSelection.addGroup(groupName);
	if (groupIndex != -1)
	{
		CSheetSelectionGroup *csg = _CtrlSheetSelection.getGroup(groupIndex);
		csg->setTexture(texName);
		csg->setColor(color);
		csg->enableGlobalColor(globalColor);
	}
	return true;
}

// ***************************************************************************
bool CInterfaceParser::addLink(CInterfaceLink *link, const std::string &id)
{
	if (!link)
	{
		// todo hulud interface syntax error
		nlwarning("link empty");
		return false;
	}
	TLinkMap::const_iterator it = _LinkMap.find(id);
	if (it != _LinkMap.end())
	{
		// todo hulud interface syntax error
		nlwarning("<CInterfaceParser::addLink> link %s added twice", id.c_str());
		return false;
	}
	#ifdef NL_DEBUG
		link->LinkName = id;
	#endif
	_LinkMap[id] = link;
	return false;
}

// ***************************************************************************
bool CInterfaceParser::removeLink(const std::string &id)
{
	TLinkMap::iterator it = _LinkMap.find(id);
	if (it == _LinkMap.end())
	{
		// todo hulud interface syntax error
		nlwarning("<CInterfaceParser::removeLink> unknown link %s", id.c_str());
		return false;
	}
	CSmartPtr<CInterfaceLink> &link = it->second; // dont need to copy a smart ptr on link since still in map
	for (uint k = 0; k < link->getNumTargets(); ++k)
	{
		link->getTarget(k)->removeLink(link); // remove the link from the list & delete it
	}
	it->second->uninit();
	_LinkMap.erase(it); // NB : the link is holded by a smart ptr, to do this decrease the ref count
	return true;
}


// ***************************************************************************
xmlNodePtr CInterfaceParser::searchTreeNodeInHierarchy(xmlNodePtr root, const char *node)
{
	// if I match...
	CXMLAutoPtr prop((const char*) xmlGetProp( root, (xmlChar*)"node" ));
	// not a valide tree node? abort.
	if (!prop) return NULL;
	// match?
	if ( !strcmp((const char*)prop, node ) )
		return root;

	// No, try with sons.
	xmlNodePtr cur= root->children;
	while(cur)
	{
		xmlNodePtr candidate= searchTreeNodeInHierarchy(cur, node);
		// if found in this branch.
		if(candidate)
			return candidate;

		// try next
		cur= cur->next;
	}

	// not found
	return NULL;
}

//==================================================================
bool CInterfaceParser::parseAnim(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	H_AUTO(parseAnim)

	CInterfaceAnim *pAnim;

	CXMLAutoPtr ptr;

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"id" );
	if (!ptr)
	{
		// todo hulud interface syntax error
		nlinfo ("anim has no id");
		return false;
	}
	string animId = ptr;
	pAnim = new CInterfaceAnim;

	if (pAnim->parse (cur, parentGroup))
	{
		if (_AnimMap.count(animId))
		{
			nlwarning("Anim %s already exists, replacing with new one", animId.c_str());
		}
		_AnimMap[animId] = pAnim;
	}
	else
	{
		delete pAnim;
		return false;
	}
	return true;
}

//==================================================================
bool CInterfaceParser::parseScene3D(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	H_AUTO(parseScene3D)

	CInterface3DScene *pScene;
	CXMLAutoPtr ptr;

	pScene = new CInterface3DScene(CViewBase::TCtorParam());

	// parse the group attributes
	if (!pScene->parse(cur,parentGroup))
	{
		delete pScene;
		// todo hulud interface syntax error
		nlinfo ("cannot parse 3d scene attributes");
		return false;
	}

	if (parentGroup)
	{
		CGroupList *pList = dynamic_cast<CGroupList*>(parentGroup);
		if (pList != NULL)
			pList->addChild (pScene);
		else
			parentGroup->addGroup (pScene);
	}
	else
	{
		string tmp = "no parent for "+pScene->getId();
		// todo hulud interface syntax error
		nlinfo (tmp.c_str());
		delete pScene;
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------------
bool CInterfaceParser::parseActionCategory(xmlNodePtr cur)
{
	H_AUTO(parseActionCategory)

	// The category
   	CCategory category;

	// Name
	CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"name" ));
	if (ptr)
		category.Name = (const char*)ptr;

	// Localized string
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"hardtext" );
	if (ptr)
		category.LocalizedName = (const char*)ptr;

	// macroisable (per category)
	ptr = (char*) xmlGetProp( cur, (xmlChar*)"macroisable" );
	if (ptr)
		category.Macroisable= CInterfaceElement::convertBool(ptr);

	// Count number of action
	category.BaseActions.resize (CIXml::countChildren(cur, "action"));


	std::string actionCategoryContext = "game";

	ptr = (char*) xmlGetProp( cur, (xmlChar*)"contexts" );
	if (ptr)
		actionCategoryContext = (const char *) ptr;

	uint actionIndex = 0;
	xmlNodePtr actionNode = CIXml::getFirstChildNode(cur, "action");
	if (actionNode)
	{
		do
		{
			// The action
			CBaseAction &action = category.BaseActions[actionIndex];

			// list of contexts in which this action is valid
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"contexts" );
			if (ptr)
				action.Contexts = (const char *) ptr;
			else
				action.Contexts = actionCategoryContext; // inherit from action category

			// Repeat flag
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"repeat" );
			if (ptr)
				fromString((const char*)ptr, action.Repeat);

			// KeyDown flag
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"keydown" );
			if (ptr)
				fromString((const char*)ptr, action.KeyDown);

			// KeyUp flag
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"keyup" );
			if (ptr)
				fromString((const char*)ptr, action.KeyUp);

			// WaitForServer flag (wait an answer from server before continuing)
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"waitforserver" );
			if (ptr)
				fromString((const char*)ptr, action.WaitForServer);

			// Action name
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"name" );
			if (ptr)
				action.Name = (const char*)ptr;


			// Action localized name
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"hardtext" );
			if (ptr)
				action.LocalizedName = (const char*)ptr;

			// macroisable (per action)
			action.Macroisable= true;
			ptr = (char*) xmlGetProp( actionNode, (xmlChar*)"macroisable" );
			if (ptr)
				action.Macroisable = CInterfaceElement::convertBool(ptr);


			// Read the parameters
			action.Parameters.resize (CIXml::countChildren(actionNode, "parameter"));

			uint parameterIndex = 0;
			xmlNodePtr paramNode = CIXml::getFirstChildNode(actionNode, "parameter");
			if (paramNode)
			{
				do
				{
					// The parameter
					CBaseAction::CParameter &parameter = action.Parameters[parameterIndex];

					// Parameter type
					ptr = (char*) xmlGetProp( paramNode, (xmlChar*)"type" );
					if (ptr)
					{
						sint32 tType;
						fromString((const char*)ptr, tType);
						parameter.Type = (CBaseAction::CParameter::TType)tType;
					}

					// Parameter name
					ptr = (char*) xmlGetProp( paramNode, (xmlChar*)"name" );
					if (ptr)
						parameter.Name = (const char*)ptr;

					// Parameter localized name
					ptr = (char*) xmlGetProp( paramNode, (xmlChar*)"hardtext" );
					if (ptr)
						parameter.LocalizedName = (const char*)ptr;

					// Default value
					ptr = (char*) xmlGetProp( paramNode, (xmlChar*)"value" );
					if (ptr)
						parameter.DefaultValue = (const char*)ptr;

					// Visible flag
					//ptr = (char*) xmlGetProp( paramNode, (xmlChar*)"visible" );
					//if (ptr)
					//	fromString((const char*)ptr, parameter.Visible);

					// Parse instance
					xmlNodePtr instanceNode = CIXml::getFirstChildNode(paramNode, "instance");
					if (instanceNode)
					{
						do
						{
							if (!parseInstance(instanceNode))
							{
								// todo hulud interface syntax error
								nlwarning("<CInterfaceParser::parseActionCategory> cannot create instance from template");
							}
						}
						while((instanceNode = CIXml::getNextChildNode(instanceNode, "instance")));
					}

					parameter.Values.resize (CIXml::countChildren(paramNode, "value"));

					uint valueIndex = 0;
					xmlNodePtr valueNode = CIXml::getFirstChildNode(paramNode, "value");
					if (valueNode)
					{
						do
						{
							// The value
							CBaseAction::CParameter::CValue &value = parameter.Values[valueIndex];

							// Value
							ptr = (char*) xmlGetProp( valueNode, (xmlChar*)"value" );
							if (ptr)
								value.Value = (const char*)ptr;

							// list of contexts in which this value is valid
							ptr = (char*) xmlGetProp( valueNode, (xmlChar*)"contexts" );
							if (ptr) value.Contexts = (const char*) ptr;
							else value.Contexts = action.Contexts; // inherit context from action

							// Localized value
							ptr = (char*) xmlGetProp( valueNode, (xmlChar*)"hardtext" );
							if (ptr)
								value.LocalizedValue = (const char*)ptr;

							valueIndex++;
						}
						while((valueNode = CIXml::getNextChildNode(valueNode, "value")));
					}

					parameterIndex++;
				}
				while((paramNode = CIXml::getNextChildNode(paramNode, "parameter")));
			}

			// Next action
			actionIndex++;
		}
		while((actionNode = CIXml::getNextChildNode(actionNode, "action")));
	}

	// Add this category to the action manager
	CActionsManager *actionManager = ActionsContext.getActionsManager (category.Name);
	if (actionManager)
	{
// They want to display debug shortcut in final version
#if FINAL_VERSION
		if ((category.Name != "debug") || ClientCfg.AllowDebugCommands)
#else // FINAL_VERSION
		if (1)
#endif // FINAL_VERSION
		{
			actionManager->removeCategory (category.Name);
			actionManager->addCategory (category);
		}
		else
		{
			// Remove thoses actions from the manager
			CAHManager *pAHFM = CAHManager::getInstance();
			uint i;
			for (i=0; i<category.BaseActions.size(); i++)
			{
				CAHManager::TFactoryMap::iterator ite = pAHFM->FactoryMap.find (category.BaseActions[i].Name);
				if (ite != pAHFM->FactoryMap.end())
				{
					IActionHandler *ah = ite->second;
					pAHFM->FactoryMap.erase (ite);
					pAHFM->NameMap.erase (ah);
				}
			}
		}
	}
	return true;
}

//==================================================================
bool CInterfaceParser::parseKey(xmlNodePtr cur)
{
	H_AUTO(parseKey)

	// Parse the key
	bool ret = false;

	// Localized string
	TKey key;
	CXMLAutoPtr ptrKey((const char*) xmlGetProp( cur, (xmlChar*)"name" ));
	if (ptrKey)
	{
		bool isNA = string((const char*)ptrKey) == string("N/A");
		// Get the key from the string
		key = CEventKey::getKeyFromString ((const char*)ptrKey);
		if (key != KeyCount || isNA)
		{
			// Get the action
			CXMLAutoPtr ptrAction((const char*) xmlGetProp( cur, (xmlChar*)"action" ));
			if (ptrAction)
			{
				// Get the params
				CXMLAutoPtr ptrParams((const char*) xmlGetProp( cur, (xmlChar*)"params" ));

				// Get the modifiers
				bool shift=false;
				bool ctrl=false;
				bool menu=false;
				CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"shift" ));
				if (ptr)
					fromString((const char*)ptr, shift);
				ptr = (char*) xmlGetProp( cur, (xmlChar*)"ctrl" );
				if (ptr)
					fromString((const char*)ptr, ctrl);
				ptr = (char*) xmlGetProp( cur, (xmlChar*)"menu" );
				if (ptr)
					fromString((const char*)ptr, menu);

				// Repeat flag
				bool repeat=false;
				ptr = (char*) xmlGetProp( cur, (xmlChar*)"repeat" );
				if (ptr)
					fromString((const char*)ptr, repeat);

				// Get the context
				CXMLAutoPtr ptrContext((const char*) xmlGetProp( cur, (xmlChar*)"context" ));
				string context = (const char*)ptrContext?(const char*)ptrContext:"";

				// Add the action
				CCombo combo;
				combo.init(key, (TKeyButton)((shift?shiftKeyButton:noKeyButton)|(ctrl?ctrlKeyButton:noKeyButton)|(menu?altKeyButton:noKeyButton)));
				::CAction::CName actionName ((const char*)ptrAction, ptrParams?(const char*)ptrParams:"");

				// Get the actions context manager
				CActionsManager *actionManager = ActionsContext.getActionsManager(context);
				if (actionManager)
				{
					bool canAdd= true;

					// for keys.xml, don't replace already defined keys
					if(getDefine("key_def_no_replace")=="1")
					{
						// if this combo key is already used for any action,
						// or if this action is already bound to any key
						if(isNA || actionManager->isComboAssociated(combo) || actionManager->isActionAssociated(actionName))
							// don't replace
							canAdd= false;
					}

					// add/replace the combo?
					if(canAdd)
					{
						actionManager->addCombo(actionName, combo);
						::CAction *action = actionManager->getAction(actionName);
						if (action && repeat) action->Repeat = true;
					}

					// if the action is to be shown in the Key interface
					if(getDefine("key_def_force_display")=="1")
						actionManager->forceDisplayForAction(actionName, true);
				}

				// Done
				ret = true;
			}
			else
			{
				// todo hulud interface syntax error
				nlwarning("<CInterfaceParser::parseKey> No action for key : %s", (const char*)ptrKey);
			}
		}
		else
		{
			// todo hulud interface syntax error
			nlwarning("<CInterfaceParser::parseKey> Unknown key : %s", (const char*)ptrKey);
		}
	}
	else
	{
		// todo hulud interface syntax error
		nlwarning("<CInterfaceParser::parseKey> No name for a key");
	}

	return ret;
}

//==================================================================
bool CInterfaceParser::parseCommand(xmlNodePtr cur)
{
	H_AUTO(parseCommand)

	// Parse the key
	bool ret = false;

	// Localized string
	CXMLAutoPtr ptrName((const char*) xmlGetProp( cur, (xmlChar*)"name" ));
	if (ptrName)
	{
		// Does the action exist ?
		string name = ptrName;
		if (!ICommand::exists (name) || (CUserCommand::CommandMap.find(name) != CUserCommand::CommandMap.end()))
		{
			// Get the action
			CXMLAutoPtr ptrAction((const char*) xmlGetProp( cur, (xmlChar*)"action" ));
			if (ptrAction)
			{
				// Get the params
				CXMLAutoPtr ptrParams((const char*) xmlGetProp( cur, (xmlChar*)"params" ));
				if (ptrParams)
				{
					CUserCommand::createCommand (ptrName, ptrAction, ptrParams);

					// if prop "ctrlchar" is declared with false, then disable ctrlchar for this command
					CXMLAutoPtr prop((const char*) xmlGetProp( cur, (xmlChar*)"ctrlchar" ));
					if( (const char*)prop && (CInterfaceElement::convertBool((const char*)prop)==false) )
						ICommand::enableControlCharForCommand(ptrName, false);

					// Done
					ret = true;
				}
			}
			else
			{
				// todo hulud interface syntax error
				nlwarning("<CInterfaceParser::parseCommand> No action for command : %s", (const char*)ptrName);
			}
		}
	}
	else
	{
		// todo hulud interface syntax error
		nlwarning("<CInterfaceParser::parseCommand> No name for a key");
	}

	return ret;
}

//==================================================================
bool CInterfaceParser::parseMacro(xmlNodePtr cur)
{
	H_AUTO(parseMacro)

	CMacroCmd cmd;
	if (cmd.readFrom(cur))
		CMacroCmdManager::getInstance()->addMacro(cmd);
	else
		return false;
	return true;
}

//==================================================================
void CInterfaceParser::freeXMLNodeAndSibblings(xmlNodePtr node)
{
	if (!node) return;
	 while (node)
	 {
		 xmlNodePtr currNode = node;
		 node = node->next;
		 xmlFreeNode(currNode);
	 }
}


// ***************************************************************************
CInterfaceGroup *CInterfaceParser::createGroupInstance(const std::string &templateName, const std::string &parentID, const std::pair<std::string,std::string> *templateParams, uint numParams, bool updateLinks /* = true */)
{
	// create basic xml node that contains infos for the template
	xmlNodePtr instance = xmlNewNode(NULL, (const xmlChar *) "instance");
	if (!instance)
	{
		// todo hulud interface syntax error
		nlwarning("<CInterfaceParser::createGroupInstance> Can't create xml node ");
		return NULL;
	}
	for(uint k = 0; k < numParams; ++k)
	{
		xmlSetProp(instance, (const xmlChar *) templateParams[k].first.c_str(), (const xmlChar *) templateParams[k].second.c_str());
	}
	xmlSetProp(instance, (const xmlChar *) "template", (const xmlChar *) templateName.c_str());
	if (!parseInstance(instance))
	{
		// todo hulud interface syntax error
		nlwarning("<CInterfaceParser::createGroupInstance> cannot create instance from template %s", templateName.c_str());
		freeXMLNodeAndSibblings(instance);
		return NULL;
	}

	// result should contain a group
	xmlNodePtr currNode = instance->next;
	while (currNode)
	{
		if (strcmp((const char *) currNode->name, "group") == 0 && currNode->type == XML_ELEMENT_NODE)
		{
			// TODO nico : build a struct from that mess
			std::map<CInterfaceElement*,std::string> localParentPositionsMap;
			std::map<CInterfaceElement*,std::string> localParentSizesMap;
			std::map<CInterfaceElement*,std::string> localParentSizesMaxMap;
			std::map<CInterfaceGroup*,std::string>	 localLuaClassAssociation;

			localParentPositionsMap.swap(_ParentPositionsMap);
			localParentSizesMap.swap(_ParentSizesMap);
			localParentSizesMaxMap.swap(_ParentSizesMaxMap);
			localLuaClassAssociation.swap(_LuaClassAssociation);

			CViewBase::TCtorParam params;
			CInterfaceGroup dummyGroup(params);
			dummyGroup.setId(parentID);
			if (!parseGroup(currNode, &dummyGroup, false))
			{
				localParentPositionsMap.swap(_ParentPositionsMap);
				localParentSizesMap.swap(_ParentSizesMap);
				localParentSizesMaxMap.swap(_ParentSizesMaxMap);
				localLuaClassAssociation.swap(_LuaClassAssociation);
				freeXMLNodeAndSibblings(instance);
				return NULL;
			}
			freeXMLNodeAndSibblings(instance);
			CInterfaceGroup *group = dummyGroup.getGroup((uint) 0);
			dummyGroup.delGroup(group, true);
			group->setParent(NULL);
			group->setParentPos(NULL);
			initCoordsAndLuaScript();
			localParentPositionsMap.swap(_ParentPositionsMap);
			localParentSizesMap.swap(_ParentSizesMap);
			localParentSizesMaxMap.swap(_ParentSizesMaxMap);
			localLuaClassAssociation.swap(_LuaClassAssociation);
			if ((group != NULL) && updateLinks)
			{
				group->updateAllLinks();
			}
			CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(group);
			if (pGC != NULL)
				pGC->setup();
			return group;
		}
		currNode = currNode->next;
	}
	// todo hulud interface syntax error
	CXMLAutoPtr ptr(xmlGetProp(instance, (const xmlChar *) templateName.c_str()));
	nlwarning("<CInterfaceParser::createGroupInstance> no group found in template %s", (const char*)ptr);
	freeXMLNodeAndSibblings(instance);
	return NULL;
}

// ***************************************************************************
CInterfaceElement *CInterfaceParser::createUIElement(const std::string &templateName, const std::string &parentID, const std::pair<std::string,std::string> *templateParams, uint numParams, bool updateLinks /* = true */)
{
	std::string elementId;

	// create basic xml node that contains infos for the template
	xmlNodePtr instance = xmlNewNode(NULL, (const xmlChar *) "instance");
	if (!instance)
	{
		// todo hulud interface syntax error
		nlwarning("<CInterfaceParser::addUIElement> Can't create xml node ");
		return NULL;
	}
	for(uint k = 0; k < numParams; ++k)
	{
		xmlSetProp(instance, (const xmlChar *) templateParams[k].first.c_str(), (const xmlChar *) templateParams[k].second.c_str());
		if (!strcmp(templateParams[k].first.c_str(), "id"))
		{
			elementId = templateParams[k].second;
		}
	}
	xmlSetProp(instance, (const xmlChar *) "template", (const xmlChar *) templateName.c_str());
	if (!parseInstance(instance))
	{
		// todo hulud interface syntax error
		nlwarning("<CInterfaceParser::addUIElement> cannot create instance from template %s", templateName.c_str());
		freeXMLNodeAndSibblings(instance);
		return NULL;
	}

	CInterfaceElement	*pIE= CWidgetManager::getInstance()->getElementFromId(parentID);
	CInterfaceGroup * parentGroup = dynamic_cast<CInterfaceGroup*>(pIE);

	if(!parentGroup)
	{
		nlwarning("<CInterfaceParser::addUIElement> no parent group %s found ", parentID.c_str());
		freeXMLNodeAndSibblings(instance);
		return NULL;
	}

	// result should contain a group
	xmlNodePtr currNode = instance->next;
	CInterfaceElement * newElement = NULL;
	while (currNode)
	{
		if (strcmp((const char *) currNode->name, "group") == 0 && currNode->type == XML_ELEMENT_NODE)
		{
			if (!parseGroup(currNode, parentGroup, false))
			{
				freeXMLNodeAndSibblings(instance);
				return NULL;
			}
			freeXMLNodeAndSibblings(instance);
			newElement = parentGroup->getGroup(elementId);
			parentGroup->delGroup((CInterfaceGroup*)newElement, true);
		}
		else if (strcmp((const char *) currNode->name, "ctrl") == 0 && currNode->type == XML_ELEMENT_NODE)
		{
			if (!parseControl(currNode, parentGroup, false))
			{
				freeXMLNodeAndSibblings(instance);
				return NULL;
			}
			freeXMLNodeAndSibblings(instance);
			newElement = parentGroup->getCtrl(elementId);
			parentGroup->delCtrl((CCtrlBase*)newElement, true);
		}

		if(newElement != NULL)
		{
			initCoordsAndLuaScript();
			if (updateLinks)
			{
				newElement->updateAllLinks();
			}
			return newElement;
		}

		currNode = currNode->next;
	}
	// todo hulud interface syntax error
	CXMLAutoPtr ptr(xmlGetProp(instance, (const xmlChar *) templateName.c_str()));
	nlwarning("<CInterfaceParser::addUIElement> no group found in template %s", (const char *)ptr);
	freeXMLNodeAndSibblings(instance);
	return NULL;
}

// ***************************************************************************
void CInterfaceParser::removeAllLinks()
{
	_LinkMap.clear();
}

// ***************************************************************************
void CInterfaceParser::removeAllProcedures()
{
	_ProcedureMap.clear();
}

// ***************************************************************************
void CInterfaceParser::removeAllDefines()
{
	_DefineMap.clear();
}

// ***************************************************************************
void CInterfaceParser::removeAllTemplates()
{
	for (uint i = 0; i < _Templates.size(); ++i)
		xmlFreeNode(_Templates[i]);
	_Templates.clear();
}

// ***************************************************************************
void CInterfaceParser::removeAllAnims()
{
	TAnimMap::iterator it = _AnimMap.begin();
	while (it != _AnimMap.end())
	{
		CInterfaceAnim *pAnim = it->second;
		delete pAnim;
		it++;
	}
	_AnimMap.clear();
}

// ***************************************************************************


// ***************************************************************************
void CInterfaceParser::removeAll()
{
	removeAllLinks();
	CWidgetManager::getInstance()->removeAllOptions();
	removeAllProcedures();
	removeAllDefines();
	removeAllTemplates();
	removeAllAnims();
	CWidgetManager::getInstance()->removeAllMasterGroups();
	_StyleMap.clear();
	_CtrlSheetSelection.deleteGroups();
}


// ------------------------------------------------------------------------------------------------
uint CInterfaceParser::getProcedureNumActions(const std::string &procName) const
{
	CstItProcedureMap	it= _ProcedureMap.find(procName);
	if(it!=_ProcedureMap.end())
	{
		const CProcedure	&proc= it->second;
		return (uint)proc.Actions.size();
	}
	else
		return 0;
}

// ------------------------------------------------------------------------------------------------
bool CInterfaceParser::getProcedureAction(const std::string &procName, uint actionIndex, std::string &ah, std::string &params) const
{
	CstItProcedureMap	it= _ProcedureMap.find(procName);
	if(it!=_ProcedureMap.end())
	{
		const CProcedure	&proc= it->second;
		if(actionIndex<proc.Actions.size())
		{
			const CAction		&action= proc.Actions[actionIndex];
			// if not a variable parametrized Params
			if(action.ParamBlocks.size()==1 && action.ParamBlocks[0].NumParam==-1)
			{
				ah= action.Action;
				params= action.ParamBlocks[0].String;
				return true;
			}
		}
	}

	return false;
}


// ***************************************************************************
bool	CInterfaceParser::parseGeneratorRootContainer(xmlNodePtr cur, xmlNodePtr	&rootTreeNode)
{
	CXMLAutoPtr prop;

	// root_container
	string	rootContainer;
	prop = xmlGetProp (cur, (xmlChar*)"root_container");
	if(prop) rootContainer= (const char*)prop;
	else
	{
		// todo hulud interface syntax error
		nlwarning("prop 'root_container' not found");
		return false;
	}

	// search root container option
	bool	rootContainerSearch= false;
	prop = xmlGetProp (cur, (xmlChar*)"root_container_search");
	if(prop) rootContainerSearch= CInterfaceElement::convertBool((const char*)prop);


	// **** In all case, create or find the root_container
	// if don't search but create the root container...
	if(!rootContainerSearch)
	{
		rootTreeNode= xmlNewNode(cur->ns, (xmlChar*)"tree" );
		xmlSetProp(rootTreeNode, (xmlChar*)"node", (xmlChar*)rootContainer.c_str());
		// add it next to us now.
		xmlAddNextSibling (cur, rootTreeNode);
	}
	else
	{
		rootTreeNode= NULL;

		// search from the cur place ALL the tree node that match rootContainer.
		xmlNodePtr curSearch= cur;
		curSearch= curSearch->next;
		while(curSearch)
		{
			// if the node is a tree node.
			if ( !strcmp((char*)curSearch->name,"tree") )
			{
				// Test if him or one of his son match the name.
				xmlNodePtr candidate= searchTreeNodeInHierarchy(curSearch, rootContainer.c_str());
				// found? stop!
				if(candidate)
				{
					rootTreeNode= candidate;
					break;
				}
			}

			curSearch= curSearch->next;
		}

		// not found? abort
		if(!rootTreeNode)
		{
			// todo hulud interface syntax error
			nlwarning("career*_generator: thee root container '%s' was not found", rootContainer.c_str());
			return false;
		}
	}

	return true;
}


// ***************************************************************************
bool CInterfaceParser::parseCareerGeneratorParams(xmlNodePtr cur,
		string		&templateCareer,
		string		&templateJob,
		string		&careerWindow,
		string		&jobWindow,
		xmlNodePtr	&rootTreeNode,
		bool		&brickTypeFilter,
		BRICK_TYPE::EBrickType	&brickType
		)
{
	CXMLAutoPtr prop;

	// **** Parse the generator properties
	// template_career
	prop = xmlGetProp (cur, (xmlChar*)"template_career");
	if(prop) templateCareer= (const char*)prop;
	else
	{
		// todo hulud interface syntax error
		nlwarning("prop 'template_career' not found");
		return false;
	}

	// template_job
	prop = xmlGetProp (cur, (xmlChar*)"template_job");
	if(prop) templateJob= (const char*)prop;
	else
	{
		// todo hulud interface syntax error
		nlwarning("prop 'template_job' not found");
		return false;
	}

	// career_window
	prop = xmlGetProp (cur, (xmlChar*)"career_window");
	if(prop) careerWindow= (const char*)prop;
	else
	{
		// todo hulud interface syntax error
		nlwarning("prop 'career_window' not found");
		return false;
	}

	// job_window
	prop = xmlGetProp (cur, (xmlChar*)"job_window");
	if(prop) jobWindow= (const char*)prop;
	else
	{
		// todo hulud interface syntax error
		nlwarning("prop 'job_window' not found");
		return false;
	}

	// get brick type
	prop = xmlGetProp (cur, (xmlChar*)"brick_type");
	if(prop)
	{
		if( !strcmp(prop, "none") )
		{
			brickTypeFilter= false;
		}
		else
		{
			brickTypeFilter= true;
			brickType= BRICK_TYPE::toBrickType((const char*)prop);
			if(brickType == BRICK_TYPE::UNKNOWN)
			{
				// todo hulud interface syntax error
				nlwarning("'brick_type' UKNOWN");
				return false;
			}
		}
	}
	else
	{
		// todo hulud interface syntax error
		nlwarning("prop 'brick_type' not found");
		return false;
	}

	// create or search the root container.
	if(!parseGeneratorRootContainer(cur, rootTreeNode))
		return false;

	return true;
}


// ***************************************************************************
bool CInterfaceParser::parseBrickCareerGenerator(xmlNodePtr /* cur */)
{
	return false;
}


// ***************************************************************************
bool CInterfaceParser::parseBrickSuffixGenerator(xmlNodePtr /* cur */)
{
	return false;
}


// ***************************************************************************
bool CInterfaceParser::parseStyle(xmlNodePtr cur)
{
	H_AUTO(parseStyle)

	string		styleId;
	CXMLAutoPtr prop;

	prop = xmlGetProp (cur, (xmlChar*)"style");
	if(prop)
		styleId= (const char*)prop;
	else
	{
		// todo hulud interface syntax error
		nlwarning("'style' not found in 'style'");
		return false;
	}

	// create or replace style
	CStyle	newStyle;

	//get the node properties
	xmlAttrPtr props = cur->properties;
	while (props)
	{
		// if not the "style" property
		if( nlstricmp((const char*)props->name, "style") != 0 )
		{
			// Append it.
			newStyle.Properties.push_back(CStyleProperty());
			newStyle.Properties.back().Name= (const char*)props->name;

			CXMLAutoPtr ptr(xmlGetProp( cur, props->name ));
			newStyle.Properties.back().Value = (const char*)ptr;
		}

		props= props->next;
	}

	// associate.
	_StyleMap[styleId]= newStyle;

	return true;
}

// ***************************************************************************
bool CInterfaceParser::parseDDX (xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	H_AUTO(parseDDX )

	CInterfaceDDX *pDDX = NULL;
	pDDX = new CInterfaceDDX;
	if (pDDX)
	{
		if (!pDDX->parse(cur,parentGroup))
		{
			delete pDDX;
			return false;
		}
		return true;
	}
	return false;
}

// ***************************************************************************
bool CInterfaceParser::parseLUAScript (xmlNodePtr cur)
{
	H_AUTO(parseLUA)

	// read fileName
	CXMLAutoPtr prop;
	string	fileName;
	prop = xmlGetProp (cur, (xmlChar*)"file");
	if(prop)
		fileName= (const char*)prop;
	else
	{
		nlwarning("'file' not found in 'lua'");
		return false;
	}

	// Append to set of reloadable FileScripts
	_LuaFileScripts.insert(fileName);

	// just display warning here.
	string	dummyError;
	return loadLUA(fileName, dummyError);
}

// ***************************************************************************
bool				CInterfaceParser::solveStyle(xmlNodePtr cur)
{
	H_AUTO(solveStyle)

	CXMLAutoPtr ptr;

	// if the node is a style, abort (not recrusive because use "style" as param name)
	if ( !strcmp((char*)cur->name,"style") )
		return true;

	// try to read a "style" param.
	ptr= (char*)xmlGetProp( cur, (xmlChar*)"style");
	if(ptr)
	{
		// get the style
		TStyleMap::iterator	it= _StyleMap.find((const char*)ptr);
		if( it==_StyleMap.end() )
		{
			// todo hulud interface syntax error
			nlwarning("style '%s' not found", (const char*)ptr);
			return false;
		}

		// the style
		CStyle	&style= it->second;

		// for all properties of the style, set them in the node
		for(uint i=0;i<style.Properties.size();i++)
		{
			const char *propName= style.Properties[i].Name.c_str();
			const char *propValue= style.Properties[i].Value.c_str();

			// replace it only if the property is not already defined, (=> user can overide style properties)
			CXMLAutoPtr ptr2(xmlGetProp( cur, (xmlChar*)propName));
			if( !ptr2 )
			{
				xmlSetProp(cur, (xmlChar*)propName, (xmlChar*)propValue);
			}
		}
	}

	// recurs to node children
	cur= cur->children;
	while(cur)
	{
		if(!solveStyle(cur))
			return false;
		cur= cur->next;
	}

	return true;
}

// ***************************************************************************

#ifdef LUA_NEVRAX_VERSION

class CLuaDebugBreakScreen : public IDebuggedAppMainLoop
{
public:
	// called by external lua debugger when application is breaked
	virtual void breakEventLoop()
	{
		Driver->clearBuffers(CRGBA(90, 90, 90));
		TextContext->setShaded(true);
		TextContext->setFontSize(40);
		TextContext->setColor(CRGBA::White);

		// TOP LEFT //
		//----------//
		TextContext->setHotSpot(NL3D::UTextContext::MiddleMiddle);
		TextContext->printfAt(0.5f, 0.5f, "Break in LUA code");
		Driver->swapBuffers();
	}
};
static CLuaDebugBreakScreen LuaDebugBreakScreen;

#endif


// ***************************************************************************
void	CInterfaceParser::initLUA()
{
	// do nothing if LUA environment already exists
	if( _LuaState != NULL )
		return;

	// create a new LUA environnement
	nlassert(_LuaState==NULL);
	CLuaManager::enableLuaDebugging();
	CLuaManager::getInstance();
	_LuaState= CLuaManager::getInstance().getLuaState();

#ifdef LUA_NEVRAX_VERSION
	extern ILuaIDEInterface *LuaDebuggerIDE;
	if (LuaDebuggerIDE) LuaDebuggerIDE->setDebuggedAppMainLoop(&LuaDebugBreakScreen);
#endif

	// register LUA methods
	CLuaIHM::registerAll(*_LuaState);
	CLuaIHMRyzom::RegisterRyzomFunctions( *_LuaState );
}

// ***************************************************************************
void	CInterfaceParser::uninitLUA()
{
	// Delete all LUA environnement (and hence variables)
    // delete _LuaState;
	// _LuaState= NULL;

	// delete all .lua file loaded
	_LuaFileScripts.clear();
}

// ***************************************************************************
bool	CInterfaceParser::loadLUA(const std::string &fileName, std::string &error)
{
	// get file

	bool needCheck = false;

	#if !FINAL_VERSION
		needCheck = false;
	#endif

	string	pathName= CPath::lookup(fileName, false);
	if(pathName.empty())
	{
		nlwarning("LUA Script '%s' not found", fileName.c_str());
		return false;
	}

	bool isInData = false;
	std::string::size_type pos = pathName.find("@");
	if (pos != string::npos)
	{
		if (CBigFile::getInstance().getBigFileName(pathName.substr(0, pos)) != "data/"+pathName.substr(0, pos))
			isInData = false;
		else
			isInData = true;
	}

	if (needCheck && !isInData)
	{
		nlwarning("You are not allowed to modify the lua files");
		// return true so it'll not generate a message box, we just ignore the file
		return true;
	}

	// Parse script
	nlassert(_LuaState);
	try
	{
		_LuaState->executeFile(pathName);
	}
	catch(const ELuaError &e)
	{
		nlwarning(e.luaWhat().c_str());
		error= e.luaWhat();
		return false;
	}

	return true;
}

