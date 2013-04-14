/*
 * CMMTagSquaredLDATestCPPUnit.cpp
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#include "CMMTagSquaredLDATestCPPUnit.h"
using namespace std;

CMMTagSquaredLDATestCPPUnit::CMMTagSquaredLDATestCPPUnit(string model_description_file, string model_compiled_file, string config_file) {

	CMMTagSquaredLDAEMFunctionoid* p_em_model = new CMMTagSquaredLDAEMFunctionoid();


	CMMTagSquaredLDAGeneralModel* MMTag2LDA_model = p_em_model->get_concrete_model();
	//if (b_verbose_global) std::cout << "model is " << CCommand::_model_bytecode << endl;
	MMTag2LDA_model->set_bytecode("MMTagSquaredLDA");

	string model_operation_mode = "test";
	MMTag2LDA_model->set_model_operation_mode(model_operation_mode);	// MUST SET THIS ---- IMPORTANT!!!

	MMTag2LDA_model->set_model_dump_mode("text");	// default is text file dumping

	// model initialization
	MMTag2LDA_model->read_config_file(config_file);
	MMTag2LDA_model->read_data_from_file();

	// use the functionoid for EM
	(*p_em_model)();

	delete p_em_model;
}

CMMTagSquaredLDATestCPPUnit::~CMMTagSquaredLDATestCPPUnit() {
	// TODO Auto-generated destructor stub
}
