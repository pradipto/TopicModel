/*
 * CCorrTagSquaredLDAEMFunctionoid.h
 *
 *  Created on: Oct 14, 2010
 *      Author: data
 */

#ifndef CCORRLDAEMFUNCTIONOID_H_
#define CCORRLDAEMFUNCTIONOID_H_

#include "CModelEMFunctionoid.h"
#include "../GraphicalModels/CCorrTagSquaredLDAGeneralModel.h"
#include "../GraphicalModels/CCorrTagSquaredLDA.h"

class CCorrTagSquaredLDA;
class CCorrTagSquaredLDAGeneralModel;

class CCorrTagSquaredLDAEMFunctionoid: public CModelEMFunctionoid {

	CCorrTagSquaredLDA* _model;
	CCorrTagSquaredLDAGeneralModel* _base_model;

	double _model_log_likelihood;

public:
	CCorrTagSquaredLDAEMFunctionoid();
	virtual ~CCorrTagSquaredLDAEMFunctionoid();

public:
	/**
	 * @param is_verbose is a boolean to indicate whether to dump debug outputs to screen
	 */
	virtual inline void set_verbose_mode(bool is_verbose) { _is_verbose = is_verbose; }
	/**
	 *
	 */
	virtual inline CCorrTagSquaredLDAGeneralModel* get_concrete_model() { return _base_model; }
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
	 * @brief expectation step for CorrLDA
	 */
	virtual void expectation_step();
	/**
	 * @brief expectation step for CorrLDA per document
	 * @return the likelihood of this document under the model
	 */
	virtual double doc_expectation_step(size_t doc_id, SCorrTagSquaredLDADocument* doc);
	/**
	 * @brief maximization step for CorrLDA
	 */
	virtual void maximization_step();
	/**
	 * @brief compute final e step to obtain doc specific variational parameters
	 */
	void final_e_step();

};

#endif /* CCORRLDAEMFUNCTIONOID_H_ */
