/*
 * MFClientPollTask.h
 *
 *  Created on: 09.03.2020
 *      Author: michl
 */

#ifndef MFNETCLIENTCLASSES_MFCLIENTTASKS_MFCLIENTPOLLTASK_H_
#define MFNETCLIENTCLASSES_MFCLIENTTASKS_MFCLIENTPOLLTASK_H_

#include <MFTasks/MFAbstractTask.h>

#include "../../MFNetworkInterfaces/MFINetPollInput.h"

class MFClientPollTask: public MFAbstractTask {
private:
	MFINetPollInput
		*mp_netPollInterface=nullptr;
	bool
		m_pollingFailure=false;
public:
	MFClientPollTask(MFINetPollInput* pPollInterface);
	virtual ~MFClientPollTask();
	virtual bool doWork(){
		return mp_netPollInterface->pollInput();
	}

	virtual bool undoWork(){return true;};
};

#endif /* MFNETCLIENTCLASSES_MFCLIENTTASKS_MFCLIENTPOLLTASK_H_ */
