/*
 * CParameterMatrixReaderWriter.cpp
 *
 *  Created on: Jun 16, 2010
 *      Author: pdas
 */

#include "CParameterMatrixReaderWriter.h"
#include "../Printer/CPrinter.h"
#include "../DataSet/CDictionaryReader.h"
#include "../Utilities/CUtilities.h"
#include "../Utilities/TemplateAlgo.h"
#include "../Utilities/TemplateMath.h"

using namespace std;

CParameterMatrixReaderWriter::CParameterMatrixReaderWriter() {
	_param_mat_tracking_vec = new vector<vector<vector<double>* >* >();
}

CParameterMatrixReaderWriter::~CParameterMatrixReaderWriter() {
	if (_param_mat_tracking_vec ) {
		/*size_t num_of_matrices = _param_mat_tracking_vec->size();
		for ( size_t m = 0; m < num_of_matrices; ++m ) {
			vector<vector<double>* >* param_mat = _param_mat_tracking_vec->at(m);
			if ( param_mat != 0 ) {
				size_t num_rows = param_mat->size();
				for ( size_t i = 0; i < num_rows; i++ )
					delete param_mat->at(i);
				delete param_mat;
			}
		}*/
		delete _param_mat_tracking_vec;
	}
}

/**
 *
 */
void CParameterMatrixReaderWriter::print_topics(string parameter_matrix_store_filepath,
															string datum_label_index_map_filepath,
															size_t top_n, ostream& ofs) {
	map<size_t, string>* datum_index_label_map = 0;

	if ( datum_label_index_map_filepath == "" ) {
		if (b_verbose_global) std::cout << "no 'datum_label_index_map_filepath' exists.. returning" << endl;
		return;
	} else {
		datum_index_label_map = CDictionaryReader::read_datum_label_to_id_dictionary_from_text_file(datum_label_index_map_filepath);
	}

	CPrinter prn;
	prn.set_printing_stream(ofs);
	prn.sort_and_print_matrix<double, size_t, string>(parameter_matrix_store_filepath, *datum_index_label_map, top_n);
	if ( datum_index_label_map ) delete datum_index_label_map;
}

/**
 *
 */
void CParameterMatrixReaderWriter::print_topics(vector<vector<double>* >* parameter_matrix,
															string datum_label_index_map_filepath,
															size_t top_n, ostream& ofs) {
	map<size_t, string>* datum_index_label_map = 0;

	if ( datum_label_index_map_filepath == "" ) {
		if (b_verbose_global) std::cout << "no 'datum_label_index_map_filepath' exists.. returning" << endl;
		return;
	} else {
		datum_index_label_map = CDictionaryReader::read_datum_label_to_id_dictionary_from_text_file(datum_label_index_map_filepath);
		if ( datum_index_label_map->size() == 0 ) {
			if (b_verbose_global) std::cout << "Cannot print topics because 'datum_label_index_map' is of size 0.. did your dictionary files get overwritten?" << endl;
			return;
		}
	}

	CPrinter prn;
	prn.set_printing_stream(ofs);
	prn.sort_and_print_matrix<double, size_t, string>(*parameter_matrix, *datum_index_label_map, top_n);
	if ( datum_index_label_map ) delete datum_index_label_map;
}


/**
 *
 */
