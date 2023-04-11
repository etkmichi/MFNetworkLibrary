/*
 * MFNetDataDispatchTask.h
 *
 *  Created on: 03.03.2020
 *      Author: michl
 */

#ifndef MFNETWORKTASKS_MFNETEVENTDISPATCHTASK_H_
#define MFNETWORKTASKS_MFNETEVENTDISPATCHTASK_H_

#include <vector>
#include <mutex>
#include <MFTasks/MFAbstractTask.h>
#include <MFObjects/MFObject.h>
#include "../MF0InterfacesNetwork/MFINetChannelSink.h"
#include "../MFEnetStructs.h"
/**
 * This is a parant class which provides functionality for dispatching received data
 * from enet connections. This class provides thread safe processing of network
 * events (The processing of an event will not be done parallel!). The
 * dispatchEvent can be overwritten in a subclass to specify own data processing.
 * Default behavior of dispatchEvent will use added MFINetChannelSink's to redirect
 * received data.
 * TODO test multiple processing tasks on one event.
 */
class MFNetEventDispatchTask: public MFAbstractTask {
private:
	//TODO use EventBuffer as destination (not source)
	//->no resizing of vector...
	S_MF_EventBuffer
		*mp_dispatchableEvents=nullptr;
	S_MF_EventBuffer
		*mp_returnBuffer=nullptr;
	std::vector<MFINetChannelSink*>
		*mp_vecInternalChannelSinks,
		*mp_vecChannelSinks;
	std::mutex
		lockVecChannelSinks;
public:
	virtual bool undoWork(){return true;};
	/**
	 * The implementation of dispatchEvent(...) must process the data given by pNE.
	 * Default implementation will use the channel on which the pkg was received
	 * as index into a vector of MFINetChannelSink's.
	 * @param pNE
	 * @return true if processing was successfull.
	 */
	virtual bool dispatchEvent(S_MF_NetworkEvent* pNE);

public:
	MFNetEventDispatchTask();
	virtual ~MFNetEventDispatchTask();

	void addChannelSink(MFINetChannelSink* pSink){
		mp_vecChannelSinks->push_back(pSink);
	}

	std::vector<MFINetChannelSink*>* getChannelSinks(){return mp_vecChannelSinks;};

	void setChannelSink(uint8_t index,MFINetChannelSink* pSink){
		lockVecChannelSinks.lock();
		if(mp_vecChannelSinks->size()>index){
			mp_vecChannelSinks->data()[index]=pSink;
		}else{
			MFObject::printErr("MFNetEventDispatchTask::setChannelSink - "
					"index out of bounds, use "
					"addChannelSink function to increase size of vec.!");
		}
		lockVecChannelSinks.unlock();
	}

	void setVecChannelSinks(std::vector<MFINetChannelSink*>* pVecSinks){
		lockVecChannelSinks.lock();
		mp_vecChannelSinks=pVecSinks;
		lockVecChannelSinks.unlock();
	}

	void addInputEvent(S_MF_NetworkEvent* inputEvent);

	void setEventReturnBuffer(S_MF_EventBuffer* returnBuffer){
		mp_returnBuffer=returnBuffer;
	};

	bool doWork();
};

#endif /* MFNETWORKTASKS_MFNETEVENTDISPATCHTASK_H_ */
