/*
 * MFPollTask.h
 *
 *  Created on: 10.03.2020
 *      Author: michl
 */

#ifndef MFNETWORKINTERFACES_MFPOLLTASK_H_
#define MFNETWORKINTERFACES_MFPOLLTASK_H_

#include <MFTasks/MFAbstractTask.h>

#include "MFINetPollInput.h"

class MFPollTask: public MFAbstractTask {
private:
	MFINetPollInput
		*mp_netServerInterface=nullptr;
	bool
		m_pollingFailure=false;
public:
	MFPollTask(MFINetPollInput* pServerInterface);
	virtual ~MFPollTask();
	virtual bool doWork(){
		return mp_netServerInterface->pollInput();
	}

	virtual bool undoWork(){return true;};
};

#endif /* MFNETWORKINTERFACES_MFPOLLTASK_H_ */
