/*
 * CDirichletOptimizer.cpp
 *
 *  Created on: Dec 15, 2011
 *      Author: data
 */

#include "CDirichletOptimizer.h"
#include "TemplateMath.h"
#include <cfloat>
using namespace std;

CDirichletOptimizer::CDirichletOptimizer() {
}

CDirichletOptimizer::~CDirichletOptimizer() {
}

double CDirichletOptimizer::symmetric_dirichlet_hyper_param_lhood(double a, double ss, int D, int K) {
	return(D * (lgamma(K * a) - K * lgamma(a)) + (a - 1) * ss);
}

double CDirichletOptimizer::d_symmetric_dirichlet_hyper_param_lhood(double a, double ss, int D, int K) {
	return(D * (K * CMathUtilities::digamma(K * a) - K * CMathUtilities::digamma(a)) + ss);
}

double CDirichletOptimizer::d2_symmetric_dirichlet_hyper_param_lhood(double a, int D, int K) {
	return(D * (K * K * CMathUtilities::trigamma(K * a) - K * CMathUtilities::trigamma(a)));
}

/*
 * newtons method for symmetric dirichlet
 *
 */
double CDirichletOptimizer::opt_symmetric_dirichlet_hyper_param(double ss, int D, int K) {

	double a, log_a, init_a = 100;
	double f, df, d2f;
	int iter = 0;

	log_a = log(init_a);
	do
	{
		iter++;
		a = exp(log_a);
		if (isnan(a))
		{
			init_a = init_a * 10;
			printf("warning : Dirichlet hyperparameter is nan; new init = %5.5f\n", init_a);
			a = init_a;
			log_a = log(a);
		}
		f = symmetric_dirichlet_hyper_param_lhood(a, ss, D, K);
		//cout << "** " << a << ":" << ss << ":" << D << ":" << K << endl;
		df = d_symmetric_dirichlet_hyper_param_lhood(a, ss, D, K);
		//if ( isnan(df) ) {
		//	cout << "HypParam derivative assertion failed " << endl;
		//}
		d2f = d2_symmetric_dirichlet_hyper_param_lhood(a, D, K);
		//assert( !isnan(df) );
		log_a = log_a - df/(d2f * a + df);
		if (isnan(log_a))
		{
			init_a = init_a * 10;
			printf("warning : Dirichlet hyperparameter is nan; new init = %5.5f\n", init_a);
			a = init_a;
			log_a = log(a);

		}
		if ( b_verbose_global ) printf("Dirichlet hyperparameter maximization : %5.5f   %5.5f   %5.5f : %10.5f\n", f, df, d2f, ss);
	}
	while ((fabs(df) > 1e-5) && (iter < 1000));
	assert( !isnan(log_a) );
	return(exp(log_a));
}

vector<double> CDirichletOptimizer::initialize_a(vector<double>& exp_ss, vector<double>& exp_square_ss, int D, int K) {
	vector<double> a(K, 0), m2(K, 0), s;
	for (int k = 0; k < K; ++k) {
		a[k] = exp_ss[k]/D;
		m2[k] = exp_square_ss[k]/D;
	}
	for (int k = 0; k < K; ++k) {
		if ( a[k] > 0 ) {
			s.push_back( (a[k] - m2[k])/(m2[k] - a[k]*a[k]) );
		}
	}
	if ( s.size() > 0 ) {
		size_t n = s.size()/2;
		nth_element(s.begin(), s.begin()+n, s.end());
		double median_s = s[n];

		for (int k = 0; k < K; ++k)
			a[k] *= median_s;
	}
	return a;
}

/**
 *
 */