void CParameterMatrixReaderWriter::print_topics(vector<vector<double>* >* parameter_matrix,
															map<size_t, size_t>& test_term_id_to_sequential_index_id_map,
															string datum_label_index_map_filepath,
															size_t top_n, ostream& ofs) {
	map<size_t, string>* datum_index_label_map = 0;

	if ( datum_label_index_map_filepath == "" ) {
		if (b_verbose_global) std::cout << "no 'datum_label_index_map_filepath' exists.. returning" << endl;
		return;
	} else {
		datum_index_label_map = CDictionaryReader::read_datum_label_to_id_dictionary_from_text_file(datum_label_index_map_filepath);
		if ( datum_index_label_map->size() == 0 ) {
			if (b_verbose_global) std::cout << "Cannot print topics because 'datum_label_index_map' is of size 0.. did your dictionary files get overwritten?" << endl;
			return;
		}
	}

	// create another datum_index_label_map that maps the indices [0,...,_V_test-1] to the correct word labels
	map<size_t, string> datum_index_label_subset_map;
	map<size_t, size_t>::iterator m_itr;
	map<size_t, string>::iterator d_itr;
	size_t test_term_sequential_index, test_term_id;
	string word_label;
	for ( m_itr = test_term_id_to_sequential_index_id_map.begin() ; m_itr != test_term_id_to_sequential_index_id_map.end() ; ++m_itr ) {
		test_term_id = m_itr->first;
		test_term_sequential_index = m_itr->second;
		d_itr = datum_index_label_map->find(test_term_id);
		if ( d_itr == datum_index_label_map->end() ) {
			if (b_verbose_global) std::cout << "test term with id " << test_term_id << " is not found in the datum_index_label_map" << endl;
		} else word_label = d_itr->second;

		datum_index_label_subset_map.insert( make_pair(test_term_sequential_index, word_label) );
	}

	CPrinter prn;
	prn.set_printing_stream(ofs);
	prn.sort_and_print_matrix<double, size_t, string>(*parameter_matrix, datum_index_label_subset_map, top_n);
	if ( datum_index_label_map ) delete datum_index_label_map;
}

/**
 * Read from a text file of a matrix of space-separated doubles (each line representing a row of the matrix)
 * @ param the full filepath to the parameter matrix TEXT file
 */
vector<vector<double>* >* CParameterMatrixReaderWriter::read_param_matrix_from_text_file(string parameter_matrix_store_filepath) {
	ifstream ifs;
	string line;
	vector<string> token_vec;
	string delim = " \t\r\n";

	double param_value;
	size_t dataset_size = 0;

	vector<vector<double>* >* param_mat = new vector<vector<double>* >();

	ifs.open(parameter_matrix_store_filepath.c_str());
	if ( ifs.is_open() ) {
		while ( !ifs.eof() ) {
			getline( ifs, line );

			if ( line == "" ) continue;

			token_vec.clear();
			CUtilities::tokenize(line, token_vec, delim);
			if ( dataset_size != 0 ) {
				if ( token_vec.size() != dataset_size ) {
					if (b_verbose_global) std::cout << "parameter matrix is not well formed" << endl; exit(DATALOADEREXIT);
				}
			} else if ( dataset_size ==0 ) {
				dataset_size = token_vec.size();
			}

			// create the new vector row
			vector<double>* v = new vector<double>(token_vec.size());
			for ( size_t j = 0 ; j < token_vec.size(); ++j ) {
				CUtilities::string_to_number(param_value, token_vec[j], std::dec);
				v->at(j) = param_value;
			}
			param_mat->push_back(v);
		}
		ifs.close();
	}

	_param_mat_tracking_vec->push_back(param_mat);

	return param_mat;
}

vector<double>* CParameterMatrixReaderWriter::read_param_vector_from_text_file(string parameter_vector_store_filepath) {

	vector<double>* param_vec = new vector<double>();
	// read the file containing the asymmetric values
	// NOTE: if this variable is true, then this vector also needs to be saved
	string line;
	size_t k = 0;
	vector<string> token_vec;
	ifstream fs(parameter_vector_store_filepath.c_str());
	if ( fs.is_open() ) {

		while ( !fs.eof() ) {
			getline( fs, line );

			if ( line.length() == 0 ) continue;				// ignore new lines
			CUtilities::tokenize(line, token_vec, " \n\r\t");
			for ( size_t i = 0; i < token_vec.size(); ++i ) {
				double value;
				CUtilities::string_to_number<double>(value, token_vec[i], std::dec);
				param_vec->push_back(value);
				++k;
			}
		}
	}
	fs.close();

	return param_vec;
}

