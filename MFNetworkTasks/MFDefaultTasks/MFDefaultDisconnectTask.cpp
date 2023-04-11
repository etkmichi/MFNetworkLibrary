/*
 * MFDefaultDisconnectTask.cpp
 *
 *  Created on: 04.03.2020
 *      Author: michl
 */

#include "MFDefaultDisconnectTask.h"

MFDefaultDisconnectTask::MFDefaultDisconnectTask() {
}

MFDefaultDisconnectTask::~MFDefaultDisconnectTask() {
}

bool MFDefaultDisconnectTask::dispatchEvent(S_MF_NetworkEvent* pNE){
	printInfo("MFDefaultDisconnectTask - disconnecting peer:");
	MFEnetHelper::printInformations(pNE);
	return true;
};
