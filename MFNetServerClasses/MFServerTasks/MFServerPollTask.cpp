/*
 * MFServerPollTask.cpp
 *
 *  Created on: 02.03.2020
 *      Author: michl
 */

#include "../../MFNetServerClasses/MFServerTasks/MFServerPollTask.h"

MFServerPollTask::MFServerPollTask(MFINetPollInput* pNetServer) {
	mp_netServerInterface=pNetServer;
}

MFServerPollTask::~MFServerPollTask() {
}

