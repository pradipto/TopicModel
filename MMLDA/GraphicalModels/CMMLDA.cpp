/*
 * CMMLDA.cpp
 *
 *  Created on: Jan 10, 2011
 *      Author: data
 */

#include "CMMLDA.h"
using namespace std;

/**
 * @brief constructor
 */
CMMLDA::CMMLDA() {

	// these are created when the data files are read
	_corpus = 0;
	_D = 0;
	_V = 0;			// this should be vocabulary size
	_corrV = 0;

	_n_max_datum_in_a_doc = 0;
	_n_max_corr_datum_in_a_doc = 0;
	_training_datum_total_count = 0;
	_training_corr_datum_total_count = 0;

	// these are initialized when configuration files are read
	_K = 0;

	_alpha = 0.0;
	_b_use_asym_alpha = false;
	_asym_alpha = 0;
	_asym_alpha_ss = 0;
	_asym_alpha_exp_ss = 0;
	_asym_alpha_concentration_parameter = 10.0;
}

/**
 * @brief destructor
 */
CMMLDA::~CMMLDA() {
	// * delete parameter matrices
	/*
	 * delete the pi parameter matrix
	 */
	if ( _log_rho ) { CTemplateMatrixMath<double>::delete_2D_matrix(_log_rho); _log_rho = 0; }
	/*
	 * delete the beta parameter matrix
	 */
	if ( _log_beta ) { CTemplateMatrixMath<double>::delete_2D_matrix(_log_beta); _log_beta = 0; }
	/*
	 * delete the phi variational matrix
	 */
	if ( _var_phi ) { CTemplateMatrixMath<double>::delete_2D_matrix(_var_phi); _var_phi = 0; }
	/*
	 * delete the lambda variational matrix
	 */
	if ( _var_lambda ) { CTemplateMatrixMath<double>::delete_2D_matrix(_var_lambda); _var_lambda = 0; }
	/*
	 * delete the gamma variational vector D x K (substitute for topic proportions)
	 */
	if ( _var_gamma ) { CTemplateMatrixMath<double>::delete_2D_matrix(_var_gamma); _var_gamma = 0; }

	// * delete sufficient statistics matrices
	/*
	 * delete the rho_ss suffstat matrix
	 */
	if ( _rho_ss ) { CTemplateMatrixMath<double>::delete_2D_matrix(_rho_ss); _rho_ss = 0; }
	delete _rho_total_ss;

	/*
	 * delete the beta_ss suffstat matrix
	 */
	if ( _beta_ss ) { CTemplateMatrixMath<double>::delete_2D_matrix(_beta_ss); _beta_ss = 0; }
	delete _beta_total_ss;

	if ( _asym_alpha ) { delete _asym_alpha; _asym_alpha = 0; }
	if ( _asym_alpha_ss ) { delete _asym_alpha_ss; _asym_alpha_ss = 0; }
	if ( _asym_alpha_exp_ss ) { delete _asym_alpha_exp_ss; }

}

/**
 * @brief (re)initialize the sufficient statistics matrices and scalars to 0
 */
bool CMMLDA::zero_initialize_suff_stats() {
	CTemplateMatrixMath<double>::zero_initialize_matrix(_beta_ss);
	CTemplateMatrixMath<double>::zero_initialize_matrix(_rho_ss);
	CTemplateMatrixMath<double>::zero_initialize_vector(_beta_total_ss);
	CTemplateMatrixMath<double>::zero_initialize_vector(_rho_total_ss);
	if ( _b_use_asym_alpha) {
		CTemplateMatrixMath<double>::zero_initialize_vector(_asym_alpha_ss);
		CTemplateMatrixMath<double>::zero_initialize_vector(_asym_alpha_exp_ss);
		CTemplateMatrixMath<double>::zero_initialize_vector(_asym_alpha_exp_square_ss);
	}
	_alpha_ss = 0.0;
	return true;
}

/**
 * @brief reset the data structures
 */
void CMMLDA::reset_data_structures() {

}

/**
 * @brief read training or testing data from file
 */
void CMMLDA::read_data_from_file() {
	if ( _operation_mode == "train" )
		read_data_from_file_for_training();
	else if ( _operation_mode == "test" || _operation_mode == "predict" )
		read_data_from_file_for_testing();
}

