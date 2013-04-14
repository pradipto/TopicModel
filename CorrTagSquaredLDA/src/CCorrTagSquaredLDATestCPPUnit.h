/*
 * CCorrTagSquaredLDATestCPPUnit.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CCORRTAGSQUAREDLDATESTCPPUNIT_H_
#define CCORRTAGSQUAREDLDATESTCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CCorrTagSquaredLDAEMFunctionoid.h"
#include "../GraphicalModels/CCorrTagSquaredLDAGeneralModel.h"
#include "../Printer/CPrinter.h"

class CCorrTagSquaredLDATestCPPUnit {
public:
	CCorrTagSquaredLDATestCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CCorrTagSquaredLDATestCPPUnit();
};

#endif /* CCORRTAGSQUAREDLDATESTCPPUNIT_H_ */
