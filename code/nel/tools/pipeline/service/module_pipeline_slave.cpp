/**
 * \file module_pipeline_slave.cpp
 * \brief CModulePipelineSlave
 * \date 2012-03-03 16:26GMT
 * \author Jan Boon (Kaetemi)
 * CModulePipelineSlave
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
#include "module_pipeline_slave_itf.h"

// STL includes

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/mutex.h>
#include <nel/misc/task_manager.h>

// Project includes
#include "info_flags.h"
#include "module_pipeline_master_itf.h"
#include "pipeline_service.h"
#include "../plugin_library/process_info.h"
#include "pipeline_workspace.h"
#include "pipeline_process_impl.h"
#include "database_status.h"
#include "pipeline_project.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace PIPELINE {

#define PIPELINE_INFO_SLAVE_RELOAD_SHEETS "S_RELOAD_SHEETS"
#define PIPELINE_ERROR_SHEETS_CRC32_FAILED "Failed sheets CRC32. Sheets were modified inbetween launching services. This causes newly loaded services to be out of sync. Not allowed. Reload the sheets from the master service, and restart this slave service"

#define PIPELINE_INFO_BUILD_TASK "S_BUILD_TASK"

#define PIPELINE_INFO_STATUS_UPDATE_MASTER "S_ST_UPD_MASTER"
#define PIPELINE_INFO_STATUS_UPDATE_SLAVE "S_ST_UPD_SLAVE"
#define PIPELINE_INFO_GET_REMOVE "S_GET_REMOVE"

#define PIPELINE_INFO_ABORTING "S_ABORTING"

#define PIPELINE_INFO_MASTER_CRASH "#S_MASTER_CRASH"

enum TRequestState
{
	REQUEST_NONE, 
	REQUEST_MADE, 
	REQUEST_WORKING, 
};

/**
 * \brief CModulePipelineSlave
 * \date 2012-03-03 16:26GMT
 * \author Jan Boon (Kaetemi)
 * CModulePipelineSlave
 */