/**
 * @brief read data from file for training
 */
void CMMLDA::read_data_from_file_for_training() {

	// make sure that the config file has been read
	assert( _K != 0 );

	// read the topic model index (tmi) file

	ifstream model_index_file_stream(_model_input_filename.c_str());

	// need a vector for tokenization
	vector<string> token_vec, indv_model_vec;
	string line, model1, model2;
	size_t datum_id, datum_count;
	string ws_token_delim = " \n\r\t";			// the standard set of delimiter strings
	string model_delim = "|\n\r";

	vector<string> token_split_vec;

	size_t doc_id;
	size_t num_datum = 0;

	_corpus = new SMMLDACorpus();

	if ( model_index_file_stream.is_open() )	{
		doc_id = 0;

		while ( !model_index_file_stream.eof() ) {
			num_datum = 0;

			getline( model_index_file_stream, line );	// each line is a document  consisting of 2 views
			if ( line.length() == 0 ) continue;

			// create a document
			SMMLDADocument *doc = new SMMLDADocument();

			// tokenize the line into individual model input strings
			indv_model_vec.clear();
			CUtilities::tokenize(line, indv_model_vec, model_delim);

			assert(indv_model_vec.size() == 2);
			for ( size_t m = 0; m < 2; ++m ) {

				// tokenize each model input and insert the values in the row
				token_vec.clear();
				CUtilities::tokenize(indv_model_vec[m], token_vec, ws_token_delim);

				CUtilities::string_to_number<size_t>(num_datum, token_vec[0], std::dec);
				if ( m == 0 ) {
					if ( num_datum > _n_max_datum_in_a_doc )
						_n_max_datum_in_a_doc = num_datum;
				}
				if ( m == 1 ) {
					if ( num_datum > _n_max_corr_datum_in_a_doc )
						_n_max_corr_datum_in_a_doc = num_datum;
				}

				num_datum = 0;
				for (size_t i = 1; i < token_vec.size(); ++i ) {	// the first token is the # of unique terms
					// split tokens[i] by ":"

					token_split_vec.clear();
					CUtilities::tokenize(token_vec[i], token_split_vec, ":");

					CUtilities::string_to_number(datum_id, token_split_vec[0], std::dec);
					CUtilities::string_to_number(datum_count, token_split_vec[1], std::dec);

					if ( m == 0 ) {
						doc->_datum_id_vec->push_back(datum_id);
						doc->_datum_id_count_vec->push_back(datum_count);
					}
					if ( m == 1 ) {
						doc->_corr_datum_id_vec->push_back(datum_id);
						doc->_corr_datum_id_count_vec->push_back(datum_count);
					}

					num_datum += datum_count;

					if ( m == 0) {
						if ( _V < datum_id ) _V = datum_id;
					}
					if ( m == 1) {
						if ( _corrV < datum_id ) _corrV = datum_id;
					}
				}

				_training_datum_total_count += num_datum;

				if ( m == 0 ) {
					doc->_num_of_datum = doc->_datum_id_vec->size();
					doc->_total_num_of_datum = num_datum;
				}
				if ( m == 1 ) {
					doc->_num_of_corr_datum = doc->_corr_datum_id_vec->size();
					doc->_total_num_of_corr_datum = num_datum;
				}
			}

			_corpus->_document_vec->push_back(doc);

			// increment document number
			++doc_id;

		}
		_D = doc_id;	// _D is the total number of documents
		++_V; 			// _V is the total number of terms not the term ids
		++_corrV;
		_corpus->_n_docs = _corpus->_document_vec->size();

		model_index_file_stream.close();
	} else {
		cout << "Couldn't open the model-index file: " << _model_input_filename << endl;
		exit(APPLICATIONEXIT);
	}

	/*
	 * Create the parameter matrices
	 */
	create_and_initialize_data_structures();
	if (b_verbose_global) print_parameters();

}

/**
 * -- Loads the model first based on the path in config file
 * -- creates the test corpus data structure
 *
 */