vector<double> CDirichletOptimizer::opt_asymmetric_dirichlet_hyper_param(vector<double>* ss, int D, int K) {

	vector<double> a(K, 100);	// a is initially a vector with all values set to 100. the data will optimize each component
	vector<double> g(K, 0), hg(K, 0);
	vector<double> a_minus_hg(K, 0);

	double old_e = D*asymmetric_dirichlet_hyper_param_lhood(a, ss, D, K);
	double new_e;

	double eps = DBL_EPSILON;
	double lambda = 0.1;
	int max_iter = 100;

	bool abort;
	int iter;
	for ( iter = 1; iter < max_iter; ++iter ) {
		vector<double> old_a = a;
		double sum_a = std::accumulate(a.begin(), a.end(), 0.0);
		if ( sum_a == 0 )
			break;

		for ( int k = 0; k < K; ++k ) {
			g[k] = CMathUtilities::digamma(sum_a) - CMathUtilities::digamma(a[k]) + (*ss)[k]/D;
		}
		abort = false;
		while (true) {
			asymmetric_dirichlet_hessian_times_gradient(a, g, lambda, K, hg);
			if ( is_all_less(hg, a) ) {
				for ( int k = 0; k < K; ++k )
					a_minus_hg[k] = a[k] - hg[k];	// a-hg = a_k - hg_k

				new_e = D*asymmetric_dirichlet_hyper_param_lhood(a_minus_hg, ss, D, K);
				if ( new_e > old_e ) {
					old_e = new_e;
					copy( a_minus_hg.begin(), a_minus_hg.end(), a.begin() );	// a - hg
					lambda /= 10;
					break;
				}
			}
			lambda *= 10;
			if ( lambda > 1e+6 ) {
				abort = true;
				break;
			}
		}
		if ( abort ) {
			new_e = old_e;
			break;
		}
		// change all elements in a that are less that eps to eps
		for ( int k = 0; k < K; ++k )
			if ( a[k] < eps ) a[k] = eps;

		double max_abs_a_minus_olda = abs(a[0]-old_a[0]);
		for ( int k = 1; k < K; ++k )
			if ( abs(a[k] - old_a[k]) > max_abs_a_minus_olda )
				max_abs_a_minus_olda = abs(a[k] - old_a[k]);
		if ( max_abs_a_minus_olda < 1e-10) {
			break;
		}
	}
	cout << "alpha opt iterations : " << iter << endl;

	return a;
}

/**
 *
 */
vector<double> CDirichletOptimizer::opt_asymmetric_dirichlet_hyper_param(vector<double>* ss, vector<double>* exp_ss, vector<double>* exp_square_ss, int D, int K) {

	vector<double> a = initialize_a( *exp_ss, *exp_square_ss, D, K);
	vector<double> g(K, 0), hg(K, 0);
	vector<double> a_minus_hg(K, 0);

	//print_vector(a, "initial a");
	double old_e = D*asymmetric_dirichlet_hyper_param_lhood(a, ss, D, K);
	double new_e;

	double eps = DBL_EPSILON;
	double lambda = 0.1;
	int max_iter = 100;

	bool abort;
	int iter;
	//cout << "initial old_e : " << old_e << endl;
	for ( iter = 1; iter < max_iter; ++iter ) {
		vector<double> old_a = a;
		//print_vector(old_a, "old_a");
		//print_vector(a, "a");
		double sum_a = accumulate(a.begin(), a.end(), 0.0);
		//cout << "!!! sum_a = " << sum_a << endl;
		if ( sum_a == 0 )
			break;

		for ( int k = 0; k < K; ++k ) {
			g[k] = CMathUtilities::digamma(sum_a) - CMathUtilities::digamma(a[k]) + (*ss)[k]/D;
		}
		abort = false;
		while (true) {

			asymmetric_dirichlet_hessian_times_gradient(a, g, lambda, K, hg);
			//print_vector(a, "a");
			//print_vector(hg, "hg");

			if ( is_all_less(hg, a) ) {
				for ( int k = 0; k < K; ++k )
					a_minus_hg[k] = a[k] - hg[k];	// a-hg = a_k - hg_k

				//print_vector(a_minus_hg, "a minus hg");
				new_e = D*asymmetric_dirichlet_hyper_param_lhood(a_minus_hg, ss, D, K);
				//cout << "iteration : " << iter << " : new_e " << new_e << " : old_e "<< old_e << endl << endl;
				if ( new_e > old_e ) {
					old_e = new_e;
					copy( a_minus_hg.begin(), a_minus_hg.end(), a.begin() );	// a - hg
					lambda /= 10;
					break;
				}
			}
			lambda *= 10;
			if ( lambda > 1e+6 ) {
				abort = true;
				break;
			}
		}
		if ( abort ) {
			new_e = old_e;
			break;
		}
		// change all elements in a that are less that eps to eps
		for ( int k = 0; k < K; ++k )
			if ( a[k] < eps ) a[k] = eps;

		double max_abs_a_minus_olda = abs(a[0]-old_a[0]);
		for ( int k = 1; k < K; ++k )
			if ( abs(a[k] - old_a[k]) > max_abs_a_minus_olda )
				max_abs_a_minus_olda = abs(a[k] - old_a[k]);
		if ( max_abs_a_minus_olda < 1e-10) {
			break;
		}
	}
	//print_vector(a, "final a");
	cout << "alpha opt iterations : " << iter << endl;

	return a;
}