/**
 * Read from a binary file of a matrix of doubles (each line representing a row of the matrix)
 * @ param the full filepath to the parameter matrix BINARY file
 */
vector<vector<double>* >* CParameterMatrixReaderWriter::read_param_matrix_from_binary_file(string parameter_matrix_store_filepath) {
	//TODO: reading from a binary formatted matrix
	vector<vector<double>* >* param_mat = new vector<vector<double>* >();

	_param_mat_tracking_vec->push_back(param_mat);
	return param_mat;
}

/**
 * this function just writes out an "matrix" of doubles to a file in ascii text mode
 */
void CParameterMatrixReaderWriter::write_parameter_matrix_in_text_mode(string filename, vector<vector<double>* >* param_mat) {

	FILE * fout_model_param = fopen(filename.c_str(), "w");
	if ( !fout_model_param ) {
		printf("Cannot open file %s to save!\n", filename.c_str());
		return ;
	}

	size_t n_rows = param_mat->size();
	size_t n_cols = param_mat->at(0)->size();

	for ( size_t i = 0; i < n_rows; ++i ) {
		for ( size_t j = 0; j < n_cols; ++j )
			fprintf( fout_model_param, " %10.6f", (*(*param_mat)[i])[j] );
		fprintf( fout_model_param, "\n");
	}

	if ( fout_model_param ) fclose( fout_model_param );
}

/**
 * this function just writes out an "matrix" of doubles to a file in ascii text mode
 */
void CParameterMatrixReaderWriter::write_parameter_matrix_in_text_mode(string filename, vector<double>* param_vec) {

	FILE * fout_model_param = fopen(filename.c_str(), "w");
	if ( !fout_model_param ) {
		printf("Cannot open file %s to save!\n", filename.c_str());
		return ;
	}

	size_t n_cols = param_vec->size();

	for ( size_t j = 0; j < n_cols; ++j )
		fprintf( fout_model_param, " %10.6f", (*param_vec)[j] );

	if ( fout_model_param ) fclose( fout_model_param );
}

/**
 * TODO: this function just writes out an "matrix" of doubles to a file in binary mode
 */
void CParameterMatrixReaderWriter::write_parameter_matrix_in_binary_mode(string filename, vector<vector<double>* >* param_mat) {

	FILE * fout_model_param = fopen(filename.c_str(), "wb");
	if ( !fout_model_param ) {
		printf("Cannot open file %s to save!\n", filename.c_str());
		return ;
	}

	if ( fout_model_param ) fclose( fout_model_param );
}

vector<string> CParameterMatrixReaderWriter::expand_topic_multinomial_for_entire_corpus(
		string model_train_saved_topic_mult_filename,
		size_t index,
		string datum_label_id_map_filepath,
		int top_n,
		string model_dump_mode) {

	map<size_t, string>* datum_id_label_map = 0;
	vector<string> expanded_topic_multinomial_string_vec;

	if ( datum_label_id_map_filepath == "" ) {
		if (b_verbose_global) std::cout << "no 'datum_label_index_map_filepath' exists.. returning" << endl;
		return expanded_topic_multinomial_string_vec;
	} else {
		datum_id_label_map = CDictionaryReader::read_datum_label_to_id_dictionary_from_text_file(datum_label_id_map_filepath);
		if ( datum_id_label_map->size() == 0 ) {
			if (b_verbose_global) std::cout << "Cannot print topics because 'datum_label_index_map' is of size 0.. did your dictionary files get overwritten?" << endl;
			return expanded_topic_multinomial_string_vec;
		}
	}

	static vector<vector<double>* >* parameter_matrix = 0;
	static bool topic_multinomial_loaded_flag;

	assert( model_train_saved_topic_mult_filename != "" );
	if ( (model_dump_mode == "text") || (model_dump_mode == "txt") ) {
		if ( !topic_multinomial_loaded_flag ) {
			parameter_matrix = read_param_matrix_from_text_file(model_train_saved_topic_mult_filename);
			topic_multinomial_loaded_flag = true;
		}

	} else if ( (model_dump_mode == "binary") || (model_dump_mode == "bin") ) {
		if (b_verbose_global) std::cout << "Binary model dump mode is not yet supported! To be implemented as and when need arises" << endl;
		return expanded_topic_multinomial_string_vec;

	} else {
		if (b_verbose_global) std::cout << "Model dump mode is not yet recognized!" << endl;
		return expanded_topic_multinomial_string_vec;
	}

	expanded_topic_multinomial_string_vec =
	CTemplateAlgo::get_top_K_data_from_vector_using_heap<double, size_t, string>(
			*(parameter_matrix->at(index)),
			*datum_id_label_map,
			top_n);
	if ( datum_id_label_map ) delete datum_id_label_map;
	return expanded_topic_multinomial_string_vec;
}

