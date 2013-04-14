/*
 * CMMTagSquaredLDAEMFunctionoid.cpp
 *
 *  Created on: Oct 14, 2010
 *      Author: data
 */

#include "CMMTagSquaredLDAEMFunctionoid.h"
#include "../DataSet/CTagLDAParameterMatrixReaderWriter.h"
#include "../Utilities/TemplateMath.h"
#include "../Utilities/TemplateAlgo.h"
#include "../Utilities/CDirichletOptimizer.h"
using namespace std;

CMMTagSquaredLDAEMFunctionoid::CMMTagSquaredLDAEMFunctionoid() {
	_model = new CMMTagSquaredLDA();
	_base_model = dynamic_cast<CMMTagSquaredLDAGeneralModel*>(_model);
	_model_log_likelihood = 0;

}

CMMTagSquaredLDAEMFunctionoid::~CMMTagSquaredLDAEMFunctionoid() {
	delete _model;
}

void CMMTagSquaredLDAEMFunctionoid::operator ()() {
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
	 * initialize the parameters;
	 * while ( log_likelihood has converged or maxIter has exceeded ) do
	 * 	inference() // compute variational matrices and update suff stats
	 * 	mle();		// fill in the parameter matrices from the suff stats
	 * done
	 * write output to file
	 */
	if ( _model->_model_operation_mode == "train" ) {
		_model->semi_random_initialize();
		// do an initial mle to transfer the contents of the suff stats to the model parameters
		_model->_estimate_hypparam = false;

		maximization_step();
	} // else you already have loaded the model and therefore no need to corpus initialize

	// run expectation maximization
	_model->_estimate_hypparam = true;

	int i = 0;
	double log_likelihood_old = 0, converged = 1;
	double EM_CONVERGED = 0.0005;

	while (((converged < 0) || (abs(converged) > EM_CONVERGED) || (i <= 2))
			&& (i < _model->_n_em_iter)) {

		++i;
		cout << "**** EM iteration " << i << " ****" << endl;

		// e-step
		expectation_step();
		// m-step
		maximization_step();

		// compute likelihood - don't really have to do that since the e-step does that for you
		//_model_log_likelihood = _model->compute_log_likelihood();

		// check for convergence

		converged = (log_likelihood_old - _model_log_likelihood) / (log_likelihood_old);
		if (converged < 0) { _model->_n_em_var_iter = _model->_n_em_var_iter + 50; _model->_n_em_iter += 2; if ( _model->_n_em_iter >= 100 ) _model->_n_em_iter = 100;}
		log_likelihood_old = _model_log_likelihood;
		//cout << "likelihood = " << likelihood << " : converged = " << converged << " num_em_steps = " << i << " (out of " << EM_MAX_ITER<< ")"<< endl;
		fprintf(stdout, "log_likelihood = % 14.8f  : converged = % 10.7f  num_em_steps = %- d (out of %- d) with em_var_iter = %d\n", _model_log_likelihood, converged, i, _model->_n_em_iter, _model->_n_em_var_iter);
		fflush(stdout);
		fprintf(fp_likelihood, "likelihood = % 14.8f  : converged = % 10.7f  num_em_steps = %- d (out of %- d) with em_var_iter = %d\n", _model_log_likelihood, converged, i, _model->_n_em_iter, _model->_n_em_var_iter);
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
		string corr_index_file = _model->_corr_datum_to_id_dict_filename;
		string term_tag_index_file = _model->_tag_to_id_dict_filename;

		if (b_verbose_global) std::cout << "reading index file " << index_file << " and writing topics to file " << tw_filename << endl;
		CTagLDAParameterMatrixReaderWriter pmr;
		pmr.print_topics(_model->_log_beta, _model->_log_pi, index_file, term_tag_index_file, 50, ofs);

		CParameterMatrixReaderWriter pmr2;
		string beta_filename = _model->get_model_root_dir() +	CUtilities::_file_path_separator + "topic_word.txt"; // deleted + CUtilities::number_to_string(_model->_K)
		string pi_filename = _model->get_model_root_dir() +	CUtilities::_file_path_separator + "tag_word.txt"; // deleted + CUtilities::number_to_string(_model->_K)
		string rho_filename = _model->get_model_root_dir() +	CUtilities::_file_path_separator + "topic_corrword.txt"; // deleted + CUtilities::number_to_string(_model->_K)
		ofstream ofs_topic_word(beta_filename.c_str());
		ofstream ofs_tag_word(pi_filename.c_str());
		ofstream ofs_topic_corrword(rho_filename.c_str());


		cout << "Printing topic distribution " << endl;
		pmr2.print_topics(_model->_log_beta, index_file, 50, ofs_topic_word);
		cout << "Printing word-tag distribution" << endl;
		pmr2.print_topics(_model->_log_pi, index_file, 50, ofs_tag_word);
		cout << "Printing document-tag distribution " << endl;
		pmr2.print_topics(_model->_log_rho, corr_index_file, 50, ofs_topic_corrword);

		ofs.close();
		ofs_topic_word.close();
		ofs_tag_word.close();
		ofs_topic_corrword.close();

	} else if ( _model->get_model_operation_mode() == "test" ) {
		cout << "Performing final e-step() for document level assignments" << endl;
		final_e_step();
	}

	cout << endl << "done EM." << endl;
}

