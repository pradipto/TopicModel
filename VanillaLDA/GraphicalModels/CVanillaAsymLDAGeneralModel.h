/**
 *  file CVanillaAsymLDAGeneralModel.h
 *
 *  Created on: May 21, 2010
 *      Author: pdas
 */

#ifndef CVanillaLDAGeneralMODEL_H_
#define CVanillaLDAGeneralMODEL_H_

#include "../Utilities/includes.h"
#include "AGraphicalModel.h"
#include "../DataSet/VanillaAsymLDADataset.h"

/**
 * @brief An base class used for deriving sub classes to implement other graphical models
 *
 * @date: May 21, 2010
 * @author: Pradipto Das. email: pdas3@buffalo.edu
 *
 * This base class is derived from the abstract base class AGraphicalModel and thus implements
 * functions: \n
 * 	virtual void set_model_operation_mode(std::string mode) = 0;\n
	virtual bool create_and_initialize_data_structures() = 0;\n
	virtual double compute_log_likelihood() = 0;\n
	virtual std::string to_string() = 0;\n

 * The public methods in this base class CVanillaAsymLDAGeneralModel opens up the API for accessing the model
 * data structures
 *
 * @see AGraphicalModel

 */
class CVanillaAsymLDAGeneralModel : public AGraphicalModel{

protected:
	/*!
	 * @brief A model bytecode is a std::string representation of the model. A simple 3 tier Latent Dirichlet Allocation (LDA) model looks like:\n
	 * mult$dirsym(beta)(hidden)(k)(0.01)(v)|mult(w)(observed)(dn)(beta;z;n)$dirsym(theta)(hidden)(d)(0.01)(k)|mult(z)(hidden)(dn)(theta;d)$
	 *
	 * Each "$" sign separates a level that starts at 0 indicating the observed variable. \n
	 * Within each level, "|" separates a pair of conjugate distributions. \n
	 * Each conjugate distribution pair is of the form: \<prior distribution\>|\<likelihood distribution\>. If there are more than one conjugate distribution in a particular
	 * level then the conjugate distributions are alphabetically sorted by the prior distribution name
	 *
	 * For the paper on LDA model @see www.cs.princeton.edu/~blei/papers/BleiNgJordan2003.pdf
	 *
	 */
	std::string _bytecode;
	/*!
	 * @brief This the mode (binary or text) in which all files are written. Default is "text".\n
	 * The text mode is very useful for debugging purposes and generating graphs.
	 */
	std::string _model_dump_mode;
	/*!
	 * @brief Can only be the std::strings "train" or "test". This needs to be set before any operation on the model takes place
	 */
	std::string _model_operation_mode;
	/*!
	 * @brief This is a std::map that maps the datum id in the test or validation set to a sequential index starting from 0.\n
	 * For e.g., 5001 can be mapped to 0, 4789 can be mapped to 1 and so on. The size of the map gives the number of
	 * unique datum ids found in the test or validation corpus
	 */
	std::map<size_t, size_t> _test_datum_id_to_sequential_index_id_map;

public:
	/**
	 * @brief This function sets the bytecode of the model to the bytecode in the parameter
	 * @see CVanillaAsymLDAGeneralModel::_bytecode
	 * @param bytecode sets the bytecode of the model to the bytecode in the parameter
	 */
	inline virtual void set_bytecode(std::string bytecode) { _bytecode = bytecode; }
	/**
	 * @brief This function returns the model bytecode which is a std::std::string
	 * @see CVanillaAsymLDAGeneralModel::_bytecode
	 * @return the model bytecode which is a std::std::string
	 */
	inline virtual std::string get_bytecode() { return _bytecode; }

	/**
	 * @brief The constructor in the base model just sets the default file dump mode to "text"
	 */
	CVanillaAsymLDAGeneralModel() { _model_dump_mode = "text"; }
	/**
	 * @brief virtual destructor for CVanillaAsymLDAGeneralModel
	 */
	virtual ~CVanillaAsymLDAGeneralModel() {}

