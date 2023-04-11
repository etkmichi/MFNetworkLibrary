/*
 * MFPollTask.cpp
 *
 *  Created on: 10.03.2020
 *      Author: michl
 */

#include "MFPollTask.h"

MFPollTask::MFPollTask(MFINetPollInput* pServerInterface) {
	mp_netServerInterface=pServerInterface;
}

MFPollTask::~MFPollTask() {
}

