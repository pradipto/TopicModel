/*
 * CMMLDA.h
 *
 *  Created on: Jan 10, 2011
 *      Author: data
 */

#ifndef CMMLDA_H_
#define CMMLDA_H_

#include "../DataSet/MMLDADataset.h"
#include "../DataSet/CParameterMatrixReaderWriter.h"
#include "AGraphicalModel.h"
#include "CMMLDAGeneralModel.h"
#include "../EMAlgorithms/CMMLDAEMFunctionoid.h"
#include "../Utilities/CUtilities.h"
#include "../Utilities/TemplateMath.h"
#include "../Utilities/cokus.h"

#define myrand_Cokus() (double) (((unsigned long) randomMT()) / 4294967296.0)


class CMMLDA : public CMMLDAGeneralModel {

protected:

	friend class CMMLDAEMFunctionoid;
	/*!
	 * @brief This is needed for printing topics as datum labels to file or console
	 */
	std::string _datum_to_id_dict_filename;
	/*!
	 * @brief This is needed for printing topics as datum labels to file or console
	 */
	std::string _corr_datum_to_id_dict_filename;
	/*!
	 * @brief This member variable contains the absolute filename of the saved topic datum multinomial parameters
	 */
	std::string _model_train_saved_topic_mult_filename;
	/*!
	 * @brief This member variable stores the root directory for the model
	 */
	std::string _model_root_dir;
	/*!
	 * @brief This member variable stores the training root directory for the model - useful during inference
	 */
	std::string _model_train_root_dir;
	/*!
	 * @brief full path for the model index file
	 */
	std::string _model_input_filename;
	/*!
	 * The asymmetric dirichlet parameter file in case use of asymmetric dirichlet for topic proportions is set to true
	 */
	std::string _asym_alpha_initial_file;
	/*!
	 * If set to true, then the asym_alpha_initial_file is read
	 */
	bool _b_use_asym_alpha;
	/*!
	 * @brief This member variable indicates whether to read data in grouped format or sequential format.
	 * Default value is the std::string "grouped".
	 * Only two values are allowed - "grouped" or "sequential"
	 */
	std::string _data_input_mode;
	/*!
	 * @brief a boolean to accomodate for unknown word ids
	 */
	bool _accommodate_for_unknown_datum;
	/*!
	 * @brief the total datum count in the training corpus - useful for measuring Bayesian Information Criterion
	 */
	long _training_datum_total_count;
	/*!
	 * @brief the total datum count in the training corpus - useful for measuring Bayesian Information Criterion
	 */
	long _training_corr_datum_total_count;
	/*!
	 * @brief number of EM iterations
	 */
	int _n_em_iter;
	/*!
	 * @brief number of variational EM iterations
	 */
	int _n_em_var_iter;

	/*!
	 * @brief likelihood under this model for a given value of K and the set of tags
	 */
	double _log_prob;

protected:
	/*
	 * declaring the parameters of the model
	 */
	std::vector<std::vector<double>* >* _log_rho;	// this is the K x T matrix: T is the tag corresponding to the word
	std::vector<std::vector<double>* >* _log_beta;	// this is the K x V matrix
	double _alpha;									// hyperparameter for topic proportions

	/*
	 * sufficient statistics
	 */
	std::vector<std::vector<double>* >* _rho_ss;	// this is the K x corrV matrix
	std::vector<std::vector<double>* >* _beta_ss;	// this is the K x V matrix
	double _alpha_ss;								// hyperparameter for topic proportions
	std::vector<double>* _asym_alpha;				// assymetric alpha for topic proportions
	std::vector<double>* _asym_alpha_ss;			// assymetric alpha for topic proportions
	std::vector<double>* _asym_alpha_exp_ss;		// initializer in optimization of assymetric alpha for topic proportions
	std::vector<double>* _asym_alpha_exp_square_ss;	// initializer in optimization of assymetric alpha for topic proportions
	double _asym_alpha_concentration_parameter;			// concentration parameter for alpha - set to 10 in the constructor for now

	std::vector<double>* _rho_total_ss;				// for normalizing rho
	std::vector<double>* _beta_total_ss;			// for normalizing beta

	SMMLDACorpus* _corpus;

	size_t _D;
	size_t _V;
	size_t _K;
	size_t _corrV;
	size_t _n_max_datum_in_a_doc;
	size_t _n_max_corr_datum_in_a_doc;

