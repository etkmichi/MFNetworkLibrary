/*
 * MFINetChannelSink.h
 *
 *  Created on: 10.07.2020
 *      Author: michl
 */

#ifndef MF0INTERFACESNETWORK_MFINETCHANNELSINK_H_
#define MF0INTERFACESNETWORK_MFINETCHANNELSINK_H_
#include <MFObjects/MFObject.h>
#include "../MFEnetStructs.h"
class MFINetChannelSink {

/*virtual functions of MFINetChannelSink*/
public:
	/**
	 * The implementation of this function must dispatch the received data.
	 * If function is called by MFNetEventDispatchTask class, pNE must not be used any
	 * further after dispatchEvent(...) returned!
	 * @param pNE
	 * @return
	 */
	virtual bool dispatchEvent(S_MF_NetworkEvent* pNE){
		MFObject::printWarning("MFINetChannelSink::dispatchEvent - "
				"no impl.!");
		return true;
	}
public:
	MFINetChannelSink();
	virtual ~MFINetChannelSink();
};

#endif /* MFENGINEMODULES_MFNETWORKMODULES_MFINTERFACESNETWORK_MFINETCHANNELSINK_H_ */
