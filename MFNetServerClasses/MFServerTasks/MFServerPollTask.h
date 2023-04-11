/*
 * MFServerPollTask.h
 *
 *  Created on: 02.03.2020
 *      Author: michl
 */

#ifndef MFNETSERVERCLASSES_MFSERVERTASKS_MFSERVERPOLLTASK_H_
#define MFNETSERVERCLASSES_MFSERVERTASKS_MFSERVERPOLLTASK_H_

#include <MFTasks/MFAbstractTask.h>

#include "../../MFNetworkInterfaces/MFINetPollInput.h"

class MFServerPollTask: public MFAbstractTask {
private:
	MFINetPollInput
		*mp_netServerInterface=nullptr;
	bool
		m_pollingFailure=false;
public:
	MFServerPollTask(MFINetPollInput* pServerInterface);
	virtual ~MFServerPollTask();
	virtual bool doWork(){
		return mp_netServerInterface->pollInput();
	}

	virtual bool undoWork(){return true;};
};

#endif /* MFNETSERVERCLASSES_MFSERVERTASKS_MFSERVERPOLLTASK_H_ */
