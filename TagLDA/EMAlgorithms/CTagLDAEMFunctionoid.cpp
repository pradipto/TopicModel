/*
 * CTagLDAEMFunctionoid.cpp
 *
 *  Created on: Oct 14, 2010
 *      Author: data
 */

#include "CTagLDAEMFunctionoid.h"
#include "../DataSet/CTagLDAParameterMatrixReaderWriter.h"
#include "../Utilities/CDirichletOptimizer.h"
using namespace std;

CTagLDAEMFunctionoid::CTagLDAEMFunctionoid() {
	_model = new CTagLDA();
	_base_model = dynamic_cast<CTagLDAGeneralModel*>(_model);
	_model_log_likelihood = 0;

}

CTagLDAEMFunctionoid::~CTagLDAEMFunctionoid() {
	delete _model;
}

void CTagLDAEMFunctionoid::operator ()() {
	////////////////////////////////////////////////////////////////
	//////////////////////////////// open likelihood file for saving
	string likelihood_filename = _model->get_model_root_dir() +
			CUtilities::_file_path_separator + "likelihood." + _model->get_model_operation_mode() + ".txt";
	FILE* fp_likelihood = fopen(likelihood_filename.c_str(), "w");
	if ( !fp_likelihood ) {
		if (b_verbose_global) cout << "Cannot open likelihood file " << likelihood_filename << endl;
		return;
	}
	/*
	 * initialize() the parameters;
	 * while ( log_likelihood has converged or maxIter has exceeded ) do
	 * 	inference() // compute variational matrices and update suff stats
	 * 	mle();		// fill in the parameter matrices from the suff stats
	 * done
	 * write output to file
	 */
	if ( _model->_model_operation_mode == "train" ) {
		_model->random_initialize();
		// do an initial mle to transfer the contents of the suff stats to the model parameters
		_model->_estimate_hypparam = false;

		maximization_step();
	}

	// run expectation maximization
	_model->_estimate_hypparam = true;

	int i = 0;
	double log_likelihood_old = 0, converged = 1;
	double EM_CONVERGED = 0.0005;

	while (((converged < 0) || (converged > EM_CONVERGED) || (i <= 2))
			&& (i < _model->_n_em_iter)) {

		++i;
		cout << "**** em iteration " << i << " ****\n";

		// e-step
		expectation_step();
		// m-step
		maximization_step();

		// check for convergence

		converged = (log_likelihood_old - _model_log_likelihood) / (log_likelihood_old);
		if (converged < 0) { _model->_n_em_var_iter = _model->_n_em_var_iter + 50; _model->_n_em_iter += 2;  if ( _model->_n_em_iter >= 100 ) _model->_n_em_iter = 100; }
		log_likelihood_old = _model_log_likelihood;
		//cout << "likelihood = " << likelihood << " : converged = " << converged << " num_em_steps = " << i << " (out of " << EM_MAX_ITER<< ")"<< endl;
		fprintf(stdout, "log_likelihood = % 14.8f  : converged = % 10.5f  num_em_steps = %- d (out of %- d)\n", _model_log_likelihood, converged, i, _model->_n_em_iter);
		fflush(stdout);
		fprintf(fp_likelihood, "likelihood = % 14.8f  : converged = % 10.5f  num_em_steps = %- d (out of %- d)\n", _model_log_likelihood, converged, i, _model->_n_em_iter);
		fflush(fp_likelihood);

	}

	// save model
	_model->save_model();
	fclose(fp_likelihood);

	// DIAGNOSTICS
	if ( _model->get_model_operation_mode() == "train" ) {
		string tw_filename = _model->get_model_root_dir() +
				CUtilities::_file_path_separator + "topicwords.txt"; // deleted + CUtilities::number_to_string(_model->_K)
		ofstream ofs(tw_filename.c_str());
		string index_file = _model->_datum_to_id_dict_filename;
		string tag_file = _model->_tag_to_id_dict_filename;
		if (b_verbose_global) std::cout << "reading index file " << index_file << " and writing topics to file " << tw_filename << endl;
		CTagLDAParameterMatrixReaderWriter pmr;
		pmr.print_topics(_model->_log_beta, _model->_log_pi, index_file, tag_file, 40, ofs);

		CParameterMatrixReaderWriter pmr2;
		string beta_filename = _model->get_model_root_dir() +	CUtilities::_file_path_separator + "topic_word.txt"; // deleted + CUtilities::number_to_string(_model->_K)
		string pi_filename = _model->get_model_root_dir() +	CUtilities::_file_path_separator + "tag_word.txt"; // deleted + CUtilities::number_to_string(_model->_K)
		ofstream ofs_topic_word(beta_filename.c_str());
		ofstream ofs_tag_word(pi_filename.c_str());

		cout << "Printing topic distribution " << endl;
		pmr2.print_topics(_model->_log_beta, index_file, 40, ofs_topic_word);
		cout << "Printing word-tag distribution" << endl;
		pmr2.print_topics(_model->_log_pi, index_file, 40, ofs_tag_word);

		ofs.close();
		ofs_topic_word.close();
		ofs_tag_word.close();

	} else if ( _model->get_model_operation_mode() == "test" ) {
		// run a final e-step
		cout << "Performing final e-step() for document level assignments" << endl;
		final_e_step();
	}

	cout << endl << "done EM." << endl;
}

