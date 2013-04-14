/*
 * CMMLDAPredictCPPUnit.cpp
 *
 *  Created on: Dec 18, 2011
 *      Author: data
 */

#include "CMMLDAPredictCPPUnit.h"
using namespace std;

CMMLDAPredictCPPUnit::CMMLDAPredictCPPUnit(string model_description_file, string model_compiled_file, string config_file) {
	CMMLDAEMFunctionoid* p_em_model = new CMMLDAEMFunctionoid();


	CMMLDAGeneralModel* MMLDA_model = p_em_model->get_concrete_model();
	//if (b_verbose_global) std::cout << "model is " << CCommand::_model_bytecode << endl;
	MMLDA_model->set_bytecode("MMLDA");

	string model_operation_mode = "predict";
	MMLDA_model->set_model_operation_mode(model_operation_mode);	// MUST SET THIS ---- IMPORTANT!!!

	MMLDA_model->set_model_dump_mode("text");	// default is text file dumping

	// model initialization
	MMLDA_model->read_config_file(config_file);
	MMLDA_model->read_data_from_file();

	// use the functionoid for EM
	(*p_em_model)();

	delete p_em_model;
}

CMMLDAPredictCPPUnit::~CMMLDAPredictCPPUnit() {
	// TODO Auto-generated destructor stub
}