void CMMLDA::read_data_from_file_for_testing() {

	/*
	 * load the model
	 */
	load_model();	// Note that the load_model() function creates and populates the parameter matrices
	/*
	 * read test data
	 */
	// make sure that the config file has been read
	assert( _K != 0 );

	// read the topic model index (tmi) file

	ifstream model_index_file_stream(_model_input_filename.c_str());

	// need a vector for tokenization
	vector<string> token_vec, indv_model_vec;
	string line, model1, model2;
	size_t datum_id, datum_count;
	string ws_token_delim = " \n\r\t";			// the standard set of delimiter strings
	string model_delim = "|\n\r";

	vector<string> token_split_vec;

	size_t doc_id;
	size_t num_datum = 0;

	_corpus = new SMMLDACorpus();

	if ( model_index_file_stream.is_open() )	{
		doc_id = 0;

		while ( !model_index_file_stream.eof() ) {
			num_datum = 0;

			getline( model_index_file_stream, line );	// each line is a document  consisting of 2 views
			if ( line.length() == 0 ) continue;

			// create a document
			SMMLDADocument *doc = new SMMLDADocument();

			// tokenize the line into individual model input strings
			indv_model_vec.clear();
			CUtilities::tokenize(line, indv_model_vec, model_delim);

			assert(indv_model_vec.size() == 2);
			for ( size_t m = 0; m < 2; ++m ) {

				// tokenize each model input and insert the values in the row
				token_vec.clear();
				CUtilities::tokenize(indv_model_vec[m], token_vec, ws_token_delim);

				CUtilities::string_to_number<size_t>(num_datum, token_vec[0], std::dec);
				if ( m == 0 ) {
					if ( num_datum > _n_max_datum_in_a_doc )
						_n_max_datum_in_a_doc = num_datum;
				}
				if ( m == 1 ) {
					if ( num_datum > _n_max_corr_datum_in_a_doc )
						_n_max_corr_datum_in_a_doc = num_datum;
				}

				num_datum = 0;
				for (size_t i = 1; i < token_vec.size(); ++i ) {	// the first token is the # of unique terms
					// split tokens[i] by ":"

					token_split_vec.clear();
					CUtilities::tokenize(token_vec[i], token_split_vec, ":");

					CUtilities::string_to_number(datum_id, token_split_vec[0], std::dec);
					CUtilities::string_to_number(datum_count, token_split_vec[1], std::dec);

					if ( m == 0 ) {
						if (datum_id < _V) {
							doc->_datum_id_vec->push_back(datum_id);
							doc->_datum_id_count_vec->push_back(datum_count);
							num_datum += datum_count;
						}
					}
					if ( m == 1 ) {
						if (datum_id < _corrV) {
							doc->_corr_datum_id_vec->push_back(datum_id);
							doc->_corr_datum_id_count_vec->push_back(datum_count);
							num_datum += datum_count;
						}
					}

					/*
					if ( m == 0) {
						if ( _V < datum_id ) _V = datum_id;
					}
					if ( m == 1) {
						if ( _corrV < datum_id ) _corrV = datum_id;
					}
					*/
				}

				_training_datum_total_count += num_datum;

				if ( m == 0 ) {
					doc->_num_of_datum = doc->_datum_id_vec->size();
					doc->_total_num_of_datum = num_datum;
				}
				if ( m == 1 ) {
					doc->_num_of_corr_datum = doc->_corr_datum_id_vec->size();
					doc->_total_num_of_corr_datum = num_datum;
				}
			}

			_corpus->_document_vec->push_back(doc);

			// increment document number
			++doc_id;

		}
		_D = doc_id;	// _D is the total number of documents
		//++_V; 			// _V is the total number of terms not the term ids
		//++_corrV;
		_corpus->_n_docs = _corpus->_document_vec->size();

		model_index_file_stream.close();
	} else {
		cout << "Couldn't open the model-index file: " << _model_input_filename << endl;
	}

	/*
	 * Create the parameter matrices
	 */
	create_and_initialize_data_structures();
	if (b_verbose_global) print_parameters();

}


/**
 *
 */
bool CMMLDA::create_and_initialize_data_structures() {
	bool status = true;
	if ( _operation_mode == "train" ) {
		status &= create_and_initialize_data_structures_for_parameters();
		status &= create_and_initialize_data_structures_for_hidden_variables_and_suff_stats();
	} else if ( _operation_mode == "test" || _operation_mode == "predict" )
		status &= create_and_initialize_data_structures_for_hidden_variables_and_suff_stats();

	return status;
}