/**
 * @brief expectation step for TagLDA
 */
double CTagLDAEMFunctionoid::doc_expectation_step(size_t doc_id, STagLDADocument* doc) {
	double likelihood = 1, likelihood_old = 0.0, likelihood_temp, converged = 1.0;
	size_t k, n;

	double digamma_var_gamma_sum;

	double digamma_var_gamma[_model->_K];
	double old_var_phi[_model->_K];
	double var_zeta_sum, var_phi_sum;

	// inference
	/*
	 * initialize vectors var_gamma (D x K)
	 */
	digamma_var_gamma_sum = 0;
	for ( k = 0 ; k < _model->_K; ++k ) {
		if ( !_model->_b_use_asym_alpha ) (*(*_model->_var_gamma)[doc_id])[k] = _model->_alpha + (double)doc->_total_num_of_datum/_model->_K;
		else (*(*_model->_var_gamma)[doc_id])[k] = (*_model->_asym_alpha)[k] + (double)doc->_total_num_of_datum/_model->_K;
		digamma_var_gamma[k] = CMathUtilities::digamma( (*(*_model->_var_gamma)[doc_id])[k]);
		digamma_var_gamma_sum += digamma_var_gamma[k];
	}

	/*
	 * initialize vectors var_zeta (N x 1)
	 */
	for ( n = 0; n < doc->_num_of_datum; ++n )
		(*_model->_var_zeta_perdoc)[n] = doc->_total_num_of_datum/2.0;

	/*
	 * initialize matrix var_phi (N x K)
	 */
	for ( n = 0; n < doc->_num_of_datum; ++n )
		for ( k = 0 ; k < _model->_K; ++k )
			(*(*_model->_var_phi)[n])[k] = 1.0/_model->_K;


	int var_iter = 0;
	double VAR_CONVERGED = 0.0001;

	while ((converged > VAR_CONVERGED) &&
			((var_iter < _model->_n_em_var_iter) || (_model->_n_em_var_iter == -1))) {

		++var_iter;

		/*
		 * var_zeta --------------------------
		 */
		double phi_n_k;


		for (n = 0; n < doc->_num_of_datum; ++n) {

			var_zeta_sum = 0;
			for ( k = 0; k < _model->_K; ++k ) {
				phi_n_k = ((*(*_model->_var_phi)[n])[k]);

				var_zeta_sum += phi_n_k *
						( (*(*_model->_beta_pi_sum_mat)[k])[ (*doc->_tag_id_vec)[n] ] );

			}
			// assert that the denominator is not zero
			if ( var_zeta_sum <= 0 || isnan(var_zeta_sum) )
				cout << "var_zeta_sum = " << var_zeta_sum << endl;
			assert( var_zeta_sum > 0 );

			(*_model->_var_zeta_perdoc)[n] = var_zeta_sum;

		}

		/*
		 * var_phi --------------------------
		 * Note the way _gamma^{t} leads to updating _gamma^{t+1} due to the presence of \alpha from the original model.
		 * 				\gamma_k^{t+1} = \alpha_k + \phi_{nk}^{t+1} where ((*(*_model->_var_phi)[n])[k]) = \phi_{nk}
		 * 				\alpha_k = \gamma_k^{t} - \phi_{nk}^{t}
		 * \therefore, 	\gamma_k^{t+1} = \gamma_k^{t} + (\phi_{nk}^{t+1} - \phi_{nk}^{t})
		 */

		for ( n = 0; n < doc->_num_of_datum; ++n ) {

			for ( k = 0 ; k < _model->_K; ++k ) {

				old_var_phi[k] = (*(*_model->_var_phi)[n])[k];

				assert( (*_model->_var_zeta_perdoc)[n] > 0 );

				// update log phi
				(*(*_model->_var_phi)[n])[k] =
						(*doc->_datum_id_count_vec)[n] *
						(
								digamma_var_gamma[k]
								                  + ( (*(*_model->_log_beta)[k])[ (*doc->_datum_id_vec)[n] ]
								                                                           + (*(*_model->_log_pi)[ (*doc->_tag_id_vec)[n] ])[ (*doc->_datum_id_vec)[n] ] )
								                                                           - ( (1.0/( *_model->_var_zeta_perdoc)[n]) * (*(*_model->_beta_pi_sum_mat)[k])[ (*doc->_tag_id_vec)[n] ] )
						);
				// phi is initially in log space in TagLDA  just like LDA
				if ( k > 0 ) {
					var_phi_sum = CMathUtilities::log_sum( var_phi_sum, (*(*_model->_var_phi)[n])[k] );
				} else {
					var_phi_sum = (*(*_model->_var_phi)[n])[k];
				}

				assert(!isnan(var_phi_sum));

			}
			// phi is normalized to [0,1] through var_phi_sum

			// note that we are still continuing on a particular n at this point

			/*
			 * updating var_gamma ----------------------
			 */

			for ( k = 0 ; k < _model->_K; ++k ) {

				(*(*_model->_var_phi)[n])[k] =
						exp( (*(*_model->_var_phi)[n])[k] - var_phi_sum );		// phi normalizer

				assert( (*(*_model->_var_phi)[n])[k] >= 0 && (*(*_model->_var_phi)[n])[k] <= 1 );

				// var_gamma[d][k] update (posterior dirichlet for topic proportions)
				(*(*_model->_var_gamma)[doc_id])[k] +=
						(*doc->_datum_id_count_vec)[n] * ( (*(*_model->_var_phi)[n])[k] - old_var_phi[k] );

				assert( (*(*_model->_var_gamma)[doc_id])[k] > 0 );	// problem here
				digamma_var_gamma[k] = CMathUtilities::digamma((*(*_model->_var_gamma)[doc_id])[k]);

				assert(!isnan(digamma_var_gamma[k]));

			}

		}

		//cout << "About to compute doc_log_likelihood" << endl;	//^^^^^
		likelihood = _model->compute_doc_log_likelihood(doc_id, doc);
		//cout << "Computed doc_log_likelihood" << endl;	//^^^^^

		assert(!isnan(likelihood));
		converged = (likelihood_old - likelihood) / likelihood_old;
		//cout << "likelihood for docID " << doc_id << " = " << likelihood << " at var_iter : " << var_iter
		//		<< " out of (" << _model->_n_em_var_iter
		//		<< ") and converged = " << converged << " with VAR_CONVERGED = "
		//		<< VAR_CONVERGED << endl;
		likelihood_temp = likelihood_old;
		likelihood_old = likelihood;
	}


	//cout << "likelihood for docID " << doc_id << " = " << likelihood << " at var_iter : " << var_iter << endl;
	return likelihood_temp;
}

