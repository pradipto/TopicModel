/*
 * CDirichletOptimizer.h
 *
 *  Created on: Dec 15, 2011
 *      Author: data
 */

#ifndef CDIRICHLETOPTIMIZER_H_
#define CDIRICHLETOPTIMIZER_H_

#include "includes.h"

class CDirichletOptimizer {
public:
	CDirichletOptimizer();
	virtual ~CDirichletOptimizer();

	static double symmetric_dirichlet_hyper_param_lhood(double a, double ss, int D, int K);
	static double d_symmetric_dirichlet_hyper_param_lhood(double a, double ss, int D, int K);
	static double d2_symmetric_dirichlet_hyper_param_lhood(double a, int D, int K);
	static double opt_symmetric_dirichlet_hyper_param(double ss, int D, int K);

	static std::vector<double> opt_asymmetric_dirichlet_hyper_param(std::vector<double>* ss, int D, int K);
	static std::vector<double> opt_asymmetric_dirichlet_hyper_param(std::vector<double>* ss, std::vector<double>* exp_ss, std::vector<double>* exp_square_ss, int D, int K);
	static double asymmetric_dirichlet_hyper_param_lhood(std::vector<double>& a, std::vector<double>* ss, int D, int K);

private:
	static std::vector<double> initialize_a(std::vector<double>& exp_ss, std::vector<double>& exp_square_ss, int D, int K);
	static void asymmetric_dirichlet_hessian_times_gradient(std::vector<double>& a, std::vector<double>& g, double lambda, int K, std::vector<double>& hg);	// hg is hessian^{-1} x gradient
	static bool is_all_less(std::vector<double>& a, std::vector<double>& b);

	static void print_vector(std::vector<double>& a, std::string name);

};

#endif /* CDIRICHLETOPTIMIZER_H_ */
