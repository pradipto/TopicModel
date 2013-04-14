/*
 * CModelEMFunctionoid.h
 *
 *  Created on: Oct 13, 2010
 *      Author: data
 */

#ifndef CMODELEMFUNCTIONOID_H_
#define CMODELEMFUNCTIONOID_H_

#include "../Utilities/includes.h"
#include "AModelEMFunctionoid.h"
#include "../GraphicalModels/CMMTagSquaredLDAGeneralModel.h"

class CModelEMFunctionoid : public AModelEMFunctionoid{
public:
	CModelEMFunctionoid();
	virtual ~CModelEMFunctionoid();

protected:
	/*!
	 * A structure used by normalize_vector_double()
	 */
	struct SNormFunctionObj {
		double _sum;
		SNormFunctionObj(double sum) : _sum(sum) {}
		double operator() (double n) const { return n/_sum; }
	};

	/*!
	 * a boolean indicating whether the verbose output is set to true. By default it is set to true
	 */
	bool _is_verbose;

public:
	/**
	 * @param is_verbose is a boolean to indicate whether to dump debug outputs to screen
	 */
	virtual inline void set_verbose_mode(bool is_verbose) { _is_verbose = is_verbose; }
	/**
	 *
	 */
	virtual inline CMMTagSquaredLDAGeneralModel* get_concrete_model() { if (b_verbose_global) std::cout << "You should derive your own C<YourModel>EMFunctionoid from CModelEMFunctionoid" << std::endl; return 0; }
	/**
	 *
	 */
	virtual inline void operator()() { expectation_step(); maximization_step(); }
	/**
	 *
	 */
	virtual double get_final_model_log_likelihood() { if (b_verbose_global) std::cout << "returning likelihood from base (shouldn't happen) = " ; return 0; }

private:
	/**
	 *
	 */
	virtual void expectation_step() { if (b_verbose_global) std::cout << "The base functionoid doesn't do any imputation" << std::endl; }
	/**
	 *
	 */
	virtual void maximization_step() { if (b_verbose_global) std::cout << "The base functionoid doesn't do any posterior calculations either" << std::endl; }

protected:
	/**
	 *
	 */
	void normalize_vector_double(std::vector<double>* v);
};

#endif /* CMODELEMFUNCTIONOID_H_ */