/**
 * @brief expectation step for TagLDA
 */
void CTagLDAEMFunctionoid::expectation_step() {
	_model_log_likelihood = 0;

	STagLDADocument* doc;
	size_t d, k, n;

	// zero initialize sufficient statistics
	_model->zero_initialize_suff_stats();

	// precompute the beta and pi sums for each row of those parameter matrices
	_model->precompute_beta_and_pi_row_sums();

	_model_log_likelihood = 0;
	// iterate over all documents and compute the new _model_log_likelihood for the
	// new setting of parameters

	vector<double> alpha_ss_exp_temp(_model->_K);
	vector<double> alpha_ss_exp_square_temp(_model->_K);

	for ( d = 0; d < _model->_D; ++d ) {

		if ( d % 450 == 0 )
			cout << "Processing document " << d << endl;

		// compute doc expectation -----------------------------------
		doc = _model->_corpus->_document_vec->at(d);
		_model_log_likelihood += doc_expectation_step(d, doc);

		// update marginal statistics ------------------------------
		double temp_ms_term1, temp_ms_term2;
		/*
		 * beta_ms
		 */
		//double initial_log_beta_ss[_model->_K];
		double phi_nk;
		for (n = 0; n < doc->_num_of_datum; ++n) {

			for (k = 0; k < _model->_K; ++k) {

				phi_nk = (*(*_model->_var_phi)[n])[k];

				(*(*_model->_beta_ms_term1)[k])[ (*doc->_datum_id_vec)[n] ] +=
						(*doc->_datum_id_count_vec)[n] * phi_nk;

				(*(*_model->_beta_ms_term2)[k])[ (*doc->_datum_id_vec)[n] ] +=
						(*doc->_datum_id_count_vec)[n] *
						(
								(1.0/(*_model->_var_zeta_perdoc)[n]) * phi_nk *
								exp( (*(*_model->_log_pi)[ (*doc->_tag_id_vec)[n] ])[ (*doc->_datum_id_vec)[n] ] )
						);

			}

		}

		/*
		 * pi_ms
		 */
		for (n = 0; n < doc->_num_of_datum; ++n) {

			temp_ms_term1 = 0; temp_ms_term2 = 0;
			for (k = 0; k < _model->_K; ++k) {

				phi_nk = (*(*_model->_var_phi)[n])[k];

				temp_ms_term1 +=
						phi_nk;

				temp_ms_term2 +=
						(
								phi_nk *
								exp( (*(*_model->_log_beta)[k])[ (*doc->_datum_id_vec)[n] ] )
						);
				// note that beta and pi are in log space

			}

			temp_ms_term2 *= ( 1.0/(*_model->_var_zeta_perdoc)[n] );

			(*(*_model->_pi_ms_term1)[(*doc->_tag_id_vec)[n]])[ (*doc->_datum_id_vec)[n] ] += (*doc->_datum_id_count_vec)[n] * temp_ms_term1;
			(*(*_model->_pi_ms_term2)[(*doc->_tag_id_vec)[n]])[ (*doc->_datum_id_vec)[n] ] += (*doc->_datum_id_count_vec)[n] * temp_ms_term2;

		}

		if ( !_model->_b_use_asym_alpha ) {
			double gamma_sum = 0;	// compute E[log(\theta_k)|\alpha]
			for (k = 0; k < _model->_K; ++k) {
				gamma_sum += (*(*_model->_var_gamma)[d])[k];
				_model->_alpha_ss +=
						CMathUtilities::digamma( (*(*_model->_var_gamma)[d])[k] );
			}
			_model->_alpha_ss -= _model->_K * CMathUtilities::digamma(gamma_sum);
			// alpha suffstats is just cumulative over all topics
		} else {
			// asymmetric alpha
			double gamma_sum = 0;	// compute E[log(\theta_k)|\alpha]
			for (k = 0; k < _model->_K; ++k) {
				double gamma_dk = (*(*_model->_var_gamma)[d])[k];
				assert(gamma_dk > 0);
				gamma_sum += gamma_dk;
				double digamma_gamma_dk = CMathUtilities::digamma( gamma_dk );
				(*_model->_asym_alpha_ss)[k] += digamma_gamma_dk;
				alpha_ss_exp_temp[k] = digamma_gamma_dk;
			}
			double log_digamma_gammasum = CMathUtilities::digamma( gamma_sum );
			for (k = 0; k < _model->_K; ++k) {
				(*_model->_asym_alpha_ss)[k] -= log_digamma_gammasum;
				alpha_ss_exp_temp[k] -= log_digamma_gammasum;
			}
			for (k = 0; k < _model->_K; ++k) {
				double exp_element = exp(alpha_ss_exp_temp[k]);
				(*_model->_asym_alpha_exp_ss)[k] += exp_element;
				(*_model->_asym_alpha_exp_square_ss)[k] += (exp_element*exp_element);
			}

		}

	} // finished iterating over all documents

}

