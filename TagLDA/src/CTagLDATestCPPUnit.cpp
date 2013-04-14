/*
 * CTagLDATestCPPUnit.cpp
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#include "CTagLDATestCPPUnit.h"
using namespace std;

CTagLDATestCPPUnit::CTagLDATestCPPUnit(string model_description_file, string model_compiled_file, string config_file) {

	CPrinter prn;

	CTagLDAEMFunctionoid* p_em_model = new CTagLDAEMFunctionoid();


	CTagLDAGeneralModel* TagLDA_model = p_em_model->get_concrete_model();
	//if (b_verbose_global) std::cout << "model is " << CCommand::_model_bytecode << endl;
	TagLDA_model->set_bytecode("TagLDA");

	string model_operation_mode = "test";
	TagLDA_model->set_model_operation_mode(model_operation_mode);	// MUST SET THIS ---- IMPORTANT!!!

	TagLDA_model->set_model_dump_mode("text");	// default is text file dumping

	// model initialization
	TagLDA_model->read_config_file(config_file);
	TagLDA_model->read_data_from_file();

	if (!TagLDA_model->create_and_initialize_data_structures()) {
		cout << "TagLDA Couldn't create and initialize model data structures.. possibly wrong filepaths" << endl;
		return;
	}

	// use the functionoid for EM
	(*p_em_model)();

	delete p_em_model;
}

CTagLDATestCPPUnit::~CTagLDATestCPPUnit() {
	// TODO Auto-generated destructor stub
}
