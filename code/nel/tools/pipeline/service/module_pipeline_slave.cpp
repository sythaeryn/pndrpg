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

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace PIPELINE {

#define PIPELINE_INFO_SLAVE_RELOAD_SHEETS "S_RELOAD_SHEETS"
#define PIPELINE_ERROR_SHEETS_CRC32_FAILED "Failed sheets CRC32. Sheets were modified inbetween launching services. This causes newly loaded services to be out of sync. Not allowed. Reload the sheets from the master service, and restart this slave service"

#define PIPELINE_INFO_BUILD_TASK "S_BUILD_TASK"

#define PIPELINE_INFO_STATUS_UPDATE_MASTER "S_ST_UPD_MASTER"
#define PIPELINE_INFO_STATUS_UPDATE_SLAVE "S_ST_UPD_SLAVE"

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

	std::map<std::string, CFileStatus> m_FileStatusCache;
	
public:
	CModulePipelineSlave() : m_Master(NULL), m_TestCommand(false), m_ReloadSheetsState(REQUEST_NONE), m_BuildReadyState(false), m_SlaveTaskState(IDLE_WAIT_MASTER), m_TaskManager(NULL), m_StatusUpdateMasterDone("StatusUpdateMasterDone"), m_StatusUpdateSlaveDone("StatusUpdateSlaveDone"), m_ActiveProject(NULL), m_ActiveProcess(NULL), m_AbortRequested(false)
	{
		NLMISC::CSynchronized<bool>::CAccessor(&m_StatusUpdateMasterDone).value() = false;
		NLMISC::CSynchronized<bool>::CAccessor(&m_StatusUpdateSlaveDone).value() = false;

		m_TaskManager = new NLMISC::CTaskManager();
	}

	virtual ~CModulePipelineSlave()
	{
		nldebug("START ~CModulePipelineSlave");
		// TODO: IF MASTER STILL CONNECTED, NOTIFY INSANITY
		// TODO: ABORT RUNNING BUILD PROCESS
		abortBuildTask(NULL);
		leaveBuildReadyState(NULL);

		// TODO?
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
						// Done with the status updating, now do something fancey
						// ... TODO ...
						// not implemented, so abort.
						// abortBuildTask(NULL);
					}
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
	public:
		CStatusUpdateSlaveTask(CModulePipelineSlave *slave) : m_Slave(slave) { }
		virtual void run()
		{
			// ****************************************************************************** TODO...
			// Read the last build results
			// Update the database status of those files

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

		// Start the client update
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_STATUS_UPDATE_SLAVE);
		m_TaskManager->addTask(new CStatusUpdateSlaveTask(this));

		// Start the master update
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_STATUS_UPDATE_MASTER);
		std::vector<std::string> result;
		switch (m_ActivePlugin.InfoType)
		{
		case PIPELINE::PLUGIN_REGISTERED_CLASS:
			{
				PIPELINE::IProcessInfo *processInfo = static_cast<PIPELINE::IProcessInfo *>(NLMISC::CClassRegistry::create(m_ActivePlugin.Info));
				processInfo->setPipelineProcess(m_ActiveProcess);
				processInfo->getDependentDirectories(result);
				for (std::vector<std::string>::iterator it = result.begin(), end = result.end(); it != end; ++it)
					m_Master->vectorPushString(this, PIPELINE::macroPath(*it));
				result.clear();
				processInfo->getDependentFiles(result);
				for (std::vector<std::string>::iterator it = result.begin(), end = result.end(); it != end; ++it)
					m_Master->vectorPushString(this, PIPELINE::macroPath(*it));
				result.clear();
			}
			break;
		default:
			nlwarning("Plugin type not implemented");
			break;
		}
		m_Master->updateDatabaseStatusByVector(this);
	}

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

	void finalizeAbort()
	{
		m_ActiveProject = NULL;
		m_ActiveProcess = NULL;
		m_FileStatusCache.clear();
		m_SlaveTaskState = IDLE_WAIT_MASTER;
		if (m_Master) // else was disconnect
			m_Master->slaveAbortedBuildTask(this);
		m_AbortRequested = false;
		CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_ABORTING);
		CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_BUILD_TASK);
	}

	void finishedTask(TProcessResult errorLevel, const std::string &errorMessage)
	{
		m_ActiveProject = NULL;
		m_ActiveProcess = NULL;
		m_FileStatusCache.clear();
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

	virtual void addFileStatusToCache(NLNET::IModuleProxy *sender, const std::string &macroPath, const CFileStatus &fileStatus)
	{
		nlassert(sender == NULL || m_Master->getModuleProxy() == sender); // sanity check
		
		nldebug("Add file status: '%s' (macro path)", macroPath.c_str());

		std::string filePath = unMacroPath(macroPath);
		nlassert(m_FileStatusCache.find(filePath) == m_FileStatusCache.end());
		m_FileStatusCache[filePath] = fileStatus;
	}

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
