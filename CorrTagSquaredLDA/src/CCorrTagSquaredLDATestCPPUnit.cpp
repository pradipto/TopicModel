/*
 * CCorrTagSquaredLDATestCPPUnit.cpp
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#include "CCorrTagSquaredLDATestCPPUnit.h"
using namespace std;

CCorrTagSquaredLDATestCPPUnit::CCorrTagSquaredLDATestCPPUnit(string model_description_file, string model_compiled_file, string config_file) {

	CPrinter prn;

	CCorrTagSquaredLDAEMFunctionoid* p_em_model = new CCorrTagSquaredLDAEMFunctionoid();


	CCorrTagSquaredLDAGeneralModel* CorrTag2LDA_model = p_em_model->get_concrete_model();
	//if (b_verbose_global) std::cout << "model is " << CCommand::_model_bytecode << endl;
	CorrTag2LDA_model->set_bytecode("CorrTag2LDA");

	string model_operation_mode = "test";
	CorrTag2LDA_model->set_model_operation_mode(model_operation_mode);	// MUST SET THIS ---- IMPORTANT!!!

	CorrTag2LDA_model->set_model_dump_mode("text");	// default is text file dumping

	// model initialization
	CorrTag2LDA_model->read_config_file(config_file);
	CorrTag2LDA_model->read_data_from_file();

	// use the functionoid for EM
	(*p_em_model)();

	delete p_em_model;
}

CCorrTagSquaredLDATestCPPUnit::~CCorrTagSquaredLDATestCPPUnit() {
	// TODO Auto-generated destructor stub
}