class CModulePipelineSlave :
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
	public CModulePipelineSlaveSkel
{
public:
	CModulePipelineMasterProxy *m_Master;
	bool m_TestCommand;
	TRequestState m_ReloadSheetsState;
	bool m_BuildReadyState;

	enum TSlaveTaskState
	{
		IDLE_WAIT_MASTER, 
		SOMEWHERE_INBETWEEN, 
		STATUS_UPDATE, 
		GET_REMOVE, 
		// ...
	};
	TSlaveTaskState m_SlaveTaskState; // only set and used by update!! used on other threads for sanity checks

	NLMISC::CTaskManager *m_TaskManager;

	NLMISC::CSynchronized<bool> m_StatusUpdateMasterDone;
	NLMISC::CSynchronized<bool> m_StatusUpdateSlaveDone;

	CPipelineProject *m_ActiveProject;
	CPipelineProcessImpl *m_ActiveProcess; // TODO: Maybe it would be easier to go directly to CPipelineProject from the plugin and provide an interface therefore.
	CProcessPluginInfo m_ActivePlugin;

	bool m_AbortRequested;

	std::vector<std::string> m_DependentDirectories;
	std::vector<std::string> m_DependentFiles;

	CProcessResult m_ResultPreviousSuccess;
	std::map<std::string, CFileStatus> m_FileStatusInputCache;
	std::map<std::string, CFileStatus> m_FileStatusOutputCache;

	/// List of dependencies (files) that were removed, or never existed (in which case remove time is 0).
	std::map<std::string, CFileRemove> m_FileRemoveInputCache;

	/// Result of current process
	CProcessResult m_ResultCurrent;
	
	/// Result of the last subtask (usually in the taskmanager), check this after the task has completed to make sure all went fine.
	TProcessResult m_SubTaskResult;
	std::string m_SubTaskErrorMessage;


	std::set<std::string> m_ListInputAdded;
	std::set<std::string> m_ListInputChanged;
	std::set<std::string> m_ListInputRemoved;
	
	std::set<std::string> m_ListOutputChanged;
	std::set<std::string> m_ListOutputChangedNG; // after .depend check, found that dependencies changed, so not good
	std::set<std::string> m_ListOutputChangedOK; // idem but dependencies did not change, so ok
	std::set<std::string> m_ListOutputRemoved; // changed_ng and removed end up being the same, it needs to be rebuilt ;)

	std::set<std::string> m_ListDependentDirectories;
	std::set<std::string> m_ListDependentFiles;

public:
	CModulePipelineSlave() : m_Master(NULL), m_TestCommand(false), m_ReloadSheetsState(REQUEST_NONE), m_BuildReadyState(false), m_SlaveTaskState(IDLE_WAIT_MASTER), m_TaskManager(NULL), m_StatusUpdateMasterDone("StatusUpdateMasterDone"), m_StatusUpdateSlaveDone("StatusUpdateSlaveDone"), m_ActiveProject(NULL), m_ActiveProcess(NULL), m_AbortRequested(false), m_SubTaskResult(FINISH_NOT)
	{
		NLMISC::CSynchronized<bool>::CAccessor(&m_StatusUpdateMasterDone).value() = false;
		NLMISC::CSynchronized<bool>::CAccessor(&m_StatusUpdateSlaveDone).value() = false;

		m_TaskManager = new NLMISC::CTaskManager();
		m_ResultPreviousSuccess.clear();
		m_ResultCurrent.clear();
	}

	virtual ~CModulePipelineSlave()
	{
		nldebug("START ~CModulePipelineSlave");
		// ?TODO? IF MASTER STILL CONNECTED, NOTIFY INSANITY
		// ?TODO? ABORT RUNNING BUILD PROCESS
		abortBuildTask(NULL);
		leaveBuildReadyState(NULL);

		// ?TODO?
		// wait till build task has exited if still was running
		// wait for other things to exist in case there are...
		nldebug("Wait for tasks on the slave");
		while (m_TaskManager->getNumWaitingTasks() > 0)
			nlSleep(10);
		delete m_TaskManager;
		m_TaskManager = NULL;

		if (m_AbortRequested)
			finalizeAbort(); // hopefully this is fine

		// temp sanity
		nlassert(m_ActiveProject == NULL);
		nlassert(m_ActiveProcess == NULL);
		nlassert(m_SlaveTaskState == IDLE_WAIT_MASTER);
		nldebug("END ~CModulePipelineSlave");
	}

	virtual bool initModule(const TParsedCommandLine &initInfo)
	{
		CModuleBase::initModule(initInfo);
		CModulePipelineSlaveSkel::init(this);
		return true;
	}

	virtual void onModuleUp(IModuleProxy *moduleProxy)
	{
		if (moduleProxy->getModuleClassName() == "ModulePipelineMaster")
		{
			nlinfo("Master UP (%s)", moduleProxy->getModuleName().c_str());

			nlassert(m_Master == NULL);
			
			m_Master = new CModulePipelineMasterProxy(moduleProxy);
		}
	}

	/// This is here instead of inside onModuleUp because we don't know the order of onModuleUp on local systems. This is only called once per master up like onModuleUp.
	virtual void submitToMaster(NLNET::IModuleProxy *sender)
	{
		// TODO: AUTHENTICATE OR GATEWAY SECURITY?
		CModulePipelineMasterProxy master(sender);
		if (!g_PipelineWorkspace->loadCRC32())
			nlerror(PIPELINE_ERROR_SHEETS_CRC32_FAILED);
		sendMasterAvailablePlugins(&master);
	}
	
	virtual void onModuleDown(IModuleProxy *moduleProxy)
	{
		if (moduleProxy->getModuleClassName() == "ModulePipelineMaster")
		{
			nlinfo("Master DOWN (%s)", moduleProxy->getModuleName().c_str());

			nlassert(m_Master->getModuleProxy() == moduleProxy);

			if (m_BuildReadyState)
			{
				CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_MASTER_CRASH);
			}

			switch (m_SlaveTaskState)
			{
			case STATUS_UPDATE:
				bool masterDone;
				{
					// We won't receive this anymore from the master after disconnect. Do as if from master.
					NLMISC::CSynchronized<bool>::CAccessor statusUpdateMasterDone(&m_StatusUpdateMasterDone);
					masterDone = statusUpdateMasterDone.value();
				}
				if (!masterDone)
				{
					masterUpdatedDatabaseStatus(NULL);
				}
				break;
			default:
				// Nothing to do for now
				break;
			}

			// TODO: ABORT RUNNING BUILD PROCESS
			abortBuildTask(NULL);
			leaveBuildReadyState(NULL); // leave state if building

			delete m_Master;
			m_Master = NULL;
		}
	}

	virtual void onModuleUpdate()
	{
		if (m_ReloadSheetsState == REQUEST_MADE)
		{
			if (PIPELINE::reloadSheets())
			{
				m_ReloadSheetsState = REQUEST_WORKING;
			}
		}
		else if (m_ReloadSheetsState == REQUEST_WORKING)
		{
			if (PIPELINE::isServiceStateIdle())
			{
				m_ReloadSheetsState = REQUEST_NONE;
				if (!g_PipelineWorkspace->loadCRC32())
					nlerror(PIPELINE_ERROR_SHEETS_CRC32_FAILED);
				sendMasterAvailablePlugins(m_Master);
				m_Master->slaveReloadedSheets(this);
				CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_SLAVE_RELOAD_SHEETS);
			}
		}

		switch (m_SlaveTaskState)
		{
		case IDLE_WAIT_MASTER:
			// Keep calm and carry on
			break;
		case STATUS_UPDATE:
			{
				bool bothReady = false;
				NLMISC::CSynchronized<bool>::CAccessor statusUpdateMasterDone(&m_StatusUpdateMasterDone);
				if (statusUpdateMasterDone.value())
				{
					NLMISC::CSynchronized<bool>::CAccessor statusUpdateSlaveDone(&m_StatusUpdateSlaveDone);
					if (statusUpdateSlaveDone.value())
					{
						statusUpdateMasterDone.value() = false;
						statusUpdateSlaveDone.value() = false;
						bothReady = true;
					}
				}
				if (bothReady)
				{
					m_SlaveTaskState = SOMEWHERE_INBETWEEN;
					if (m_AbortRequested)
					{
						nlinfo("Aborted slave task after status update");
						finalizeAbort();
					}
					else
					{
						nlinfo("Slave task: Status update done");
						beginGetRemove();
					}
				}
			}
			break;
		case GET_REMOVE:
			{
				if (m_RemovedTaskDone)
				{
					m_SlaveTaskState = SOMEWHERE_INBETWEEN;
					CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_GET_REMOVE);

					if (m_SubTaskResult != FINISH_SUCCESS)
					{
						// Bye bye.
						finishedTask(m_SubTaskResult, m_SubTaskErrorMessage);
						break;
					}
					
					// Build the lists of files added changed removed
					buildListsOfFiles();
					
					// Set the actual start time of the actual build
					m_ResultCurrent.BuildStart = NLMISC::CTime::getSecondsSince1970();

					// TODO *************************************************************************** *************** **** BUILD
					beginTheBuildThing();
				}
			}
			break;
		default:
			finishedTask(FINISH_ERROR, "Task got lost somewhere inbetween the code of the slave service. This is a programming error. Implementation may be incomplete.");
			break;
		}
		
		// Nothing to do here, move along
	}

	///////////////////////////////////////////////////////////////////

	class CStatusUpdateSlaveTask : public NLMISC::IRunnable
	{
		void cbFile(const std::string &filePath, const CFileStatus &fileStatus, bool success)
		{
			if (success)
				m_Slave->m_FileStatusOutputCache[filePath] = fileStatus;
		}

		void cbDone()
		{
			// Not used because we wait for the updateDatabaseStatus function; it's easier.
		}

	public:
		CStatusUpdateSlaveTask(CModulePipelineSlave *slave) : m_Slave(slave) { }
		virtual void run()
		{
			g_DatabaseStatus->updateDatabaseStatus(
				CCallback<void>(this, &CStatusUpdateSlaveTask::cbDone), 
				TFileStatusCallback(this, &CStatusUpdateSlaveTask::cbFile), 
				m_Slave->m_ResultPreviousSuccess.MacroPaths, true, false);
			
			// Mark as done
			{
				NLMISC::CSynchronized<bool>::CAccessor(&m_Slave->m_StatusUpdateSlaveDone).value() = true;
				CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_STATUS_UPDATE_SLAVE);
			}

			delete this;
		}
	private:
		CModulePipelineSlave *m_Slave;
	};

	/// Begin with the status update of the current task
	void beginTaskStatusUpdate()
	{
		// Set the task state
		m_SlaveTaskState = STATUS_UPDATE;
		nlinfo("Slave task: Status update begin");

		// Sanity check
		{
			nlassert(NLMISC::CSynchronized<bool>::CAccessor(&m_StatusUpdateMasterDone).value() == false);
			nlassert(NLMISC::CSynchronized<bool>::CAccessor(&m_StatusUpdateSlaveDone).value() == false);
		}
		nlassert(m_ResultCurrent.BuildStart == 0);
		nlassert(m_ResultPreviousSuccess.BuildStart == 0);

		// Set start time (not necessary, it will be set again later, when the actual build starts)
		m_ResultCurrent.BuildStart = NLMISC::CTime::getSecondsSince1970();

		// Read the previous process result
		CMetadataStorage::readProcessResult(m_ResultPreviousSuccess, CMetadataStorage::getResultPath(m_ActiveProject->getName(), m_ActivePlugin.Handler));

		// Start the client update
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_STATUS_UPDATE_SLAVE);
		m_TaskManager->addTask(new CStatusUpdateSlaveTask(this));

		// Start the master update
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_STATUS_UPDATE_MASTER);
		//std::vector<std::string> result;
		switch (m_ActivePlugin.InfoType)
		{
		case PIPELINE::PLUGIN_REGISTERED_CLASS:
			{
				PIPELINE::IProcessInfo *processInfo = static_cast<PIPELINE::IProcessInfo *>(NLMISC::CClassRegistry::create(m_ActivePlugin.Info));
				processInfo->setPipelineProcess(m_ActiveProcess);
				m_DependentDirectories.clear();
				processInfo->getDependentDirectories(m_DependentDirectories);
				for (std::vector<std::string>::iterator it = m_DependentDirectories.begin(), end = m_DependentDirectories.end(); it != end; ++it)
					m_Master->vectorPushString(this, PIPELINE::macroPath(*it));
				//result.clear();
				m_DependentFiles.clear();
				processInfo->getDependentFiles(m_DependentFiles);
				for (std::vector<std::string>::iterator it = m_DependentFiles.begin(), end = m_DependentFiles.end(); it != end; ++it)
					m_Master->vectorPushString(this, PIPELINE::macroPath(*it));
				//result.clear();
			}
			break;
		default:
			nlwarning("Plugin type not implemented");
			break;
		}
		m_Master->updateDatabaseStatusByVector(this);
	}

	void beginGetRemove()
	{
		m_SlaveTaskState = GET_REMOVE;
		m_SubTaskResult = FINISH_NOT;
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_GET_REMOVE);
		m_RemovedTaskDone = false;
		m_TaskManager->addTask(new CGetRemovedTask(this));
	}
	
	bool m_RemovedTaskDone;
	class CGetRemovedTask : public IRunnable
	{
	public:
		CGetRemovedTask(CModulePipelineSlave *slave) : m_Slave(slave) { }
		virtual void run()
		{
			breakable
			{
				if (!g_DatabaseStatus->getRemoved(m_Slave->m_FileRemoveInputCache, m_Slave->m_DependentDirectories))
				{
					m_Slave->m_SubTaskResult = FINISH_ERROR;
					m_Slave->m_SubTaskErrorMessage = "Metadata for removed files in directories not sane";
					break;
				}
				if (!g_DatabaseStatus->getRemoved(m_Slave->m_FileRemoveInputCache, m_Slave->m_DependentFiles))
				{
					m_Slave->m_SubTaskResult = FINISH_ERROR;
					m_Slave->m_SubTaskErrorMessage = "Metadata for removed files not sane";
					break;
				}
				m_Slave->m_SubTaskResult = FINISH_SUCCESS;
			}
			m_Slave->m_RemovedTaskDone = true;
		}
	private:
		CModulePipelineSlave *m_Slave;
	};

	///////////////////////////////////////////////////////////////////

	void buildListsOfFiles()
	{
		for (std::map<std::string, CFileStatus>::const_iterator it = m_FileStatusInputCache.begin(), end = m_FileStatusInputCache.end(); it != end; ++it)
		{
			if (it->second.FirstSeen > m_ResultPreviousSuccess.BuildStart)
			{
				m_ListInputAdded.insert(it->first);
			}
			else if (it->second.LastUpdate > m_ResultPreviousSuccess.BuildStart)
			{
				m_ListInputChanged.insert(it->first);
			}
		}
		for (std::map<std::string, CFileRemove>::const_iterator it = m_FileRemoveInputCache.begin(), end = m_FileRemoveInputCache.end(); it != end; ++it)
		{
			if (it->second.Lost > m_ResultPreviousSuccess.BuildStart) // or >= ?
			{
				m_ListInputRemoved.insert(it->first);
			}
		}
		for (std::map<std::string, CFileStatus>::const_iterator it = m_FileStatusOutputCache.begin(), end = m_FileStatusOutputCache.end(); it != end; ++it)
		{
			if (it->second.FirstSeen > m_ResultPreviousSuccess.BuildStart)
			{
				m_ListOutputChanged.insert(it->first);
			}
		}
		//for (std::vector<std::string>::const_iterator it = m_ResultPreviousSuccess.MacroPaths.begin(), end = m_ResultPreviousSuccess.MacroPaths.end(); it != end; ++it)
		for (uint i = 0; i < m_ResultPreviousSuccess.MacroPaths.size(); ++i)
		{
			std::string filePath = unMacroPath(m_ResultPreviousSuccess.MacroPaths[i]);
			std::map<std::string, CFileStatus>::const_iterator statusIt = m_FileStatusOutputCache.find(filePath);
			if (statusIt == m_FileStatusOutputCache.end())
			{
				nlwarning("Spotted remove by not being in the cache");
				m_ListOutputRemoved.insert(filePath);
			}
			else if (m_ResultPreviousSuccess.FileResults[i].CRC32 == 0)
			{
				nlwarning("Spotted remove by dummy CRC32 value"); // not sure which of these two .. dunno if non-existant files were in the cache // keep these codes anyway
				m_ListOutputRemoved.insert(filePath);
			}
			else
			{
				const CFileStatus &status = statusIt->second;
				if (status.CRC32 != m_ResultPreviousSuccess.FileResults[i].CRC32)
				{
					nlwarning("Output file was illegally modified, probably by a previous build run that failed");
					m_ListOutputChanged.insert(filePath);
					// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					// (in the needbuild(something) function)
					// TODO: IF THE OUTPUT IS ILLEGALLY MODIFIED (always the case in m_ListOutputChanged)
					// USE THE .DEPEND FILE TO COMPARE THE CRC32 OF THE CURRENT STATUS WITH THE .DEPEND LOGGED CRC32 (this is the one from the failed build)
					// AND ALSO COMPARE THE CRC32 OF ALL DEPENDENCY FILES WITH THE CACHED STATUS CRC32 OF THESE INPUT FILES
					// IF ANY IS DIFFERENT, IT NEEDS A REBUILD
					// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				}
			}
		}
		for (std::vector<std::string>::iterator it = m_DependentDirectories.begin(), end = m_DependentDirectories.end(); it != end; ++it)
			m_ListDependentDirectories.insert(*it);
		m_DependentDirectories.clear();
		for (std::vector<std::string>::iterator it = m_DependentFiles.begin(), end = m_DependentFiles.end(); it != end; ++it)
			m_ListDependentFiles.insert(*it);
		m_DependentFiles.clear();
		m_SubTaskResult = FINISH_SUCCESS;
	}

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	void beginTheBuildThing()
	{
		// TODO ************/////////////////########################################################################### BUILD THING
	}

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	bool isFileDependency(const std::string &path)
	{
		if (m_ListDependentFiles.find(path) == m_ListDependentFiles.end()) // check if it's directly in the dependent files
		{
			// Not found!
			std::string pathParent = path.substr(0, path.find_last_of('/') + 1);
			if (m_ListDependentDirectories.find(pathParent) == m_ListDependentDirectories.end()) // check if it's inside one of the dependent directories
			{
				// Still not found!
				return false;
			}
		}
		return true;
	} // ok

	bool isDirectoryDependency(const std::string &path)
	{
		return m_ListDependentDirectories.find(path) != m_ListDependentDirectories.end();
	} // ok

	bool hasInputDirectoryBeenModified(const std::string &inputDirectory)
	{
		// Check if any files added/changed/removed are part of this directory (slow).
		for (std::set<std::string>::const_iterator itr = m_ListInputAdded.begin(), endr = m_ListInputAdded.end(); itr != endr; ++itr)
		{
			const std::string &pathr = *itr;
			if ((pathr.size() > inputDirectory.size())
				&& (pathr.substr(0, inputDirectory.size()) == inputDirectory) // inside the path
				&& (pathr.substr(inputDirectory.size(), pathr.size() - inputDirectory.size())).find('/') == std::string::npos) // not in a further subdirectory 
			{
				nldebug("Found added '%s' in dependency directory '%s'", pathr.c_str(), inputDirectory.c_str());
				return true;
			}
		}
		for (std::set<std::string>::const_iterator itr = m_ListInputChanged.begin(), endr = m_ListInputChanged.end(); itr != endr; ++itr)
		{
			const std::string &pathr = *itr;
			if ((pathr.size() > inputDirectory.size())
				&& (pathr.substr(0, inputDirectory.size()) == inputDirectory) // inside the path
				&& (pathr.substr(inputDirectory.size(), pathr.size() - inputDirectory.size())).find('/') == std::string::npos) // not in a further subdirectory 
			{
				nldebug("Found changed '%s' in dependency directory '%s'", pathr.c_str(), inputDirectory.c_str());
				return true;
			}
		}
		for (std::set<std::string>::const_iterator itr = m_ListInputRemoved.begin(), endr = m_ListInputRemoved.end(); itr != endr; ++itr)
		{
			const std::string &pathr = *itr;
			if ((pathr.size() > inputDirectory.size())
				&& (pathr.substr(0, inputDirectory.size()) == inputDirectory) // inside the path
				&& (pathr.substr(inputDirectory.size(), pathr.size() - inputDirectory.size())).find('/') == std::string::npos) // not in a further subdirectory 
			{
				nldebug("Found removed '%s' in dependency directory '%s'", pathr.c_str(), inputDirectory.c_str());
				return true;
			}
		}
		// nldebug("Input directory '%s' not modified", inputDirectory.c_str());
		return false;
	} // ok

	bool hasInputFileBeenModified(const std::string &inputFile)
	{
		return m_ListInputAdded.find(inputFile) != m_ListInputAdded.end()
			|| m_ListInputChanged.find(inputFile) != m_ListInputChanged.end()
			|| m_ListInputRemoved.find(inputFile) != m_ListInputRemoved.end();
	} // ok

	bool needsToBeRebuilt(const std::vector<std::string> &inputPaths)
	{
		if (m_SubTaskResult != FINISH_SUCCESS)
			return false; // Cannot continue on previous failure.

		m_SubTaskResult = FINISH_NOT;

		// Sanity check
		for (std::vector<std::string>::const_iterator it = inputPaths.begin(), end = inputPaths.end(); it != end; ++it)
		{
			const std::string &path = *it;
			if (path[path.size() - 1] == '/') // isDirectory
			{
				m_SubTaskResult = FINISH_ERROR;
				m_SubTaskErrorMessage = std::string("Input file '") + path + "' cannot be a directory";
				return false; // Error, cannot rebuild.
			}
			if (!isFileDependency(path))
			{
				m_SubTaskResult = FINISH_ERROR;
				m_SubTaskErrorMessage = std::string("File '") + path + "' is not part of the dependencies";
				return false; // Error, cannot rebuild.
			}
		}

		// Find out if anything happened to the input files.
		bool inputFilesDifferent = false;
		for (std::vector<std::string>::const_iterator it = inputPaths.begin(), end = inputPaths.end(); it != end; ++it)
		{
			const std::string &path = *it;
			if (hasInputFileBeenModified(path))
			{
				// Found!
				nldebug("Found added/changed/removed input file '%s'", path.c_str());
				inputFilesDifferent = true;
				break;
			}
		}
		if (!inputFilesDifferent)
		{
			nldebug("No added/changed/removed input files in this request");
			if (m_ListOutputChanged.size() == 0 && m_ListOutputRemoved.size() == 0)
			{
				nldebug("No output files were tampered with since last successful build, rebuild not needed");
				m_SubTaskResult = FINISH_SUCCESS;
				return false; // No rebuild required.
			}
			else
			{
				nldebug("Output files may have changed, find out which output files are part of these input files");
				m_SubTaskResult = FINISH_SUCCESS;
				return needsToBeRebuildSubByOutput(inputPaths, false);
			}
		}
		else // input files have changed
		{
			// Find out if all the input files have a .output file
			bool allInputFilesHaveOutputFile = true;
			for (std::vector<std::string>::const_iterator it = inputPaths.begin(), end = inputPaths.end(); it != end; ++it)
			{
				const std::string &path = *it;
				std::string outputMetaPath = CMetadataStorage::getOutputPath(path, m_ActiveProject->getName(), m_ActivePlugin.Handler);
				if (!CFile::fileExists(outputMetaPath))
				{
					nldebug("Found an input file that has no .output meta file");
					allInputFilesHaveOutputFile = false;
					break;
				}
			}
			if (!allInputFilesHaveOutputFile)
			{
				nldebug("Not all input files have an .output files, rebuild");
				m_SubTaskResult = FINISH_SUCCESS;
				return true; // Rebuild
			}
			else
			{
				nldebug("Output files may or may not be up to date, find out more, after the break");
				m_SubTaskResult = FINISH_SUCCESS;
				return needsToBeRebuildSubByOutput(inputPaths, true);
			}
		}
		// not reachable
	} // ok

	bool needsToBeRebuildSubByOutput(const std::vector<std::string> &inputPaths, bool inputChanged)
	{
		if (m_SubTaskResult != FINISH_SUCCESS)
			return false; // Cannot continue on previous failure.
		
		m_SubTaskResult = FINISH_NOT;

		std::vector<std::string> outputPaths;

		bool needsFurtherInfo = false;

		// For each inputPath
		for (std::vector<std::string>::const_iterator it = inputPaths.begin(), end = inputPaths.end(); it != end; ++it)
		{
			const std::string &path = *it;
			// FIXME: What if an input file does not exist? Deleted long ago? Just print a warning (& notify terminal), keep calm, and carry on.
			nlassert(!CFile::isDirectory(path)); // All input are files! Coding error otherwise, because should already be checked.
			std::string metaOutputPath = CMetadataStorage::getOutputPath(path, m_ActiveProject->getName(), m_ActivePlugin.Handler);
			nlassert(CFile::fileExists(metaOutputPath)); // Coding error otherwise, already must have checked beforehand that they all exist.

			// Read the .output file
			CFileOutput metaOutput;
			CMetadataStorage::readOutput(metaOutput, metaOutputPath);
			std::vector<std::string> localOutputPaths;
			localOutputPaths.reserve(metaOutput.MacroPaths.size());
			for (std::vector<std::string>::const_iterator it = metaOutput.MacroPaths.begin(), end = metaOutput.MacroPaths.end(); it != end; ++it)
				localOutputPaths.push_back(unMacroPath(*it));

			// If inputChanged & hasInputFileBeenModified(path)
			if (inputChanged && hasInputFileBeenModified(path))
			{
				// If .output file was empty
				if (metaOutput.MacroPaths.empty())
				{
					// Require rebuild because we don't know if there's new output
					nldebug("Input file '%s' has output file with no output files, state not known, so rebuild", path.c_str());
					m_SubTaskResult = FINISH_SUCCESS;
					return true; // Rebuild.
				}
			}
			
			// Copy the .output file into the output paths
			for (std::vector<std::string>::const_iterator it = localOutputPaths.begin(), end = localOutputPaths.end(); it != end; ++it)
				outputPaths.push_back(*it);
			// End
		}

		// Output files may have already been built by a failed build,
		// or they might have been tampered with after they were built.
		nldebug("Need further information");
		m_SubTaskResult = FINISH_SUCCESS;
		return needsToBeRebuiltSub(inputPaths, outputPaths, inputChanged); // to skip input changed checks because these are only input files and no input folders
	} // ok ? ?

	bool needsToBeRebuilt(const std::vector<std::string> &inputPaths, const std::vector<std::string> &outputPaths)
	{
		if (m_SubTaskResult != FINISH_SUCCESS)
			return false; // Cannot continue on previous failure.
		
		m_SubTaskResult = FINISH_NOT;

		// Sanity check of all the input paths, 
		// they must be either files that are dependency files or inside dependency directories
		// or directories that are dependency directories.
		for (std::vector<std::string>::const_iterator it = inputPaths.begin(), end = inputPaths.end(); it != end; ++it)
		{
			const std::string &path = *it;
			if (path[path.size() - 1] == '/') // isDirectory
			{
				// Check if this directory is in the dependencies.
				if (!isDirectoryDependency(path))
				{
					m_SubTaskResult = FINISH_ERROR;
					m_SubTaskErrorMessage = std::string("Directory '") + path + "' is not part of the dependencies";
					return false; // Error, cannot rebuild.
				}
			}
			else // isFile
			{
				// Check if this file is in the dependencies.
				if (!isFileDependency(path))
				{
					m_SubTaskResult = FINISH_ERROR;
					m_SubTaskErrorMessage = std::string("File '") + path + "' is not part of the dependencies";
					return false; // Error, cannot rebuild.
				}
			}
		}
		
		// Check if any of the input has changed
		bool inputFilesDifferent = false;
		for (std::vector<std::string>::const_iterator it = inputPaths.begin(), end = inputPaths.end(); it != end; ++it)
		{
			const std::string &path = *it;
			if (path[path.size() - 1] == '/') // isDirectory
			{
				if (hasInputDirectoryBeenModified(path))
				{
					nldebug("Found modified input directory '%s'", path.c_str());
					inputFilesDifferent = true;
					break;
				}
			}
			else
			{
				if (hasInputFileBeenModified(path))
				{
					nldebug("Found modified input file '%s'", path.c_str());
					inputFilesDifferent = true;
					break;
				}
			}
		}

		if (inputFilesDifferent)
		{
			nldebug("Input files were modified, check if output files were already built");
			m_SubTaskResult = FINISH_SUCCESS;
			return needsToBeRebuiltSub(inputPaths, outputPaths, true);
		}
		else
		{
			nldebug("Input files were not modified");
			if (m_ListOutputChanged.size() == 0 && m_ListOutputRemoved.size() == 0)
			{
				nldebug("No output files were tampered with since last successful build, rebuild not needed");
				m_SubTaskResult = FINISH_SUCCESS;
				return false; // No rebuild required.
			}
			else
			{
				nldebug("Output files may have changed, find out more");
				m_SubTaskResult = FINISH_SUCCESS;
				return needsToBeRebuiltSub(inputPaths, outputPaths, false);
			}
		}
	}

	bool needsToBeRebuiltSub(const std::vector<std::string> &inputPaths, const std::vector<std::string> &outputPaths, bool inputModified)
	{
		if (m_SubTaskResult != FINISH_SUCCESS)
			return false; // Cannot continue on previous failure.
		
		m_SubTaskResult = FINISH_NOT;

		// Sanity check of all the output paths
		// These must all be files, no directories allowed
		for (std::vector<std::string>::const_iterator it = outputPaths.begin(), end = outputPaths.end(); it != end; ++it)
		{
			const std::string &path = *it;
			if (path[path.size() - 1] == '/') // isDirectory
			{
				m_SubTaskResult = FINISH_ERROR;
				m_SubTaskErrorMessage = std::string("Output file '") + path + "' cannot be a directory";
				return false; // Error, cannot rebuild.
			}
		}

		// Check if any of the output paths are part of the removed output files
		for (std::vector<std::string>::const_iterator it = outputPaths.begin(), end = outputPaths.end(); it != end; ++it)
		{
			const std::string &path = *it;
			if (m_ListOutputRemoved.find(*it) != m_ListOutputRemoved.end())
			{
				// If so, rebuild
				nldebug("Output file '%s' has been removed, rebuild", path.c_str());
				m_SubTaskResult = FINISH_SUCCESS;
				return true; // Rebuild.
			}
		}

		// Check if any of the output files don't exist
		for (std::vector<std::string>::const_iterator it = outputPaths.begin(), end = outputPaths.end(); it != end; ++it)
		{
			// If so, rebuild
			const std::string &path = *it;
			if (!CFile::isExists(path))
			{
				// If so, rebuild
				nldebug("Output file '%s' does not exist, rebuild", path.c_str());
				m_SubTaskResult = FINISH_SUCCESS;
				return true; // Rebuild.
			}
		}
		
		// Check if any of the output paths are part of the changed output files
		bool outputChanged = false; 
		for (std::vector<std::string>::const_iterator it = outputPaths.begin(), end = outputPaths.end(); it != end; ++it)
		{
			const std::string &path = *it;
			if (m_ListOutputChanged.find(*it) != m_ListOutputChanged.end())
			{
				nldebug("Output file '%s' has been changed", path.c_str());
				outputChanged = true;
				break;
			}
		}

		if (!outputChanged && !inputModified)
		{
			nlerror("Should never reach this, this must have been cought earlier normally");
		}
		
		// Check the .depend files of all the output files // also check that they exist :)
		for (std::vector<std::string>::const_iterator it = outputPaths.begin(), end = outputPaths.end(); it != end; ++it)
		{
			const std::string &path = *it;
			std::string metaDependPath = CMetadataStorage::getDependPath(path);
			CFileDepend metaDepend;
			if (!CMetadataStorage::readDepend(metaDepend, metaDependPath))
			{
				nlwarning("Depend file for existing output '%s' does not exist, this should not happen, rebuild", path.c_str());
				m_SubTaskResult = FINISH_SUCCESS;
				return true; // Rebuild.
			}
			else
			{
				if (outputChanged)
				{
					// Compare the output checksum with the status output checksum
					CFileStatus metaStatus;
					if (!CDatabaseStatus::updateFileStatus(metaStatus, path))
					{
						m_SubTaskResult = FINISH_ERROR;
						m_SubTaskErrorMessage = std::string("Could not get status for output file '") + path + "'";
						return false; // Error, cannot rebuild.
					}
					else
					{
						if (metaStatus.CRC32 != metaDepend.CRC32)
						{
							nlwarning("Status checksum for '%s' does match depend checksum, output file was modified, this should not happen, rebuild", path.c_str());
							m_SubTaskResult = FINISH_SUCCESS;
							return true; // Rebuild.
						}
					}
				}
				if (inputModified)
				{
					// Compare the input checksums with the cached input checksums

				}
			}
		}

		// (if any checksum was different, require rebuild, if no checksums were different, no rebuild is needed)
		nldebug("No differences found, no rebuild needed");
		m_SubTaskResult = FINISH_SUCCESS;
		return false; // Rebuild not necessary.
	}

	/// Set the exit message, exit the plugin immediately afterwards.
	void setExit(const TProcessResult exitLevel, const std::string &exitMessage)
	{
		m_SubTaskResult = exitLevel;
		m_SubTaskErrorMessage = exitMessage;
	}

	/// Must verify this regularly to exit the plugin in case something went wrong.
	bool needsExit()
	{
		// TODO: Bypass error feature.
		if (m_SubTaskResult != FINISH_SUCCESS)
			return true;
		return false;
	}

	// TODO: Make maps of the dependent files and directories after the vectors no longer needed
	// Provide a function to check if a dependency is either in the dependent files or inside one of the directories to ensure the plugin is behaving sanely
	// Slave should check this when it reads the depend log of a tool

	// TODO: Provide a function: bool needsToBeRebuilt(inputpaths, outputfiles)
	// It should check if anything was added, changed or removed in the input paths.
	// It should check if the output files were changed or removed.
	// NOT NECESSARY AT ALL --- It should check if the dependencies in the metadata of the output in case the dependencies are not given in the inputpaths.
	
	// NOT NECESSARY --- TODO: Provide a function for removing output files, it will also erase the status file and create a remove metadata file.

	// PROBABLY NOT NECESSARY -- Could check the .depend of all output files from the previous build if necessary instead of having output files and use those to flag additional removals.
	
	// TODO: (use this one for known input to unknown output) Also, instead of the .output file, we can check outputfileremoved and outputfilechanged's depend to flag input files that have lost output files!!!

	// TODO: If the state of an output file that is changed & ok or not changed is error, return error!

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	virtual void startBuildTask(NLNET::IModuleProxy *sender, const std::string &projectName, uint32 pluginId)
	{
		nlassert(m_Master->getModuleProxy() == sender); // sanity check
		nlassert(m_ActiveProject == NULL);
		nlassert(m_ActiveProcess == NULL);
		
		// TODO: ERROR HANDLING !!! (see sanity check above)
		// master->slaveRefusedBuildTask(this);

		// Set the task state somewhere inbetween
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_BUILD_TASK);
		m_SlaveTaskState = SOMEWHERE_INBETWEEN;

		// Set the active project and get the plugin information
		m_ActiveProject = g_PipelineWorkspace->getProject(projectName);
		m_ActiveProcess = new CPipelineProcessImpl(m_ActiveProject);
		g_PipelineWorkspace->getProcessPlugin(m_ActivePlugin, pluginId);
		
		// TODO: ERROR HANDLING !!! (see sanity check above)
		// master->slaveRefusedBuildTask(this);

		// Begin with status update of the dependent directories and files on the master, and previous output files on the slave
		beginTaskStatusUpdate();
	}

	void clearActiveProcess()
	{
		m_ActiveProject = NULL;
		m_ActiveProcess = NULL;
		m_ResultPreviousSuccess.clear();
		m_FileStatusInputCache.clear();
		m_FileStatusOutputCache.clear();
		m_FileRemoveInputCache.clear();
		m_ResultCurrent.clear();
		m_DependentDirectories.clear();
		m_DependentFiles.clear();
		m_ListInputAdded.clear();
		m_ListInputChanged.clear();
		m_ListInputRemoved.clear();
		m_ListOutputChanged.clear();
		m_ListOutputRemoved.clear();
		m_ListOutputChangedNG.clear();
		m_ListOutputChangedOK.clear();
		m_ListDependentDirectories.clear();
		m_ListDependentFiles.clear();
		m_SubTaskResult = FINISH_NOT; // ! //
	}

	void finalizeAbort()
	{
		nldebug("abort");
		clearActiveProcess();
		m_SlaveTaskState = IDLE_WAIT_MASTER;
		if (m_Master) // else was disconnect
			m_Master->slaveAbortedBuildTask(this);
		m_AbortRequested = false;
		CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_ABORTING);
		CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_BUILD_TASK);
	}

	void finishedTask(TProcessResult errorLevel, const std::string &errorMessage)
	{
		nlinfo("errorLevel: %i, errorMessage: %s", (uint32)errorLevel, errorMessage.c_str());
		clearActiveProcess();
		m_SlaveTaskState = IDLE_WAIT_MASTER;
		if (m_Master) // else was disconnect
			m_Master->slaveFinishedBuildTask(this, (uint8)errorLevel, errorMessage);
		CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_BUILD_TASK);
	}

	/// Master or user request to abort.
	virtual void abortBuildTask(NLNET::IModuleProxy *sender)
	{
		if (m_ActiveProject && !m_AbortRequested)
		{
			// Sender NULL is request from slave (user exit, command or master disconnect), otherwise request from master.
			nlassert(sender == NULL || m_Master->getModuleProxy() == sender); // sanity check

			nlwarning("Aborting");
			// ?TODO? Actually wait for the task manager etc to end before sending the aborted confirmation.
			CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_ABORTING);
			m_AbortRequested = true;

			// ?TODO?
			//m_ActiveProject = NULL;
			//m_ActiveProcess = NULL;
			//m_Master->slaveAbortedBuildTask(this);
			// ?TODO?

			// KABOOM
		}
	}
	
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	// If this returns false, the status is not sane, and the build must error; or the file does not exist and must error or warn.
	// Gets the file status as it was known at the beginning of the build or when this file was first known.
	bool getDependencyFileStatusCached(CFileStatus &fileStatus, const std::string &filePath) // not by macro path :)
	{
		std::map<std::string, CFileStatus>::iterator statusIt = m_FileStatusInputCache.find(filePath);
		if (statusIt == m_FileStatusInputCache.end())
		{
			/*nldebug("File status '%s' not requested before, ensure this is not a dependency", filePath.c_str());
			if (g_DatabaseStatus->getFileStatus(fileStatus, filePath))
			{
				m_FileStatusCache[filePath] = fileStatus;
				return true;
			}
			else*/ return false;
		}
		fileStatus = statusIt->second; // copy
		return true;
	}

	// If this returns false, the status is not sane, and the build must error; or the file does not exist and must error or warn.
	bool getDependencyFileStatusLatest(CFileStatus &fileStatus, const std::string &filePath)
	{
		return g_DatabaseStatus->getFileStatus(fileStatus, filePath);
	}

	virtual void addFileStatusToCache(NLNET::IModuleProxy *sender, const std::string &macroPath, const CFileStatus &fileStatus)
	{
		nlassert(/*sender == NULL || */m_Master->getModuleProxy() == sender); // sanity check
		
		// nldebug("Add file status: '%s' (macro path)", macroPath.c_str());

		std::string filePath = unMacroPath(macroPath);
		// m_FileStatusInitializeMutex.enter();
		nlassert(m_FileStatusInputCache.find(filePath) == m_FileStatusInputCache.end()); // for now don't allow depending on own output within process :)
		m_FileStatusInputCache[filePath] = fileStatus;
		// m_FileStatusInitializeMutex.leave();
	}
	
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	virtual void masterUpdatedDatabaseStatus(NLNET::IModuleProxy *sender)
	{
		nlassert(sender == NULL || m_Master->getModuleProxy() == sender); // sanity check

		if (m_TestCommand)
		{
			endedRunnableTask();
			m_SlaveTaskState = IDLE_WAIT_MASTER;
		}
		else
		{
			nldebug("Master updated database status");
			nlassert(m_SlaveTaskState == STATUS_UPDATE);
			// Notify the update function that the master update has arrived.
			NLMISC::CSynchronized<bool>::CAccessor(&m_StatusUpdateMasterDone).value() = true;
			CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_STATUS_UPDATE_MASTER);
		}
	}

	virtual void reloadSheets(NLNET::IModuleProxy *sender)
	{
		nlassert(m_Master->getModuleProxy() == sender); // sanity check

		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_SLAVE_RELOAD_SHEETS);
		if (PIPELINE::reloadSheets()) m_ReloadSheetsState = REQUEST_WORKING;
		else m_ReloadSheetsState = REQUEST_MADE;
	}
	
	virtual void enterBuildReadyState(NLNET::IModuleProxy *sender)
	{
		nlassert(m_Master->getModuleProxy() == sender); // sanity check

		if (!m_BuildReadyState && PIPELINE::tryBuildReady())
		{
			m_BuildReadyState = true;
			m_Master->slaveBuildReadySuccess(this);
		}
		else
		{
			m_Master->slaveBuildReadyFail(this);
		}
	}
	
	virtual void leaveBuildReadyState(NLNET::IModuleProxy *sender)
	{
		nlassert(sender == NULL || m_Master->getModuleProxy() == sender); // sanity check

		if (m_BuildReadyState)
		{
			m_BuildReadyState = false;
			PIPELINE::endedBuildReady();
		}
	}

	void sendMasterAvailablePlugins(CModulePipelineMasterProxy *master)
	{
		std::vector<uint32> availablePlugins;
		g_PipelineWorkspace->listAvailablePlugins(availablePlugins);
		master->setAvailablePlugins(this, availablePlugins);
	}
	