/**
 * @brief maximization step for TagLDA
 */
void CTagLDAEMFunctionoid::maximization_step() {
	size_t k, j, t;

	//cout << "M-step... " << endl;
	// normalize the sufficient statistics

	double sigma = _model->_sigma_regl_topic_word;
	double sigma_square = sigma*sigma;
	double term_1, term_2;
	/*
	 * beta
	 */
	for (k = 0; k < _model->_K; ++k) {

		for (j = 0; j < _model->_V; ++j) {

			term_1 = (*(*_model->_beta_ms_term1)[k])[j];
			term_2 = (*(*_model->_beta_ms_term2)[k])[j];

			double exp_log_beta_k_j = -sigma_square*term_2 + sigma*sqrt(sigma_square*term_2*term_2 + 4*term_1);
			// exp_log_beta_k_j can be 0 if both term_1 and term_2 are 0 OR the larger expression is 0 due to underflow/overflow
			// need to guard against the latter
			if ( exp_log_beta_k_j > 0 ) {
				(*(*_model->_log_beta)[k])[j] =
						log( exp_log_beta_k_j );
			} else {
				(*(*_model->_log_beta)[k])[j] = -100;
			}

		}

	}


	sigma = _model->_sigma_regl_tag_word;
	sigma_square = sigma*sigma;
	/*
	 * pi
	 */
	for ( t = 0; t < _model->_T; ++t )	{

		for ( j = 0; j < _model->_V; ++j )	{

			term_1 = (*(*_model->_pi_ms_term1)[t])[j];
			term_2 = (*(*_model->_pi_ms_term2)[t])[j];

			double exp_log_pi_t_j = -sigma_square*term_2 + sigma*sqrt(sigma_square*term_2*term_2 + 4*term_1);

			if ( exp_log_pi_t_j > 0 ) {
				(*(*_model->_log_pi)[t])[j] = log( exp_log_pi_t_j );

			} else
				(*(*_model->_log_pi)[t])[j] = -100;
		}

	}

	// estimate alpha and eta
	if ( _model->_estimate_hypparam ) {
		if ( _model->_estimate_hypparam ) {
			if ( b_verbose_global ) cout << "Estimating Alpha" << endl;
			if ( !_model->_b_use_asym_alpha )
				_model->_alpha = CDirichletOptimizer::opt_symmetric_dirichlet_hyper_param(_model->_alpha_ss, _model->_D, _model->_K);
			else	// using asymmetric alpha
				//(*_model->_asym_alpha) = CDirichletOptimizer::opt_asymmetric_dirichlet_hyper_param( _model->_asym_alpha_ss, _model->_D, _model->_K );
				(*_model->_asym_alpha) = CDirichletOptimizer::opt_asymmetric_dirichlet_hyper_param( _model->_asym_alpha_ss, _model->_asym_alpha_exp_ss, _model->_asym_alpha_exp_square_ss, _model->_D, _model->_K );
			if ( b_verbose_global && _model->_b_use_asym_alpha ) {
				cout << "Alpha: " ;
				for ( size_t k = 0; k < _model->_K; ++k )
					cout << (*_model->_asym_alpha)[k] << " ";
				cout << endl;
			}
		}
	}

}

