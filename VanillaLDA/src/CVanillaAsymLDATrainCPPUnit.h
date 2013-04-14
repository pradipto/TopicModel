/*
 * CVanillaAsymLDATrainCPPUnit.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CVANILLAASYMLDATRAINCPPUNIT_H_
#define CVANILLAASYMLDATRAINCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CVanillaAsymLDAEMFunctionoid.h"
#include "../GraphicalModels/CVanillaAsymLDAGeneralModel.h"
#include "../Printer/CPrinter.h"

class CVanillaAsymLDATrainCPPUnit {
public:
	CVanillaAsymLDATrainCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CVanillaAsymLDATrainCPPUnit();
};

#endif /* CVANILLAASYMLDATRAINCPPUNIT_H_ */
