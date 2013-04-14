/*
 * CParameterMatrixReaderWriter.h
 *
 *  Created on: Jun 16, 2010
 *      Author: pdas
 */

#ifndef CPARAMETERMATRIXREADERWRITER_H_
#define CPARAMETERMATRIXREADERWRITER_H_

#include "../Utilities/includes.h"


class CParameterMatrixReaderWriter {

	std::vector<std::vector<std::vector<double>* >* >* _param_mat_tracking_vec;

public:
	CParameterMatrixReaderWriter();
	~CParameterMatrixReaderWriter();

	/**
	 * Prints the top n words as topics from a two dimensional parameter matrix to console - auxiliary functions for diagnosis
	 * TODO: more to comment
	 */
	void print_topics(std::string parameter_matrix_store_filepath,
										std::string datum_label_index_map_filepath = "",
										size_t top_n = 60, std::ostream& ofs = std::cout);
	void print_topics(std::vector<std::vector<double>* >* parameter_matrix,
										std::string datum_label_index_map_filepath = "",
										size_t top_n = 60, std::ostream& ofs = std::cout);
	void print_topics(std::vector<std::vector<double>* >* parameter_matrix,
										std::map<size_t, size_t>& test_term_id_to_sequential_index_id_map,
										std::string datum_label_index_map_filepath = "",
										size_t top_n = 60, std::ostream& ofs = std::cout);

	/**
	 * main functions
	 */
	std::vector<std::vector<double>* >* read_param_matrix_from_text_file(std::string parameter_matrix_store_filepath);
	std::vector<std::vector<double>* >* read_param_matrix_from_binary_file(std::string parameter_matrix_store_filepath);
	std::vector<double>* read_param_vector_from_text_file(std::string parameter_vector_store_filepath);
	/**
	 * this function just writes out an "matrix" of doubles to a file in ascii text mode
	 */
	void write_parameter_matrix_in_text_mode(std::string filename, std::vector<std::vector<double>* >* param_mat);
	void write_parameter_matrix_in_text_mode(std::string filename, std::vector<double>* param_vec);
	/**
	 * TODO: this function just writes out an "matrix" of doubles to a file in binary mode
	 */
	void write_parameter_matrix_in_binary_mode(std::string filename, std::vector<std::vector<double>* >* param_mat);
	/**
	 *
	 */
	std::vector<std::string> expand_topic_multinomial_for_entire_corpus(
										std::string model_train_saved_topic_mult_filename,
										size_t index,
										std::string datum_label_id_map_filepath = "",
										int top_n = 25,
										std::string model_dump_mode = "text");
	/**
	 *
	 */
	std::vector<std::string> expand_topic_multinomial_for_this_document(
										std::string model_train_saved_topic_mult_filename,
										std::vector<double>* topic_prop_mult_of_this_doc_vec,
										std::string datum_label_id_map_filepath = "",
										int top_n = 25,
										std::string model_dump_mode = "text");

};

#endif /* CPARAMETERMATRIXREADERWRITER_H_ */
