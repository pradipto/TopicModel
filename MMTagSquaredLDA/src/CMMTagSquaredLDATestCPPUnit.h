/*
 * CMMTagSquaredLDATestCPPUnit.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CMMTAGSQUAREDLDATESTCPPUNIT_H_
#define CMMTAGSQUAREDLDATESTCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CMMTagSquaredLDAEMFunctionoid.h"
#include "../GraphicalModels/CMMTagSquaredLDAGeneralModel.h"
#include "../Printer/CPrinter.h"

class CMMTagSquaredLDATestCPPUnit {
public:
	CMMTagSquaredLDATestCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CMMTagSquaredLDATestCPPUnit();
};

#endif /* CMMTAGSQUAREDLDATESTCPPUNIT_H_ */
