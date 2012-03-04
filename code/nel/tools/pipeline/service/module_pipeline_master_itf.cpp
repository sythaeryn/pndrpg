
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
			
			res = handlers.insert(std::make_pair(std::string("RE_BT_OK"), &CModulePipelineMasterSkel::slaveFinishedBuildTask_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("RE_BT_FAIL"), &CModulePipelineMasterSkel::slaveRefusedBuildTask_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("RE_SHEETS_OK"), &CModulePipelineMasterSkel::slaveReloadedSheets_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("VEC_PUSH_STR"), &CModulePipelineMasterSkel::vectorPushString_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("UPD_DB_ST"), &CModulePipelineMasterSkel::updateDatabaseStatusByVector_skel));
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
		H_AUTO(CModulePipelineMasterSkel_slaveFinishedBuildTask_RE_BT_OK);
		uint32	taskId;
			nlRead(__message, serial, taskId);
		slaveFinishedBuildTask(sender, taskId);
	}

	void CModulePipelineMasterSkel::slaveRefusedBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_slaveRefusedBuildTask_RE_BT_FAIL);
		uint32	taskId;
			nlRead(__message, serial, taskId);
		slaveRefusedBuildTask(sender, taskId);
	}

	void CModulePipelineMasterSkel::slaveReloadedSheets_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_slaveReloadedSheets_RE_SHEETS_OK);
		slaveReloadedSheets(sender);
	}

	void CModulePipelineMasterSkel::vectorPushString_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_vectorPushString_VEC_PUSH_STR);
		std::string	str;
			nlRead(__message, serial, str);
		vectorPushString(sender, str);
	}

	void CModulePipelineMasterSkel::updateDatabaseStatusByVector_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_updateDatabaseStatusByVector_UPD_DB_ST);
		updateDatabaseStatusByVector(sender);
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
		// 
	void CModulePipelineMasterProxy::slaveReloadedSheets(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->slaveReloadedSheets(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_slaveReloadedSheets(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineMasterProxy::vectorPushString(NLNET::IModule *sender, const std::string &str)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->vectorPushString(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), str);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_vectorPushString(__message, str);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineMasterProxy::updateDatabaseStatusByVector(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateDatabaseStatusByVector(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateDatabaseStatusByVector(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_slaveFinishedBuildTask(NLNET::CMessage &__message, uint32 taskId)
	{
		__message.setType("RE_BT_OK");
			nlWrite(__message, serial, taskId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_slaveRefusedBuildTask(NLNET::CMessage &__message, uint32 taskId)
	{
		__message.setType("RE_BT_FAIL");
			nlWrite(__message, serial, taskId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_slaveReloadedSheets(NLNET::CMessage &__message)
	{
		__message.setType("RE_SHEETS_OK");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_vectorPushString(NLNET::CMessage &__message, const std::string &str)
	{
		__message.setType("VEC_PUSH_STR");
			nlWrite(__message, serial, const_cast < std::string& > (str));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_updateDatabaseStatusByVector(NLNET::CMessage &__message)
	{
		__message.setType("UPD_DB_ST");


		return __message;
	}

}
