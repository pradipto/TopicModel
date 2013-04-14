/*
 * CTagLDATrainCPPUnit.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CTAGLDATRAINCPPUNIT_H_
#define CTAGLDATRAINCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CTagLDAEMFunctionoid.h"
#include "../GraphicalModels/CTagLDAGeneralModel.h"
#include "../Printer/CPrinter.h"

class CTagLDATrainCPPUnit {
public:
	CTagLDATrainCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CTagLDATrainCPPUnit();
};

#endif /* CTAGLDATRAINCPPUNIT_H_ */
