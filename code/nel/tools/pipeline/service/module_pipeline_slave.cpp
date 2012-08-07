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
#include "../plugin_library/process_handler.h"
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
#define PIPELINE_INFO_PLUGIN_WORKING "S_PLUGIN_WORKING"

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
		ACTUALLY_BUILDING, 
	};
	TSlaveTaskState m_SlaveTaskState; // only set and used by update!! used on other threads for sanity checks

	NLMISC::CTaskManager *m_TaskManager;

	NLMISC::CSynchronized<bool> m_StatusUpdateMasterDone;
	NLMISC::CSynchronized<bool> m_StatusUpdateSlaveDone;

	bool m_AbortRequested;

	CPipelineProject *m_ActiveProject;
	CPipelineProcessImpl *m_ActiveProcess; // TODO: Maybe it would be easier to go directly to CPipelineProject from the plugin and provide an interface therefore.
	
	std::vector<std::string> m_DependentDirectories;
	std::vector<std::string> m_DependentFiles;
	
	bool m_PluginBuildDone;

	struct CErrorLogData
	{
		TError Type;
		std::string Path;
		std::string Time;
		std::string Error;
	};

	NLMISC::CSynchronized<std::vector<CErrorLogData> > m_ErrorLogWaiting;

public:
	CModulePipelineSlave() : m_Master(NULL), m_TestCommand(false), m_ReloadSheetsState(REQUEST_NONE), m_BuildReadyState(false), m_SlaveTaskState(IDLE_WAIT_MASTER), m_TaskManager(NULL), m_StatusUpdateMasterDone("StatusUpdateMasterDone"), m_StatusUpdateSlaveDone("StatusUpdateSlaveDone"), m_ActiveProject(NULL), m_ActiveProcess(NULL), m_AbortRequested(false), m_PluginBuildDone(false), m_ErrorLogWaiting("ErrorLogWaiting")
	{
		NLMISC::CSynchronized<bool>::CAccessor(&m_StatusUpdateMasterDone).value() = false;
		NLMISC::CSynchronized<bool>::CAccessor(&m_StatusUpdateSlaveDone).value() = false;

		m_TaskManager = new NLMISC::CTaskManager();
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

			clearErrorLogQueue();

			delete m_Master;
			m_Master = NULL;
		}
	}

	void clearErrorLogQueue()
	{
		NLMISC::CSynchronized<std::vector<CErrorLogData> >::CAccessor errorLogQueue(&m_ErrorLogWaiting);
		errorLogQueue.value().clear();
	}

	void sendErrorLogQueue()
	{
		NLMISC::CSynchronized<std::vector<CErrorLogData> >::CAccessor errorLogQueue(&m_ErrorLogWaiting);
		for (std::vector<CErrorLogData>::iterator it = errorLogQueue.value().begin(), end = errorLogQueue.value().end(); it != end; ++it)
		{
			const CErrorLogData &erdt = *it;
			nlassert(m_Master);
			m_Master->slaveLoggedToolError(this, (uint8)erdt.Type, macroPath(erdt.Path), erdt.Time, erdt.Error);
		}
		errorLogQueue.value().clear();
	}

	virtual void onModuleUpdate()
	{
		sendErrorLogQueue();

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

					if (m_ActiveProcess->m_SubTaskResult != FINISH_SUCCESS)
					{
						// Bye bye.
						finishedTask(m_ActiveProcess->m_SubTaskResult, m_ActiveProcess->m_SubTaskErrorMessage);
						break;
					}
					if (m_AbortRequested)
					{
						nlinfo("Aborted slave task after getting removed files");
						finalizeAbort();
						break;
					}
					
					// Build the lists of files added changed removed
					buildListsOfFiles();
					
					// Set the actual start time of the actual build
					m_ActiveProcess->m_ResultCurrent.BuildStart = NLMISC::CTime::getSecondsSince1970();

					// TODO *************************************************************************** *************** **** BUILD
					beginTheBuildThing();
				}
			}
			break;
		case ACTUALLY_BUILDING:
			if (m_PluginBuildDone)
			{
				m_SlaveTaskState = SOMEWHERE_INBETWEEN;
				CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_PLUGIN_WORKING);
				sendErrorLogQueue();
				if (m_AbortRequested)
				{
					nlinfo("Aborted slave task while plugin was working");
					finalizeAbort();
				}
				else
				{
					finishedTask(m_ActiveProcess->m_SubTaskResult, m_ActiveProcess->m_SubTaskErrorMessage);
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
				m_Slave->m_ActiveProcess->m_FileStatusOutputCache[filePath] = fileStatus;
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
				m_Slave->m_ActiveProcess->m_ResultPreviousSuccess.MacroPaths, true, false);
			
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
		nlassert(m_ActiveProcess);

		// Set start time (not necessary, it will be set again later, when the actual build starts)
		m_ActiveProcess->m_ResultCurrent.BuildStart = NLMISC::CTime::getSecondsSince1970();

		// Read the previous process result
		CMetadataStorage::readProcessResult(m_ActiveProcess->m_ResultPreviousSuccess, CMetadataStorage::getResultPath(m_ActiveProject->getName(), m_ActiveProcess->m_ActivePlugin.Handler));

		// Start the client update
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_STATUS_UPDATE_SLAVE);
		m_TaskManager->addTask(new CStatusUpdateSlaveTask(this));

		// Start the master update
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_STATUS_UPDATE_MASTER);
		//std::vector<std::string> result;
		switch (m_ActiveProcess->m_ActivePlugin.InfoType)
		{
		case PIPELINE::PLUGIN_REGISTERED_CLASS:
			{
				PIPELINE::IProcessInfo *processInfo = static_cast<PIPELINE::IProcessInfo *>(NLMISC::CClassRegistry::create(m_ActiveProcess->m_ActivePlugin.Info));
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
		m_RemovedTaskDone = false;
		m_SlaveTaskState = GET_REMOVE;
		m_ActiveProcess->m_SubTaskResult = FINISH_NOT;
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_GET_REMOVE);
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
				if (!g_DatabaseStatus->getRemoved(m_Slave->m_ActiveProcess->m_FileRemoveInputCache, m_Slave->m_DependentDirectories))
				{
					m_Slave->m_ActiveProcess->m_SubTaskResult = FINISH_ERROR;
					m_Slave->m_ActiveProcess->m_SubTaskErrorMessage = "Metadata for removed files in directories not sane";
					break;
				}
				if (!g_DatabaseStatus->getRemoved(m_Slave->m_ActiveProcess->m_FileRemoveInputCache, m_Slave->m_DependentFiles))
				{
					m_Slave->m_ActiveProcess->m_SubTaskResult = FINISH_ERROR;
					m_Slave->m_ActiveProcess->m_SubTaskErrorMessage = "Metadata for removed files not sane";
					break;
				}
				m_Slave->m_ActiveProcess->m_SubTaskResult = FINISH_SUCCESS;
			}
			m_Slave->m_RemovedTaskDone = true;
		}
	private:
		CModulePipelineSlave *m_Slave;
	};

	///////////////////////////////////////////////////////////////////

	void buildListsOfFiles()
	{
		for (std::map<std::string, CFileStatus>::const_iterator it = m_ActiveProcess->m_FileStatusInputCache.begin(), end = m_ActiveProcess->m_FileStatusInputCache.end(); it != end; ++it)
		{
			if (it->second.FirstSeen > m_ActiveProcess->m_ResultPreviousSuccess.BuildStart)
			{
				m_ActiveProcess->m_ListInputAdded.insert(it->first);
			}
			else if (it->second.LastUpdate > m_ActiveProcess->m_ResultPreviousSuccess.BuildStart)
			{
				m_ActiveProcess->m_ListInputChanged.insert(it->first);
			}
		}
		for (std::map<std::string, CFileRemove>::const_iterator it = m_ActiveProcess->m_FileRemoveInputCache.begin(), end = m_ActiveProcess->m_FileRemoveInputCache.end(); it != end; ++it)
		{
			if (it->second.Lost > m_ActiveProcess->m_ResultPreviousSuccess.BuildStart) // or >= ?
			{
				m_ActiveProcess->m_ListInputRemoved.insert(it->first);
			}
		}
		for (std::map<std::string, CFileStatus>::const_iterator it = m_ActiveProcess->m_FileStatusOutputCache.begin(), end = m_ActiveProcess->m_FileStatusOutputCache.end(); it != end; ++it)
		{
			if (it->second.FirstSeen > m_ActiveProcess->m_ResultPreviousSuccess.BuildStart)
			{
				m_ActiveProcess->m_ListOutputChanged.insert(it->first);
			}
		}
		//for (std::vector<std::string>::const_iterator it = m_ResultPreviousSuccess.MacroPaths.begin(), end = m_ResultPreviousSuccess.MacroPaths.end(); it != end; ++it)
		for (uint i = 0; i < m_ActiveProcess->m_ResultPreviousSuccess.MacroPaths.size(); ++i)
		{
			std::string filePath = unMacroPath(m_ActiveProcess->m_ResultPreviousSuccess.MacroPaths[i]);
			std::map<std::string, CFileStatus>::const_iterator statusIt = m_ActiveProcess->m_FileStatusOutputCache.find(filePath);
			if (statusIt == m_ActiveProcess->m_FileStatusOutputCache.end())
			{
				nlwarning("Spotted remove by not being in the cache");
				m_ActiveProcess->m_ListOutputRemoved.insert(filePath);
			}
			else if (m_ActiveProcess->m_ResultPreviousSuccess.FileResults[i].CRC32 == 0)
			{
				nlwarning("Spotted remove by dummy CRC32 value"); // not sure which of these two .. dunno if non-existant files were in the cache // keep these codes anyway
				m_ActiveProcess->m_ListOutputRemoved.insert(filePath);
			}
			else
			{
				const CFileStatus &status = statusIt->second;
				if (status.CRC32 != m_ActiveProcess->m_ResultPreviousSuccess.FileResults[i].CRC32)
				{
					nlwarning("Output file was illegally modified, probably by a previous build run that failed");
					m_ActiveProcess->m_ListOutputChanged.insert(filePath);
				}
			}
		}
		for (std::vector<std::string>::iterator it = m_DependentDirectories.begin(), end = m_DependentDirectories.end(); it != end; ++it)
			m_ActiveProcess->m_ListDependentDirectories.insert(*it);
		m_DependentDirectories.clear();
		for (std::vector<std::string>::iterator it = m_DependentFiles.begin(), end = m_DependentFiles.end(); it != end; ++it)
			m_ActiveProcess->m_ListDependentFiles.insert(*it);
		m_DependentFiles.clear();
		m_ActiveProcess->m_SubTaskResult = FINISH_SUCCESS;
	}

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	void notifyErrorLog(TError type, const std::string &path, const std::string &time, const std::string &error)
	{
		// Received an error log from a plugin tool
		NLMISC::CSynchronized<std::vector<CErrorLogData> >::CAccessor errorLogQueue(&m_ErrorLogWaiting);
		CErrorLogData erdt;
		erdt.Type = type;
		erdt.Path = path;
		erdt.Time = time;
		erdt.Error = error;
		errorLogQueue.value().push_back(erdt);
	}

	class CPluginBuildTask : public IRunnable
	{
	public:
		CPluginBuildTask(CModulePipelineSlave *slave) : m_Slave(slave) { }
		virtual void run()
		{
			// Call the build plugin
			switch (m_Slave->m_ActiveProcess->m_ActivePlugin.HandlerType)
			{
			case PIPELINE::PLUGIN_REGISTERED_CLASS:
				{
					PIPELINE::IProcessHandler *processHandler = static_cast<PIPELINE::IProcessHandler *>(NLMISC::CClassRegistry::create(m_Slave->m_ActiveProcess->m_ActivePlugin.Handler));
					m_Slave->m_ActiveProcess->m_ErrorLogCallback = TErrorLogCallback(m_Slave, &CModulePipelineSlave::notifyErrorLog);
					processHandler->setPipelineProcess(m_Slave->m_ActiveProcess);
					m_Slave->m_ActiveProcess->m_SubTaskResult = FINISH_SUCCESS;
					processHandler->build();
				}
				break;
			default:
				nlwarning("Plugin type not implemented");
				break;
			}
			
			// Write the results file
			// ... ****************************** TODO

			// Done
			m_Slave->m_PluginBuildDone = true;
			delete this;
		}
	private:
		CModulePipelineSlave *m_Slave;
	};
	
	void beginTheBuildThing()
	{
		m_PluginBuildDone = false;
		m_SlaveTaskState = ACTUALLY_BUILDING;
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_PLUGIN_WORKING);
		m_TaskManager->addTask(new CPluginBuildTask(this));
	}

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	
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
		g_PipelineWorkspace->getProcessPlugin(m_ActiveProcess->m_ActivePlugin, pluginId);
		
		// TODO: ERROR HANDLING !!! (see sanity check above)
		// master->slaveRefusedBuildTask(this);

		// Begin with status update of the dependent directories and files on the master, and previous output files on the slave
		beginTaskStatusUpdate();
	}

	void clearActiveProcess()
	{
		m_ActiveProject = NULL;
		delete m_ActiveProcess;
		m_ActiveProcess = NULL;
		/*m_ResultPreviousSuccess.clear();
		m_FileStatusInputCache.clear();
		m_FileStatusOutputCache.clear();
		m_FileRemoveInputCache.clear();
		m_ResultCurrent.clear();*/
		m_DependentDirectories.clear();
		m_DependentFiles.clear();
		/*m_ListInputAdded.clear();
		m_ListInputChanged.clear();
		m_ListInputRemoved.clear();
		m_ListOutputChanged.clear();
		m_ListOutputRemoved.clear();
		m_ListOutputChangedNG.clear();
		m_ListOutputChangedOK.clear();
		m_ListDependentDirectories.clear();
		m_ListDependentFiles.clear();
		m_SubTaskResult = FINISH_NOT;*/ // ! //
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

	void finishedTask(TProcessResult errorLevel, std::string errorMessage)
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

			if (m_ActiveProcess)
				m_ActiveProcess->m_Aborting = true;

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

	

	virtual void addFileStatusToCache(NLNET::IModuleProxy *sender, const std::string &macroPath, const CFileStatus &fileStatus)
	{
		nlassert(/*sender == NULL || */m_Master->getModuleProxy() == sender); // sanity check
		
		// nldebug("Add file status: '%s' (macro path)", macroPath.c_str());

		std::string filePath = unMacroPath(macroPath);
		// m_FileStatusInitializeMutex.enter();
		//nlassert(m_ActiveProcess->m_FileStatusInputCache.find(filePath) == m_ActiveProcess->m_FileStatusInputCache.end()); // for now don't allow depending on own output within process :)
		if (m_ActiveProcess->m_FileStatusInputCache.find(filePath) != m_ActiveProcess->m_FileStatusInputCache.end())
		{
			nlwarning("*******************************");
			nlwarning("Received twice the same file '%s'", filePath.c_str());
			nlwarning("*******************************");
		}
		m_ActiveProcess->m_FileStatusInputCache[filePath] = fileStatus;
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
		// NLMISC_COMMAND_HANDLER_ADD(CModulePipelineSlave, testGetFileStatus, "Test reading of file status from slave on dependencies", "<projectName> <processName>")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(testUpdateDatabaseStatus);
	// NLMISC_CLASS_COMMAND_DECL(testGetFileStatus);

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
/*
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

}*/ /* anonymous namespace */
/*
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
*/
///////////////////////////////////////////////////////////////////////

void module_pipeline_slave_forceLink() { }
NLNET_REGISTER_MODULE_FACTORY(CModulePipelineSlave, "ModulePipelineSlave");

} /* namespace PIPELINE */

/* end of file */
