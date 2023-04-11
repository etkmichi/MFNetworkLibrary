/*
 * MFDefaultReceiveTask.h
 *
 *  Created on: 04.03.2020
 *      Author: michl
 */

#ifndef MFNETWORKTASKS_MFDEFAULTTASKS_MFDEFAULTRECEIVETASK_H_
#define MFNETWORKTASKS_MFDEFAULTTASKS_MFDEFAULTRECEIVETASK_H_

#include "../../MFEnetHelper.h"
#include "../MFNetEventDispatchTask.h"

class MFDefaultReceiveTask: public MFNetEventDispatchTask {
public:
	MFDefaultReceiveTask();
	virtual ~MFDefaultReceiveTask();
	bool dispatchEvent(S_MF_NetworkEvent* pNE);
};

#endif /* MFNETWORKTASKS_MFDEFAULTTASKS_MFDEFAULTRECEIVETASK_H_ */
