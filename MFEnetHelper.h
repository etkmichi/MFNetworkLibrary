/*
 * MFENetHelper.h
 *
 *  Created on: 11.03.2020
 *      Author: michl
 */

#ifndef MFENETHELPER_H_
#define MFENETHELPER_H_
#include <string>
#include <MFObjects/MFObject.h>
#include <enet/enet.h>
#include "MFEnetStructs.h"
class MFEnetHelper {
public:
	static std::string addressToIPV4String(uint32_t address){
		uint8_t
			p1=0xFF000000&address,
			p2=0x00FF0000&address,
			p3=0x0000FF00&address,
			p4=0x000000FF&address;
		std::string addressString=
				std::to_string(p4)+"."+std::to_string(p3)+"."+std::to_string(p2)
				+"."+std::to_string(p1);
		return addressString;
	}
	static void printInformations(ENetHost* mp_enetLocalHost, ENetPeer* mp_destinationPeer){
		std::string
			clientInfo="",
			peerInfo="";
		if(mp_enetLocalHost!=nullptr){
			clientInfo+="address - "+addressToIPV4String(
					mp_enetLocalHost->address.host)+":";
			clientInfo+=std::to_string(mp_enetLocalHost->address.port)+"\n";
			clientInfo+="connectedPeers - "+std::to_string(mp_enetLocalHost->connectedPeers)+"\n";
		}else{
			clientInfo+="mp_enetLocalHost==nullptr!";
		}
		if(mp_destinationPeer!=nullptr){

			peerInfo+="address - "+addressToIPV4String(
					mp_destinationPeer->address.host)+":";
			peerInfo+=std::to_string(mp_destinationPeer->address.port)+"\n";
		}else{
			peerInfo="mp_destinationPeer==nullptr!";
		}
		MFObject::printInfo("\n"
				"client (local) infos:\n"+clientInfo+"\n"
				"peer (dst) infos:\n"+peerInfo+"");
	}
	static void printInformations(S_MF_NetworkEvent* pNE){
		printInformations(pNE->pHost, pNE->pEvent->peer);
	}
	static void printDataAsString(S_MF_NetworkEvent* pNE){
		if(pNE->pEvent==nullptr || pNE->pEvent->packet==nullptr ||
				pNE->pEvent->packet->data==nullptr){
			MFObject::printErr(""
					"pNE->pEvent==nullptr || "
					"pNE->pEvent->packet==nullptr ||"
					"pNE->pEvent->packet->data==nullptr");
			return;
		}
		std::string data="";
		data+=std::string((const char*)(pNE->pEvent->packet->data));
		MFObject::printInfo(data);
	}
	static std::string getInformations(ENetHost* mp_enetLocalHost, ENetPeer* mp_destinationPeer){
		std::string
			clientInfo="client (local) infos:\n",
			peerInfo="peer (dst) infos:\n";
		if(mp_enetLocalHost!=nullptr){
			clientInfo+="address - "+addressToIPV4String(
					mp_enetLocalHost->address.host)+":";
			clientInfo+=std::to_string(mp_enetLocalHost->address.port)+"\n";
			clientInfo+="connectedPeers - "+std::to_string(mp_enetLocalHost->connectedPeers)+"\n";
		}else{
			clientInfo+="mp_client==nullptr!\n";
		}
		if(mp_destinationPeer!=nullptr){
			peerInfo+="address - "+addressToIPV4String(
					mp_destinationPeer->address.host)+":";
			peerInfo+=std::to_string(mp_destinationPeer->address.port)+"\n";
		}else{
			peerInfo="mp_destinationPeer==nullptr!";
		}
		return clientInfo+peerInfo;
	}
};



#endif /* MFENETHELPER_H_ */
