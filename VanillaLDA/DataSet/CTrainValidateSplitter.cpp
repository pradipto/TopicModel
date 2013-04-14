/*
 * CTrainValidateSplitter.cpp
 *
 *  Created on: Jun 23, 2010
 *      Author: pdas
 */

#include "CTrainValidateSplitter.h"
#include "../Utilities/CUtilities.h"

using namespace std;

CTrainValidateSplitter::CTrainValidateSplitter(
		string model_index_filename,
		string split_file_store_root_dir,
		size_t topic_num,
		size_t num_folds,
		string model_save_mode) :
	_model_index_filename(model_index_filename), _split_file_store_root_dir(split_file_store_root_dir),
	_topic_num(topic_num), _num_folds(num_folds),
	_model_save_mode(model_save_mode) {
			assert(_num_folds > 1);
}

CTrainValidateSplitter::~CTrainValidateSplitter() {

}

/**
 * doc to come later on
 */
void CTrainValidateSplitter::split() {
	if ( _model_save_mode == "text" || _model_save_mode == "txt" ) {
		split_model_index_as_text_file();
	} else if ( _model_save_mode == "binary" || _model_save_mode == "bin" ) {
		split_model_index_as_binary_file();
	} else
		if (b_verbose_global) std::cout << "Unrecognized model_save_mode : " << _model_save_mode << endl;
}

/**
 *
 * doc to come later on
 */
void CTrainValidateSplitter::split_model_index_as_text_file() {
	size_t num_of_docs = 0;
	string line;
	ifstream ifs;
	//////////////////////////////////////////
	////////// complete first pass to ////////
	////////// get the number of lines ///////
	// read the model file
	ifs.open(_model_index_filename.c_str());
	if ( ifs.is_open() ) {
		while ( !ifs.eof() ) {
			getline(ifs, line);
			CUtilities::trim_left_right(line);
			if ( line.length() > 0 )
				++num_of_docs;
		}
		ifs.close();
	} else {
		if (b_verbose_global) std::cout << "Couldn't open model index file " << _model_index_filename << endl; return;
	}

	vector<bool> v(num_of_docs);
	ofstream ofs_train, ofs_validate;
	string train_filename, validation_filename, train_store_root_dir, validate_store_root_dir;
	string config_filename_train, config_filename_validate, config_file_train_contents, config_file_validate_contents;

	size_t fold_offset = num_of_docs/_num_folds;
	size_t split_index_begin, split_index_end;

	for ( size_t fold = 0; fold < _num_folds; ++fold ) {

		split_index_begin = fold * fold_offset;
		if ( split_index_begin + fold_offset - 1 < num_of_docs )
			split_index_end = split_index_begin + fold_offset - 1;
		else split_index_end = num_of_docs - 1;
		if (b_verbose_global) std::cout << "Validate interval [" << split_index_begin << ", " << split_index_end << "]" << endl;

		////////////////////////////////////////////
		//size_t split_index = (num_of_docs*_train_perc)/100;
		//assert( _train_perc <= 100 );
		for ( size_t i = 0; i < num_of_docs; ++i )
			v.at(i) = true;													// TRAINING = {all indices} - [split_index_begin, split_index_end]
		for ( size_t i = split_index_begin; i <= split_index_end; ++i )
			v.at(i) = false;												// TEST


		//struct SUniqueNumber {
		//  int _current;
		//  SUniqueNumber() { _current = 0; }
		//  int operator()() {return ++_current;}
		//};
		//std::generate(v.begin(), v.end(), SUniqueNumber);	// v is a vector containing 1 2 3 4 ... num_of_lines why is this not working?

		//std::random_shuffle(v.begin(), v.end());

		string topic_num_string = CUtilities::number_to_string<size_t>(_topic_num);
		train_store_root_dir = _split_file_store_root_dir + CUtilities::_file_path_separator + topic_num_string +
				CUtilities::_file_path_separator +"fold" + CUtilities::number_to_string<size_t>(fold) +
				CUtilities::_file_path_separator + "train";
		validate_store_root_dir = _split_file_store_root_dir + CUtilities::_file_path_separator + topic_num_string +
				CUtilities::_file_path_separator +"fold" + CUtilities::number_to_string<size_t>(fold) +
				CUtilities::_file_path_separator + "validation";

		//if (b_verbose_global) std::cout << train_store_root_dir << endl;
		//if (b_verbose_global) std::cout << validate_store_root_dir << endl;
		CUtilities::make_directory(train_store_root_dir);
		CUtilities::make_directory(validate_store_root_dir);

		_train_store_root_dir_vec.push_back(train_store_root_dir);
		_validation_store_root_dir_vec.push_back(validate_store_root_dir);

		// both train and validation filenames takes in the _model_index_filename full path automatically
		train_filename = _model_index_filename + "." + topic_num_string + ".fold" + CUtilities::number_to_string<size_t>(fold) +".train"  ;
		validation_filename = _model_index_filename + "." + topic_num_string +  ".fold" + CUtilities::number_to_string<size_t>(fold) +".validate"  ;
		_model_index_file_split_train_filename_vec.push_back(train_filename);
		_model_index_file_split_validate_filename_vec.push_back(validation_filename);

		size_t i;
		ifs.open(_model_index_filename.c_str());
		if ( ifs.is_open() ) {

			ofs_train.open(train_filename.c_str()); ofs_validate.open(validation_filename.c_str());
			if ( ofs_train.is_open() && ofs_validate.is_open() ) {
				i = 0;
				while ( !ifs.eof() ) {
					getline(ifs, line);
					if ( line.length() == 0 ) continue;
					if ( v[i] ) ofs_train << line << endl;
					else ofs_validate << line << endl;
					++i;
				}
			}

			ifs.close();
			ofs_train.close();
			ofs_validate.close();

		} else {
			if (b_verbose_global) std::cout << "Couldn't open model index file " << _model_index_filename << endl; return;
		}
		// generate config filenames and contents here
		///////////////////////////////////////////////
		/////////////////// TRAIN /////////////////////
		config_filename_train = train_store_root_dir + CUtilities::_file_path_separator +"LDA-Config.topic" + topic_num_string + ".fold" + CUtilities::number_to_string(fold) +".train.txt";
		_config_filenames_train_vec.push_back(config_filename_train);

		//////////////////////////////////////////////////
		/////////////////// VALIDATE /////////////////////
		config_filename_validate = validate_store_root_dir + CUtilities::_file_path_separator + "LDA-Config.topic" + topic_num_string + ".fold" + CUtilities::number_to_string(fold) +".validate.txt";
		_config_filenames_validate_vec.push_back(config_filename_validate);
	}
}

