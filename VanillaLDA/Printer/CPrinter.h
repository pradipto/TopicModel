/*
 * CPrinter.h
 *
 *  Created on: May 19, 2010
 *      Author: pdas
 */

#ifndef CPRINTER_H_
#define CPRINTER_H_

#include "../Utilities/includes.h"
#include "../Utilities/CUtilities.h"

class CPrinter {

protected:
	std::ostream *_ofs;	// base class for std::ostreams


public:
	CPrinter();
	~CPrinter();

	inline void set_printing_stream(std::ostream& stream_type) { _ofs = &stream_type; }

	/**
	 *
	 */
	template<class T, class K, class V>
	void print_top_K_from_vector_using_multimap(std::vector<T>& row_vec,
												std::map<K, V>& datum_index_label_map,
												size_t num_datum_to_print,
												std::ostream& ofs) {
		T param_value;
		std::multimap<T, size_t, std::greater<T> > sorted_row_multmap;
		size_t top, dataset_size = row_vec.size();

		for ( size_t j = 0 ; j < dataset_size; ++j ) {
			param_value = row_vec.at(j);
			sorted_row_multmap.insert(std::pair<T, size_t>(param_value, j));
		}

		typename std::multimap<T, size_t, std::greater<T> >::iterator mm_itr;
		typename std::map<K, V>::iterator m_il_itr;

		K index;
		V datum_label;
		for ( mm_itr = sorted_row_multmap.begin(), top = 0; (mm_itr != sorted_row_multmap.end()) && (top < num_datum_to_print); ++mm_itr, ++top ) {
			index = mm_itr->second;
			m_il_itr = datum_index_label_map.find(index);
			ofs << " " << m_il_itr->second;
		}
		ofs << std::endl;

	}

	/**
	 * This is experimental..using a std::priority_queue instead of a std::multimap with std::greater
	 */
	template<class T, class N = size_t>
	struct compare_pair_T_size_t_ascending {
		bool operator() ( const std::pair<T, N>& p1, const std::pair<T, N>& p2 ) const { return p1.first < p2.first; }
	};
	template<class T, class N = size_t>
	struct compare_pair_T_size_t_descending {
		bool operator() ( const std::pair<T, N>& p1, const std::pair<T, N>& p2 ) const { return p1.first > p2.first; }
	};

	/**
	 *
	 */
	template<class T, class K, class V>
	void print_top_K_from_vector_using_priority_queue(std::vector<T>& row_vec,
											std::map<K, V>& datum_index_label_map,
											size_t num_datum_to_print,
											std::ostream& ofs) {

		T param_value;

		std::priority_queue<std::pair<T, size_t>, std::vector<std::pair<T, size_t> >, compare_pair_T_size_t_descending<T> > pq_queue;

		size_t top, dataset_size = row_vec.size();
		// make an initial heap out of the first num_data_to_print elements
		for ( size_t j = 0 ; j < num_datum_to_print; ++j ) {
			param_value = row_vec.at(j);
			pq_queue.push( make_pair(param_value, j) );
		}
		// modify the top-k heap
		std::pair<T, size_t> a_pair;
		for ( size_t j = num_datum_to_print ; j < dataset_size; ++j ) {
			param_value = row_vec.at(j);
			a_pair = pq_queue.top();
			if ( a_pair.first < param_value ) {
				pq_queue.pop();
				pq_queue.push( make_pair(param_value, j) );
			}
		}

		typename std::map<K, V>::iterator m_il_itr;
		K index;
		V datum_label;
		std::stack<V> stk;
		for ( top = 0; (!pq_queue.empty()) && (top < num_datum_to_print); ++top ) {
			a_pair = pq_queue.top();
			index = a_pair.second;
			m_il_itr = datum_index_label_map.find(index);
			//ofs << " " << m_il_itr->second;
			stk.push(m_il_itr->second);

			pq_queue.pop();	// NOTE: the pq_queue.pop() pops in ascending order
		}

		while ( ! stk.empty() ) {
			ofs << stk.top();
			stk.pop();
		}
		ofs << std::endl;
	}

	/**
	 *
	 */
	template<class T, class K, class V>
	void print_top_K_from_vector_using_heap(std::vector<T>& row_vec,
			std::map<K, V>& datum_index_label_map,
			size_t num_datum_to_print,
			std::ostream& ofs) {

		T param_value;

		std::vector<std::pair<T, size_t> > heap_vec;

		size_t dataset_size = row_vec.size();
		// make an initial heap out of the first num_data_to_print elements
		for ( size_t j = 0 ; j < num_datum_to_print; ++j ) {
			param_value = row_vec.at(j);
			heap_vec.push_back( std::make_pair(param_value, j) );
		}


		// modify the top-num_data_to_print heap
		std::make_heap(heap_vec.begin(), heap_vec.end(), compare_pair_T_size_t_descending<T>() );

		std::pair<T, size_t> a_pair;
		for ( size_t j = num_datum_to_print ; j < dataset_size; ++j ) {
			param_value = row_vec.at(j);
			a_pair = heap_vec.front();
			if ( a_pair.first < param_value ) {
				pop_heap(heap_vec.begin(), heap_vec.end(), compare_pair_T_size_t_descending<T>() );
				heap_vec.pop_back();

				heap_vec.push_back( make_pair(param_value, j) );
				push_heap(heap_vec.begin(), heap_vec.end(), compare_pair_T_size_t_descending<T>() );
			}
		}
		std::sort_heap(heap_vec.begin(), heap_vec.end(), compare_pair_T_size_t_descending<T>() );

		typename std::map<K, V>::iterator m_il_itr;
		K index;
		V datum_label;
		for ( size_t k = 0; k < num_datum_to_print; ++k ) {
			a_pair = heap_vec[k];
			index = a_pair.second;
			m_il_itr = datum_index_label_map.find(index);
			if ( m_il_itr == datum_index_label_map.end() )
				ofs << "UNKNOWN " ;
			else ofs << m_il_itr->second << " " ;
		}
		ofs << std::endl;
	}

	/**
	 *
	 */
	template<class T, class K, class V>
	void sort_and_print_matrix(std::vector<std::vector<T>* >& param_matrix, std::map<K, V>& datum_index_label_map, int top_n = 25) {

		std::ostream& ofs = *_ofs;


		std::vector<T>* row_vec;
		size_t num_datum_to_print = top_n, row;

		size_t dataset_size = param_matrix.at(0)->size();
		size_t n_factors = param_matrix.size();

		for ( row = 0; row < n_factors; ++row ) {

			row_vec = param_matrix.at(row);

			// iterate over the std::multimap for each of the top_n terms, look up the label index map and push it to _ofs
			if ( top_n == -1 ) num_datum_to_print = dataset_size;

			ofs << "TopicHeap" << row << ":";
			print_top_K_from_vector_using_heap(*row_vec, datum_index_label_map, num_datum_to_print, ofs);
		}

	}

	/**
	 *
	 */
	template<class T, class K, class V>
	void sort_and_print_matrix(std::string param_matrix_filename, std::map<K, V>& datum_index_label_map, int top_n = 25) {
		std::ostream& ofs = *_ofs;

		std::ifstream ifs;
		std::string line;
		std::vector<std::string> token_vec;
		std::string delim = " \t\r\n";

		std::vector<double> row_vec;

		double param_value; size_t num_datum_to_print = top_n, row;

		size_t dataset_size = 0;

		ifs.open(param_matrix_filename.c_str());
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
			if (b_verbose_global) std::cout << "unable to open file " << param_matrix_filename << std::endl;
		}
	}


};

#endif /* CPRINTER_H_ */