	/**
	 * @brief This function creates and initializes the model data structures
	 * @see AGraphicalModel::create_and_initialize_data_structures()
	 */
	virtual bool create_and_initialize_data_structures() { return false; }
	/**
	 * @brief This function returns the data log likelihood given the model
	 * @see AGraphicalModel::compute_log_likelihood()
	 */
	virtual double compute_log_likelihood() { return 0; }
	/**
	 * @brief returns the size of the test or validation vocabulary which is the same as the size of the
	 * std::map _test_datum_id_to_sequential_index_id_map.
	 *
	 * @return the size of the std::map member variable, _test_datum_id_to_sequential_index_id_map. \n
	 * Returns 0 from base class if the base class is not derived
	 */
	virtual size_t get_test_vocab_size() { return 0; }

	/**
	 * @brief This function sets the value of the std::std::string member variable _model_dump_mode
	 *
	 * @param model_dump_mode is the mode in which the model files are written to disc. Default value is "text"
	 * @note The only acceptable values of the std::string model_dump_mode are "text", "txt", "binary" and "bin". The binary model save mode
	 * implementation has been saved in the TO-DO list
	 */
	virtual void set_model_dump_mode(std::string model_dump_mode = "text") {
		if ( (model_dump_mode != "text") || (model_dump_mode != "txt") || (model_dump_mode != "bin") || (model_dump_mode != "binary") ) {
			if (b_verbose_global) std::cout << "unrecognized dump mode " << model_dump_mode << std::endl; exit(APPLICATIONEXIT);
		}
		if ( (model_dump_mode != "text") || (model_dump_mode != "txt") )
			_model_dump_mode = "text";
		if ( (model_dump_mode != "bin") || (model_dump_mode != "binary") )
			_model_dump_mode = "binary";
	}
	/**
	 * @brief returns the model_dump_mode  which is a std::std::string
	 * @see CVanillaAsymLDAGeneralModel::_model_dump_mode
	 *
	 * @return the model_dump_mode  which is a std::std::string
	 */
	virtual std::string get_model_dump_mode() { return _model_dump_mode; }
	/**
	 * @brief sets the member variable of the model_operation_mode to the value of the parameter
	 * @see CVanillaAsymLDAGeneralModel::_model_operation_mode
	 *
	 * @param model_operation_mode is the mode represented as a std::std::string in which the model operates. \n
	 * The value can only be "train" or "test"
	 */
	virtual void set_model_operation_mode(std::string model_operation_mode) { _model_operation_mode = model_operation_mode; }
	/**
	 * @brief returns the value of the std::std::string _model_operation_mode
	 * @see CVanillaAsymLDAGeneralModel::_model_operation_mode
	 * @return the value of the std::std::string _model_operation_mode
	 */
	virtual std::string get_model_operation_mode() { return _model_operation_mode; }

