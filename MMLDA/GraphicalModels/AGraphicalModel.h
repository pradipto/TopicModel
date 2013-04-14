/*
 * AGraphicalModel.h
 *
 *  Created on: May 21, 2010
 *      Author: pdas
 */

#ifndef AGRAPHICALMODEL_H_
#define AGRAPHICALMODEL_H_

#include "../Utilities/includes.h"
//using namespace std;

class AGraphicalModel {

public:
	virtual ~AGraphicalModel() {}	// this is provided to override a compiler warning

	virtual void set_model_operation_mode(std::string mode) = 0;
	virtual bool create_and_initialize_data_structures() = 0;
	virtual double compute_log_likelihood() = 0;
	virtual std::string to_string() = 0;
};

#endif /* AGRAPHICALMODEL_H_ */