protected:
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CModulePipelineSlave, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CModulePipelineSlave, testUpdateDatabaseStatus, "Test master request for database status update on dependencies", "<projectName> <processName>")
		NLMISC_COMMAND_HANDLER_ADD(CModulePipelineSlave, testGetFileStatus, "Test reading of file status from slave on dependencies", "<projectName> <processName>")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(testUpdateDatabaseStatus);
	NLMISC_CLASS_COMMAND_DECL(testGetFileStatus);

}; /* class CModulePipelineSlave */

//return PIPELINE::tryRunnableTask(stateName, task);

///////////////////////////////////////////////////////////////////////

namespace {

class CTestUpdateDatabaseStatusCommand : public NLMISC::IRunnable
{
public:
	NLMISC::CLog *Log;
	std::string Project;
	std::string Process;
	CModulePipelineSlave *Slave;

	virtual void getName(std::string &result) const 
	{ result = "CTestUpdateDatabaseStatusCommand"; }

	virtual void run()
	{
		Slave->m_TestCommand = true;

		// std::string tempDirectory = PIPELINE::IPipelineProcess::getInstance()->getTempDirectory();
		std::vector<PIPELINE::CProcessPluginInfo> plugins;
		PIPELINE::g_PipelineWorkspace->getProcessPlugins(plugins, Process);
		PIPELINE::CPipelineProject *project = PIPELINE::g_PipelineWorkspace->getProject(Project);
		if (project)
		{
			std::vector<std::string> result;
			PIPELINE::IPipelineProcess *pipelineProcess = new PIPELINE::CPipelineProcessImpl(project);
			for (std::vector<PIPELINE::CProcessPluginInfo>::iterator plugin_it = plugins.begin(), plugin_end = plugins.end(); plugin_it != plugin_end; ++plugin_it)
			{
				switch (plugin_it->InfoType)
				{
				case PIPELINE::PLUGIN_REGISTERED_CLASS:
					{
						PIPELINE::IProcessInfo *processInfo = static_cast<PIPELINE::IProcessInfo *>(NLMISC::CClassRegistry::create(plugin_it->Info));
						processInfo->setPipelineProcess(pipelineProcess);
						processInfo->getDependentDirectories(result);
						for (std::vector<std::string>::iterator it = result.begin(), end = result.end(); it != end; ++it)
							Slave->m_Master->vectorPushString(Slave, PIPELINE::macroPath(*it));
						result.clear();
						processInfo->getDependentFiles(result);
						for (std::vector<std::string>::iterator it = result.begin(), end = result.end(); it != end; ++it)
							Slave->m_Master->vectorPushString(Slave, PIPELINE::macroPath(*it));
						result.clear();
					}
					break;
				default:
					nlwarning("Not implemented");
					break;
				}
			}
		}
		else
		{
			Log->displayNL("Project '%s' does not exist", Project.c_str());
		}

		Slave->m_Master->updateDatabaseStatusByVector(Slave);
		
		delete this;
	}
};

} /* anonymous namespace */

