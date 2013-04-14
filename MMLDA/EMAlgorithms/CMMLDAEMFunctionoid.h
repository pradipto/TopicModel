/*
 * CMMLDAEMFunctionoid.h
 *
 *  Created on: Oct 14, 2010
 *      Author: data
 */

#ifndef CCORRLDAEMFUNCTIONOID_H_
#define CCORRLDAEMFUNCTIONOID_H_

#include "CModelEMFunctionoid.h"
#include "../GraphicalModels/CMMLDAGeneralModel.h"
#include "../GraphicalModels/CMMLDA.h"

class CMMLDA;
class CMMLDAGeneralModel;

class CMMLDAEMFunctionoid: public CModelEMFunctionoid {

	CMMLDA* _model;
	CMMLDAGeneralModel* _base_model;

	double _model_log_likelihood;

public:
	CMMLDAEMFunctionoid();
	virtual ~CMMLDAEMFunctionoid();

public:
	/**
	 * @param is_verbose is a boolean to indicate whether to dump debug outputs to screen
	 */
	virtual inline void set_verbose_mode(bool is_verbose) { _is_verbose = is_verbose; }
	/**
	 *
	 */
	virtual inline CMMLDAGeneralModel* get_concrete_model() { return _base_model; }
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
	virtual double doc_expectation_step(size_t doc_id, SMMLDADocument* doc);
	/**
	 * @brief maximization step for CorrLDA
	 */
	virtual void maximization_step();
	/**
	 * @brief compute final e step to obtain doc specific variational parameters
	 */
	virtual void final_e_step();
	virtual void final_e_step_predict();
};

#endif /* CCORRLDAEMFUNCTIONOID_H_ */
