
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "module_pipeline_slave_itf.h"

namespace PIPELINE
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CModulePipelineSlaveSkel::TMessageHandlerMap &CModulePipelineSlaveSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("SBT"), &CModulePipelineSlaveSkel::startBuildTask_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;
	}
	bool CModulePipelineSlaveSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
	{
		const TMessageHandlerMap &mh = getMessageHandlers();

		TMessageHandlerMap::const_iterator it(mh.find(message.getName()));

		if (it == mh.end())
		{
			return false;
		}

		TMessageHandler cmd = it->second;
		(this->*cmd)(sender, message);

		return true;
	}

	
	void CModulePipelineSlaveSkel::startBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineSlaveSkel_startBuildTask_SBT);
		uint32	taskId;
			nlRead(__message, serial, taskId);
		std::string	projectName;
			nlRead(__message, serial, projectName);
		std::string	processHandler;
			nlRead(__message, serial, processHandler);
		startBuildTask(sender, taskId, projectName, processHandler);
	}
		// 
	void CModulePipelineSlaveProxy::startBuildTask(NLNET::IModule *sender, uint32 taskId, const std::string &projectName, const std::string &processHandler)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->startBuildTask(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), taskId, projectName, processHandler);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_startBuildTask(__message, taskId, projectName, processHandler);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineSlaveProxy::buildMessageFor_startBuildTask(NLNET::CMessage &__message, uint32 taskId, const std::string &projectName, const std::string &processHandler)
	{
		__message.setType("SBT");
			nlWrite(__message, serial, taskId);
			nlWrite(__message, serial, const_cast < std::string& > (projectName));
			nlWrite(__message, serial, const_cast < std::string& > (processHandler));


		return __message;
	}

}
