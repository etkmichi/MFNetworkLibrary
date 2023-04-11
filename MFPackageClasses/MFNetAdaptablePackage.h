/*
 * MFNetAdaptablePackage.h
 *
 *  Created on: 18.03.2020
 *      Author: michl
 */

#ifndef MFPACKAGECLASSES_MFNETADAPTABLEPACKAGE_H_
#define MFPACKAGECLASSES_MFNETADAPTABLEPACKAGE_H_



class MFNetAdaptablePackage  {
public:
	MFNetAdaptablePackage();
	virtual ~MFNetAdaptablePackage();
	virtual void setupPackageStructure(){return;};
};

#endif /* MFPACKAGECLASSES_MFNETADAPTABLEPACKAGE_H_ */
