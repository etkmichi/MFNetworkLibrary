/*
 * MFDefaultReceiveTask.cpp
 *
 *  Created on: 04.03.2020
 *      Author: michl
 */

#include "MFDefaultReceiveTask.h"

MFDefaultReceiveTask::MFDefaultReceiveTask() {
}

MFDefaultReceiveTask::~MFDefaultReceiveTask() {
}

bool MFDefaultReceiveTask::dispatchEvent(S_MF_NetworkEvent* pNE){
	std::string data="";
			data+=std::string((const char*)(pNE->pEvent->packet->data));
	printInfo("MFDefaultReceiveTask - received data:\n"+data);
	return true;
};
