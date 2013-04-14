/*
 * CVanillaAsymLDATrainCPPUnit.cpp
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#include "CVanillaAsymLDATrainCPPUnit.h"
using namespace std;

/**
 * @brief the model_description_file and the model_compiled_file are not used in this project
 */
CVanillaAsymLDATrainCPPUnit::CVanillaAsymLDATrainCPPUnit(string model_description_file, string model_compiled_file, string config_file) {

	CPrinter prn;

	CVanillaAsymLDAEMFunctionoid* p_em_model = new CVanillaAsymLDAEMFunctionoid();


	CVanillaAsymLDAGeneralModel* VanillaAsymLDA_model = p_em_model->get_concrete_model();
	//if (b_verbose_global) std::cout << "model is " << CCommand::_model_bytecode << endl;
	VanillaAsymLDA_model->set_bytecode("VanillaAsymLDA");

	string model_operation_mode = "train";
	VanillaAsymLDA_model->set_model_operation_mode(model_operation_mode);	// MUST SET THIS ---- IMPORTANT!!!

	VanillaAsymLDA_model->set_model_dump_mode("text");	// default is text file dumping

	// model initialization
	VanillaAsymLDA_model->read_config_file(config_file);
	VanillaAsymLDA_model->read_data_from_file();

	// use the functionoid for EM
	(*p_em_model)();

	delete p_em_model;

}

CVanillaAsymLDATrainCPPUnit::~CVanillaAsymLDATrainCPPUnit() {

}
