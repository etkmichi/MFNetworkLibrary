/*
 * MFEnetUser.cpp
 *
 *  Created on: 03.03.2020
 *      Author: michl
 */

#include "MFEnetUser.h"
#include <MFObjects/MFObject.h>
std::vector<MFEnetUser*> MFEnetUser::ms_vecInetUser;
std::mutex MFEnetUser::lockVecEnetUser;
std::mutex MFEnetUser::sm_enetLock;
MFEnetUser::MFEnetUser(bool startThread) :
	MFTaskThread("MFEnetUser",startThread),
	MFINetPollInput(this)
{
	lockVecEnetUser.lock();
	ms_vecInetUser.push_back(this);
	lockVecEnetUser.unlock();
	mp_execThread=this;
	m_maxEventCount=100;
	mp_unusedEvents=new S_MF_EventBuffer();
	mp_vecChannelReceiveTasks=new std::vector<MFNetEventDispatchTask*>();
	mp_connectTask=nullptr;
	mp_disconnectTask=nullptr;
	mp_defaultPollTask=new MFPollTask(this);
	mp_pollTask=mp_defaultPollTask;
	m_enetSetup.m_channelCount=8;
	mp_defReceiveTask=new MFNetEventDispatchTask();
	mp_defConnectTask=new MFDefaultConnectTask();
	mp_defDisconnectTask=new MFDefaultDisconnectTask();
	m_pollTimeout=0;//should stay 0 bc. lock of enet will block
}

MFEnetUser::~MFEnetUser() {
	exitEnetInstance();
	delete mp_defaultPollTask;
}

bool MFEnetUser::initEnetInstance(){
	if(m_isInitialized){
		printInfo("MFEnetUser::initEnetInstance - already"
				"initialized! for reinit call exitEnetInstance before!");
		//TODO test reinit!
		return true;
	}
	if(!mp_unusedEvents->init(m_maxEventCount,true)){
		printErr("MFServerClientInstance::initInstance() - "
				"failed to init event buffer!");
		return false;
	}
	bool ret=MFEnetInstance::addUser();
	if(!ret){
		printErr("MFEnetUser::initEnetInstance() - "
				"failed to register enet user, init will be canceled!");
		return false;
	}
	mp_vecChannelReceiveTasks->resize(m_enetSetup.m_channelCount);

	for(uint32_t counter=0;counter<m_enetSetup.m_channelCount;counter++){
		mp_vecChannelReceiveTasks->data()[counter]=mp_defReceiveTask;
	}
	setEventReturnBuffer(mp_unusedEvents);

	if(!initInstance(this)){
		printErr("MFEnetUser::initEnetInstance() - Failed to init Enet!");
		return false;
	}
	m_isInitialized=true;
	return true;
}

void MFEnetUser::setInstanceDescription(std::string desc){m_instanceDescription=desc;}

void MFEnetUser::setInstanceName(std::string name){m_instanceName=name;}

bool MFEnetUser::exitEnetInstance(){
	if(!m_isInitialized)return true;
	if(!exitInstance(this)){
		printErr("MFEnetUser::exitEnetInstance() - Failed to exit Enet!"
				"");//TODO what to do?
	}
	m_isInitialized=!MFEnetInstance::takeUser();
	return !m_isInitialized;
}

bool MFEnetUser::exitAllEnetUsers(bool blockNewInits){
	lockVecEnetUser.lock();
	bool ret=true;
	for(MFEnetUser* pEU:ms_vecInetUser){
		ret&=pEU->exitEnetInstance();
	}
	if(blockNewInits)
		MFEnetInstance::blockInit();
	lockVecEnetUser.unlock();
	return ret;
}

void MFEnetUser::enqueueDisconnectTask(){
	MFNetEventDispatchTask* pTask=nullptr;
	if(mp_disconnectTask!=nullptr){
		pTask=mp_disconnectTask;
	}else{
		pTask=(mp_defDisconnectTask);
	}
	if(pTask!=nullptr)
		mp_execThread->addTask(pTask);
	else
		printErr("MFEnetUser::enqueueDisconnectTask() - "
				"no valid task for enqueueing!");
}

void MFEnetUser::enqueueTasks(){
	if(m_isPollThreadStarted && mp_execThread->isStarted()){
		enqueueReceiveTasks();
		enqueueConnectTask();
		enqueueDisconnectTask();
		mp_execThread->addTask(mp_pollTask);
	}
}

void MFEnetUser::enqueueReceiveTasks(){
	bool addDefaultReceiveDispTask=false;
	for(MFAbstractTask* pTask:*mp_vecChannelReceiveTasks){
		if(pTask!=nullptr){
			mp_execThread->addTask(pTask);
		}else{
			addDefaultReceiveDispTask=true;
		}
	}
	if(addDefaultReceiveDispTask)
		mp_execThread->addTask(mp_defReceiveTask);
}

void MFEnetUser::enqueueConnectTask(){
	MFNetEventDispatchTask* pTask=nullptr;
	if(mp_connectTask!=nullptr){
		pTask=(mp_connectTask);
	}else{
		pTask=(mp_defConnectTask);
	}
	if(pTask!=nullptr)
		mp_execThread->addTask(pTask);
	else
		printErr("MFEnetUser::enqueueDisconnectTask() - "
				"no valid task for enqueueing!");
}

