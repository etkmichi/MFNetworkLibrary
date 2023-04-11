/*
 * MFNetworkLibraryHelper.h
 *
 *  Created on: 02.04.2020
 *      Author: michl
 */

#ifndef MFNETWORKLIBRARYHELPER_H_
#define MFNETWORKLIBRARYHELPER_H_
#include "MFEnetStructs.h"
#include "MFNetClientClasses/MFNetClient.h"
class MFNetworkLibraryHelper {
public:
	/**
	 *
	 * @param pNetworkEvent
	 * @return the client which is connected to the given network event.
	 */
	static MFNetClient* getClient(S_MF_NetworkEvent* pNetworkEvent){
		return MFNetClient::getClient(pNetworkEvent);
	}
};



#endif /* MFNETWORKLIBRARYHELPER_H_ */
