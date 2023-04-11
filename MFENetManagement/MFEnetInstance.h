/*
 * MFEnetInstance.h
 *
 *  Created on: 03.03.2020
 *      Author: michl
 */

#ifndef MFENETINSTANCE_H_
#define MFENETINSTANCE_H_
#include <mutex>
#include <enet/enet.h>

class MFEnetInstance {
private:
	static std::mutex
		lockEnetInstance,
		lockUserCounter;
	static uint32_t
		m_userCounter;
	static bool
		m_isInitialized,
		m_blockInit;
public:
	static bool initEnet();
	static bool exitEnet();
	/**
	 * If enet is not initialized, this function will init enet for the user.
	 * If enet is init, this f will increase a counter. If count==0 then enet will
	 * exit, till a user is added again.
	 */
	static bool addUser();
	static bool takeUser();
	static void blockInit(){m_blockInit=true;};
	static void unblockInit(){m_blockInit=false;};
};

#endif /* MFENETINSTANCE_H_ */