NLMISC_CLASS_COMMAND_IMPL(CModulePipelineSlave, testUpdateDatabaseStatus)
{
	// EXAMPLE USAGE: slave.testUpdateDatabaseStatus common_interface Interface

	if (args.size() != 2) return false;
	
	PIPELINE::CPipelineProject *project = PIPELINE::g_PipelineWorkspace->getProject(args[0]);
	if (!project)
	{ 
		log.displayNL("Project '%s' does not exist", args[0].c_str());
		return false;
	}
	
	CTestUpdateDatabaseStatusCommand *runnableCommand = new CTestUpdateDatabaseStatusCommand();
	runnableCommand->Log = &log;
	runnableCommand->Project = args[0];
	runnableCommand->Process = args[1];
	runnableCommand->Slave = this;
	
	if (!tryRunnableTask("SLAVE_TEST_UPD_DB_STATUS", runnableCommand))
	{ log.displayNL("BUSY"); delete runnableCommand; return false; }
	return true;
}

///////////////////////////////////////////////////////////////////////

namespace {

class CTestGetFileStatusCommand : public NLMISC::IRunnable
{
public:
	NLMISC::CLog *Log;
	std::string Project;
	std::string Process;
	CModulePipelineSlave *Slave;

	virtual void getName(std::string &result) const 
	{ result = "CTestGetFileStatusCommand"; }

