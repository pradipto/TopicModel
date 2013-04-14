/*
 * CTrainValidateSplitter.h
 *
 *  Created on: Jun 23, 2010
 *      Author: pdas
 */

#ifndef CTRAINVALIDATESPLITTER_H_
#define CTRAINVALIDATESPLITTER_H_

#include "../Utilities/includes.h"
//using namespace std;

class CTrainValidateSplitter {

	std::vector<std::string> _config_filenames_train_vec;					// the full path
	std::vector<std::string> _config_filenames_validate_vec;					// the full path
	std::vector<std::string> _model_index_file_split_train_filename_vec;		// the full path
	std::vector<std::string> _model_index_file_split_validate_filename_vec;	// the full path
	std::vector<std::string> _train_store_root_dir_vec;						// the full path
	std::vector<std::string> _validation_store_root_dir_vec;					// the full path

	std::string _model_index_filename;		// the model index file - [std::string != ""] std::string representing the model index file
	std::string _split_file_store_root_dir;	// model split file storage root dir - [std::string != ""] std::string representing the location on disk where the splitted files will be stored
	size_t _topic_num;					// the topic num
	size_t _num_folds;					// number of folds - [integer >= 1]
	std::string _model_save_mode;			// the mode in which files are saved - either text or binary


public:

	CTrainValidateSplitter( std::string model_index_filename,
							std::string split_file_store_root_dir,
							size_t topic_num = 1,
							size_t num_folds = 5,
							std::string model_save_mode = "text");

	~CTrainValidateSplitter();

	// getters ////////////////////////////////////////////////////////////////////////////
	inline std::vector<std::string> get_config_filenames_train_vec() { return _config_filenames_train_vec; }
	inline std::vector<std::string> get_config_filenames_validation_vec() { return _config_filenames_validate_vec; }
	inline std::vector<std::string> get_model_index_file_split_train_filename_vec() { return _model_index_file_split_train_filename_vec; }
	inline std::vector<std::string> get_model_index_file_split_validate_filename_vec() { return _model_index_file_split_validate_filename_vec; }
	inline std::vector<std::string> get_train_store_root_dir_vec() { return _train_store_root_dir_vec; }
	inline std::vector<std::string> get_validation_store_root_dir_vec() { return _validation_store_root_dir_vec; }
	inline std::string get_model_index_filename() { return _model_index_filename; }
	inline std::string get_split_file_store_root_dir() { return _split_file_store_root_dir;	}
	inline int get_num_folds() { return _num_folds; }
	inline std::string get_model_save_mode() { return _model_save_mode; }
	// setters //////////////////////////////////////////////////////////////////////////////////////
	inline void set_model_index_filename(std::string filename) { _model_index_filename = filename; }
	inline void set_split_file_store_root_dir(std::string filename) { _split_file_store_root_dir = filename;	}
	inline void set_num_folds(int folds) { _num_folds = folds; }
	inline void set_model_save_mode(std::string save_mode) { _model_save_mode = save_mode; }

	void split();

private:
	void split_model_index_as_text_file();
	void split_model_index_as_binary_file();
};

#endif /* CTRAINVALIDATESPLITTER_H_ */