/**
 * @brief expectation step for TagLDA
 */
void CMMTagSquaredLDAEMFunctionoid::expectation_step() {

	SMMTagSquaredLDADocument* doc;
	size_t d, k, n, m;
	double lambda_mk;

	// zero initialize sufficient statistics
	_model->zero_initialize_suff_stats();

	// precompute the beta and pi sums for each row of those parameter matrices
	_model->precompute_beta_and_pi_row_sums();

	_model_log_likelihood = 0.0;

	vector<double> alpha_ss_exp_temp(_model->_K);
	vector<double> alpha_ss_exp_square_temp(_model->_K);

	for ( d = 0; d < _model->_D; ++d ) {

		if ( d % 450 == 0 )
			cout << "Processing document " << d << endl;

		doc = _model->_corpus->_document_vec->at(d);
		_model_log_likelihood += doc_expectation_step(d, doc);

		// update sufficient/marginal statistics
		double temp_ms_term1, temp_ms_term2;
		/*
		 * beta_ms
		 */
		double phi_nk;
		for (k = 0; k < _model->_K; ++k) {

			for (m = 0; m < doc->_num_of_datum; ++m) {

				lambda_mk = (*(*_model->_var_lambda)[m])[k];

				(*(*_model->_beta_ms_term1)[k])[ (*doc->_datum_id_vec)[m] ] +=
						(*doc->_datum_id_count_vec)[m] * lambda_mk;

				(*(*_model->_beta_ms_term2)[k])[ (*doc->_datum_id_vec)[m] ] +=
						(*doc->_datum_id_count_vec)[m] *
						(
								(1.0/(*_model->_var_zeta_perdoc)[m]) * lambda_mk *
								exp( (*(*_model->_log_pi)[ (*doc->_tag_id_vec)[m] ])[ (*doc->_datum_id_vec)[m] ] )
						);

			}

		}

		/*
		 * pi_ms
		 */
		for (m = 0; m < doc->_num_of_datum; ++m) {

			temp_ms_term1 = 0; temp_ms_term2 = 0;
			for (k = 0; k < _model->_K; ++k) {

				lambda_mk = (*(*_model->_var_lambda)[m])[k];

				temp_ms_term1 +=
						lambda_mk;

				temp_ms_term2 +=
						(
								lambda_mk *
								exp( (*(*_model->_log_beta)[k])[ (*doc->_datum_id_vec)[m] ] )
						);
				// note that beta and pi are in log space

			}

			temp_ms_term2 *= ( 1.0/(*_model->_var_zeta_perdoc)[m] );

			(*(*_model->_pi_ms_term1)[(*doc->_tag_id_vec)[m]])[ (*doc->_datum_id_vec)[m] ]
			                                                             += (*doc->_datum_id_count_vec)[m] * temp_ms_term1;
			(*(*_model->_pi_ms_term2)[(*doc->_tag_id_vec)[m]])[ (*doc->_datum_id_vec)[m] ]
			                                                             += (*doc->_datum_id_count_vec)[m] * temp_ms_term2;

		}

		/*
		 * rho_ss
		 */
		for (k = 0; k < _model->_K; ++k) {

			for (n = 0; n < doc->_num_of_corr_datum; ++n) {

				phi_nk = (*(*_model->_var_phi)[n])[k];

				(*(*_model->_rho_ss)[k])[ (*doc->_corr_datum_id_vec)[n] ] +=
						(*doc->_corr_datum_id_count_vec)[n] * phi_nk;

				(*_model->_rho_total_ss)[k] +=
						(*doc->_corr_datum_id_count_vec)[n] * phi_nk;
			}
		}


		// update alpha suff stats
		if ( !_model->_b_use_asym_alpha ) {
			double gamma_sum = 0;	// compute E[log(\theta_k)|\alpha]
			for (k = 0; k < _model->_K; ++k) {
				gamma_sum += (*(*_model->_var_gamma)[d])[k];
				_model->_alpha_ss += CMathUtilities::digamma( (*(*_model->_var_gamma)[d])[k] );
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
 * @brief expectation step for CorrLDA
 */
double CMMTagSquaredLDAEMFunctionoid::doc_expectation_step(size_t doc_id, SMMTagSquaredLDADocument* doc) {
	double likelihood = 1, likelihood_old = 0.0, likelihood_temp, converged = 1.0;
	size_t k, n, m;

	double var_gamma_sum;
	double digamma_var_gamma_sum;

	double digamma_var_gamma[_model->_K];
	double old_var_phi[_model->_K];
	double old_var_lambda[_model->_K];
	double var_phi_sum, var_lambda_sum, var_zeta_sum;

	// inference
	/*
	 * initialize vectors var_gamma
	 */
	var_gamma_sum = 0.0;
	for ( k = 0 ; k < _model->_K; ++k ) {
		if ( !_model->_b_use_asym_alpha ) (*(*_model->_var_gamma)[doc_id])[k] = _model->_alpha + ((double)doc->_total_num_of_corr_datum + doc->_total_num_of_datum)/_model->_K;
		else (*(*_model->_var_gamma)[doc_id])[k] = (*_model->_asym_alpha)[k] + ((double)doc->_total_num_of_corr_datum + doc->_total_num_of_datum)/_model->_K;
		digamma_var_gamma[k] = CMathUtilities::digamma( (*(*_model->_var_gamma)[doc_id])[k]);
		var_gamma_sum += (*(*_model->_var_gamma)[doc_id])[k];
	}
	digamma_var_gamma_sum = CMathUtilities::digamma(var_gamma_sum);

	/*
	 * initialize matrix var_lambda (M_max x K)
	 */
	for ( m = 0; m < doc->_num_of_datum; ++m )
		for ( k = 0; k < _model->_K; ++k )
			(*(*_model->_var_lambda)[m])[k] = 1.0/_model->_K;

	/*
	 * initialize matrix var_phi (N_max x K)
	 */
	for ( n = 0; n < doc->_num_of_corr_datum; ++n )
		for ( k = 0 ; k < _model->_K; ++k )
			(*(*_model->_var_phi)[n])[k] = 1.0/_model->_K;

	int var_iter = 0;
	double VAR_CONVERGED = 0.0001;


	while ( (converged > VAR_CONVERGED) &&
			((var_iter < _model->_n_em_var_iter) || (_model->_n_em_var_iter == -1)) ) {

		++var_iter;
		/*
		 * var_zeta --------------------------
		 */

		for (m = 0; m < doc->_num_of_datum; ++m) {

			var_zeta_sum = 0;
			for ( k = 0; k < _model->_K; ++k ) {

				var_zeta_sum += (*(*_model->_var_lambda)[m])[k] *
						( (*(*_model->_beta_pi_sum_mat)[k])[ (*doc->_tag_id_vec)[m] ] );

			}
			// assert that the denominator is not zero
			if ( var_zeta_sum <= 0 || isnan(var_zeta_sum) )
				cout << "var_zeta_sum = " << var_zeta_sum << endl;
			assert( var_zeta_sum > 0 );

			(*_model->_var_zeta_perdoc)[m] = var_zeta_sum;

		}


		/*
		 * var_lambda --------------------------
		 */
		//cout << "computing var lambda" << endl;	//^^^^^^^^^^

		for (m = 0; m < doc->_num_of_datum; ++m) {

			for (k = 0; k < _model->_K; ++k) {

				old_var_lambda[k] = (*(*_model->_var_lambda)[m])[k];

				assert( (*_model->_var_zeta_perdoc)[m] > 0 );

				// update log lambda
				(*(*_model->_var_lambda)[m])[k] =
						(
								digamma_var_gamma[k]
								                  + ( (*(*_model->_log_beta)[k])[ (*doc->_datum_id_vec)[m] ]
								                  + (*(*_model->_log_pi)[ (*doc->_tag_id_vec)[m] ])[ (*doc->_datum_id_vec)[m] ] )
								                  - ( (1.0/( *_model->_var_zeta_perdoc)[m])
								                  * (*(*_model->_beta_pi_sum_mat)[k])[ (*doc->_tag_id_vec)[m] ] )
						);

				if ( k > 0 )
					var_lambda_sum =
							CMathUtilities::log_sum((*(*_model->_var_lambda)[m])[k], var_lambda_sum);
				else
					var_lambda_sum = (*(*_model->_var_lambda)[m])[k];
			}

			/*
			 * updating var_gamma from lambda ----------------------
			 */
			for (k = 0; k < _model->_K; ++k) {

				//if ( isinf((*(*_model->_log_beta)[k])[ (*doc->_datum_id_vec)[m] ]) ) continue;	// added CHECK

				(*(*_model->_var_lambda)[m])[k] =
						exp( (*(*_model->_var_lambda)[m])[k] - var_lambda_sum );

				assert(!isnan((*(*_model->_var_lambda)[m])[k]));
				(*(*_model->_var_gamma)[doc_id])[k] +=
						(*doc->_datum_id_count_vec)[m] *
						((*(*_model->_var_lambda)[m])[k] - old_var_lambda[k]);

			}
		}

		/*
		 * var_phi --------------------------
		 * Note the way _gamma^{t} leads to updating _gamma^{t+1}
		 * 				\gamma_k^{t+1} = \alpha_k + \phi_{nk}^{t+1} + \lambda_{mk}^{t}
		 * 				\alpha_k = \gamma_k^{t} - \phi_{nk}^{t} - \lambda_{mk}^t
		 * \therefore, 	\gamma_k^{t+1} = \gamma_k^{t} + (\phi_{nk}^{t+1} - \phi_{nk}^{t}) + (\lambda_{mk}^{t+1} - \lambda_{mk}^{t})
		 */
		//cout << "computing var phi" << endl;	//^^^^^^^^^^

		for ( n = 0; n < doc->_num_of_corr_datum; ++n ) {

			for ( k = 0 ; k < _model->_K; ++k ) {

				old_var_phi[k] = (*(*_model->_var_phi)[n])[k];

				(*(*_model->_var_phi)[n])[k] =
						(
								digamma_var_gamma[k]
								                  + (*(*_model->_log_rho)[k])[ (*doc->_corr_datum_id_vec)[n] ]
						);

				// phi is in log space just like in LDA
				if ( k > 0 )
					var_phi_sum = CMathUtilities::log_sum(var_phi_sum, (*(*_model->_var_phi)[n])[k]);
				else
					var_phi_sum = (*(*_model->_var_phi)[n])[k];

				assert(!isnan(var_phi_sum));

			}

			/*
			 * further updating var_gamma from phi ----------------------
			 */
			var_gamma_sum = 0;
			for ( k = 0 ; k < _model->_K; ++k ) {

				// phi is normalized to [0,1] through var_phi_sum
				(*(*_model->_var_phi)[n])[k] =
						exp( (*(*_model->_var_phi)[n])[k] - var_phi_sum );

				(*(*_model->_var_gamma)[doc_id])[k] +=
						(*doc->_corr_datum_id_count_vec)[n] *
						((*(*_model->_var_phi)[n])[k] - old_var_phi[k]);

			}

		}

		for ( k = 0; k < _model->_K; ++k ) {
			digamma_var_gamma[k] = CMathUtilities::digamma((*(*_model->_var_gamma)[doc_id])[k]);
		}

		likelihood = _model->compute_doc_log_likelihood(doc_id, doc);

		assert(!isnan(likelihood));
		converged = (likelihood_old - likelihood) / likelihood_old;
		//cout << "likelihood for docID " << doc_id << " = " << likelihood << " at var_iter : " << var_iter
		//		<< " out of (" << _model->_n_em_var_iter
		//		<< ") and converged = " << converged << endl;
		likelihood_temp = likelihood_old;
		likelihood_old = likelihood;
	}
	//cout << "likelihood for docID " << doc_id << " = " << likelihood << " at var_iter : " << var_iter << endl;
	return likelihood_temp;
}

/**
 * @brief maximization step for CorrLDA
 */
void CMMTagSquaredLDAEMFunctionoid::maximization_step() {
	size_t k, j, t;

	// normalize the sufficient statistics
	/*
	 * rho
	 */
	for ( k = 0; k < _model->_K; ++k )	{

		for ( j = 0; j < _model->_corrV; ++j )	{

			if ( (*(*_model->_rho_ss)[k])[j] > 0) {
				(*(*_model->_log_rho)[k])[j] =
						log((*(*_model->_rho_ss)[k])[j])
						- log( (*_model->_rho_total_ss)[k] );
			} else
				(*(*_model->_log_rho)[k])[j] = -100;
		}
	}

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
				(*(*_model->_log_pi)[t])[j] =
						log( exp_log_pi_t_j );

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
 * @brief compute final e step to obtain doc specific variational parameters
 */
void CMMTagSquaredLDAEMFunctionoid::final_e_step() {
	SMMTagSquaredLDADocument* doc;
	size_t d, k, n, m;
	double lambda_mk;

	// zero initialize sufficient statistics
	_model->zero_initialize_suff_stats();

	// precompute the beta and pi sums for each row of those parameter matrices
	_model->precompute_beta_and_pi_row_sums();

	_model_log_likelihood = 0.0;

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
		vector<vector<double>* >* lambda = _model->_var_lambda;
		for ( m = 0; m < doc->_num_of_datum; ++m ) {
			vector<double>* lambda_m = (*lambda)[m];
			term_topic = CTemplateVectorMath<double>::get_max_index_from_vector(*lambda_m);
			ofs_per_doc_assignments << " " << (*doc->_datum_id_vec)[m] << ":" << term_topic << ":";
			for ( size_t i = 0; i < _model->_K; ++i )
				ofs_per_doc_assignments << (*(*lambda)[m])[i] << ",";
		}
		ofs_per_doc_assignments << endl;

		if ( d % 500 == 0 )
			cout << "Processing document " << d << endl;

		// update sufficient/marginal statistics
		double temp_ms_term1, temp_ms_term2;
		/*
		 * beta_ms
		 */
		//double initial_log_beta_ss[_model->_K];
		double phi_nk;
		for (k = 0; k < _model->_K; ++k) {

			for (m = 0; m < doc->_num_of_datum; ++m) {

				lambda_mk = (*(*_model->_var_lambda)[m])[k];

				(*(*_model->_beta_ms_term1)[k])[ (*doc->_datum_id_vec)[m] ] +=
						(*doc->_datum_id_count_vec)[m] * lambda_mk;

				(*(*_model->_beta_ms_term2)[k])[ (*doc->_datum_id_vec)[m] ] +=
						(*doc->_datum_id_count_vec)[m] *
						(
								(1.0/(*_model->_var_zeta_perdoc)[m]) * lambda_mk *
								exp( (*(*_model->_log_pi)[ (*doc->_tag_id_vec)[m] ])[ (*doc->_datum_id_vec)[m] ] )
						);

				// the following two terms were defined just for diagnostics
				temp_ms_term1 = (*doc->_datum_id_count_vec)[m] * lambda_mk;
				temp_ms_term2 = (*doc->_datum_id_count_vec)[m] *
						(
								(1.0/(*_model->_var_zeta_perdoc)[m]) * lambda_mk *
								exp( (*(*_model->_log_pi)[ (*doc->_tag_id_vec)[m] ])[ (*doc->_datum_id_vec)[m] ] )
						);


			}

		}

		/*
		 * pi_ms
		 */
		for (m = 0; m < doc->_num_of_datum; ++m) {

			temp_ms_term1 = 0; temp_ms_term2 = 0;
			for (k = 0; k < _model->_K; ++k) {

				lambda_mk = (*(*_model->_var_lambda)[m])[k];

				temp_ms_term1 +=
						lambda_mk;

				temp_ms_term2 +=
						(
								lambda_mk *
								exp( (*(*_model->_log_beta)[k])[ (*doc->_datum_id_vec)[m] ] )
						);
				// note that beta and pi are in log space

			}

			temp_ms_term2 *= ( 1.0/(*_model->_var_zeta_perdoc)[m] );


			(*(*_model->_pi_ms_term1)[(*doc->_tag_id_vec)[m]])[ (*doc->_datum_id_vec)[m] ]
			                                                             += (*doc->_datum_id_count_vec)[m] * temp_ms_term1;
			(*(*_model->_pi_ms_term2)[(*doc->_tag_id_vec)[m]])[ (*doc->_datum_id_vec)[m] ]
			                                                             += (*doc->_datum_id_count_vec)[m] * temp_ms_term2;

		}

		/*
		 * rho_ss
		 */
		for (k = 0; k < _model->_K; ++k) {

			for (n = 0; n < doc->_num_of_corr_datum; ++n) {

				phi_nk = (*(*_model->_var_phi)[n])[k];

				(*(*_model->_rho_ss)[k])[ (*doc->_corr_datum_id_vec)[n] ] +=
						(*doc->_corr_datum_id_count_vec)[n] * phi_nk;

				(*_model->_rho_total_ss)[k] +=
						(*doc->_corr_datum_id_count_vec)[n] * phi_nk;
			}
		}


		// update alpha suff stats
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

