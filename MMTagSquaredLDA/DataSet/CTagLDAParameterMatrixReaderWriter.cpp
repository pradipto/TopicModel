/*
 * CTagLDAParameterMatrixReaderWriter.cpp
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#include "CTagLDAParameterMatrixReaderWriter.h"
using namespace std;

CTagLDAParameterMatrixReaderWriter::CTagLDAParameterMatrixReaderWriter() {

}

CTagLDAParameterMatrixReaderWriter::~CTagLDAParameterMatrixReaderWriter() {

}

/**
 *
 */
void CTagLDAParameterMatrixReaderWriter::print_topics(	vector<vector<double>* >* topic_term_matrix,
														vector<vector<double>* >* tag_term_matrix,
														string datum_label_index_map_filepath,
														string tag_label_index_map_filepath,
														size_t top_n, ostream& ofs) {
	map<size_t, string> *datum_index_label_map = 0, *tag_index_label_map = 0;

	if ( (datum_label_index_map_filepath == "") || (tag_label_index_map_filepath == "") ) {
		if (b_verbose_global) std::cout << "problem with dictionary filepaths.. returning" << endl;
		return;
	} else {
		datum_index_label_map = CDictionaryReader::read_datum_label_to_id_dictionary_from_text_file(datum_label_index_map_filepath);
		tag_index_label_map = CDictionaryReader::read_datum_label_to_id_dictionary_from_text_file(tag_label_index_map_filepath);

		if ( datum_index_label_map->size() == 0 ) {
			if (b_verbose_global) std::cout << "Cannot print topics because 'datum_label_index_map' is of size 0.. did your dictionary files get overwritten?" << endl;
			return;
		}
	}

	CTagLDAPrinter prn;
	prn.set_printing_stream(ofs);
	prn.sort_and_print_matrix<double, size_t, string>(
			*topic_term_matrix,
			*tag_term_matrix,
			*datum_index_label_map,
			*tag_index_label_map,
			top_n);
	if ( datum_index_label_map ) delete datum_index_label_map;
	if ( tag_index_label_map ) delete tag_index_label_map;
}
