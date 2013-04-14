/*
 * CDictionaryReader.h
 *
 *  Created on: Jun 16, 2010
 *      Author: pdas
 */

#ifndef CDICTIONARYREADER_H_
#define CDICTIONARYREADER_H_

#include "../Utilities/includes.h"
//using namespace std;

/**
 * @brief This class packages up
 * - read_datum_label_to_id_dictionary_from_text_file() \b static function useful for reading dictionaries in an adhoc manner
 */
class CDictionaryReader {
public:
	CDictionaryReader();
	~CDictionaryReader();
	/**
	 * @brief This function reads from the global datum_label=datum_id dictionary in file name supplied as a parameter
	 * @param filepath of the dictionary which contains lines like datum_label=datum_id
	 * @return a pointer to an stl::map containing <datum_id, datum_label> pairs
	 *
	 * @note
	 * This function is a static member function
	 */
	static std::map<size_t, std::string>* read_datum_label_to_id_dictionary_from_text_file(
			std::string filepath,
			std::string delimiter = "="
			);
};


#endif /* CDICTIONARYREADER_H_ */
