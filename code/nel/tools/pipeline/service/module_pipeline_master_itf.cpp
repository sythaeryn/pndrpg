
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "module_pipeline_master_itf.h"

namespace PIPELINE
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CModulePipelineMasterSkel::TMessageHandlerMap &CModulePipelineMasterSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("SFBT"), &CModulePipelineMasterSkel::slaveFinishedBuildTask_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SRBT"), &CModulePipelineMasterSkel::slaveRefusedBuildTask_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;
	}
	bool CModulePipelineMasterSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CModulePipelineMasterSkel::slaveFinishedBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_slaveFinishedBuildTask_SFBT);
		uint32	taskId;
			nlRead(__message, serial, taskId);
		slaveFinishedBuildTask(sender, taskId);
	}

	void CModulePipelineMasterSkel::slaveRefusedBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_slaveRefusedBuildTask_SRBT);
		uint32	taskId;
			nlRead(__message, serial, taskId);
		slaveRefusedBuildTask(sender, taskId);
	}
		// 
	void CModulePipelineMasterProxy::slaveFinishedBuildTask(NLNET::IModule *sender, uint32 taskId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->slaveFinishedBuildTask(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), taskId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_slaveFinishedBuildTask(__message, taskId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineMasterProxy::slaveRefusedBuildTask(NLNET::IModule *sender, uint32 taskId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->slaveRefusedBuildTask(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), taskId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_slaveRefusedBuildTask(__message, taskId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_slaveFinishedBuildTask(NLNET::CMessage &__message, uint32 taskId)
	{
		__message.setType("SFBT");
			nlWrite(__message, serial, taskId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_slaveRefusedBuildTask(NLNET::CMessage &__message, uint32 taskId)
	{
		__message.setType("SRBT");
			nlWrite(__message, serial, taskId);


		return __message;
	}

}
