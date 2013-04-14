/*
 * CTagLDAParameterMatrixReaderWriter.h
 *
 *  Created on: Nov 25, 2010
 *      Author: data
 */

#ifndef CTAGLDAPARAMETERMATRIXREADERWRITER_H_
#define CTAGLDAPARAMETERMATRIXREADERWRITER_H_

#include "CParameterMatrixReaderWriter.h"
#include "CDictionaryReader.h"
#include "../Printer/CTagLDAPrinter.h"

class CTagLDAParameterMatrixReaderWriter : public CParameterMatrixReaderWriter {
public:
	CTagLDAParameterMatrixReaderWriter();
	virtual ~CTagLDAParameterMatrixReaderWriter();

	/**
	 *
	 */
	void print_topics(
			std::vector<std::vector<double>* >* topic_term_matrix,
			std::vector<std::vector<double>* >* tag_term_matrix,
			std::string datum_label_index_map_filepath,
			std::string tag_label_index_map_filepath,
			size_t top_n,
			std::ostream& ofs
			);
};

#endif /* CTAGLDAPARAMETERMATRIXREADERWRITER_H_ */
