/*
 * MFEnetUser.h
 *
 *  Created on: 03.03.2020
 *      Author: michl
 */

#ifndef MFENETUSER_H_
#define MFENETUSER_H_
#include <vector>
#include <mutex>
#include "MFEnetInstance.h"
#include "../MFNetworkTasks/MFNetEventDispatchTask.h"
#include "../MFNetworkTasks/MFDefaultTasks/MFDefaultConnectTask.h"
#include "../MFNetworkTasks/MFDefaultTasks/MFDefaultDisconnectTask.h"
#include "../MFNetworkTasks/MFDefaultTasks/MFDefaultReceiveTask.h"
#include "../MFNetworkInterfaces/MFPollTask.h"
class MFEnetUser;
/**
 * This class uses the MFEnetInstance to initiate Enet and exit Enet automatically,
 * after the initEnetInstance and exitEnetInstace function are called.
 */
class MFEnetUser :
    public MFTaskThread,
    public MFINetPollInput{
private:
  static std::mutex
  lockVecEnetUser;

  static std::vector<MFEnetUser*>
  ms_vecInetUser;

  MFAbstractTask
  *mp_defaultPollTask;

  std::string
  m_instanceName="",
  m_instanceDescription="";

  std::vector<MFNetEventDispatchTask*>
  *mp_vecChannelReceiveTasks;

protected://TODO movec to private
  bool
  m_isInitialized=false,
  m_isConnected=false;

  MFAbstractTask
  *mp_pollTask;

  uint32_t
  m_maxEventCount,
  m_pollTimeout=0;

  S_MF_ENetHostClientSetup
  m_enetSetup;

  ENetHost
  *mp_enetLocalHost=nullptr;

  S_MF_EventBuffer
  *mp_unusedEvents;

  S_MF_NetworkEvent*
  mp_nextInputEvent=nullptr;

  MFNetEventDispatchTask
  *mp_connectTask=nullptr,
  *mp_disconnectTask=nullptr,
  *mp_defConnectTask=nullptr,
  *mp_defDisconnectTask=nullptr,
  *mp_defReceiveTask=nullptr;

protected:/*virtual functions of MFEnetUser*/

  virtual bool initInstance(const MFEnetUser *enetUser){return false;};
  virtual bool exitInstance(const MFEnetUser *enetUser){return false;};

public:/*virtual functions of MFEnetUser*/
  static std::mutex
  sm_enetLock;

  /**
   *
   * @param pEvent
   * @return true if pEvent was enqueued for dispatching, false if no dispatch task
   * is available.
   */
  virtual bool dispatchReceiveEvent(S_MF_NetworkEvent* pEvent);
  virtual bool dispatchConnectEvent(S_MF_NetworkEvent* pEvent);
  virtual bool dispatchDisconnectEvent(S_MF_NetworkEvent* pEvent);
  virtual bool ckeckPollConditions(){return true;};
  /**
   * Default implementation will execute while(enet_host_service(...)>0) and redirect
   * the received events/data to the corresponding type's (connect/receive/
   * disconnect/none) dispatch function (Must be implemented by a subclass!).
   *
   * @return
   */
  virtual bool pollInput();

public:
  MFEnetUser(bool startThread=true);
  virtual ~MFEnetUser();
  bool initEnetInstance();
  bool exitEnetInstance();
  bool isInitialized(){return m_isInitialized;};

  void setPort(uint16_t port){m_enetSetup.mp_address->port=port;};

  void setInstanceName(std::string name);
  void setInstanceDescription(std::string desc);
  static bool exitAllEnetUsers(bool blockNewInits);

  /**
   * This function enqueues the receive, connect, disconnect and polling tasks to the
   * executing task thread. This will ensure that the received data, which was already
   * polled by the poll task, will be dispatched by a thread.
   */
  void enqueueTasks();
  void enqueueReceiveTasks();
  void enqueueConnectTask();
  void enqueueDisconnectTask();

  void setPollTask(MFAbstractTask* pPollTask){mp_pollTask=pPollTask;};

  void setChannelCount(uint8_t channelCount){
    uint32_t chC=0+channelCount;
    m_enetSetup.m_channelCount=chC;
    if(isInitialized()){
      printWarning("MFEnetUser::setChannelCount - MFEnetUser already initialized! "
          "Reinitialization will be needed to change setup!");
    }
  }
  void setConnectDispatchTask(MFNetEventDispatchTask* pDispatcher);
  void setDisconnectDispatchTask(MFNetEventDispatchTask* pDispatcher);
  /**
   * Sets the same dispatch task for all channels.
   * @param pDispatcher
   */
  void setReceiveDispatchTask(MFNetEventDispatchTask* pDispatcher);
  /**
   * Sets the task at channel index.
   * @param pTask - task for dispatching data from channel channelIndex.
   * @param channelIndex - index to tasks vector and channel of network connection.
   * @return true if task was successfull set. False if MFEnetUser wasn't set up with
   * enough channels.
   */
  bool setReceiveDispatchTask(uint8_t channel,MFNetEventDispatchTask* pDispatcher);

  MFNetEventDispatchTask* getDefaultConnectTask(){return mp_defConnectTask;};
  MFNetEventDispatchTask* getDefaultDisconnectTask(){return mp_defDisconnectTask;};
  MFNetEventDispatchTask* getDefaultReceiveTask(){return mp_defReceiveTask;};
  MFNetEventDispatchTask* getConnectTask(){return mp_connectTask;};
  MFNetEventDispatchTask* getDisconnectTask(){return mp_disconnectTask;};
  std::vector<MFNetEventDispatchTask*>* getVecChannelReceiveTasks(){
    return mp_vecChannelReceiveTasks;
  };

  /**
   * Sets the count of events which is used for event buffer creation.
   * @param count
   */
  void setMaxEventCount(uint32_t count){m_maxEventCount=count;};

  /**
   * Sets the event return buffer for all tasks to the given sourceBuffer
   * @param pSourceBuffer - buffer which all events are taken from. The tasks
   * will return the event after all tasks are done with it.
   */
  void setEventReturnBuffer(S_MF_EventBuffer* pSourceBuffer){
    if(mp_defConnectTask!=nullptr)
      mp_defConnectTask->setEventReturnBuffer(pSourceBuffer);
    if(mp_defDisconnectTask!=nullptr)
      mp_defDisconnectTask->setEventReturnBuffer(pSourceBuffer);
    if(mp_defReceiveTask!=nullptr)
      mp_defReceiveTask->setEventReturnBuffer(pSourceBuffer);
    if(mp_connectTask!=nullptr)
      mp_connectTask->setEventReturnBuffer(pSourceBuffer);
    if(mp_disconnectTask!=nullptr)
      mp_disconnectTask->setEventReturnBuffer(pSourceBuffer);
    for(MFNetEventDispatchTask* pTask:*mp_vecChannelReceiveTasks){
      if(pTask!=nullptr)
        pTask->setEventReturnBuffer(pSourceBuffer);
    }
  }
  //TODO enable/disable dispatching and implement it in poll function.
};

#endif /* MFENETUSER_H_ */