/**
 * p = gammaln(sum(a)) - sum(gammaln(a)) + sum((a-1).*bar_p);
 */
double CDirichletOptimizer::asymmetric_dirichlet_hyper_param_lhood(vector<double>& a, vector<double>* ss, int D, int K) {
	double lhood = 0.0;

	//print_vector(a,"a from within lhood");
	double sum_a = accumulate(a.begin(), a.end(), 0.0);
	lhood += lgamma(sum_a);
	assert( !isinf(lhood) );
	//cout << "lhood = " << lhood << " sum_a = " << sum_a << " ";
	for ( int k = 0; k < K; ++k ) {
		lhood += (a[k]-1)*(*ss)[k]/D - lgamma(a[k]);
		//cout << "(" << k << " : " << a[k] << " : " << lgamma(a[k]) << " : " << ((a[k]-1)*(*ss)[k]/D - lgamma(a[k])) << " : " << lhood << ") ";
	}
	//cout << endl;

	return lhood;
}

/**
 * The last parameter is a reference to the output - this avoids returning a local vector object and that being copied into the lvalue
 * sa = sum(a);
 * q = -trigamma(a);
 * z = trigamma(sa);
 * q = q - lambda;
 * q = 1./q;
 * b = sum(g .* q)/(1/z + sum(q));
 * hg = (g - b).*q;
 *
 * hg is the output variable
 */
void CDirichletOptimizer::asymmetric_dirichlet_hessian_times_gradient(vector<double>& a, vector<double>& g, double lambda, int K, vector<double>& hg) {
	for ( int k = 0; k < K; ++k )
		hg[k] = 0;

	double sum_a = accumulate(a.begin(), a.end(), 0.0);
	vector<double> q = a;	// copy constructor
	for ( int k = 0; k < K; ++k )
		q[k] = -CMathUtilities::trigamma(a[k]) - lambda;

	double z = CMathUtilities::trigamma(sum_a);

	for ( int k = 0; k < K; ++k ) {
		q[k] = 1.0/q[k];
	}
	double sum_q = accumulate(q.begin(), q.end(), 0.0);

	double b = 0;
	double sum_g_dot_star_q = 0;
	for ( int k = 0; k < K; ++k ) {
		sum_g_dot_star_q += g[k]*q[k];
	}
	b = sum_g_dot_star_q/(1.0/z + sum_q);

	for ( int k = 0; k < K; ++k ) {
		hg[k] = (g[k] - b)*q[k];
	}
}

bool CDirichletOptimizer::is_all_less(std::vector<double>& a, std::vector<double>& b) {
	size_t size = a.size();
	for ( size_t i = 0; i < size; ++i ) {
		if ( a[i] >= b[i] )
			return false;
	}
	return true;
}

void CDirichletOptimizer::print_vector(vector<double>& a, string name) {
	int K = a.size();
	cout << name << " : " ;
	for ( int k = 0; k < K; ++k )
		cout << a[k] << " ";
	cout << endl;
}

