/*
 * TemplateAlgo.h
 *
 *  Created on: Mar 1, 2010
 *      Author: pdas3
 */

#ifndef TEMPLATEALGO_H_
#define TEMPLATEALGO_H_

#include "includes.h"

class CTemplateAlgo{

public:

	/**
	 * This is experimental..using a std::priority_queue instead of a std::multimap with std::greater
	 */
	template<class T, class N = size_t>
	struct compare_pair_T_size_t_ascending {
		bool operator() ( const std::pair<T, N>& p1, const std::pair<T, N>& p2 ) { return p1.first < p2.first; }
	};
	template<class T, class N = size_t>
	struct compare_pair_T_size_t_descending {
		bool operator() ( const std::pair<T, N>& p1, const std::pair<T, N>& p2 ) { return p1.first > p2.first; }
	};

	/**
	 *
	 */
	template<class T, class K, class V>
	static std::vector<std::string> get_top_K_data_from_vector_using_heap(
			std::vector<T>& row_vec,
			std::map<K, V>& datum_index_label_map,
			int num_expansions
	) {

		T param_value;
		size_t num_datum_to_expand = 0;
		if ( num_expansions >= 0 ) num_datum_to_expand = (size_t)num_expansions;

		std::vector< std::pair<T, size_t> > heap_vec;

		size_t dataset_size = row_vec.size();
		// make an initial heap out of the first num_data_to_print elements
		for ( size_t j = 0 ; j < num_datum_to_expand; ++j ) {
			param_value = row_vec.at(j);
			heap_vec.push_back( make_pair(param_value, j) );
		}

		// modify the top-num_data_to_print heap
		std::make_heap(heap_vec.begin(), heap_vec.end(), compare_pair_T_size_t_descending<T>() );

		std::pair<T, size_t> a_pair;
		for ( size_t j = num_datum_to_expand ; j < dataset_size; ++j ) {
			param_value = row_vec.at(j);
			a_pair = heap_vec.front();
			if ( a_pair.first < param_value ) {
				pop_heap(heap_vec.begin(), heap_vec.end(), compare_pair_T_size_t_descending<T>() );
				heap_vec.pop_back();

				heap_vec.push_back( make_pair(param_value, j) );
				push_heap(heap_vec.begin(), heap_vec.end(), compare_pair_T_size_t_descending<T>() );
			}
		}
		sort_heap(heap_vec.begin(), heap_vec.end(), compare_pair_T_size_t_descending<T>() );

		typename std::map<K, V>::iterator m_il_itr;
		K index;
		V datum_label;
		std::vector<std::string> expanded_multinomial_vec;
		for ( size_t k = 0; k < num_datum_to_expand; ++k ) {
			a_pair = heap_vec[k];
			index = a_pair.second;
			m_il_itr = datum_index_label_map.find(index);
			if ( m_il_itr->second == "UNSEEN_WORD" ) continue;
			expanded_multinomial_vec.push_back( m_il_itr->second ) ;
		}

		// return a full copy of the std::vector
		return expanded_multinomial_vec;
	}
};

#endif /* TEMPLATEALGO_H_ */
