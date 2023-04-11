/*
 * MFNetClient.cpp
 *
 *  Created on: 02.03.2020
 *      Author: michl
 */

#include "../MFNetClientClasses/MFNetClient.h"

#include <MFObjects/MFObject.h>
#include <MFTime/MFTickCounter.h>

#include "../MFEnetHelper.h"

MFNetClient::MFNetClient(
    uint8_t channelCount,
    bool useInternalThread):
    MFEnetUser(useInternalThread){
  m_maxEventCount=500;
  m_enetSetup.m_clientCount=1;
  m_enetSetup.m_channelCount=channelCount;
  m_enetSetup.m_inBandwidth=0;
  m_enetSetup.m_outBandwidth=0;
  mp_clientEvent=new ENetEvent();
  mp_destinationAddress=new ENetAddress();
  mp_destinationAddress->port=42421;
  mp_unusedEvents=new S_MF_EventBuffer();
  mp_pollTask=new MFClientPollTask(this);
  mp_execThread=this;
  m_serverIndex=0xFFFFFFFF;
}

MFNetClient::MFNetClient(ENetHost* host,bool isServerClient):
	            MFEnetUser(!isServerClient){
  if(isServerClient)m_maxEventCount=1;//No events needed, the servers events are used!
  else m_maxEventCount=500;
  m_enetSetup.m_clientCount=1;
  m_enetSetup.m_channelCount=4;
  m_enetSetup.m_inBandwidth=0;
  m_enetSetup.m_outBandwidth=0;
  mp_clientEvent=new ENetEvent();//Not needed?
  mp_enetLocalHost=host;
  m_isServerClient=isServerClient;
  mp_unusedEvents=new S_MF_EventBuffer();
  mp_destinationAddress=new ENetAddress();
  mp_pollTask=new MFClientPollTask(this);
  mp_execThread=this;
  m_serverIndex=0xFFFFFFFF;
}

MFNetClient::~MFNetClient() {
  stop();
  mp_unusedEvents->destroy();
  delete mp_unusedEvents;
  delete mp_pollTask;
}

MFNetClient* MFNetClient::getClient(S_MF_NetworkEvent* pNetworkEvent){
  MFNetClient* pC=static_cast<MFNetClient*>(pNetworkEvent->pEvent->peer->data);
  return pC;
}

bool MFNetClient::initInstance(const MFEnetUser *enetUser){
  m_isDisconnecting=false;
  m_isConnected=false;
  if(m_isServerClient){
    m_isConnected=true;
    printInfo("MFNetClient::initEnetInstance - is a server client!");
    return true;
  }
  sm_enetLock.lock();
  mp_enetLocalHost=enet_host_create(
      NULL,/*its a client*/
      m_enetSetup.m_clientCount,
      m_enetSetup.m_channelCount,
      m_enetSetup.m_inBandwidth,
      m_enetSetup.m_outBandwidth);
  sm_enetLock.unlock();
  if(mp_enetLocalHost==NULL){
    printErr("MFNetClient::initEnetInstance - enet failed to "
        "create client host!");
    return false;
  }
  return true;
}

bool MFNetClient::exitInstance(const MFEnetUser *enetUser){
  stopPollThread();
  disconnect(500);
  sm_enetLock.lock();
  enet_host_destroy(mp_enetLocalHost);
  sm_enetLock.unlock();
  mp_unusedEvents->destroy();
  return true;
}

bool MFNetClient::ckeckPollConditions(){
  if(!isInitialized()){
    //TODO skip checks in release, if much polling is done they
    //will slow down a little bit
    printErr("MFNetClient::pollInput() - not initialized!");
    return false;
  }
  if(!m_isConnected || m_isDisconnecting){
    printErr("MFNetClient::pollInput() - !m_isConnected "
        "|| m_isDisconnecting!");
    return false;
  }
  if(m_isServerClient){//dispatching is done by the server object
    printWarning("MFNetClient::pollInput - client is sub part of a"
        "server, no input poll necessary!");
    return false;
  }
  return true;
}

