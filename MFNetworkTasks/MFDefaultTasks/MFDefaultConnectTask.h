/*
 * MFDefaultConnectTask.h
 *
 *  Created on: 04.03.2020
 *      Author: michl
 */

#ifndef MFNETWORKTASKS_MFDEFAULTTASKS_MFDEFAULTCONNECTTASK_H_
#define MFNETWORKTASKS_MFDEFAULTTASKS_MFDEFAULTCONNECTTASK_H_

#include "../../MFEnetHelper.h"
#include "../MFNetEventDispatchTask.h"
class MFDefaultConnectTask: public MFNetEventDispatchTask {
public:
	MFDefaultConnectTask();
	virtual ~MFDefaultConnectTask();
	virtual bool dispatchEvent(S_MF_NetworkEvent* pNE);
};

#endif /* MFNETWORKTASKS_MFDEFAULTTASKS_MFDEFAULTCONNECTTASK_H_ */
