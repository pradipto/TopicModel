/*
 * CMMLDAPredictCPPUnit.h
 *
 *  Created on: Dec 18, 2011
 *      Author: data
 */

#ifndef CMMLDAPREDICTCPPUNIT_H_
#define CMMLDAPREDICTCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CMMLDAEMFunctionoid.h"
#include "../GraphicalModels/CMMLDAGeneralModel.h"
#include "../Printer/CPrinter.h"

class CMMLDAPredictCPPUnit {
public:
	CMMLDAPredictCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CMMLDAPredictCPPUnit();
};

#endif /* CMMLDAPREDICTCPPUNIT_H_ */