/**
 * @brief create and initialize the data structures for parameters
 */
bool CMMLDA::create_and_initialize_data_structures_for_parameters() {
	bool status = false;
	/*
	 * create the rho parameter matrix
	 */
	//cout << "Initializing rho.. " << endl;
	CTemplateMatrixMath<double>::create_2D_matrix(&_log_rho, _K, _corrV);

	/*
	 * create the beta parameter matrix
	 */
	//cout << "Initializing beta.. " << endl;
	CTemplateMatrixMath<double>::create_2D_matrix(&_log_beta, _K, _V);

	if ( _b_use_asym_alpha ) {
		_asym_alpha = new vector<double>(_K,0.0);
		// read the file containing the asymmetric values
		// NOTE: if this variable is true, then this vector also needs to be saved
		string line;
		size_t k = 0;
		vector<string> token_vec;
		ifstream fs(_asym_alpha_initial_file.c_str());
		if ( fs.is_open() ) {

			while ( !fs.eof() && k < _K ) {
				getline( fs, line );
				CUtilities::trim_left_right(line);

				if ( line.length() == 0 ) continue;				// ignore new lines
				CUtilities::tokenize(line, token_vec, " \n\r\t");
				for ( size_t i = 0; i < token_vec.size(); ++i ) {
					CUtilities::string_to_number<double>((*_asym_alpha)[k], token_vec[i], std::dec);
					++k;
				}
			}
		} else {
			cout << "No asym_alpha_initial file found.. using deafult values of base measure as 0.1 to be normalized by " << _K << " and multiplied by scale parameter " << _asym_alpha_concentration_parameter << endl;
		}
		// not all alpha's may be in the file
		// one can only specify the max alpha_k's
		double other_alphas = 0.1;
		if ( k < _K ) {
			for ( size_t i = k; i < _K; ++i )
				(*_asym_alpha)[i] = other_alphas;
		}
		// now change all alphas based on concentration parameter
		vector<double>& a = (*_asym_alpha);
		double sum_a = accumulate(a.begin(), a.end(), 0.0);

		for ( size_t i = 0; i < _K; ++i )
			(*_asym_alpha)[i] = _asym_alpha_concentration_parameter*(a[i]/sum_a);

		fs.close();
	}

	//for ( size_t k = 0; k < _K; ++k )
	//	cout << (*_asym_alpha)[k] << " ";
	//cout << endl;

	status = true;
	return status;
}

/**
 * @brief create and initialize the data structures for hidden variables
 */
bool CMMLDA::create_and_initialize_data_structures_for_hidden_variables_and_suff_stats() {
	bool status = false;
	/*
	 * create the phi variational matrix N_max x K
	 */
	//cout << "Initializing var_phi.. " << endl;
	CTemplateMatrixMath<double>::create_2D_matrix(&_var_phi, _n_max_corr_datum_in_a_doc, _K);

	//cout << "Initializing var_lambda.. " << endl;
	CTemplateMatrixMath<double>::create_2D_matrix(&_var_lambda, _n_max_datum_in_a_doc, _K);

	/*
	 * create the gamma variational vector D x K (alpha substitute for topic proportions)
	 */
	//cout << "Initializing var_gamma.. " << endl;
	CTemplateMatrixMath<double>::create_2D_matrix(&_var_gamma, _D, _K);

	//          Sufficient Statistics ----------------
	/*
	 * create the rho parameter suff stat matrix
	 */
	//cout << "Initializing rho_ss.. " << endl;
	CTemplateMatrixMath<double>::create_2D_matrix(&_rho_ss, _K, _corrV);
	_rho_total_ss = new vector<double>(_K, 0.0);

	/*
	 * create the beta parameter suff stat matrix
	 */
	//cout << "Initializing beta_ss.. " << endl;
	CTemplateMatrixMath<double>::create_2D_matrix(&_beta_ss, _K, _V);
	_beta_total_ss = new vector<double>(_K, 0.0);

	if ( _b_use_asym_alpha ) _asym_alpha_ss = new vector<double>(_K, 0.0);
	if ( _b_use_asym_alpha ) _asym_alpha_exp_ss = new vector<double>(_K, 0.0);
	if ( _b_use_asym_alpha ) _asym_alpha_exp_square_ss = new vector<double>(_K, 0.0);

	// not really important now
	status = true;
	return status;
}

