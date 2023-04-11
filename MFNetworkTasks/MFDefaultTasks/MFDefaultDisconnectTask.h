/*
 * MFDefaultDisconnectTask.h
 *
 *  Created on: 04.03.2020
 *      Author: michl
 */

#ifndef MFNETWORKTASKS_MFDEFAULTTASKS_MFDEFAULTDISCONNECTTASK_H_
#define MFNETWORKTASKS_MFDEFAULTTASKS_MFDEFAULTDISCONNECTTASK_H_

#include "../../MFEnetHelper.h"
#include "../MFNetEventDispatchTask.h"
class MFDefaultDisconnectTask : public MFNetEventDispatchTask {
public:
	MFDefaultDisconnectTask();
	virtual ~MFDefaultDisconnectTask();
	bool dispatchEvent(S_MF_NetworkEvent* pNE);
};

#endif /* MFNETWORKTASKS_MFDEFAULTTASKS_MFDEFAULTDISCONNECTTASK_H_ */