bool MFNetClient::dispatchReceiveEvent(S_MF_NetworkEvent* pE){
  MFNetEventDispatchTask* pTask=nullptr;
  if(getVecChannelReceiveTasks()!=nullptr &&
      m_enetSetup.m_channelCount>=pE->pEvent->channelID){
    pTask=getVecChannelReceiveTasks()->at(pE->pEvent->channelID);
    if(pTask==nullptr){
      MFObject::printWarning("MFNetClient::dispatchReceiveEvent  - data dispatch task "
          "for channel "+std::to_string(pE->pEvent->channelID)+"==nullptr!");
      pTask=mp_defReceiveTask;
    }
  }else{
    printWarning("MFNetClient::dispatchReceiveEvent() - "
        "dispatch missmatch =P, using default dispatcher!");
    pTask=mp_defReceiveTask;
  }

  pTask->setEventReturnBuffer(mp_unusedEvents);//TODO
  pTask->addInputEvent(pE);

  return true;
}

bool MFNetClient::dispatchConnectEvent(S_MF_NetworkEvent* pE){
  if(m_isServerClient){//TODO test reuse of this object
    /*Server received a connect event and informs its client object about it.*/
    mp_destinationPeer=pE->pEvent->peer;
    mp_destinationAddress=&mp_destinationPeer->address;
    if(mp_connectTask!=nullptr){
      mp_connectTask->setEventReturnBuffer(mp_unusedEvents);//TODO
      mp_connectTask->addInputEvent(pE);
    }else{
      mp_defConnectTask->setEventReturnBuffer(mp_unusedEvents);//TODO
      mp_defConnectTask->addInputEvent(pE);
    }
  }else{
    printErr("MFNetClient::dispatchConnectEvent - if client is not "
        "created by a server, the client does not provide dispatching "
        "a connect event!");
    printInformations();
    return false;
  }
  printInformations();
  return true;
}

bool MFNetClient::dispatchDisconnectEvent(S_MF_NetworkEvent* pE){
  if(mp_disconnectTask!=nullptr){
    mp_disconnectTask->setEventReturnBuffer(mp_unusedEvents);//TODO
    mp_disconnectTask->addInputEvent(pE);
  }else{
    mp_defDisconnectTask->setEventReturnBuffer(mp_unusedEvents);//TODO
    mp_defDisconnectTask->addInputEvent(pE);
  }
  if(m_isDisconnecting && m_isServerClient && mp_destinationPeer!=nullptr){
    sm_enetLock.lock();
    enet_peer_reset (mp_destinationPeer);
    sm_enetLock.unlock();

    m_isConnected=false;
    m_isDisconnecting=false;
  }
  return true;
}

bool MFNetClient::connect(const std::string &address, uint16_t port,uint32_t timeout){
  if(!isInitialized()){
    if(!initEnetInstance()){
      printErr("MFNetClient::connect - failed to init enet!");
      return false;
    }
  }
  if(mp_enetLocalHost==nullptr){
    printErr("MFNetClient::connect - mp_enetLocalHost==nullptr!");
    return false;
  }
  if(m_isServerClient){
    printWarning("MFNetClient::connect - was created by a server, "
        "connect() call is invalid!");
    return m_isConnected;
  }
  if(m_isConnected){
    printWarning("MFNetClient::connect - established connection will be "
        "disconnected!");
    printInformations();
    disconnect(1000);
  }
  mp_destinationAddress->port=port;
  sm_enetLock.lock();
  enet_address_set_host(mp_destinationAddress,address.data());
  sm_enetLock.unlock();

  m_isConnected=false;
  sm_enetLock.lock();
  mp_destinationPeer=enet_host_connect(
      mp_enetLocalHost,
      mp_destinationAddress,
      m_enetSetup.m_channelCount,
      0);
  sm_enetLock.unlock();
  if(mp_destinationPeer==nullptr){
    printErr("MFNetClient::connect - failed to connect to"+
        address+":"+std::to_string(port));
    return false;
  }
  int ret=1;
  int64_t startPt=MFTickCounter::current();
  while(MFTickCounter::millisSince(startPt)<timeout){
    sm_enetLock.lock();
    ret=enet_host_service(mp_enetLocalHost,mp_clientEvent,10);
    sm_enetLock.unlock();
    if(ret>0){
      if(mp_clientEvent->type==ENET_EVENT_TYPE_CONNECT){
        printInfo("MFNetClient::connect - successful!");
        printInformations();
        m_isConnected=true;
        break;
      }
      if(mp_clientEvent->type==ENET_EVENT_TYPE_DISCONNECT){
        printInfo("MFNetClient::connect - failed! enet_host_service "
            "ENET_EVENT_TYPE_DISCONNECT -> no connection could be made!");
        printInformations();
        sm_enetLock.lock();
        enet_peer_reset(mp_destinationPeer);
        sm_enetLock.unlock();
        return false;
      }
    }
  }
  if(m_isConnected==false){
    printInfo("MFNetClient::connect - failed! connect request timed out!");
    sm_enetLock.lock();
    enet_peer_reset(mp_destinationPeer);
    sm_enetLock.unlock();
    return false;
  }
  printInfo("MFNetClient::connect - starting poll thread!");
  startPollThread(mp_pollTask);
  return m_isConnected;
}