/**
 * This function reads the configuration settings for the model
 * the configuration file looks like
 *
 */
void CMMLDA::read_config_file(string model_config_filename) {
	if (b_verbose_global) cout << "model is reading from config file " << model_config_filename << " ... "  ;

	vector<string> token_vec;
	string delimiter = ":";

	int num_topics, num_em_iterations, num_em_var_iterations;
	double topic_prop_prior;

	string line;
	ifstream fs(model_config_filename.c_str());
	if ( fs.is_open() ) {

		while ( !fs.eof() ) {
			getline( fs, line );
			//if (b_verbose_global) std::cout << line << endl;	//---

			if ( line.length() == 0 ) continue;				// ignore new lines
			if ( line.find_first_of('#') == 0 ) continue;	// ignore comment lines

			token_vec.clear();
			CUtilities::tokenize(line, token_vec, delimiter);
			if ( token_vec.size() != 2 ) { if (b_verbose_global) std::cout << "skipping this line (" << line << ") in model config file... not in correct format.." << endl; continue; }
			CUtilities::trim(token_vec[0]);
			CUtilities::trim_left_right(token_vec[1]);

			if ( token_vec[0].find("inputfile") != string::npos ) {
				_model_input_filename = token_vec[1];
			}

			if ( token_vec[0].find("datainputmode") != string::npos )
				_data_input_mode = token_vec[1];

			if ( token_vec[0].find("inputdic") == 0 ) {
				_datum_to_id_dict_filename = token_vec[1];
			}

			if ( token_vec[0].find("correspondinginputdic") == 0 ) {
				_corr_datum_to_id_dict_filename = token_vec[1];
			}

			if ( token_vec[0].find("modelrootdir") != string::npos ) {
				_model_root_dir = token_vec[1];
				CUtilities::make_directory(_model_root_dir);
			}

			if ( token_vec[0].find("modeltrainrootdir") != string::npos ) {
				_model_train_root_dir = token_vec[1];
				CUtilities::make_directory(_model_train_root_dir);
			}

			if ( token_vec[0].find("numtopics") != string::npos ) {
				CUtilities::string_to_number(num_topics, token_vec[1], std::dec);
				_K = num_topics;
			}

			if ( token_vec[0] == "emiter" ) {
				CUtilities::string_to_number(num_em_iterations, token_vec[1], std::dec);
				_n_em_iter = num_em_iterations;
			}

			if ( token_vec[0] == "emvariter" ) {
				CUtilities::string_to_number(num_em_var_iterations, token_vec[1], std::dec);
				_n_em_var_iter = num_em_var_iterations;
			}

			if ( token_vec[0].find("topicproportionprior") != string::npos ) {
				CUtilities::string_to_number<double>(topic_prop_prior, token_vec[1], std::dec);
				_alpha = topic_prop_prior;
			}

			if ( token_vec[0].find("useasymalpha") != string::npos ) {
				int alpha_asym_choice = 0;
				CUtilities::string_to_number<int>(alpha_asym_choice, token_vec[1], std::dec);
				_b_use_asym_alpha = (alpha_asym_choice == 0) ? false : true;
			}

			if ( token_vec[0].find("asymalphafile") != string::npos ) {
				_asym_alpha_initial_file = token_vec[1];
			}

			if ( token_vec[0].find("asymalphaconcentrationparam") != string::npos ) {
				CUtilities::string_to_number<double>(_asym_alpha_concentration_parameter, token_vec[1], std::dec);
			}

		}
		fs.close();
	} else {
		if (b_verbose_global) cout << "FATAL ERROR: Couldn't open " << model_config_filename << " .. exiting..." << endl;
		exit(APPLICATIONEXIT);
	}
	assert(_K != 0);

	if (b_verbose_global) cout << "done" << endl;
}

/**
 *	printing for diagnostics
 */
