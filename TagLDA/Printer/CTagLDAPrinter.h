/*
 * CTagLDAPrinter.h
 *
 *  Created on: Nov 24, 2010
 *      Author: data
 */

#ifndef CTAGLDAPRINTER_H_
#define CTAGLDAPRINTER_H_

#include "CPrinter.h"

class CTagLDAPrinter : public CPrinter {
public:
	CTagLDAPrinter();
	virtual ~CTagLDAPrinter();

	/**
	 * @brief prints the topic heap
	 */
	template<class T, class K, class V>
	void sort_and_print_matrix (
			std::vector<std::vector<T>* >& param_matrix_beta,
			std::vector<std::vector<T>* >& param_matrix_pi,
			std::map<K, V>& datum_index_label_map,
			std::map<K, V>& tag_index_label_map,
			int top_n = 25) {

		std::ostream& ofs = *_ofs;

		std::vector<T> *row_vec_beta, *row_vec_pi;
		size_t num_datum_to_print = top_n, row_beta, row_pi;

		size_t dataset_size = param_matrix_beta.at(0)->size();
		assert ( dataset_size == param_matrix_pi.at(0)->size() );

		std::vector<T> beta_pi_vec;

		size_t n_topics = param_matrix_beta.size();
		size_t n_tags = param_matrix_pi.size();

		std::string tag_label;
		typename std::map<K, V>::iterator tag_il_itr;

		for ( row_beta = 0; row_beta < n_topics; ++row_beta ) {

			row_vec_beta = param_matrix_beta.at(row_beta);
			beta_pi_vec.resize(row_vec_beta->size(), 0);

			for ( row_pi = 0; row_pi < n_tags; ++row_pi ) {

				row_vec_pi = param_matrix_pi.at(row_pi);

				// calculate the word generation probability here
				for ( size_t v = 0; v < dataset_size; ++v )
					beta_pi_vec.at(v) = exp(row_vec_beta->at(v) + row_vec_pi->at(v));

				// iterate over the std::multimap for each of the top_n terms, look up the label index map and push it to _ofs
				if ( top_n == -1 ) num_datum_to_print = dataset_size;

				if ( tag_index_label_map.size() == 0 ) tag_label = CUtilities::number_to_string<size_t>(row_pi);
				else {
					tag_il_itr = tag_index_label_map.find(row_pi);
					if ( tag_il_itr == tag_index_label_map.end() ) tag_label = "UnknownTag";
					else tag_label = tag_il_itr->second;
				}

				ofs << "TopicTagHeap(" << row_beta << ","<< tag_label << "):";
				print_top_K_from_vector_using_heap(beta_pi_vec, datum_index_label_map, num_datum_to_print, ofs);
			}
		}

	}

	/**
	 *
	 */
	template<class T, class K, class V>
	void sort_and_print_matrix (
			std::string param_matrix_filename_beta,
			std::string param_matrix_filename_pi,
			std::map<K, V>& datum_index_label_map,
			int top_n = 25 ) {
		std::ostream& ofs = *_ofs;

		std::ifstream ifs;
		std::string line;
		std::vector<std::string> token_vec;
		std::string delim = " \t\r\n";

		std::vector<double> row_vec;

		double param_value; size_t num_datum_to_print = top_n, row;

		size_t dataset_size = 0;

		ifs.open(param_matrix_filename_beta.c_str());
		if ( ifs.is_open() ) {
			row = 0;
			while ( !ifs.eof() ) {
				getline( ifs, line );

				if ( line == "" ) continue;

				token_vec.clear();
				CUtilities::tokenize(line, token_vec, delim);
				if ( dataset_size != 0 ) {
					if ( token_vec.size() != dataset_size ) {
						if (b_verbose_global) std::cout << "parameter matrix is not well formed" << std::endl; exit(DATALOADEREXIT);
					}
				} else if ( dataset_size ==0 ) {
					dataset_size = token_vec.size();
				}

				// clear and reuse the std::vector row_vec
				row_vec.clear();
				for ( size_t j = 0 ; j < token_vec.size(); ++j ) {
					CUtilities::string_to_number(param_value, token_vec[j], std::dec);
					row_vec.push_back(param_value);
				}

				if ( top_n == -1 ) num_datum_to_print = dataset_size;

				ofs << "Topic" << row << ":";
				print_top_K_from_vector_using_multimap(row_vec, datum_index_label_map, num_datum_to_print, ofs);


				++row;
			}
			ifs.close();
		} else {
			if (b_verbose_global) std::cout << "unable to open file " << param_matrix_filename_beta << std::endl;
		}
	}


};

#endif /* CTAGLDAPRINTER_H_ */
