/*
 * MFEnetInstance.cpp
 *
 *  Created on: 03.03.2020
 *      Author: michl
 */

#include "MFEnetInstance.h"
#include <MFObjects/MFObject.h>
std::mutex MFEnetInstance::lockEnetInstance;
std::mutex MFEnetInstance::lockUserCounter;
bool MFEnetInstance::m_isInitialized=false;
bool MFEnetInstance::m_blockInit=false;
uint32_t MFEnetInstance::m_userCounter=0;
bool MFEnetInstance::initEnet(){
	if(m_blockInit)return m_isInitialized;
	if(m_isInitialized){
		MFObject::printWarning("MFEnetInstance::initEnet() - instance"
				"already initialized, returning true!");
		return true;
	}
	if(enet_initialize()!=0){
		MFObject::printErr("MFEnetInstance::initEnet() - !Yay, init "
				"failed!");
		return false;
	}else{
		MFObject::printInfo("MFEnetInstance::initEnet() - static init for "
				"enet was successful!");
	}
	return true;
}
bool MFEnetInstance::exitEnet(){
	if(!m_isInitialized){
		enet_deinitialize();
		m_isInitialized=false;
	}
	MFObject::printInfo("MFEnetInstance::exitEnet() - static exit for "
			"enet was successful!");
	return true;
}
bool MFEnetInstance::addUser(){
	if(m_blockInit){
		MFObject::printWarning("MFEnetInstance::addUser() - init is blocked!");
		return false;
	}
	bool ret = true;
	lockUserCounter.lock();
	m_userCounter++;
	if(m_userCounter==1){
		if(!initEnet()){
			MFObject::printErr("Failed to init enet!");
			m_userCounter--;
			ret=false;
		}
	}
	lockUserCounter.unlock();
	return ret;
}
bool MFEnetInstance::takeUser(){
	bool ret = true;
	lockUserCounter.lock();
	m_userCounter--;
	if(m_userCounter==0){
		if(!exitEnet()){
			MFObject::printErr("MFEnetInstance::takeUser - Failed to exit enet!");
			m_userCounter++;
			ret=false;
		}
	}
	lockUserCounter.unlock();
	return ret;

}

