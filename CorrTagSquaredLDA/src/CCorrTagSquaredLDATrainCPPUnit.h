/*
 * CCorrTagSquaredLDATrainCPPUnit.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CCORRTAGSQUAREDLDATRAINCPPUNIT_H_
#define CCORRTAGSQUAREDLDATRAINCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CCorrTagSquaredLDAEMFunctionoid.h"
#include "../GraphicalModels/CCorrTagSquaredLDAGeneralModel.h"
#include "../Printer/CPrinter.h"

class CCorrTagSquaredLDATrainCPPUnit {
public:
	CCorrTagSquaredLDATrainCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CCorrTagSquaredLDATrainCPPUnit();
};

#endif /* CCORRTAGSQUAREDLDATRAINCPPUNIT_H_ */
