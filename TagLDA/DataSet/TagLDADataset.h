/*
 * Dataset.h
 *
 *  Created on: May 21, 2010
 *      Author: pdas
 */

#ifndef TAGLDADATASET_H_
#define TAGLDADATASET_H_

#include "../Utilities/includes.h"
#include "../Utilities/CUtilities.h"

/**
 * @brief represents a document in a corpus
 */
struct STagLDADocument {

	/*!
	 * @brief the stl::std::vector representing the datum ids in a document
	 *
	 */
	std::vector<int> *_datum_id_vec;
	/*!
	 * @brief the stl::std::vector representing the counts of the datum ids in a document
	 */
	std::vector<int> *_datum_id_count_vec;
	/*!
	 * @brief The size of the _datum_id_vec or the _datum_id_count_vec (both are of the same size)
	 * @note If data is read from a file in sequential format, the name _num_of_unique_datum is sort of a misnomer
	 */
	size_t _num_of_datum;
	/*!
	 * @brief
	 */
	size_t _total_num_of_datum;
	/*!
	 * @brief This is the tag associated with each term id in the document
	 */
	std::vector<int> *_tag_id_vec;

	/**
	 * @brief The constructor sets:\n
	 * _datum_id_vec = new std::vector<int>()
	 * _datum_id_count_vec = new std::vector<int>()
	 */
	STagLDADocument() {
		_datum_id_vec = new std::vector<int>();
		_datum_id_count_vec = new std::vector<int>();
		_tag_id_vec = new std::vector<int>();
	}

	~STagLDADocument() {
		delete _datum_id_vec;
		delete _datum_id_count_vec;
		delete _tag_id_vec;
	}

	/**
	 * @brief returns a std::string dump of the data structure
	 *
	 * The std::string is formed as:
	 * \code
	 * std::string s = "";
	 * s += CUtilities::number_to_string<size_t>(_num_of_unique_datum);
	 *	for ( size_t i = 0; i < _num_of_unique_datum; ++i )
	 *		s += " " + CUtilities::number_to_string<int>((*_datum_id_vec)[i]) +
	 *				":" + CUtilities::number_to_string<int>((*_datum_id_tag_id_vec)[i]);
	 * \endcode
	 *
	 * @return  a std::string dump of the data structure which might be useful for debugging
	 */
	std::string to_string() {
		std::string s = "";
		s += CUtilities::number_to_string<size_t>(_num_of_datum);
		for ( size_t i = 0; i < _num_of_datum; ++i )
			s += " " + CUtilities::number_to_string<int>((*_datum_id_vec)[i]) +
					":" + CUtilities::number_to_string<int>((*_tag_id_vec)[i]);
		return s;
	}

};

/**
 * @brief A corpus is a std::vector of documents with each document being a std::vector of datumIDs and corresponding counts.\n
 * A typical model file, which is in grouped format looks like: (each line is a separate document)\n
 * \#unique_datum datum_id_1:datum_id_1_count datum_id_2:datum_id_2_count ...\n
 *
 * A typical model file, which is in sequential format looks like: (each line is a separate document)\n
 * \#unique_datum datum_id_at_position_1 datum_id_at_position_2 ...\n
 *
 */
struct STagLDACorpus {

	/*!
	 * a corpus is a std::vector of documents. _document_vec is a std::std::vector of pointers to SDocuments. @see SDocument
	 */
	std::vector<STagLDADocument*> *_document_vec;
	/*!
	 * The number of documents in this corpus
	 */
	size_t _n_docs;
	/*!
	 * The maximum number of datum in a single doc across all documents
	 */
	size_t _max_datum_in_a_doc;
	/*!
	 * The vocabulary size of the corpus
	 */
	size_t _total_datum_in_corpus;

	/**
	 * @brief constructor for SCorpus:\n
	 * 1. Creates a new SDocument* std::std::vector\n
	 * 2. Sets _n_docs to 0\n
	 * 3. Sets _max_datum_in_a_doc to 0\n
	 * 4. Sets _total_datum_in_corpus to 0
	 */
	STagLDACorpus() {
		_document_vec = new std::vector<STagLDADocument*>();
		_n_docs = 0;
		_max_datum_in_a_doc = 0;
		_total_datum_in_corpus = 0;
	}

	/**
	 * @brief deletes the std::std::vector of SDocument pointers
	 */
	~STagLDACorpus() {
		if ( _n_docs == 0 ) { if (b_verbose_global) std::cout << "n_docs in corpus is 0 - _n_docs might not have been set properly: possible memory leak?" << std::endl; }
		assert(_n_docs == _document_vec->size());
		_n_docs = _document_vec->size();
		for ( size_t d = 0; d < _n_docs; ++d )
			delete _document_vec->at(d);
		delete _document_vec;
	}

};

#endif /* TAGLDADATASET_H_ */
