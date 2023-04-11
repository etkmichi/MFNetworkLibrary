/*
 * MFNetClient.h
 *
 *  Created on: 02.03.2020
 *      Author: michl
 */

#ifndef MFNETCLIENTCLASSES_MFNETCLIENT_H_
#define MFNETCLIENTCLASSES_MFNETCLIENT_H_

#include <enet/enet.h>
#include <MFThreadSystem/MFTaskThread.h>
#include "../MFEnetStructs.h"
#include "../MFENetManagement/MFEnetUser.h"
#include "../MFNetClientClasses/MFClientTasks/MFClientPollTask.h"
#include "../MFNetworkInterfaces/MFINetPollInput.h"
#include "../MFNetworkTasks/MFNetEventDispatchTask.h"
/**
 * Represents a client for a network connection. Client can be standalone or subpart
 * of a server (established connection to the server).
 * Dispatching of received datac an be done by:
 * 	- Implementing MFNetEventDispatchTask with specific dispatchEvent(...)
 * 	- Usage of default MFNetEventDispatchTask with specific MFINetChannelSink's
 * Data can be transmitted over different channels. For each channel a data dispatch
 * task MFNetEventDispatchTask* can be set. The default MFNetEventDispatchTask can
 * be used for all channels, if the dispatching shall be done by MFINetChannelSink
 * interfaces (The MFINetChannelSink's must be added to the MFNetEventDispatchTask).
 * TODO run static packet cleanup task which frees packages periodic f.e. every 100ms
 */
class MFNetClient :
    public MFEnetUser {

private:
  ENetPacketFreeCallback
  freeCallbackFunction=freeData;

  bool
  m_isServerClient=false,
  m_isDisconnecting=false,
  m_isPollThreadStarted=false,
  m_useExternalThread=false;

  uint32_t
  m_serverIndex;

  ENetEvent
  *mp_clientEvent;

  ENetPeer
  *mp_destinationPeer=nullptr;

  ENetAddress
  *mp_destinationAddress;

protected:
  virtual bool initInstance(const MFEnetUser *enetUser);
  virtual bool exitInstance(const MFEnetUser *enetUser);
public:
  static MFNetClient* getClient(S_MF_NetworkEvent* pNetworkEvent);
public:
  MFNetClient(
      uint8_t channelCount=8,
      bool useInternalThread=true
  );
  //TODO set server as task thread and stop this thread
  MFNetClient(ENetHost* host,bool isServerClient=true);
  virtual ~MFNetClient();

  void setDestinationPeer(ENetPeer* pDest){mp_destinationPeer=pDest;};
  void setLocalHost(ENetHost* host){mp_enetLocalHost=host;};
  void setIsServerClient(bool isServerClient){m_isServerClient=true;};
  void setServerIndex(uint32_t index){m_serverIndex=index;};
  void setChannelCount(uint8_t count){m_enetSetup.m_channelCount=count;};

  /**
   *
   * @param pEvent
   * @return true if pEvent was enqueued for dispatching, false if no dispatch task
   * is available.
   */
  bool dispatchReceiveEvent(S_MF_NetworkEvent* pEvent);
  bool dispatchConnectEvent(S_MF_NetworkEvent* pEvent);
  bool dispatchDisconnectEvent(S_MF_NetworkEvent* pEvent);
  /*TODO Add isConnecting bool*/
  bool connect(const std::string &address, uint16_t port,uint32_t timeout);
  bool isConnected(){return m_isConnected;};
  bool disconnect(uint32_t timeout);

  /**
   *
   * @param data - data for the package which will be sent
   * @param dataSize - size of the data in bytes
   * @param channel - channel to use for sending
   * @param flush - if true, data will be writen to sending driver immediately?
   * @param useInternalDataDeletion - the free callback of this object will be used
   * @param freeCallback - if useInter...== false, the freeCallback will be called
   * after? deletion.TODO
   * @param pkgCounter - if not nullptr, the counter will be incremented, before the
   * package is sent. pkgCounter can be part of the package data.
   * @return
   */
  bool sendData(
      uint8_t* data,
      uint32_t dataSize,
      uint8_t channel,
      bool flush,
      uint16_t *pPkgCounter=nullptr,
      bool useInternalDataDeletion=true,
      ENetPacketFreeCallback freeCallback=nullptr);
  bool ckeckPollConditions();
  bool checkConnection();

  void printInformations();

};

#endif /* MFNETCLIENTCLASSES_MFNETCLIENT_H_ */
