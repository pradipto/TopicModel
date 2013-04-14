/*
 * CCorrLDATrainCPPUnit.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CCORRLDATRAINCPPUNIT_H_
#define CCORRLDATRAINCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CCorrLDAEMFunctionoid.h"
#include "../GraphicalModels/CCorrLDAGeneralModel.h"
#include "../Printer/CPrinter.h"

class CCorrLDATrainCPPUnit {
public:
	CCorrLDATrainCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CCorrLDATrainCPPUnit();
};

#endif /* CCORRLDATRAINCPPUNIT_H_ */