/**
 * @brief a final e-step to get individual document level inferences
 */
void CTagLDAEMFunctionoid::final_e_step() {
	_model_log_likelihood = 0;

	STagLDADocument* doc;
	size_t d, k, n;

	// zero initialize sufficient statistics
	_model->zero_initialize_suff_stats();

	// precompute the beta and pi sums for each row of those parameter matrices
	_model->precompute_beta_and_pi_row_sums();

	//cout << "Total Docs = " << _model->_D << endl;

	_model_log_likelihood = 0;
	// iterate over all documents and compute the new _model_log_likelihood for the
	// new setting of parameters

	vector<double> alpha_ss_exp_temp(_model->_K);
	vector<double> alpha_ss_exp_square_temp(_model->_K);

	double doc_likelihood;
	string per_doc_elbo_filename = _model->get_model_root_dir() +
				CUtilities::_file_path_separator + "per_doc_likelihood.txt"; // deleted + CUtilities::number_to_string(_model->_K)
	ofstream ofs_per_doc_elbo(per_doc_elbo_filename.c_str());

	string per_doc_assignments_filename = _model->get_model_root_dir() + CUtilities::_file_path_separator +
			"per_doc_assignments.txt";
	ofstream ofs_per_doc_assignments(per_doc_assignments_filename.c_str());

	size_t doc_topic, term_topic;

	for ( d = 0; d < _model->_D; ++d ) {

		// compute doc expectation -----------------------------------
		doc = _model->_corpus->_document_vec->at(d);
		doc_likelihood = doc_expectation_step(d, doc);
		_model_log_likelihood += doc_likelihood;
		ofs_per_doc_elbo << setiosflags(ios::fixed) << setprecision(10) << doc_likelihood << endl;

		vector<double>* gamma_d = (*_model->_var_gamma)[d];
		doc_topic = CTemplateVectorMath<double>::get_max_index_from_vector(*gamma_d);
		ofs_per_doc_assignments << doc_topic << ":";
		for ( size_t i = 0; i < _model->_K; ++i )
			ofs_per_doc_assignments << (*gamma_d)[i] <<",";
		ofs_per_doc_assignments << " ";
		vector<vector<double>* >* phi = _model->_var_phi;
		for ( n = 0; n < doc->_num_of_datum; ++n ) {
			vector<double>* phi_n = (*phi)[n];
			term_topic = CTemplateVectorMath<double>::get_max_index_from_vector(*phi_n);
			ofs_per_doc_assignments << " " << (*doc->_datum_id_vec)[n] << ":" << term_topic << ":";
			for ( size_t i = 0; i < _model->_K; ++i )
				ofs_per_doc_assignments << (*(*phi)[n])[i] << ",";
		}
		ofs_per_doc_assignments << endl;

		if ( d % 500 == 0 )
			cout << "Processing document " << d << endl;

		// update sufficient statistics ------------------------------
		double temp_ms_term1, temp_ms_term2;
		/*
		 * beta_ms
		 */

		double phi_nk;
		for (n = 0; n < doc->_num_of_datum; ++n) {

			for (k = 0; k < _model->_K; ++k) {

				phi_nk = (*(*_model->_var_phi)[n])[k];

				(*(*_model->_beta_ms_term1)[k])[ (*doc->_datum_id_vec)[n] ] +=
						(*doc->_datum_id_count_vec)[n] * phi_nk;

				(*(*_model->_beta_ms_term2)[k])[ (*doc->_datum_id_vec)[n] ] +=
						(*doc->_datum_id_count_vec)[n] *
						(
								(1.0/(*_model->_var_zeta_perdoc)[n]) * phi_nk *
								exp( (*(*_model->_log_pi)[ (*doc->_tag_id_vec)[n] ])[ (*doc->_datum_id_vec)[n] ] )
						);

			}

		}

		/*
		 * pi_ms
		 */
		for (n = 0; n < doc->_num_of_datum; ++n) {

			temp_ms_term1 = 0; temp_ms_term2 = 0;
			for (k = 0; k < _model->_K; ++k) {

				phi_nk = (*(*_model->_var_phi)[n])[k];

				temp_ms_term1 +=
						phi_nk;

				temp_ms_term2 +=
						(
								phi_nk *
								exp( (*(*_model->_log_beta)[k])[ (*doc->_datum_id_vec)[n] ] )
						);
				// note that beta and pi are in log space

			}

			temp_ms_term2 *= ( 1.0/(*_model->_var_zeta_perdoc)[n] );

			(*(*_model->_pi_ms_term1)[(*doc->_tag_id_vec)[n]])[ (*doc->_datum_id_vec)[n] ] += (*doc->_datum_id_count_vec)[n] * temp_ms_term1;
			(*(*_model->_pi_ms_term2)[(*doc->_tag_id_vec)[n]])[ (*doc->_datum_id_vec)[n] ] += (*doc->_datum_id_count_vec)[n] * temp_ms_term2;

		}

		if ( !_model->_b_use_asym_alpha ) {
			double gamma_sum = 0;	// compute E[log(\theta_k)|\alpha]
			for (k = 0; k < _model->_K; ++k) {
				gamma_sum += (*(*_model->_var_gamma)[d])[k];
				_model->_alpha_ss +=
						CMathUtilities::digamma( (*(*_model->_var_gamma)[d])[k] );
			}
			_model->_alpha_ss -= _model->_K * CMathUtilities::digamma(gamma_sum);
			// alpha suffstats is just cumulative over all topics
		} else {
			// asymmetric alpha
			double gamma_sum = 0;	// compute E[log(\theta_k)|\alpha]
			for (k = 0; k < _model->_K; ++k) {
				double gamma_dk = (*(*_model->_var_gamma)[d])[k];
				assert(gamma_dk > 0);
				gamma_sum += gamma_dk;
				double digamma_gamma_dk = CMathUtilities::digamma( gamma_dk );
				(*_model->_asym_alpha_ss)[k] += digamma_gamma_dk;
				alpha_ss_exp_temp[k] = digamma_gamma_dk;
			}
			double log_digamma_gammasum = CMathUtilities::digamma( gamma_sum );
			for (k = 0; k < _model->_K; ++k) {
				(*_model->_asym_alpha_ss)[k] -= log_digamma_gammasum;
				alpha_ss_exp_temp[k] -= log_digamma_gammasum;
			}
			for (k = 0; k < _model->_K; ++k) {
				double exp_element = exp(alpha_ss_exp_temp[k]);
				(*_model->_asym_alpha_exp_ss)[k] += exp_element;
				(*_model->_asym_alpha_exp_square_ss)[k] += (exp_element*exp_element);
			}

		}

	} // finished iterating over all documents
	ofs_per_doc_elbo.close();
	ofs_per_doc_assignments.close();
}


