/*
 * CMMLDATrainCPPUnit.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CMMLDATRAINCPPUNIT_H_
#define CMMLDATRAINCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CMMLDAEMFunctionoid.h"
#include "../GraphicalModels/CMMLDAGeneralModel.h"
#include "../Printer/CPrinter.h"

class CMMLDATrainCPPUnit {
public:
	CMMLDATrainCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CMMLDATrainCPPUnit();
};

#endif /* CMMLDATRAINCPPUNIT_H_ */
