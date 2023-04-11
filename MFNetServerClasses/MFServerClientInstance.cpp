/*
 * MFServerClientInstance.cpp
 *
 *  Created on: 02.03.2020
 *      Author: michl
 */

#include "../MFNetServerClasses/MFServerClientInstance.h"

MFServerClientInstance::MFServerClientInstance(
    uint8_t channelCount) {
  m_maxEventCount=100;
  m_enetSetup.mp_address=new ENetAddress();
  m_enetSetup.mp_address->host = ENET_HOST_ANY;//TODO local as default
  m_enetSetup.mp_address->port = 42421;
  m_enetSetup.m_clientCount=4;
  m_enetSetup.m_channelCount=channelCount;

  mp_vecConnectedClients=new std::vector<MFNetClient*>();

  mp_vecClients=new std::vector<MFNetClient*>();
}

MFServerClientInstance::MFServerClientInstance(S_MF_ENetHostClientSetup setup){
  printInfo("MFServerClientInstance::MFServerClientInstance()");
  m_maxEventCount=100;
  m_enetSetup=setup;

  mp_vecConnectedClients=new std::vector<MFNetClient*>();

  mp_vecClients=new std::vector<MFNetClient*>();
}

MFServerClientInstance::~MFServerClientInstance() {
}

bool MFServerClientInstance::initInstance(const MFEnetUser *enetUser){
  sm_enetLock.lock();
  mp_enetLocalHost=enet_host_create(
      m_enetSetup.mp_address,
      m_enetSetup.m_clientCount,
      m_enetSetup.m_channelCount,
      m_enetSetup.m_inBandwidth,
      m_enetSetup.m_outBandwidth);
  sm_enetLock.unlock();
  if(mp_enetLocalHost==NULL){
    printErr("MFServerClientInstance::initInstance() - "
        "failed to create the server!");
    return false;
  }
  mp_vecClients->resize(m_enetSetup.m_clientCount);
  for(uint32_t i=0;i<m_enetSetup.m_clientCount;i++){
    MFNetClient* pClient=new MFNetClient(mp_enetLocalHost, true);
    pClient->setEventReturnBuffer(mp_unusedEvents);
    mp_vecClients->data()[i]=pClient;
  }
  m_isInitialized=true;
  startPollThread(mp_pollTask);
  printInfo("MFServerClientInstance::initInstance - server instance initialization "
      "was successful!");
  return true;
}

bool MFServerClientInstance::exitInstance(const MFEnetUser *enetUser){
  if(!m_isInitialized){
    return true;
  }
  for(uint32_t i=0;i<mp_vecClients->size();i++){
    MFNetClient* pClient=mp_vecClients->at(i);
    pClient->disconnect(0);//TODO test what happens if peer == nullptr
    pClient->stopPollThread();
    pClient->stop();
    delete pClient;
  }
  mp_vecClients->clear();
  stopPollThread();
  sm_enetLock.lock();
  enet_host_destroy(mp_enetLocalHost);
  sm_enetLock.unlock();
  m_isInitialized=false;
  printInfo("MFServerClientInstance::exitInstance - server instance exit "
      "was successful!");
  return true;
}

bool MFServerClientInstance::broadcastData(
    uint8_t* data,
    uint32_t dataSize,
    bool flush){
  if(!m_isInitialized){
    printErr("MFServerClientInstance::broadcastData - failed,"
        " enet not initialized!");
    return false;
  }
  if(data==nullptr || dataSize==0){
    printErr("MFServerClientInstance::broadcastData - invalid data!"
        " data==nullptr || dataSize==0");
    return false;
  }
  ENetPacket * packet = enet_packet_create (
      data,
      dataSize,
      ENET_PACKET_FLAG_RELIABLE);
  //TODO use packet buffer with pre allocation!
  // take care, packets are deallocated by enet!
  sm_enetLock.lock();
  enet_host_broadcast(mp_enetLocalHost,m_broadcastChannel,packet);
  sm_enetLock.unlock();
  if(flush){
    sm_enetLock.lock();
    enet_host_flush(mp_enetLocalHost);
    sm_enetLock.unlock();
  }
  return true;
}

