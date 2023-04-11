/*
 * MFNetDataDispatchTask.cpp
 *
 *  Created on: 03.03.2020
 *      Author: michl
 */

#include "MFNetEventDispatchTask.h"

MFNetEventDispatchTask::MFNetEventDispatchTask() {
	mp_dispatchableEvents=new S_MF_EventBuffer();
	/*init the buffer as a destination buffer -> buffers from elsewhere will be added*/
	mp_dispatchableEvents->init(500, false);
	mp_vecInternalChannelSinks=new std::vector<MFINetChannelSink*>();
	mp_vecChannelSinks=mp_vecInternalChannelSinks;
	setTaskName("MFNetEventDispatchTask");
}

MFNetEventDispatchTask::~MFNetEventDispatchTask() {
	delete mp_vecInternalChannelSinks;
}

bool MFNetEventDispatchTask::dispatchEvent(S_MF_NetworkEvent* pNE){
  if(mp_vecChannelSinks->size()>pNE->pEvent->channelID){
    return mp_vecChannelSinks->at(pNE->pEvent->channelID)->dispatchEvent(pNE);
  }
  MFObject::printWarning("MFNetEventDispatchTask::dispatchEvent - "
      "no dispatch task for used channel!\n"
      "channel sink count: "+std::to_string(mp_vecChannelSinks->size())+
      "\nchannel index: "+std::to_string(pNE->pEvent->channelID));
  return true;
};

bool MFNetEventDispatchTask::doWork(){
	bool ret=true;
	mp_dispatchableEvents->lockTake();
	while(mp_dispatchableEvents->getRemainingEventCount()>0){
		S_MF_NetworkEvent* pNE=mp_dispatchableEvents->takeEvent(true);
		mp_dispatchableEvents->unlockTake();
		if(pNE==nullptr)
			continue;
		if(!pNE->tryLockAddDispatchTask()){
			printInfo("MFNetEventDispatchTask::doWork() - "
					"debug info: task cant be dispatched yet, other task "
					"are being added for dispatching!");
			continue;
		}
		if(!pNE->tryLock()){
			pNE->unlockAddDispatchTask();
			printInfo("MFNetEventDispatchTask::doWork() - "
					"debug info: task is locked somewhere else! Processing will be"
					"done in next iteration");
			continue;
		}
		if(pNE->isDispatched){
			printWarning("MFNetEventDispatchTask::doWork - task already"
					"dispatched and returned, can't dispatch within this task! ");
		}else{
		  pNE->isDispatched=dispatchEvent(pNE);
			ret=pNE->isDispatched;
			if(!ret){
				printErr("MFNetEventDispatchTask::doWork - failed to "
						"dispach event!");
			}
		}
		pNE->unlock();
		if(mp_returnBuffer!=nullptr && pNE->removeDispatchingTask()){
			pNE->unlockAddDispatchTask();//TODO check if its ok to unlock at this pnt
			mp_returnBuffer->returnEvent(pNE);
		}else{
			printWarning("MFNetEventDispatchTask::doWork - no return buffer "
					"for recycling the network event!");
		}
		mp_dispatchableEvents->lockTake();
	}
	mp_dispatchableEvents->unlockTake();
	return ret;
}

void MFNetEventDispatchTask::addInputEvent(S_MF_NetworkEvent* inputEvent){
	inputEvent->addDispatchTask(this);
	if(inputEvent->pEvent->packet!=nullptr)
		inputEvent->pEvent->packet->freeCallback=freeData;
	/*enqueue the event to mp_dispatchableEvents*/
	mp_dispatchableEvents->returnEvent(inputEvent);
}
