/*
 * MFNetDataDispatchTask.h
 *
 *  Created on: 03.03.2020
 *      Author: michl
 */

#ifndef MFENETSTRUCTS_H_
#define MFENETSTRUCTS_H_

#include <mutex>
#include <vector>
#include <enet/enet.h>
#include <MFTasks/MFAbstractTask.h>
static void freeData(ENetPacket* packet){
//	MFObject::printWarning("MFENETSTRUCTS_H_ - static free callback");
//	free(packet->data);
//	delete packet;
}
struct S_MF_NetworkEvent{
public://add state like described in zim Dev::MFBasics::MFState-class
	ENetPacketFreeCallback pkgFreeCallback=freeData;
	std::mutex
		mLockEvent,
		mLockAddDispatchTask;
	ENetEvent* pEvent;
	ENetHost* pHost;
	MFAbstractTask* pDispatchTask=nullptr;
	std::vector<MFAbstractTask*> vecDispatchingTasks;
	uint32_t dispatchingTaskCount=0;
	bool isUsed=false;
	bool isDispatched=false;
	void init(){lock();pEvent=new ENetEvent();unlock();};
  void reinit(){
    lock();
    if(pEvent!=nullptr){
      delete pEvent;
    }
    vecDispatchingTasks.clear();
    pEvent=new ENetEvent();
    unlock();
  };

	void destroy(){lock();delete pEvent;unlock();};
	bool tryLock(){return mLockEvent.try_lock();};
	/*
	 * before dispatching, all tasks should be added to this event! Lock the
	 * this event with lockAddDispatchTask to prevent dispatching before all
	 * tasks are added! After adding all tasks with addDispatchTask, call
	 * unlockAddDispatchTask() to start dispatching.
	 */
	bool tryLockAddDispatchTask(){return mLockAddDispatchTask.try_lock();};
	void lockAddDispatchTask(){mLockAddDispatchTask.lock();};
	void unlockAddDispatchTask(){mLockAddDispatchTask.unlock();};
	void lock(){mLockEvent.lock();};
	void unlock(){mLockEvent.unlock();};
	/**
	 * Adds a dispatching task, if this event wasn't dispatched till now. Increments
	 * a dispatching tasks counter which is used for checking if a task still wants
	 * to dispatch this event.
	 * @param pTask
	 * @return true if this task wasn't dispatched yet.
	 */
	bool addDispatchTask(MFAbstractTask* pTask){
		bool ret;
		if(!isDispatched){
			vecDispatchingTasks.push_back(pTask);
			dispatchingTaskCount++;
			ret=true;
		}else{
			ret=false;
		}
		return ret;
	}
	/**
	 * decrements the counter for added dispatch tasks. if counter==0 then all
	 * dispatching tasks should have called this function.
	 * @return true if all dispatching tasks called this function or this function
	 * was called as often as a task was added.
	 */
	bool removeDispatchingTask(){
		dispatchingTaskCount--;
		if(dispatchingTaskCount==0){
			vecDispatchingTasks.clear();
			isDispatched=true;
			return true;
		}
		return false;
	}
};
struct S_MF_NetworkPacket{
	ENetPacket* pPacket;
	std::mutex mLockPacket;
	bool isUsed=false;
};
struct S_MF_ENetHostClientSetup{
	ENetAddress
		*mp_address=NULL;
	uint32_t
		m_clientCount=32,
		m_channelCount=4,
		m_inBandwidth=0,
		m_outBandwidth=0;
};
struct S_MF_EventBuffer{
private:
	bool m_isSource=true;
	std::vector<S_MF_NetworkEvent*>
		*pVecEventBuffer=nullptr;
	std::mutex
		mLockBuffer,
		mLockTake;
	uint32_t
		m_currentIndex=0xFFFFFFFF,
		m_maxSize;
public:
	/**
	 *
	 * @param maxEventCount
	 * @param isSourceBuffer if true the buffer will create its entries, if false,
	 * the entries were created somewhere else.
	 * @return
	 */
	bool init(uint32_t maxEventCount,bool isSourceBuffer){
		lockBuffer();
		if(pVecEventBuffer!=nullptr){
			MFObject::printWarning("S_MF_EventBuffer::init - will resize buffer, "
					"buffer was initialized already!");
			unlockBuffer();
			return resize(maxEventCount);
		}
		m_isSource=isSourceBuffer;
		if(maxEventCount==0){
			MFObject::printErr("S_MF_EventBuffer::init - "
					"maxEventCount==0");
			unlockBuffer();
			return false;
		}
		pVecEventBuffer=new std::vector<S_MF_NetworkEvent*>();
		pVecEventBuffer->resize(maxEventCount);
		m_currentIndex=0xFFFFFFFF;
		for(uint32_t i=0;i<pVecEventBuffer->size();i++){
			S_MF_NetworkEvent* pEvent=nullptr;
			if(isSourceBuffer){
				pEvent=new S_MF_NetworkEvent();
				pEvent->init();
			}
			pVecEventBuffer->at(i)=pEvent;
		}
		if(isSourceBuffer)
			m_currentIndex=pVecEventBuffer->size()-1;

		m_maxSize=pVecEventBuffer->size();
		unlockBuffer();
		return true;
	}
	bool resize(uint32_t maxEventCount){
		lockBuffer();
		if(maxEventCount==0){
			MFObject::printErr("S_MF_EventBuffer::init - "
					"maxEventCount==0");
			unlockBuffer();
			return false;
		}
		if(m_currentIndex>=maxEventCount){
			m_currentIndex=maxEventCount-1;
		}
		if(m_isSource){
			//eC=eventCount -> delete allocated resources
			for(uint32_t eC=m_maxSize; eC>maxEventCount;eC--){
				pVecEventBuffer->at(eC)->destroy();
				delete pVecEventBuffer->at(eC);
			}
			pVecEventBuffer->resize(maxEventCount);
			//if current buffer < new size than push back events
			for(uint32_t eC=m_maxSize; eC<maxEventCount;eC++){
				S_MF_NetworkEvent* pEvent=new S_MF_NetworkEvent();
				pEvent->init();
				pVecEventBuffer->at(eC)=pEvent;
			}
		}
		m_maxSize=pVecEventBuffer->size();
		unlockBuffer();
		return true;
	}
	void destroy(){
		lockBuffer();
		if(m_isSource){
			uint32_t counter=0;
			for(S_MF_NetworkEvent* pEvent:*pVecEventBuffer){
				if(pEvent!=nullptr){
					pEvent->destroy();
					delete pEvent;
					pVecEventBuffer->at(counter)=nullptr;
				}
				counter++;
			}
			pVecEventBuffer->clear();
			delete pVecEventBuffer;
		}
		unlockBuffer();
	}
	void lockBuffer(){mLockBuffer.lock();}
	void unlockBuffer(){mLockBuffer.unlock();};
	void lockTake(){mLockTake.lock();}
	void unlockTake(){mLockTake.unlock();};

