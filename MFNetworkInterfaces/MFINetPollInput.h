/*
 * MFINetPollInput.h
 *
 *  Created on: 02.03.2020
 *      Author: michl
 */

#ifndef MFNETWORKINTERFACES_MFINETSERVER_H_
#define MFNETWORKINTERFACES_MFINETSERVER_H_
#include <MFThreadSystem/MFTaskThread.h>
#include <MFTasks/MFAbstractTask.h>
class MFINetPollInput {
protected:
	bool
		m_pollingFailure=false,
		m_isPollThreadStarted=false;
	MFTaskThread
		*mp_execThread=nullptr;
	MFAbstractTask
		*mp_pollTask=nullptr;
public:
	MFINetPollInput(MFTaskThread* pExecThread){
		mp_execThread=pExecThread;
	};//TODO default poll task?
	virtual ~MFINetPollInput(){};
	virtual bool pollInput(){return false;};

	void startPollThread(MFAbstractTask *pPollTask){
		if(pPollTask!=nullptr){
			mp_pollTask=pPollTask;
			mp_execThread->addTask(pPollTask);
		}else{
			if(mp_pollTask!=nullptr)
				mp_execThread->addTask(mp_pollTask);
		}
		if(!mp_execThread->isStarted()){
			mp_execThread->startDetached();
		}
		m_isPollThreadStarted=true;
	}
	/**
	 * If a valid task thread and poll task is set, this function starts polling
	 * on the given task thread with the given task.
	 */
	void startPolling(){
		startPollThread(mp_pollTask);
	}
	void stopPollThread(){
		m_isPollThreadStarted=false;
		mp_execThread->stop();
	}
	void setTaskThread(
			MFTaskThread* pThread,
			bool stopOldThread=true){
		if(pThread!=nullptr){
			if(stopOldThread && pThread!=mp_execThread)mp_execThread->stop();
			mp_execThread=pThread;
		}

	};
};

#endif /* MFNETWORKINTERFACES_MFINETSERVER_H_ */
