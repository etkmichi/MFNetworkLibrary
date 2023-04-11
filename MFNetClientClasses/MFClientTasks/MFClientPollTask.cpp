/*
 * MFClientPollTask.cpp
 *
 *  Created on: 09.03.2020
 *      Author: michl
 */

#include "../../MFNetClientClasses/MFClientTasks/MFClientPollTask.h"

MFClientPollTask::MFClientPollTask(MFINetPollInput* pNetPoller) {
	mp_netPollInterface=pNetPoller;
}

MFClientPollTask::~MFClientPollTask() {
	// TODO Auto-generated destructor stub
}

