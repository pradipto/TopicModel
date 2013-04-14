/*
 * CCorrLDAPredictCPPUnit.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CCORRLDAPREDICTCPPUNIT_H_
#define CCORRLDAPREDICTCPPUNIT_H_

#include "../Utilities/includes.h"
#include "../EMAlgorithms/CCorrLDAEMFunctionoid.h"
#include "../GraphicalModels/CCorrLDAGeneralModel.h"
#include "../Printer/CPrinter.h"

class CCorrLDAPredictCPPUnit {
public:
	CCorrLDAPredictCPPUnit(std::string model_description_file, std::string model_compiled_file, std::string config_file);
	virtual ~CCorrLDAPredictCPPUnit();
};

#endif /* CCORRLDAPREDICTCPPUNIT_H_ */