	virtual void run()
	{
		// Slave->m_TestCommand = true;

		// std::string tempDirectory = PIPELINE::IPipelineProcess::getInstance()->getTempDirectory();
		std::vector<PIPELINE::CProcessPluginInfo> plugins;
		PIPELINE::g_PipelineWorkspace->getProcessPlugins(plugins, Process);
		PIPELINE::CPipelineProject *project = PIPELINE::g_PipelineWorkspace->getProject(Project);
		
		std::vector<std::string> dependPaths;

		if (project)
		{
			std::vector<std::string> result;
			PIPELINE::IPipelineProcess *pipelineProcess = new PIPELINE::CPipelineProcessImpl(project);
			for (std::vector<PIPELINE::CProcessPluginInfo>::iterator plugin_it = plugins.begin(), plugin_end = plugins.end(); plugin_it != plugin_end; ++plugin_it)
			{
				switch (plugin_it->InfoType)
				{
				case PIPELINE::PLUGIN_REGISTERED_CLASS:
					{
						PIPELINE::IProcessInfo *processInfo = static_cast<PIPELINE::IProcessInfo *>(NLMISC::CClassRegistry::create(plugin_it->Info));
						processInfo->setPipelineProcess(pipelineProcess);
						processInfo->getDependentDirectories(result);
						for (std::vector<std::string>::iterator it = result.begin(), end = result.end(); it != end; ++it)
							dependPaths.push_back(*it);
						result.clear();
						processInfo->getDependentFiles(result);
						for (std::vector<std::string>::iterator it = result.begin(), end = result.end(); it != end; ++it)
							dependPaths.push_back(*it);
						result.clear();
					}
					break;
				default:
					nlwarning("Not implemented");
					break;
				}
			}
		}
		else
		{
			Log->displayNL("Project '%s' does not exist", Project.c_str());
		}

		// Slave->m_Master->getFileStatusByVector(Slave);
		std::map<std::string, CFileStatus> fileStatusMap;
		std::map<std::string, CFileRemove> fileRemoveMap;
		if (g_DatabaseStatus->getFileStatus(fileStatusMap, fileRemoveMap, dependPaths))
			Log->displayNL("File status completed successfully");
		else
			Log->displayNL("File status failed");
		Log->displayNL("Found %i file statuses, %i removes", fileStatusMap.size(), fileRemoveMap.size());
		
		delete this;

		endedRunnableTask();
	}
};

} /* anonymous namespace */

NLMISC_CLASS_COMMAND_IMPL(CModulePipelineSlave, testGetFileStatus)
{
	// EXAMPLE USAGE: slave.testGetFileStatus common_interface Interface

	if (args.size() != 2) return false;
	
	PIPELINE::CPipelineProject *project = PIPELINE::g_PipelineWorkspace->getProject(args[0]);
	if (!project)
	{ 
		log.displayNL("Project '%s' does not exist", args[0].c_str());
		return false;
	}
	
	CTestGetFileStatusCommand *runnableCommand = new CTestGetFileStatusCommand();
	runnableCommand->Log = &log;
	runnableCommand->Project = args[0];
	runnableCommand->Process = args[1];
	runnableCommand->Slave = this;
	
	if (!tryRunnableTask("SLAVE_TEST_GET_F_STATUS", runnableCommand))
	{ log.displayNL("BUSY"); delete runnableCommand; return false; }
	return true;
}

///////////////////////////////////////////////////////////////////////

void module_pipeline_slave_forceLink() { }
NLNET_REGISTER_MODULE_FACTORY(CModulePipelineSlave, "ModulePipelineSlave");

} /* namespace PIPELINE */

/* end of file */