void CMMLDA::print_parameters() {
	if (b_verbose_global) cout << "\nPRINTING MODEL SETTINGS............................... " << endl;
	if (b_verbose_global) cout << "input file = " << _model_input_filename << endl;
	if (b_verbose_global) cout << "model root = " << _model_root_dir << endl;
	if (b_verbose_global) cout << "num topics = " << _K << endl;
	if (b_verbose_global) cout << "num iterations = " << _n_em_iter << endl;
	if (b_verbose_global) cout << "num var iter = " << _n_em_var_iter << endl;

	if (b_verbose_global) cout << "model num terms: " << _V << endl;
	if (b_verbose_global) cout << "model num corr terms: " << _corrV << endl;

	if (b_verbose_global) cout << "max num terms in a doc: " << _n_max_datum_in_a_doc << endl;
	if (b_verbose_global) cout << "max num corr terms in a doc: " << _n_max_corr_datum_in_a_doc << endl;

	if (b_verbose_global) cout << "Rho is " << _log_rho->size() << "x" << _log_rho->at(0)->size() << endl;
	if (b_verbose_global) cout << "Beta is " << _log_beta->size() << "x" << _log_beta->at(0)->size() << endl;
	if (b_verbose_global) {
		if ( !_b_use_asym_alpha ) cout << "Alpha is " << _alpha << endl;
		else if ( _b_use_asym_alpha ) {
			cout << "Alpha is " ;
			for ( size_t i = 0; i < _K; ++i ) cout << (*_asym_alpha)[i] << " ";
			cout << endl;
		}
	}
}

/**
 * saves the parameter matrices in a text files
 */
