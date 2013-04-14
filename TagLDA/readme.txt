readme.txt

 Created on: Dec 9, 2010
     Author: data

===== COMPILING =====
$> cd bin
$> make clean
$> make
$> cd ..

===== DATASET FORMAT =====
Sample data and resource files can be found in _sample_data folder
Each row of model.input file is a document and reads like:
num_of_positions term1_id:term1_count:term1_tag_id .... termN_id:termN_count:termN_tag_id

- where term1_id = term_id_at_position_1 

===== TRAINING =====

$> bin/TagLDA _config_files/sample-train-config-cikm11AR-20.txt train

===== INFERENCE =====
$> bin/TagLDA _config_files/sample-test-config-cikm11AR-20.txt test


===== CODE DESCRIPTION =====

1. The CTagLDAEMFunctionoid class implemented in EMAlgorithms/CTagLDAEMFunctionoid.cpp creates the TagLDA model
   implemented in file GraphicalModels/CTagLDA.cpp
2.a. The TagLDA model does the main job of reading the model input file and creating and initializing model matrices
2.b. The TagLDA model also calculates the log likelihood corresponding to the model 
		The log likelihood is calculated using the method double CTagLDA::compute_doc_log_likelihood(size_t doc_id, STagLDADocument* doc) {}

3. The CTagLDAEMFunctionoid has three major methods:
3.a. void CTagLDAEMFunctionoid::operator ()() {} drives the EM algorithm for TagLDA
3.b. void CTagLDAEMFunctionoid::expectation_step() {} is the driver for the E-step
		The sufficient matrices are updated here on a per document basis as in VB for LDA
3.b. double CTagLDAEMFunctionoid::doc_expectation_step(size_t doc_id, STagLDADocument* doc) {}  
		The optimal values of the variational matrices for the current iteration are calculated in this step
3.c. void CTagLDAEMFunctionoid::maximization_step() {} finds the optimal values of the model parameters for the current iteration
  
4. src/TagLDA.cpp is the driver file
	TagLDA.cpp invokes CTagLDATrainCPPUnit or CTagLDATestCPPUnit depending on the model operation mode
	
+-----------------------------------------------------------+
For training purposes, the following statements are executed:
/**
 * @brief the model_description_file and the model_compiled_file are not used in this project
 */
CTagLDATrainCPPUnit::CTagLDATrainCPPUnit(string model_description_file, string model_compiled_file, string config_file) {

	CPrinter prn;

	CTagLDAEMFunctionoid* p_em_model = new CTagLDAEMFunctionoid();


	CTagLDAGeneralModel* TagLDA_model = p_em_model->get_concrete_model();
	//if (b_verbose_global) std::cout << "model is " << CCommand::_model_bytecode << endl;
	TagLDA_model->set_bytecode("TagLDA");

	string model_operation_mode = "train";
	TagLDA_model->set_model_operation_mode(model_operation_mode);	// MUST SET THIS ---- IMPORTANT!!!

	TagLDA_model->set_model_dump_mode("text");	// default is text file dumping

	// model initialization
	TagLDA_model->read_config_file(config_file);
	TagLDA_model->read_data_from_file();

	// use the functionoid for EM
	(*p_em_model)();

	delete p_em_model;

}
+-----------------------------------------------------------+
