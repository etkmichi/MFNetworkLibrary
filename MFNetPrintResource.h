/*
 * MFNetPrintResource.h
 *
 *  Created on: 04.09.2020
 *      Author: michl
 */

#ifndef MFNETPRINTRESOURCE_H_
#define MFNETPRINTRESOURCE_H_
#include <MFPrinters/MFPrintTarget.h>
class MFNetPrintResource : public MFPrintTarget{
public:
  uint32_t printIndex=0;
  MFNetPrintResource(){

  }
};



#endif /* MFNETPRINTRESOURCE_H_ */
