/*
 * CMMLDATestCPPUnit.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CMMLDATESTCPPUNIT_H_
#define CMMLDATESTCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CMMLDAEMFunctionoid.h"
#include "../GraphicalModels/CMMLDAGeneralModel.h"
#include "../Printer/CPrinter.h"

class CMMLDATestCPPUnit {
public:
	CMMLDATestCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CMMLDATestCPPUnit();
};

#endif /* CMMLDATESTCPPUNIT_H_ */