	/*
	 * variational matrices and hyperparameters (updated per document)
	 */
	std::vector<std::vector<double>* >* _var_phi;			// this is the N_max x K variational matrix
	std::vector<std::vector<double>* >* _var_lambda;		// this is the M_max x corrV_max variational matrix

	std::vector<std::vector<double>* >* _var_gamma;			// D x K matrix variational hyper parameter for topic proportions

	bool _estimate_hypparam;

public:
	CMMLDA();
	~CMMLDA();

	inline SMMLDACorpus* get_corpus() { return _corpus; }

	// mutator
	inline void set_num_topics(int n_topics) { _K = n_topics; }

	// getters and setters
	/**
	 *
	 */
	virtual void set_bytecode(std::string bytecode) { CMMLDAGeneralModel::_bytecode = bytecode; }
	/**
	 *
	 */
	inline std::string get_bytecode() { return _bytecode; }
	/**
	 *
	 */
	virtual void set_model_dump_mode(std::string model_dump_mode) { CMMLDAGeneralModel::_model_dump_mode = model_dump_mode; }
	/**
	 *
	 */
	virtual void set_model_operation_mode(std::string model_operation_mode) { CMMLDAGeneralModel::_operation_mode = model_operation_mode; }
	/**
	 *
	 */
	virtual std::string get_model_operation_mode() { return CMMLDAGeneralModel::_operation_mode; }
	/**
	 *
	 */
	virtual std::string get_model_dump_mode() { return CMMLDAGeneralModel::_model_dump_mode; }
	/**
	 *
	 */
	virtual std::string get_model_root_dir() { return _model_root_dir; }
	/**
	 *
	 */
	virtual inline std::string to_string() {
		if ( CMMLDAGeneralModel::_operation_mode == "train" ) return "MMLDA-train";
		if ( CMMLDAGeneralModel::_operation_mode == "test" || CMMLDAGeneralModel::_operation_mode == "validate" ) return "MMLDA-test";
		if ( CMMLDAGeneralModel::_operation_mode == "predict" ) return "MMLDA-predict";
		return "MMLDA-EM";	// returns this std::string by default
	}
	/**
	 *
	 */
	virtual std::vector<std::vector<double>* >* get_topic_datum_multinomials() { return _log_beta; }
	/**
	 *
	 */
	virtual std::vector<std::vector<double>* >* get_topic_feature_multinomials() { return _log_rho; }
	/**
	 *
	 */
	virtual std::vector<std::vector<double>* >* get_topic_proportion_multinomials() { return _var_gamma; }
	/**
	 *
	 */
	virtual std::string get_datum_to_id_dictionary_filename() { return _datum_to_id_dict_filename; }
	/**
	 *
	 */
	virtual std::string get_saved_topic_datum_multinomial_filename() { return _model_train_saved_topic_mult_filename; }
	/**
	 *
	 */
	virtual size_t get_doc_count() { return _D; }
	/**
	 *
	 */
	virtual size_t get_vocabulary_size() { return _V; }
	/**
	 * @return a long integer indicating the total number of terms in the training corpus
	 */
	virtual long get_total_datum_count() { return _training_datum_total_count; }

	// other methods
	/**
	 * @brief
	 */
	void corpus_initialize();
	/**
	 * @brief
	 */
	void random_initialize();
	/**
	 * @brief returns a boolean indicating if the model can be loaded
	 * @return a boolean indicating if the model can be loaded
	 */
	virtual bool load_model();
	/**
	 * @brief
	 */
	virtual void save_model();
	/**
	 *
	 */
	virtual void read_data_from_file();
	/**
	 *
	 */
	virtual void read_config_file(std::string model_config_filename);
	/**
	 *
	 */
	virtual bool create_and_initialize_data_structures();
	/**
	 *
	 */
	virtual bool zero_initialize_suff_stats();
	/**
	 *
	 */
	virtual void reset_data_structures();
	/**
	 *
	 */
	virtual double compute_log_likelihood();

private:
	/**
	 *
	 */
	void read_data_from_file_for_training();
	/**
	 *
	 */
	void read_data_from_file_for_testing();
	/**
	 *
	 */
	bool create_and_initialize_data_structures_for_parameters();
	/**
	 *
	 */
	bool create_and_initialize_data_structures_for_hidden_variables_and_suff_stats();
	/**
	 *
	 */
	double compute_doc_log_likelihood(size_t doc_id, SMMLDADocument* doc);

public:
	/**
	 *  some diagnostic functions
	 */
	virtual void print_parameters();
};

#endif /* CMMLDA_H_ */