void CMMLDA::save_model() {
	if ( b_verbose_global ) cout << "saving model files in " << _model_root_dir << " ..." ;
	string extension = ".txt";

	// the model.other file will contain all one dimensional parameters i.e. the scalars
	string model_other_filename = _model_root_dir + CUtilities::_file_path_separator + "model.other";

	// the model.theta file will contain the document topic proportion matrix
	string model_theta_filename = _model_root_dir + CUtilities::_file_path_separator + "model.theta";
	_model_train_saved_topic_mult_filename = _model_root_dir + CUtilities::_file_path_separator + "model.theta";

	// the model.beta file will contain the word topic distribution matrix
	string model_beta_filename = _model_root_dir + CUtilities::_file_path_separator + "model.beta";

	// the model.pi file will contain the word topic distribution matrix
	string model_rho_filename = _model_root_dir + CUtilities::_file_path_separator + "model.rho";

	// the model.asym_alpha filename
	string model_asym_alpha_filename =  _model_root_dir + CUtilities::_file_path_separator + "model.asymalpha";

	// the model.wordassign file will contain the word topic distribution matrix
	string model_word_assign_filename = _model_root_dir + CUtilities::_file_path_separator + "model.wordtopicassign";

	// the model.wordassign file will contain the word topic distribution matrix
	string model_corr_word_assign_filename = _model_root_dir + CUtilities::_file_path_separator + "model.corrwordtopicassign";

	// other
	FILE * fout_model_other = fopen(model_other_filename.c_str(), "w");
	if ( !fout_model_other ) {
		printf("Cannot open file %s to save!\n", model_other_filename.c_str());
		return ;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////// write parameters and hidden variables here
	CParameterMatrixReaderWriter pmrw;
	// theta
	pmrw.write_parameter_matrix_in_text_mode(model_theta_filename, _var_gamma);
	// beta
	if ( _operation_mode == "train" ) pmrw.write_parameter_matrix_in_text_mode(model_beta_filename, _log_beta);
	// rho
	if ( _operation_mode == "train" ) pmrw.write_parameter_matrix_in_text_mode(model_rho_filename, _log_rho);

	cout << "writing other parameters to file ... ";
	string abs_output_file_path = _model_root_dir + CUtilities::_file_path_separator + "model.other";
	ofstream outstream(abs_output_file_path.c_str());
	if ( outstream.is_open() ) {
		outstream << "model alpha: " << _alpha << endl;
		outstream << "model num topics: " << _K << endl;
		outstream << "model num corr terms: " << _corrV << endl;
		outstream << "model num terms: " << _V << endl;

		int use_asym_alpha = 0;
		if ( _b_use_asym_alpha ) use_asym_alpha = 1;
		outstream << "model use asym alpha: " << _b_use_asym_alpha << endl;

		if ( _b_use_asym_alpha ) {
			outstream << "model asym alpha file: " << model_asym_alpha_filename << endl;
			pmrw.write_parameter_matrix_in_text_mode(model_asym_alpha_filename, _asym_alpha);
		}

		outstream.close();
	} else {
		cout << "couldn't open file " << abs_output_file_path << " for writing.\n" <<
				"Possible problems with directory permissions or directory doesn't exist" << endl;
	}
	cout << "done." << endl;
}

/**
 * loads up the parameters from the text files
 */
bool CMMLDA::load_model() {
	// load alpha, _V and _K
	int n_items_read;
	int use_asym_alpha = 0;
	char model_asymalpha_filename[512];
	CParameterMatrixReaderWriter pmrw;

	FILE* fileptr;
	string filename = _model_train_root_dir + CUtilities::_file_path_separator + "model.other";
	if ( (fileptr = fopen(filename.c_str(), "r")) ) {
		n_items_read = fscanf(fileptr, "model alpha: %lf\n", &_alpha);		// overwrites model value set in constructor
		n_items_read = fscanf(fileptr, "model num topics:  %zd\n", &_K);	// saved in file model.other
		n_items_read = fscanf(fileptr, "model num corr terms:  %zd\n", &_corrV);		// saved in file model.other
		n_items_read = fscanf(fileptr, "model num terms: %zd\n", &_V);		// this is the vocabulary size during training - overwrites
		// 		corpus value so that _log_beta has correct dimensions
		n_items_read = fscanf(fileptr, "model use asym alpha: %d\n", &use_asym_alpha);		// integer indicator for using asym alpha - needs to be converted to bool
		if ( use_asym_alpha == 0 ) _b_use_asym_alpha = false; else _b_use_asym_alpha = true;
		if ( _b_use_asym_alpha ) {
			n_items_read = fscanf(fileptr, "model asym alpha file: %s\n", model_asymalpha_filename);
			_asym_alpha = pmrw.read_param_vector_from_text_file(model_asymalpha_filename);
		}
		fclose(fileptr);
	} else
		cout << "couldn't open the file : " << filename << endl;

	_log_beta = pmrw.read_param_matrix_from_text_file(_model_train_root_dir + CUtilities::_file_path_separator + "model.beta");
	_log_rho   = pmrw.read_param_matrix_from_text_file(_model_train_root_dir + CUtilities::_file_path_separator + "model.rho");

	return true;
}

/**
 * @brief initialize the corpus data structure with a seeded set of documents
 */
void CMMLDA::corpus_initialize()
{
	size_t i, k, d, n, m, j;
	SMMLDADocument* doc;

	size_t NUM_INIT = 1;

	// choose a random set of documents
	vector<int> rand_docs;
	for (k = 0; k < _K; k++)
		for (i = 0; i < NUM_INIT; i++) {
			d = (size_t)floor(myrand_Cokus() * _D);
			cout << "initialized with document "<< d << endl;
			rand_docs.push_back(d);
		}
	// ------ done with document choosing
	cout << "corpus initializing beta_ss and rho_ss.. " << endl;
	for (k = 0; k < _K; ++k) {
		doc = _corpus->_document_vec->at(rand_docs.at(k));

		for (m = 0; m < doc->_num_of_datum; ++m) {
			_beta_ss->at(k)->at(doc->_datum_id_vec->at(m)) += doc->_datum_id_count_vec->at(m);
		}

		for ( n = 0; n < doc->_num_of_corr_datum; ++n ) {
			_rho_ss->at(k)->at(doc->_corr_datum_id_vec->at(n)) += doc->_corr_datum_id_count_vec->at(n);
		}
	}
	for (k = 0; k < _K; ++k) {
		for ( j = 0; j < _V; ++j ) {
			_beta_ss->at(k)->at(j) += 1.0;
			_beta_total_ss->at(k) += _beta_ss->at(k)->at(j);
		}

		for ( j = 0; j < _corrV; ++j ) {
			_rho_ss->at(k)->at(j) += 1.0;
			_rho_total_ss->at(k) += _rho_ss->at(k)->at(j);
		}
	}

}

/**
 * @brief initialize the corpus data structure with a seeded set of documents
 */
void CMMLDA::random_initialize() {

	size_t k, j;
	/*
	 * fill up the _beta_ss_ and rho_ss sufficient statistics matrix
	 */
	cout << "random initializing beta_ss and rho_ss.. " << endl;
	for ( k = 0; k < _K; ++k )	{
		for ( j = 0; j < _V; ++j ) {
			_beta_ss->at(k)->at(j) = 1.0/_V + myrand_Cokus();
			_beta_total_ss->at(k) += _beta_ss->at(k)->at(j);
		}
		for ( j = 0; j < _corrV; ++j ) {
			_rho_ss->at(k)->at(j) = 1.0/_corrV + myrand_Cokus();
			_rho_total_ss->at(k) += _rho_ss->at(k)->at(j);
		}
	}

}

/**
 * @brief computes the likelihood of the data under the model
 * @return the likelihood of the data under the model as a double
 */
double CMMLDA::compute_log_likelihood() {
	double log_likelihood = 0;
	for ( size_t d = 0; d < _D; ++d )
		log_likelihood += compute_doc_log_likelihood(d, _corpus->_document_vec->at(d));

	return log_likelihood;
}

/**
 * @brief computes the likelihood for each document under the model
 * @return the likelihood for each document under the model as a double
 */
double CMMLDA::compute_doc_log_likelihood(size_t doc_id, SMMLDADocument* doc) {
	double log_likelihood = 0.0;
	size_t k, n, m;
	double digamma_gamma[_K];
	double var_gamma_sum = 0.0;
	double digamma_gamma_sum = 0.0;

	// pre-computing di_gammas
	for ( k = 0; k < _K; ++k) {
		digamma_gamma[k] = CMathUtilities::digamma( (*(*_var_gamma)[doc_id])[k] );
		var_gamma_sum += (*(*_var_gamma)[doc_id])[k];
	}
	digamma_gamma_sum = CMathUtilities::digamma(var_gamma_sum);

	//cout << "*likelihood = " << likelihood << endl;	// ^^^^^^^^

	// -------- Equ. ... ---------
	if ( !_b_use_asym_alpha ) {
		log_likelihood += lgamma(_K*_alpha) - _K*lgamma(_alpha);
	} else {
		double alpha_sum = 0;
		for ( k = 0; k < _K; ++k ) {
			alpha_sum += (*_asym_alpha)[k];
			log_likelihood -= lgamma((*_asym_alpha)[k]);
		}
		log_likelihood += lgamma(alpha_sum);
	}

	for ( k = 0; k < _K; ++k ) {
		if ( !_b_use_asym_alpha ) log_likelihood += (_alpha - 1)*(digamma_gamma[k] - digamma_gamma_sum);
		else log_likelihood += ((*_asym_alpha)[k] - 1)*(digamma_gamma[k] - digamma_gamma_sum);

		log_likelihood += lgamma( (*(*_var_gamma)[doc_id])[k] );
		log_likelihood -= ( (*(*_var_gamma)[doc_id])[k] - 1)*(digamma_gamma[k]-digamma_gamma_sum);
	}
	log_likelihood -= lgamma(var_gamma_sum);
	assert(!isnan(log_likelihood));
	double phi_nk, lambda_mk;
	for ( n = 0; n < doc->_num_of_corr_datum; ++n ) {

		for ( k = 0; k < _K; ++k ) {

			phi_nk = (*(*_var_phi)[n])[k];
			if ( phi_nk > 0 ) {
				log_likelihood += phi_nk *
						(
								(digamma_gamma[k]-digamma_gamma_sum)
								- log(phi_nk)
								+ (*(*_log_rho)[k])[ (*doc->_corr_datum_id_vec)[n] ]
						) * (*doc->_corr_datum_id_count_vec)[n];
			}
		}
	}

	for ( m = 0; m < doc->_num_of_datum; ++m ) {

		for ( k = 0; k < _K; ++k ) {
			lambda_mk = (*(*_var_lambda)[m])[k];
			if ( lambda_mk > 0 ) {
				log_likelihood += lambda_mk *
				(
						(digamma_gamma[k]-digamma_gamma_sum)
						- log(lambda_mk)
						+ (*(*_log_beta)[k])[ (*doc->_datum_id_vec)[m] ]
				) * (*doc->_datum_id_count_vec)[m];
			}
		}
	}

	//cout << "asserting from compute_doc_likelihood " << log_likelihood << endl;
	assert(!isnan(log_likelihood));
	return log_likelihood;
}