bool MFEnetUser::pollInput(){
	if(!ckeckPollConditions()){
		printWarning("MFEnetUser::pollInput() - condition/s for polling "
				"not fullfilled!");
		return false;
	}
	if(mp_nextInputEvent==nullptr){
		mp_nextInputEvent=mp_unusedEvents->takeEvent();
		if(mp_nextInputEvent==nullptr){
			printErr("MFNetClient::pollInput - "
					"no input event available");
			return false;
		}
	}
  bool ret=true;
  while(1){
    uint32_t enetRequests=0;
    if(m_isConnected){
      sm_enetLock.lock();
      enetRequests=enet_host_service(
          mp_enetLocalHost,
          mp_nextInputEvent->pEvent,
          m_pollTimeout);
      sm_enetLock.unlock();
    }else{/*if no connection was made, do not lock enet access (if a local app uses
    *enet on separated threads with connect and host_service, the lock will block...)*/
      enetRequests=enet_host_service(
          mp_enetLocalHost,
          mp_nextInputEvent->pEvent,
          m_pollTimeout);
    }
    if(enetRequests==0)break;
    mp_nextInputEvent->lockAddDispatchTask();
    ENetEvent* pE=mp_nextInputEvent->pEvent;
    switch (pE->type){
    case ENET_EVENT_TYPE_CONNECT:
      printInfo("MFEnetUser::pollInput() - "
          "connection event received!");
      m_isConnected=true;
      if(!dispatchConnectEvent(mp_nextInputEvent)){
        printErr("MFEnetUser::pollInput() - "
            "dispatchConnectEvent failed!");
        mp_unusedEvents->returnEvent(mp_nextInputEvent);
      }
      break;
    case ENET_EVENT_TYPE_RECEIVE:
      if(!dispatchReceiveEvent(mp_nextInputEvent)){
        printErr("MFEnetUser::pollInput() - "
            "dispatchReceiveEvent failed!");
        mp_unusedEvents->returnEvent(mp_nextInputEvent);
      }
      break;
    case ENET_EVENT_TYPE_DISCONNECT:
      printInfo("MFEnetUser::pollInput() - "
          "peer disconnect event!");
      m_isConnected=false;
      if(!dispatchDisconnectEvent(mp_nextInputEvent)){
        printErr("MFEnetUser::pollInput() - "
            "dispatchDisconnectEvent failed!");
        mp_unusedEvents->returnEvent(mp_nextInputEvent);
      }
      break;
    case ENET_EVENT_TYPE_NONE:
      printWarning("MFNetClient::pollInput() - "
          "ENET_EVENT_TYPE_NONE occured during enet_host_service call!");
      mp_nextInputEvent->unlockAddDispatchTask();
      mp_unusedEvents->returnEvent(mp_nextInputEvent);
      ret=false;
    }
    if(ret){
      mp_nextInputEvent->unlockAddDispatchTask();
      mp_nextInputEvent=mp_unusedEvents->takeEvent();
      mp_nextInputEvent->reinit();
    }
    if(mp_nextInputEvent==nullptr){
      printErr("MFNetClient::pollInput - "
          "no input event available");
    }
  }
  enqueueTasks();

	return ret;
}
/**
 *
 * @param pEvent
 * @return true if pEvent was enqueued for dispatching, false if no dispatch task
 * is available.
 */
bool MFEnetUser::dispatchReceiveEvent(S_MF_NetworkEvent* pEvent){
  printErr("MFEnetUser::dispatchDisconnectEvent - not impl.!");
  return false;
};
bool MFEnetUser::dispatchConnectEvent(S_MF_NetworkEvent* pEvent){
  printErr("MFEnetUser::dispatchDisconnectEvent - not impl.!");
  return false;
};
bool MFEnetUser::dispatchDisconnectEvent(S_MF_NetworkEvent* pEvent){
  printErr("MFEnetUser::dispatchDisconnectEvent - not impl.!");
  return false;
};

void MFEnetUser::setConnectDispatchTask(MFNetEventDispatchTask* pDispatcher){
	mp_connectTask=pDispatcher;
}
void MFEnetUser::setDisconnectDispatchTask(MFNetEventDispatchTask* pDispatcher){
	mp_disconnectTask=pDispatcher;
}
bool MFEnetUser::setReceiveDispatchTask(
		uint8_t channel,MFNetEventDispatchTask* pDispatcher){
	if(channel>mp_vecChannelReceiveTasks->size()-1)
		return false;
	mp_vecChannelReceiveTasks->at(channel)=pDispatcher;
	return true;
}

void MFEnetUser::setReceiveDispatchTask(MFNetEventDispatchTask* pDispatcher){
	if(mp_vecChannelReceiveTasks->size()!=m_enetSetup.m_channelCount){
		mp_vecChannelReceiveTasks->resize(m_enetSetup.m_channelCount);
	}//TODO improve channel setup (init may be done before and after setChannelCount...
	printInfo("MFEnetUser::setReceiveDispatchTask setting same channel "
			"receive task for "+std::to_string(mp_vecChannelReceiveTasks->size())+
			" channels");
	for(uint32_t i=0;i<mp_vecChannelReceiveTasks->size();i++){
		mp_vecChannelReceiveTasks->at(i)=pDispatcher;
	}
}
