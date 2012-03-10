/**
 * \file pipeline_workspace.cpp
 * \brief CPipelineWorkspace
 * \date 2012-02-18 17:23GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineWorkspace
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with RYZOM CORE PIPELINE; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include "pipeline_workspace.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/georges/u_form_elm.h>
#include <nel/misc/path.h>

// Project includes
#include "pipeline_project.h"
#include "pipeline_service.h"
#include "pipeline_interface_impl.h"
#include "database_status.h"

using namespace std;
// using namespace NLMISC;
using namespace NLGEORGES;

namespace PIPELINE {

CPipelineWorkspace::CPipelineWorkspace(NLGEORGES::UFormLoader *formLoader, const std::string &sheetName) : m_FormLoader(formLoader), m_ChangedReference(0), m_FileSizeReference(0), m_CRC32(0)
{
	std::string fullSheetPath = NLMISC::CPath::lookup(sheetName);
	{
		m_ChangedReference = NLMISC::CFile::getFileModificationDate(fullSheetPath);
		m_FileSizeReference = NLMISC::CFile::getFileSize(fullSheetPath);
	}

	m_Form = formLoader->loadForm(sheetName.c_str());
	std::string description;
	m_Form->getRootNode().getValueByName(description, "Description");
	nlinfo("Loading pipeline workspace: '%s'", description.c_str());

	{
		UFormElm *plugins;
		if (m_Form->getRootNode().getNodeByName(&plugins, "Plugins"))
		{
			uint nb;
			plugins->getArraySize(nb);
			for (uint i = 0; i < nb; ++i)
			{
				std::string pluginSheet;
				if (plugins->getArrayValue(pluginSheet, i))
				{
					m_Plugins.push_back(formLoader->loadForm(pluginSheet.c_str()));
				}
				else
				{
					nlwarning("Array error in '%s'", m_Form->getFilename().c_str());
				}
			}
		}
		else
		{
			nlwarning("Missing 'Plugins' in '%s'", m_Form->getFilename().c_str());
		}
	}
	
	{
		UFormElm *projects;
		if (m_Form->getRootNode().getNodeByName(&projects, "Projects"))
		{
			uint nb;
			projects->getArraySize(nb);
			for (uint i = 0; i < nb; ++i)
			{
				std::string projectSheet;
				if (projects->getArrayValue(projectSheet, i))
				{
					std::string projectName = NLMISC::CFile::getFilenameWithoutExtension(projectSheet);
					if (m_Projects.find(projectName) == m_Projects.end())
					{
						std::string fullProjectSheetPath = NLMISC::CPath::lookup(projectSheet);
						uint32 changedReference = NLMISC::CFile::getFileModificationDate(fullProjectSheetPath);
						uint32 fileSizeReference = NLMISC::CFile::getFileSize(fullProjectSheetPath);
						CPipelineProject *project = new CPipelineProject(this, formLoader->loadForm(projectSheet.c_str()));
						m_Projects[projectName] = project;
						nlassert(changedReference == NLMISC::CFile::getFileModificationDate(fullProjectSheetPath));
						nlassert(fileSizeReference == NLMISC::CFile::getFileSize(fullProjectSheetPath));
						project->m_ChangedReference = changedReference;
						project->m_FileSizeReference = fileSizeReference;
					}
					else
						nlwarning("Project '%s' in '%s' already", projectSheet.c_str(), m_Form->getFilename().c_str());
				}
				else
				{
					nlwarning("Array error in '%s'", m_Form->getFilename().c_str());
				}
			}
		}
		else
		{
			nlwarning("Missing 'Projects' in '%s'", m_Form->getFilename().c_str());
		}
	}

	{
		nlassert(m_ChangedReference == NLMISC::CFile::getFileModificationDate(fullSheetPath));
		nlassert(m_FileSizeReference == NLMISC::CFile::getFileSize(fullSheetPath));
	}
}

CPipelineWorkspace::~CPipelineWorkspace()
{
	for (std::map<std::string, CPipelineProject *>::iterator it = m_Projects.begin(), end = m_Projects.end(); it != end; ++it)
		delete (*it).second;
	m_Projects.clear();
}

void CPipelineWorkspace::getProcessPlugins(std::vector<CProcessPluginInfo> &resultAppend, const std::string &process)
{
	uint16 pluginId = 0;
	for (std::vector<NLMISC::CRefPtr<NLGEORGES::UForm> >::iterator it = m_Plugins.begin(), end = m_Plugins.end(); it != end; ++it)
	{
		UFormElm *processHandlers;
		if ((*it)->getRootNode().getNodeByName(&processHandlers, "ProcessHandlers"))
		{
			uint nb;
			processHandlers->getArraySize(nb);
			for (uint i = 0; i < nb; ++i)
			{
				UFormElm *handler;
				if (processHandlers->getArrayNode(&handler, i))
				{
					std::string handlerProcess;
					if (handler->getValueByName(handlerProcess, "Process"))
					{
						if (handlerProcess == process)
						{
							CProcessPluginInfo processPlugin;
							uint32 handlerType;
							uint32 infoType;
							if (handler->getValueByName(handlerType, "HandlerType")
								&& handler->getValueByName(processPlugin.Handler, "Handler")
								&& handler->getValueByName(infoType, "InfoType")
								&& handler->getValueByName(processPlugin.Info, "Info"))
							{
								processPlugin.HandlerType = (TPluginType)handlerType;
								processPlugin.InfoType = (TPluginType)infoType;
								processPlugin.Id.Sub.Plugin = pluginId;
								processPlugin.Id.Sub.Handler = i;
								resultAppend.push_back(processPlugin);

								// nldebug("Found '%s': '%s', '%s'", process.c_str(), processPlugin.Handler.c_str(), processPlugin.Info.c_str());
							}
							else
							{
								nlwarning("Missing value in '%s' at 'ProcessHandlers' at '%i'", (*it)->getFilename().c_str(), i);
							}
						}
					}
					else
					{
						nlwarning("Missing 'Process' in '%s' at 'ProcessHandlers' at '%i'", (*it)->getFilename().c_str(), i);
					}
				}
				else
				{
					nlwarning("Array error in '%s'", (*it)->getFilename().c_str());
				}
			}
		}
		else
		{
			nlwarning("Missing 'ProcessHandlers' in '%s'", (*it)->getFilename().c_str());
		}
		++pluginId;
	}
}

bool CPipelineWorkspace::getProcessPlugin(CProcessPluginInfo &result, uint32 globalId)
{
	CProcessPluginId id;
	id.Global = globalId;
	if (id.Sub.Plugin >= m_Plugins.size())
	{
		nlwarning("Plugin id out of range");
		return false;
	}
	NLMISC::CRefPtr<NLGEORGES::UForm> pluginForm = m_Plugins[id.Sub.Plugin];
	UFormElm *processHandlers;
	if (!pluginForm->getRootNode().getNodeByName(&processHandlers, "ProcessHandlers")) return false;
	UFormElm *handler;
	if (!processHandlers->getArrayNode(&handler, id.Sub.Handler)) return false;
	uint32 handlerType;
	uint32 infoType;
	if (handler->getValueByName(handlerType, "HandlerType")
		&& handler->getValueByName(result.Handler, "Handler")
		&& handler->getValueByName(infoType, "InfoType")
		&& handler->getValueByName(result.Info, "Info"))
	{
		result.HandlerType = (TPluginType)handlerType;
		result.InfoType = (TPluginType)infoType;
		result.Id = id;
		return true;
	}
	else return false;
}

bool CPipelineWorkspace::getProcessPluginDependencies(std::vector<std::string> &resultProcesses, uint32 globalId)
{
	resultProcesses.clear();
	CProcessPluginId id;
	id.Global = globalId;
	if (id.Sub.Plugin >= m_Plugins.size())
	{
		nlwarning("Plugin id out of range");
		return false;
	}
	NLMISC::CRefPtr<NLGEORGES::UForm> pluginForm = m_Plugins[id.Sub.Plugin];
	UFormElm *processHandlers;
	if (!pluginForm->getRootNode().getNodeByName(&processHandlers, "ProcessHandlers")) return false;
	UFormElm *handler;
	if (!processHandlers->getArrayNode(&handler, id.Sub.Handler)) return false;
	UFormElm *processDependencies;
	if (!handler->getNodeByName(&processDependencies, "ProcessDependencies")) return false;
	if (!processDependencies) return false;
	uint nb;
	if (!processDependencies->getArraySize(nb)) return false;
	resultProcesses.resize(nb);
	for (uint i = 0; i < nb; ++i)
	{
		if (!processDependencies->getArrayValue(resultProcesses[i], i))
		{
			resultProcesses.clear();
			return false;
		}
	}
	return true;
}

void CPipelineWorkspace::listAvailablePlugins(std::vector<uint32> &result)
{
	result.clear();
	uint16 pluginId = 0;
	for (std::vector<NLMISC::CRefPtr<NLGEORGES::UForm> >::iterator it = m_Plugins.begin(), end = m_Plugins.end(); it != end; ++it)
	{
		UFormElm *processHandlers;
		if ((*it)->getRootNode().getNodeByName(&processHandlers, "ProcessHandlers"))
		{
			uint nb;
			processHandlers->getArraySize(nb);
			for (uint i = 0; i < nb; ++i)
			{
				UFormElm *handler;
				if (processHandlers->getArrayNode(&handler, i))
				{
					std::string handlerProcess;
					if (handler->getValueByName(handlerProcess, "Process"))
					{
						CProcessPluginInfo processPlugin;
						uint32 handlerType;
						uint32 infoType;
						if (handler->getValueByName(handlerType, "HandlerType")
							&& handler->getValueByName(processPlugin.Handler, "Handler")
							&& handler->getValueByName(infoType, "InfoType")
							&& handler->getValueByName(processPlugin.Info, "Info"))
						{
							processPlugin.HandlerType = (TPluginType)handlerType;
							processPlugin.InfoType = (TPluginType)infoType;
							processPlugin.Id.Sub.Plugin = pluginId;
							processPlugin.Id.Sub.Handler = i;
							
							bool handlerAvailable = false;
							switch (processPlugin.HandlerType)
							{
							case PLUGIN_REGISTERED_CLASS:
								handlerAvailable = (find(g_PipelineInterfaceImpl->RegisteredClasses.begin(), g_PipelineInterfaceImpl->RegisteredClasses.end(), processPlugin.Handler) != g_PipelineInterfaceImpl->RegisteredClasses.end());
								break;
							default:
								nlwarning("Not implemented");
								break;
							}

							if (handlerAvailable)
							{
								bool infoAvailable = false;
								switch (processPlugin.InfoType)
								{
								case PLUGIN_REGISTERED_CLASS:
									infoAvailable = (find(g_PipelineInterfaceImpl->RegisteredClasses.begin(), g_PipelineInterfaceImpl->RegisteredClasses.end(), processPlugin.Info) != g_PipelineInterfaceImpl->RegisteredClasses.end());
									break;
								default:
									nlwarning("Not implemented");
									break;
								}
								if (infoAvailable)
								{
									nldebug("Available '%s', '%s'", processPlugin.Handler.c_str(), processPlugin.Info.c_str());
									result.push_back(processPlugin.Id.Global);
								}
							}
							
						}
						else
						{
							nlwarning("Missing value in '%s' at 'ProcessHandlers' at '%i'", (*it)->getFilename().c_str(), i);
						}
					}
					else
					{
						nlwarning("Missing 'Process' in '%s' at 'ProcessHandlers' at '%i'", (*it)->getFilename().c_str(), i);
					}
				}
				else
				{
					nlwarning("Array error in '%s'", (*it)->getFilename().c_str());
				}
			}
		}
		else
		{
			nlwarning("Missing 'ProcessHandlers' in '%s'", (*it)->getFilename().c_str());
		}
		++pluginId;
	}
}

CPipelineProject *CPipelineWorkspace::getProject(const std::string &project)
{
	std::map<std::string, CPipelineProject *>::const_iterator it = m_Projects.find(project);
	if (it == m_Projects.end())
	{
		nlwarning("Project '%s' does not exist", project.c_str());
		return NULL;
	}
	return it->second;
}

bool CPipelineWorkspace::loadCRC32()
{
	bool ok = true;
	{
		std::string sheetPath = NLMISC::CPath::lookup(m_Form->getFilename());
		CFileStatus fileStatus;
		if (g_DatabaseStatus->getFileStatus(fileStatus, sheetPath)
			&& fileStatus.LastChangedReference == m_ChangedReference
			&& fileStatus.LastFileSizeReference == m_FileSizeReference)
		{
			m_CRC32 = fileStatus.CRC32;
		}
		else
		{
			nlwarning("Workspace sheet '%s' not synchronized", sheetPath.c_str());
			ok = false;
		}
	}
	for (std::map<std::string, CPipelineProject *>::iterator it = m_Projects.begin(), end = m_Projects.end(); it != end; ++it)
	{
		CPipelineProject *project = it->second;
		{
			std::string sheetPath = NLMISC::CPath::lookup(project->m_Form->getFilename());
			CFileStatus fileStatus;
			if (g_DatabaseStatus->getFileStatus(fileStatus, sheetPath)
				&& fileStatus.LastChangedReference == project->m_ChangedReference
				&& fileStatus.LastFileSizeReference == project->m_FileSizeReference)
			{
				project->m_CRC32 = fileStatus.CRC32;
			}
			else
			{
				nlwarning("Workspace sheet '%s' not synchronized", sheetPath.c_str());
				ok = false;
			}
		}
	}
	if (!ok)
	{
		nlwarning("Workspace not synchronized");
	}
	else
	{
		nldebug("Workspace CRC32 synchronized");
	}
	return ok;
}

} /* namespace PIPELINE */

/* end of file */