/**
 *
 */
vector<string> CParameterMatrixReaderWriter::expand_topic_multinomial_for_this_document(
		string model_train_saved_topic_mult_filename,
		vector<double>* topic_prop_mult_of_this_doc_vec,
		string datum_label_id_map_filepath,
		int top_n,
		string model_dump_mode) {

	map<size_t, string>* datum_id_label_map = 0;
	vector<string> expanded_topic_multinomial_string_for_this_doc_vec;

	if ( datum_label_id_map_filepath == "" ) {
		if (b_verbose_global) std::cout << "no 'datum_label_index_map_filepath' exists.. returning" << endl;
		return expanded_topic_multinomial_string_for_this_doc_vec;
	} else {
		datum_id_label_map = CDictionaryReader::read_datum_label_to_id_dictionary_from_text_file(datum_label_id_map_filepath);
		if ( datum_id_label_map->size() == 0 ) {
			if (b_verbose_global) std::cout << "Cannot print topics because 'datum_label_index_map' is of size 0.. did your dictionary files get overwritten?" << endl;
			return expanded_topic_multinomial_string_for_this_doc_vec;
		}
	}

	static vector<vector<double>* >* parameter_matrix = 0;
	static bool topic_multinomial_loaded_flag;

	assert( model_train_saved_topic_mult_filename != "" );
	if ( (model_dump_mode == "text") || (model_dump_mode == "txt") ) {
		if ( !topic_multinomial_loaded_flag ) {
			parameter_matrix = read_param_matrix_from_text_file(model_train_saved_topic_mult_filename);
			topic_multinomial_loaded_flag = true;
		}

	} else if ( (model_dump_mode == "binary") || (model_dump_mode == "bin") ) {
		if (b_verbose_global) std::cout << "Binary model dump mode is not yet supported! To be implemented as and when need arises" << endl;
		return expanded_topic_multinomial_string_for_this_doc_vec;

	} else {
		if (b_verbose_global) std::cout << "Model dump mode is not yet recognized!" << endl;
		return expanded_topic_multinomial_string_for_this_doc_vec;
	}

	size_t num_topics, num_datum_in_voc;
	num_topics = parameter_matrix->size();
	num_datum_in_voc = parameter_matrix->at(0)->size();

	vector<double> weighted_topic_datum_multinomial(num_datum_in_voc);
	vector<double> temp(num_topics);
	for ( size_t j = 0; j < num_datum_in_voc; ++j ) {
		for ( size_t i = 0; i < num_topics; ++i )
			temp[i] = topic_prop_mult_of_this_doc_vec->at(i) * parameter_matrix->at(i)->at(j);

		weighted_topic_datum_multinomial[j] = *( std::max_element(temp.begin(), temp.end()) );
	}

	expanded_topic_multinomial_string_for_this_doc_vec =
			CTemplateAlgo::get_top_K_data_from_vector_using_heap<double, size_t, string>(
					weighted_topic_datum_multinomial,
					*datum_id_label_map,
					top_n);

	if ( datum_id_label_map ) delete datum_id_label_map;
	return expanded_topic_multinomial_string_for_this_doc_vec;
}

