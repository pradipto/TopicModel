/*
 * CTagLDATestCPPUnit.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CTAGLDATESTCPPUNIT_H_
#define CTAGLDATESTCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CTagLDAEMFunctionoid.h"
#include "../GraphicalModels/CTagLDAGeneralModel.h"
#include "../Printer/CPrinter.h"

class CTagLDATestCPPUnit {
public:
	CTagLDATestCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CTagLDATestCPPUnit();
};

#endif /* CTAGLDATESTCPPUNIT_H_ */
