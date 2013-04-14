/*
 * CCorrLDATestCPPUnit.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CCORRLDATESTCPPUNIT_H_
#define CCORRLDATESTCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CCorrLDAEMFunctionoid.h"
#include "../GraphicalModels/CCorrLDAGeneralModel.h"
#include "../Printer/CPrinter.h"

class CCorrLDATestCPPUnit {
public:
	CCorrLDATestCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CCorrLDATestCPPUnit();
};

#endif /* CCORRLDATESTCPPUNIT_H_ */