	/**
	 * Takes a event from the buffer and decrements the current index
	 * @param forceTake - true if lockTake() was called outside.
	 * @return
	 */
	S_MF_NetworkEvent* takeEvent(bool forceTake=false){
		S_MF_NetworkEvent* pEvent;
		if(!forceTake)lockTake();
		lockBuffer();
		if(pVecEventBuffer==nullptr){
			unlockBuffer();
			MFObject::printErr("S_MF_EventBuffer::takeEvent - failed, "
					"pVecEventBuffer==nullptr");
			return nullptr;
		}
		if((m_currentIndex)==0xFFFFFFFF){
			unlockBuffer();
			MFObject::printWarning("S_MF_NetworkEvent* takeEvent - "
					"no buffer available!");
			return nullptr;
		}
		pEvent=pVecEventBuffer->data()[m_currentIndex];
		if(m_isSource){//if its not a source buffer, its a buffer for processing
			pEvent->isUsed=true;
			pEvent->isDispatched=false;
			pEvent->unlock();
		}
		//TODO set buffer at current index to dummy, for data invalidation?
		m_currentIndex--;
		unlockBuffer();
		if(!forceTake)unlockTake();
		return pEvent;
	};

	void returnEvent(S_MF_NetworkEvent* pEvent){
		lockBuffer();
		if(pEvent==nullptr){
			MFObject::printErr("S_MF_EventBuffer::returnEvent - failed, "
					"pEvent==nullptr!");
			unlockBuffer();
			return;
		}
		if(pVecEventBuffer==nullptr){
			MFObject::printErr("S_MF_EventBuffer::returnEvent - failed, "
					"pVecEventBuffer!=nullptr!");
			unlockBuffer();
			return;
		}
		if((m_currentIndex+1)>m_maxSize){
			MFObject::printErr("S_MF_EventBuffer::returnEvent - failed, "
					"(m_currentIndex+1)>=m_maxSize!");
			unlockBuffer();
			return;
		}
		if(m_isSource){//if its not a source buffer, its a buffer for processing
			//if its a source buffer, pEvent isnt used anymore -> lock it and
			//set isUsed to false -> unlock will be called if taken from buffer
			pEvent->lock();//Will be unlocked if taken from source buffer
			pEvent->isUsed=false;
			//isDispatched set to true by S_MF_NetworkEvent::removeDispatchingTask
		}
		m_currentIndex++;
		pVecEventBuffer->data()[m_currentIndex]=pEvent;
		if(pEvent->pEvent->packet!=nullptr && pEvent->pEvent->packet->freeCallback!=nullptr)
			pEvent->pEvent->packet->freeCallback(pEvent->pEvent->packet);
		unlockBuffer();
	}

	/**
	 * If neccessary use lockTake() before calling this function!
	 * @return
	 */
	uint32_t getRemainingEventCount(){
		uint32_t count=0;
		lockBuffer();
		count=m_currentIndex+1;
		unlockBuffer();
		return count;
	}
};
#endif
