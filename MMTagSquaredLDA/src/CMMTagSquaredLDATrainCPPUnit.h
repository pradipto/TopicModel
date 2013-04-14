/*
 * CMMTagSquaredLDATrainCPPUnit.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CMMTAGSQUAREDLDATRAINCPPUNIT_H_
#define CMMTAGSQUAREDLDATRAINCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CMMTagSquaredLDAEMFunctionoid.h"
#include "../GraphicalModels/CMMTagSquaredLDAGeneralModel.h"
#include "../Printer/CPrinter.h"

class CMMTagSquaredLDATrainCPPUnit {
public:
	CMMTagSquaredLDATrainCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CMMTagSquaredLDATrainCPPUnit();
};

#endif /* CMMTAGSQUAREDLDATRAINCPPUNIT_H_ */