/**
 * binary mode
 * NOTE: in binary mode, the model file is saved as
 * [num_terms_in_doc_0(size_t)][term_id][term_count]...[term_id][term_count]
 * [num_terms_in_doc_1(size_t)][term_id][term_count]...[term_id][term_count]... etc till eof
 *
 * doc to come later on
 */
void CTrainValidateSplitter::split_model_index_as_binary_file() {
	size_t num_of_docs = 0;
	string line;
	ifstream ifs;
	size_t num_terms_in_this_doc;

	//////////////////////////////////////////
	////////// complete first pass to ////////
	////////// get the number of lines ///////
	// read the model file
	ifs.open(_model_index_filename.c_str(), ios_base::binary);
	if ( ifs.is_open() ) {
		while ( !ifs.eof() ) {
			ifs.read(reinterpret_cast<char*>(&num_terms_in_this_doc),sizeof(size_t));

			if ( num_terms_in_this_doc > 0 )
				++num_of_docs;

			ifs.seekg(num_terms_in_this_doc * 2 * sizeof(size_t) , ios::cur);
		}
		ifs.close();
	} else {
		if (b_verbose_global) std::cout << "Couldn't open model index file " << _model_index_filename << endl; return;
	}

	vector<bool> v(num_of_docs, true);
	ofstream ofs_train, ofs_validate;
	string train_filename, validation_filename, train_store_root_dir, validate_store_root_dir;
	string config_filename_train, config_filename_validate, config_file_train_contents, config_file_validate_contents;

	size_t fold_offset = num_of_docs/_num_folds;
	size_t split_index_begin, split_index_end;

	struct STerm_id_count {
		size_t _term_id;
		size_t _term_count;
		STerm_id_count() {
			_term_id = 0; _term_count = 0;
		}
		STerm_id_count(size_t term_id, size_t term_count) {
			_term_id = term_id; _term_count = term_count;
		}
	};
	STerm_id_count a_term_id_count_pair;

	for ( size_t fold = 0; fold < _num_folds; ++fold ) {

		split_index_begin = fold * fold_offset;
		if ( split_index_begin + fold_offset - 1 < num_of_docs )
			split_index_end = split_index_begin + fold_offset - 1;
		else split_index_end = num_of_docs - 1;

		////////////////////////////////////////////
		//size_t split_index = (num_of_docs*_train_perc)/100;
		//assert( _train_perc <= 100 );
		for ( size_t i = 0; i < num_of_docs; ++i )
			v.at(i) = true;													// TRAINING = {all indices} - [split_index_begin, split_index_end]
		for ( size_t i = split_index_begin; i <= split_index_end; ++i )
			v.at(i) = false;												// TEST


		//struct SUniqueNumber {
		//  int _current;
		//  SUniqueNumber() { _current = 0; }
		//  int operator()() {return ++_current;}
		//};
		//std::generate(v.begin(), v.end(), SUniqueNumber);	// v is a vector containing 1 2 3 4 ... num_of_lines why is this not working?

		//std::random_shuffle(v.begin(), v.end());

		string topic_num_string = CUtilities::number_to_string<size_t>(_topic_num);
		train_store_root_dir = _split_file_store_root_dir + CUtilities::_file_path_separator + topic_num_string +
				CUtilities::_file_path_separator + "fold" + CUtilities::number_to_string<size_t>(fold) +
				CUtilities::_file_path_separator + "train";
		validate_store_root_dir = _split_file_store_root_dir + CUtilities::_file_path_separator + topic_num_string +
				CUtilities::_file_path_separator + "fold" + CUtilities::number_to_string<size_t>(fold) +
				CUtilities::_file_path_separator + "validation";

		//if (b_verbose_global) std::cout << train_store_root_dir << endl;
		//if (b_verbose_global) std::cout << validate_store_root_dir << endl;
		CUtilities::make_directory(train_store_root_dir);
		CUtilities::make_directory(validate_store_root_dir);

		_train_store_root_dir_vec.push_back(train_store_root_dir);
		_validation_store_root_dir_vec.push_back(validate_store_root_dir);

		// both train and validation filenames takes in the _model_index_filename full path automatically
		train_filename = _model_index_filename + "." + topic_num_string + ".fold" + CUtilities::number_to_string<size_t>(fold) +".train"  ;
		validation_filename = _model_index_filename + "." + topic_num_string +  ".fold" + CUtilities::number_to_string<size_t>(fold) +".validate"  ;
		_model_index_file_split_train_filename_vec.push_back(train_filename);
		_model_index_file_split_validate_filename_vec.push_back(validation_filename);

		size_t i;
		ifs.open(_model_index_filename.c_str(), ios_base::binary);
		if ( ifs.is_open() ) {

			ofs_train.open(train_filename.c_str(), ios_base::binary); ofs_validate.open(validation_filename.c_str(), ios_base::binary);
			if ( ofs_train.is_open() && ofs_validate.is_open() ) {
				i = 0;
				while ( !ifs.eof() ) {
					ifs.read(reinterpret_cast<char*>(&num_terms_in_this_doc),sizeof(size_t));
					STerm_id_count *object_array = new STerm_id_count[num_terms_in_this_doc];
					for ( size_t n = 0; n < num_terms_in_this_doc; ++n ) {
						ifs.read(reinterpret_cast<char*>(&a_term_id_count_pair),sizeof(STerm_id_count));
						object_array[n] = a_term_id_count_pair;	// object copy
					}

					if ( v[i] ) {
						ofs_train.write(reinterpret_cast<char*>(&num_terms_in_this_doc),sizeof(size_t));
						for ( size_t n = 0; n < num_terms_in_this_doc; ++n ) {
							ofs_train.write(reinterpret_cast<char*>(object_array),sizeof(STerm_id_count)*num_of_docs);
						}
					} else {
						ofs_validate.write(reinterpret_cast<char*>(&num_terms_in_this_doc),sizeof(size_t));
						for ( size_t n = 0; n < num_terms_in_this_doc; ++n ) {
							ofs_validate.write(reinterpret_cast<char*>(object_array),sizeof(STerm_id_count)*num_of_docs);
						}
					}

					delete object_array;
					++i;
				}
			}

			ifs.close();
			ofs_train.close();
			ofs_validate.close();

		} else {
			if (b_verbose_global) std::cout << "Couldn't open model index file " << _model_index_filename << endl; return;
		}
		// generate config filenames and contents here
		///////////////////////////////////////////////
		/////////////////// TRAIN /////////////////////
		config_filename_train = train_store_root_dir +
				CUtilities::_file_path_separator + "LDA-Config.topic" + topic_num_string + ".fold" + CUtilities::number_to_string(fold) +".train.txt";
		_config_filenames_train_vec.push_back(config_filename_train);

		//////////////////////////////////////////////////
		/////////////////// VALIDATE /////////////////////
		config_filename_validate = validate_store_root_dir +
				CUtilities::_file_path_separator + "LDA-Config.topic" + topic_num_string + ".fold" + CUtilities::number_to_string(fold) +".validate.txt";
		_config_filenames_validate_vec.push_back(config_filename_validate);
	}
}