bool MFNetClient::disconnect(uint32_t timeout){
  printInfo("MFNetClient::disconnect - disconnecting client!");
  if(!m_isConnected){
    return true;
  }
  m_isDisconnecting=true;
  //TODO test with server on other thread!hostRequests
  sm_enetLock.lock();
  enet_peer_disconnect(mp_destinationPeer,0);
  sm_enetLock.unlock();
  if(m_isServerClient){
    printWarning("MFNetClient::disconnect - this is a server client,"
        " dispatching and host_service will be done by server object!");
    m_isDisconnecting=true;
    return true;//final disconnect will be done by server object
  }
  ENetEvent discEve;

  int64_t startPt=MFTickCounter::current();
  while(MFTickCounter::millisSince(startPt)<timeout){
    sm_enetLock.lock();
    int hostRequests=enet_host_service(mp_enetLocalHost,&discEve,10);
    sm_enetLock.unlock();
    if(hostRequests>0)
      switch(discEve.type) {
      case ENET_EVENT_TYPE_DISCONNECT:
        printInfo("MFNetClient::disconnect - disconnected!");
        m_isConnected=false;
        m_isDisconnecting=false;
        return true;
      default:
        sm_enetLock.lock();
        enet_packet_destroy(discEve.packet);
        sm_enetLock.unlock();
        break;
      }
  }
  printWarning("MFNetClient::disconnect - forcing peer reset "
      "for disconnection!");
  sm_enetLock.lock();
  enet_peer_reset (mp_destinationPeer);
  sm_enetLock.unlock();
  m_isConnected=false;
  m_isDisconnecting=false;
  stopPollThread();
  return true;
}

bool MFNetClient::checkConnection(){
  printErr("MFNetClient::checkConnection() - Not implemented!");
  return false;
}

bool MFNetClient::sendData(
    uint8_t* data,
    uint32_t dataSize,
    uint8_t channel,
    bool flush,
    uint16_t *pkgCounter,
    bool useInternalDataDeletion,
    ENetPacketFreeCallback freeCallback){
  if(!isInitialized()){
    printErr("MFServerClientInstance::sendData - failed,"
        " enet not initialized!");
    return false;
  }

  if(data==nullptr || dataSize==0){
    printErr("MFServerClientInstance::sendData - invalid data!"
        " data==nullptr || dataSize==0");
    return false;
  }

  if(pkgCounter!=nullptr)
    (*pkgCounter)++;

  ENetPacket * packet = enet_packet_create (
      data,
      dataSize,
      ENET_PACKET_FLAG_RELIABLE);//TODO use packet buffer with pre allocation!

  if(useInternalDataDeletion)
    packet->freeCallback=freeCallbackFunction;
  else
    packet->freeCallback=freeCallback;
  sm_enetLock.lock();
  int32_t result=enet_peer_send(mp_destinationPeer,channel,packet);
  sm_enetLock.unlock();
  if(result!=0){
    printErr("MFNetClient::sendData - send result/channel :"+
        std::to_string(result)+"/"+std::to_string(channel));
  }
  if(flush){
    sm_enetLock.lock();
    enet_host_flush(mp_enetLocalHost);
    sm_enetLock.unlock();
  }
  return true;
}

void MFNetClient::printInformations(){
  printInfo("MFNetClient - Client informations:\n"
      "isServerClient - "+std::to_string(m_isServerClient)+"\n"+
      MFEnetHelper::getInformations(mp_enetLocalHost,mp_destinationPeer));
}