bool MFServerClientInstance::broadcastData(
    MFDataObject* pDataObject,
    bool sendImmediately){
  if(!pDataObject->isValid()){
    printErr("MFServerClientInstance::broadcastData - "
        "data buffer was invalidated!");
    return false;
  }
  return broadcastData(
      (uint8_t*)pDataObject->getData(),
      pDataObject->getDataSize(),
      sendImmediately);
}

bool MFServerClientInstance::dispatchReceiveEvent(S_MF_NetworkEvent* pE){
  MFNetEventDispatchTask* pTask=nullptr;
  //TODO server receive dispatch task for counting traffic...
  if(getVecChannelReceiveTasks()!=nullptr &&
      m_enetSetup.m_channelCount>=pE->pEvent->channelID){
    pTask=getVecChannelReceiveTasks()->at(pE->pEvent->channelID);
    if(pTask==nullptr){
      pTask=mp_defReceiveTask;
    }
  }else{
    pTask=mp_defReceiveTask;
  }
  if(pTask!=nullptr){
    pTask->setEventReturnBuffer(mp_unusedEvents);//TODO
    pTask->addInputEvent(pE);
  }else{
    printErr("MFServerClientInstance::dispatchReceiveEvent - "
        "no task for dispatching!");
    if(pE->pEvent->packet!=nullptr && pE->pEvent->packet->data !=nullptr){
      std::string data((char*)pE->pEvent->packet->data);
      printInfo("MFServerClientInstance::dispatchReceiveEvent data:"+
          data);
    }
    return false;
  }
  ENetPeer* pPeer=pE->pEvent->peer;
  MFNetClient* pClient=nullptr;
  if(pPeer!=nullptr && pPeer->data!=nullptr){
    pClient=(MFNetClient*)pPeer->data;
    pClient->dispatchReceiveEvent(pE);
    pClient->enqueueReceiveTasks();
  }
  return true;
}

bool MFServerClientInstance::dispatchConnectEvent(S_MF_NetworkEvent* pE){
  MFNetClient* pClient=nullptr;
  if(m_currentClientIndex>=mp_vecClients->size()){
    printWarning("MFServerClientInstance::dispatchConnectEvent - "
        "cant connect another client (max client count reached)!");
    printWarning("MFServerClientInstance::dispatchConnectEvent - "
        "no client allocated for this event, only server dispatching will be"
        "done!");
  }else{
    pClient=mp_vecClients->at(m_currentClientIndex);
    pClient->initEnetInstance();
    pClient->setServerIndex(mp_vecConnectedClients->size());
    pE->pEvent->peer->data=(void*)(pClient);
    mp_vecConnectedClients->push_back(pClient);
    pClient->setDestinationPeer(pE->pEvent->peer);
    pClient->dispatchConnectEvent(pE);
    pClient->enqueueConnectTask();
    m_currentClientIndex++;
  }
  if(mp_connectTask!=nullptr){
    mp_connectTask->setEventReturnBuffer(mp_unusedEvents);//TODO
    mp_connectTask->addInputEvent(pE);
  }else{
    mp_defConnectTask->setEventReturnBuffer(mp_unusedEvents);//TODO
    mp_defConnectTask->addInputEvent(pE);
  }
  return true;
}

bool MFServerClientInstance::dispatchDisconnectEvent(S_MF_NetworkEvent* pEvent){
  ENetPeer* pPeer=pEvent->pEvent->peer;
  MFNetClient* pClient=nullptr;
  if(pPeer!=nullptr && pPeer->data!=nullptr){
    pClient=(MFNetClient*)pPeer->data;
    pClient->dispatchDisconnectEvent(pEvent);
    pClient->enqueueDisconnectTask();
    pPeer->data=nullptr;
  }
  if(mp_disconnectTask!=nullptr){
    mp_disconnectTask->setEventReturnBuffer(mp_unusedEvents);//TODO
    mp_disconnectTask->addInputEvent(pEvent);
  }else{
    mp_defDisconnectTask->setEventReturnBuffer(mp_unusedEvents);//TODO
    mp_defDisconnectTask->addInputEvent(pEvent);
  }
  return true;
}

