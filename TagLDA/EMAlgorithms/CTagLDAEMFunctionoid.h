/*
 * CTagLDAEMFunctionoid.h
 *
 *  Created on: Oct 14, 2010
 *      Author: data
 */

#ifndef CTAGLDAEMFUNCTIONOID_H_
#define CTAGLDAEMFUNCTIONOID_H_

#include "CModelEMFunctionoid.h"
#include "../GraphicalModels/CTagLDAGeneralModel.h"
#include "../GraphicalModels/CTagLDA.h"

class CTagLDA;
class CTagLDAGeneralModel;

class CTagLDAEMFunctionoid: public CModelEMFunctionoid {

	CTagLDA* _model;
	CTagLDAGeneralModel* _base_model;

	double _model_log_likelihood;

private:
	void precompute_beta_and_pi_row_sums();

public:
	CTagLDAEMFunctionoid();
	virtual ~CTagLDAEMFunctionoid();

public:
	/**
	 * @param is_verbose is a boolean to indicate whether to dump debug outputs to screen
	 */
	virtual inline void set_verbose_mode(bool is_verbose) { _is_verbose = is_verbose; }
	/**
	 *
	 */
	virtual inline CTagLDAGeneralModel* get_concrete_model() { return _base_model; }
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
	virtual double doc_expectation_step(size_t doc_id, STagLDADocument* doc);
	/**
	 * @brief maximization step for TagLDA
	 */
	virtual void maximization_step();
	/**
	 * @brief a final e step to obtain document level parameters
	 */
	virtual void final_e_step();

};

#endif /* CTAGLDAEMFUNCTIONOID_H_ */
