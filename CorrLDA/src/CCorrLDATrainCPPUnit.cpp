/*
 * CCorrLDATrainCPPUnit.cpp
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#include "CCorrLDATrainCPPUnit.h"
using namespace std;

/**
 * @brief the model_description_file and the model_compiled_file are not used in this project
 */
CCorrLDATrainCPPUnit::CCorrLDATrainCPPUnit(string model_description_file, string model_compiled_file, string config_file) {

	CPrinter prn;

	CCorrLDAEMFunctionoid* p_em_model = new CCorrLDAEMFunctionoid();


	CCorrLDAGeneralModel* CorrLDA_model = p_em_model->get_concrete_model();
	//if (b_verbose_global) std::cout << "model is " << CCommand::_model_bytecode << endl;
	CorrLDA_model->set_bytecode("CorrLDA");

	string model_operation_mode = "train";
	CorrLDA_model->set_model_operation_mode(model_operation_mode);	// MUST SET THIS ---- IMPORTANT!!!

	CorrLDA_model->set_model_dump_mode("text");	// default is text file dumping

	// model initialization
	CorrLDA_model->read_config_file(config_file);
	CorrLDA_model->read_data_from_file();

	//if (!CorrLDA_model->create_and_initialize_data_structures()) {
	//	cout << "TagLDA Couldn't create and initialize model data structures.. possibly wrong filepaths" << endl;
	//	return;
	//}

	// use the functionoid for EM
	(*p_em_model)();

	// write model parameters to disk
	//CorrLDA_model->save_model();

	delete p_em_model;

}

CCorrLDATrainCPPUnit::~CCorrLDATrainCPPUnit() {

}
