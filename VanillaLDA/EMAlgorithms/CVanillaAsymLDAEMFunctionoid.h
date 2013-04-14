/*
 * CVanillaAsymLDAEMFunctionoid.h
 *
 *  Created on: Oct 14, 2010
 *      Author: data
 */

#ifndef CVANILLALDAEMFUNCTIONOID_H_
#define CVANILLALDAEMFUNCTIONOID_H_

#include "CModelEMFunctionoid.h"
#include "../GraphicalModels/CVanillaAsymLDAGeneralModel.h"
#include "../GraphicalModels/CVanillaAsymLDA.h"

class CVanillaAsymLDA;
class CVanillaAsymLDAGeneralModel;

class CVanillaAsymLDAEMFunctionoid: public CModelEMFunctionoid {

	CVanillaAsymLDA* _model;
	CVanillaAsymLDAGeneralModel* _base_model;

	double _model_log_likelihood;

public:
	CVanillaAsymLDAEMFunctionoid();
	virtual ~CVanillaAsymLDAEMFunctionoid();

public:
	/**
	 * @param is_verbose is a boolean to indicate whether to dump debug outputs to screen
	 */
	virtual inline void set_verbose_mode(bool is_verbose) { _is_verbose = is_verbose; }
	/**
	 *
	 */
	virtual inline CVanillaAsymLDAGeneralModel* get_concrete_model() { return _base_model; }
	/**
	 *
	 */
	virtual void operator()();
	/**
	 * @note the _model_log_likelihood is calculated in the e-step
	 */
	virtual double get_final_model_log_likelihood() { return _model_log_likelihood; }

private:
	/**
	 * @brief expectation step for TagLDA
	 */
	virtual void expectation_step();
	/**
	 * @brief expectation step for TagLDA per document
	 * @return the likelihood of this document under the model
	 */
	virtual double doc_expectation_step(size_t doc_id, SVanillaAsymLDADocument* doc);
	/**
	 * @brief maximization step for TagLDA
	 */
	virtual void maximization_step();

private:
	virtual bool write_doc_topic_assignments(std::string filename);

};

#endif /* CTAGLDAEMFUNCTIONOID_H_ */