	/**
	 * @brief This function is responsible for loading any training files into the appropriate data structures during the test or
	 * validation phases.
	 *
	 * @return true if the model files loading was successful, false otherwise
	 *
	 * @note This function attempts to read the saved model files in the mode (text or binary) given by the value of _model_dump_mode
	 */
	virtual bool load_model() { return false; }
	/**
	 * @brief This function saves the model parameter data structures to disc in the mode (text or binary)
	 * given by the value of _model_dump_mode
	 */
	virtual void save_model() {}
	/**
	 * @brief This function reads the input data from file and populates the model data structures as needed
	 * @note This function is highly model dependent
	 */
	virtual void read_data_from_file() {}
	/**
	 * @brief This function reads the contents of the config file from which the model parameters are set
	 */
	virtual void read_config_file(std::string model_config_filename) {}
	/**
	 * @return an std::std::string representing, possibly, the name of the model
	 */
	virtual std::string to_string() { return "an uninteresting base_model"; }
	/**
	 * @brief returns a pointer to a std::vector of std::vector pointers:\n
	 * The number of rows equals the number of documents in the training set.\n
	 * The number of columns equals the number of latent topics.\n
	 *
	 * @return a pointer to a std::vector of std::vector pointers.\n
	 * The number of rows equals the number of documents in the training set.\n
	 * The number of columns equals the number of latent topics.\n
	 *
	 * By default return 0 from base class if the base class is not derived
	 *
	 *  @note each row (document) represents a multinomial distribution snapshot over topic id \b proportions.\n
	 *  The value of each element in a row represents the multinomial parameter for the topic id \b proportion responsible for
	 *  generating that document. This value is very useful for reducing the dimensionality of the document into the topic space.
	 */
	virtual std::vector<std::vector<double>* >* get_topic_proportion_multinomials() { return 0; }
	/**
	 * @brief returns a pointer to a std::vector of std::vector pointers:\n
	 * The number of rows equals the number of latent topics in the model.\n
	 * The number of columns equals the number of datum in the training corpus.\n
	 *
	 * @return a pointer to a std::vector of std::vector pointers.\n
	 * The number of rows equals the number of latent topics in the model.\n
	 * The number of columns equals the number of datum in the training corpus.\n
	 *
	 * By default return 0 from base class if the base class is not derived
	 *
	 */
	virtual std::vector<std::vector<double>* >* get_topic_datum_multinomials() { return 0; }
	/**
	 * @brief returns a pointer to a std::vector of std::vector pointers:\n
	 * The number of rows equals the number of latent topics in the model.\n
	 * The number of columns equals the number of datum in the training corpus.\n
	 *
	 * @return a pointer to a std::vector of std::vector pointers.\n
	 * The number of rows equals the number of latent topics in the model.\n
	 * The number of columns equals the number of datum in the training corpus.\n
	 *
	 * By default return 0 from base class if the base class is not derived
	 *
	 */
	virtual std::vector<std::vector<double>* >* get_tag_datum_multinomials() { return 0; }
	/**
	 * @brief This function returns the std::std::string filename that corresponds to a text file of datum ids to datum labels.\n
	 * The dictionary is currently stored as a text file in the following format:\n
	 * datum_label=datum_id. This is generated during the indexing phase for documents
	 *
	 * @return the std::std::string filename that corresponds to a text file of datum ids to datum labels.
	 */
	virtual std::string get_datum_to_id_dictionary_filename() { return "no dictionary filename from base class"; }
	/**
	 * @brief returns the std::std::string filename that corresponds to the file where the topic_datum multinomials are written
	 *
	 * @return the std::std::string filename that corresponds to the file where the topic_datum multinomials are written
	 *
	 * By default returns the std::string "no train topic multinomial filename from base class" from the base class if the base class is not derived
	 */
	virtual std::string get_saved_topic_multinomial_filename() { return "no train topic multinomial filename from base class"; }

	/**
	 * @brief returns a pointer to the SCorpus representing the training corpus
	 *
	 * @return a pointer to the SCorpus representing the training corpus
	 * @see SCorpus
	 */
	virtual SVanillaAsymLDACorpus* get_corpus() { return 0; }
	/**
	 * @brief This function resets all test data structures.
	 * @note This function is highly dependent on a particular model
	 */
	virtual void reset_data_structures() {}
	/**
	 * @brief returns the number of training documents
	 * @return the number of training documents
	 * @note the function in the base class returns 0
	 */
	virtual size_t get_doc_count() { return 0; }
	/**
	 * @brief returns the training vocabulary size
	 * @return the training vocabulary size
	 * @note the function in the base class returns 0
	 */
	virtual size_t get_vocabulary_size() { return 0; }
	/**
	 * @brief returns the count of training datum
	 * @return the count of training datum
	 * @note the function in the base class returns 0
	 */
	virtual long get_total_datum_count() { return 0; }


public:
	/**
	 * @brief computes the Bayesian Information Criterion from the model for the particular setting of model parameters
	 * @return the Bayesian Information Criterion from the model for the particular setting of model parameters
	 *
	 * By default returns 0 from base class
	 */
	virtual double get_BIC() { return 0; }
	/**
	 * @brief computes the Minimum Description Length from the model for the particular setting of model parameters
	 * @return the Minimum Description Length from the model for the particular setting of model parameters
	 *
	 * By default returns 0 from base class
	 */
	virtual double get_MDL() { return 0; }
};

#endif /* CMODEL_H_ */
