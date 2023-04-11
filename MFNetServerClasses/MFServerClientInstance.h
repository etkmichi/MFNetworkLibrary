/*
 * MFServerClientInstance.h
 *
 *  Created on: 02.03.2020
 *      Author: michl
 */

#ifndef MFSERVERCLIENTINSTANCE_H_
#define MFSERVERCLIENTINSTANCE_H_

#include <MFObjects/MFObject.h>
#include <MFData/MFDataObject.h>
#include <MFThreadSystem/MFTaskThread.h>
#include <MFPrinters/MFPrintSetup.h>

#include "../MFEnetStructs.h"
#include "../MFNetworkInterfaces/MFINetPollInput.h"
#include "../MFENetManagement/MFEnetUser.h"
#include "../MFNetClientClasses/MFNetClient.h"
#include "../MFNetServerClasses/MFServerTasks/MFServerPollTask.h"
#include "../MFNetworkTasks/MFNetEventDispatchTask.h"

class MFServerClientInstance :
		public MFEnetUser{
private:
	std::mutex
		lockVecUnusedEvents,
		lockVecInputEvents,
		lockVecOutputEvents;

	uint32_t
		m_currentClientIndex=0;

	uint8_t
		m_broadcastChannel=0;

	std::vector<MFNetClient*>
		*mp_vecConnectedClients,
		*mp_vecClients;
protected:
	virtual bool initInstance(const MFEnetUser *enetUser);
	virtual bool exitInstance(const MFEnetUser *enetUser);

public:
	MFServerClientInstance(uint8_t channelCount=8);
	MFServerClientInstance(S_MF_ENetHostClientSetup setup);

	virtual ~MFServerClientInstance();

	bool checkPollConditions(){return true;};
	bool broadcastData(
			uint8_t* data,
			uint32_t dataSize,
			bool sendImmediately=true);
	bool broadcastData(
			MFDataObject* pDataObject,
			bool sendImmediately=true);
	void setServerSetup(S_MF_ENetHostClientSetup serverSetup){m_enetSetup=serverSetup;};

	std::vector<MFNetClient*>* getVecConnectedClients(){return mp_vecConnectedClients;};

	/**
	 *
	 * @param pEvent
	 * @return true if pEvent was enqueued for dispatching, false if no dispatch task
	 * is available.
	 */
	bool dispatchReceiveEvent(S_MF_NetworkEvent* pEvent);
	bool dispatchConnectEvent(S_MF_NetworkEvent* pEvent);
	bool dispatchDisconnectEvent(S_MF_NetworkEvent* pEvent);
};

#endif /* MFSERVERCLIENTINSTANCE_H_ */
