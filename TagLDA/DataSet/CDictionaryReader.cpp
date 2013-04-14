/*
 * CDictionaryReader.cpp
 *
 *  Created on: Jun 16, 2010
 *      Author: pdas
 */

#include "CDictionaryReader.h"
#include "../Utilities/CUtilities.h"

using namespace std;

CDictionaryReader::CDictionaryReader() {

}

CDictionaryReader::~CDictionaryReader() {

}

/**
 * reads from the global term_string -> term_id dictionary in file name supplied as a parameter
 * @param filename of the dictionary which contains lines like datum_label=datum_id
 * @return a stl::map containing <datum_id, datum_label> pairs
 */
map<size_t, string>* CDictionaryReader::read_datum_label_to_id_dictionary_from_text_file
(
		string filepath,
		string delimiter
) {

	map<size_t, string>* datum_id_to_label_map = new map<size_t, string>();

	ifstream ifs;
	string line;
	vector<string> tokens_vec;
	size_t term_id;
	size_t lines_read = 0;

	size_t max_term_id = 0;

	string token;

	ifs.open( filepath.c_str() );
	if ( ifs.is_open() ) {
		lines_read = 0;
		while ( !ifs.eof() ) {
			getline( ifs, line );
			if ( line == "" ) continue;

			tokens_vec.clear();
			CUtilities::tokenize(line, tokens_vec, delimiter);

			if ( tokens_vec.size() >= 2 ) {
				CUtilities::string_to_number(term_id, tokens_vec[tokens_vec.size()-1], std::dec);
				token = "";
				for ( size_t i = 0; i < tokens_vec.size()-1; ++i )
					token += tokens_vec[i];

				(*datum_id_to_label_map)[term_id] = (token);
				++lines_read;
				if ( term_id > max_term_id ) {
					max_term_id = term_id;
				}
			} else if ( tokens_vec.size() == 1 ) {
				CUtilities::string_to_number(term_id, tokens_vec[0], std::dec);
				(*datum_id_to_label_map)[term_id] = delimiter;
				++lines_read;
				if ( term_id > max_term_id ) {
					max_term_id = term_id;
				}
			} else if (b_verbose_global) std::cout << "the line containing '" << line << "' is skipped since it doesn't contain two tokens separated by "<< delimiter << endl;
		}
		ifs.close();
	} else {
		if (b_verbose_global) std::cout << "couldn't open dictionary file " << filepath << " to write to..." << endl;
	}
	assert(datum_id_to_label_map->size() == lines_read);

	// insert the ID for the UNKNOWN word
	(*datum_id_to_label_map)[max_term_id+1] = "UNSEEN_WORD";
	return datum_id_to_label_map;
}
