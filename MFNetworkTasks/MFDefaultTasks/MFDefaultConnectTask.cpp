/*
 * MFDefaultConnectTask.cpp
 *
 *  Created on: 04.03.2020
 *      Author: michl
 */

#include "MFDefaultConnectTask.h"

MFDefaultConnectTask::MFDefaultConnectTask() {

}

MFDefaultConnectTask::~MFDefaultConnectTask() {
}

bool MFDefaultConnectTask::dispatchEvent(S_MF_NetworkEvent* pNE){
	printInfo("MFDefaultConnectTask - Connection established:\n"+
			MFEnetHelper::getInformations(pNE->pHost, pNE->pEvent->peer));
	return true;
}
