/*
 * CVanillaAsymLDATestCPPUnit.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CVANILLAASYMLDATESTCPPUNIT_H_
#define CVANILLAASYMLDATESTCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CVanillaAsymLDAEMFunctionoid.h"
#include "../GraphicalModels/CVanillaAsymLDAGeneralModel.h"
#include "../DataSet/CParameterMatrixReaderWriter.h"
#include "../Printer/CPrinter.h"

class CVanillaAsymLDATestCPPUnit {
public:
	CVanillaAsymLDATestCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CVanillaAsymLDATestCPPUnit();
};

#endif /* CVANILLAASYMLDATESTCPPUNIT_H_ */
