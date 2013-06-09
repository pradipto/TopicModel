/*
 * CMMLDAEMFunctionoid.cpp
 *
 *  Created on: Oct 14, 2010
 *      Author: data
 */

#include "CMMLDAEMFunctionoid.h"
#include "../Utilities/CUtilities.h"
#include "../Utilities/CDirichletOptimizer.h"
#include "../DataSet/CDictionaryReader.h"
using namespace std;

CMMLDAEMFunctionoid::CMMLDAEMFunctionoid() {
	_model = new CMMLDA();
	_base_model = dynamic_cast<CMMLDAGeneralModel*>(_model);
	_model_log_likelihood = 0;

}

CMMLDAEMFunctionoid::~CMMLDAEMFunctionoid() {
	delete _model;
}

// some globals for aladdin project
vector<vector<double>*>* global_training_topic_word_logprob_matrix;
map<size_t, string>* global_training_datum_id_label_map;

void CMMLDAEMFunctionoid::operator ()() {
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
	if ( _model->_operation_mode == "train" ) {
		_model->corpus_initialize();
		// do an initial mle to transfer the contents of the suff stats to the model parameters
		_model->_estimate_hypparam = false;

		maximization_step();
	} // else you already have loaded the model and therefore no need to corpus initialize

	// run expectation maximization
	_model->_estimate_hypparam = true;
	if ( _model->_D == 1 ) _model->_estimate_hypparam = false;	// alpha initializer not valid or too bad for single document online inference

	///// For prediction only
	if ( _model->_operation_mode == "predict" ) {
		////// -- new for prediction with datum count set to 0
		CParameterMatrixReaderWriter pmrw;
		global_training_topic_word_logprob_matrix = pmrw.read_param_matrix_from_text_file(_model->_model_train_root_dir + CUtilities::_file_path_separator + "model.beta");
		global_training_datum_id_label_map = CDictionaryReader::read_datum_label_to_id_dictionary_from_text_file(_model->_datum_to_id_dict_filename);
	}

	int i = 0;
	double log_likelihood_old = 0, converged = 1;
	double EM_CONVERGED = 0.0005;

	while (((converged < 0) || (abs(converged) > EM_CONVERGED) || (i <= 2))
			&& (i <= _model->_n_em_iter)) {

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
		if (converged < 0) _model->_n_em_var_iter = _model->_n_em_var_iter + 10;
		log_likelihood_old = _model_log_likelihood;
		//cout << "likelihood = " << likelihood << " : converged = " << converged << " num_em_steps = " << i << " (out of " << EM_MAX_ITER<< ")"<< endl;
		fflush(stdout);
		fprintf(stdout, "log_likelihood = % 14.8f  : converged = % 10.7f  num_em_steps = %- d (out of %- d) with em_var_iter = %d\n", _model_log_likelihood, converged, i, _model->_n_em_iter, _model->_n_em_var_iter);
		fprintf(fp_likelihood, "likelihood = % 14.8f  : converged = % 10.7f  num_em_steps = %- d (out of %- d) with em_var_iter = %d\n", _model_log_likelihood, converged, i, _model->_n_em_iter, _model->_n_em_var_iter);

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
		if (b_verbose_global) cout << "reading index file " << index_file << " and writing topics to file " << tw_filename << endl;
		CParameterMatrixReaderWriter pmr;
		pmr.print_topics(_model->_log_beta, index_file, 40, ofs);

		CParameterMatrixReaderWriter pmr2;
		string beta_filename = _model->get_model_root_dir() +	CUtilities::_file_path_separator + "topic_word.txt"; // deleted + CUtilities::number_to_string(_model->_K)
		string rho_filename = _model->get_model_root_dir() +	CUtilities::_file_path_separator + "topic_corrword.txt"; // deleted + CUtilities::number_to_string(_model->_K)
		ofstream ofs_topic_word(beta_filename.c_str());
		ofstream ofs_topic_corrword(rho_filename.c_str());

		cout << "Printing topic-word distribution using " << index_file << endl;
		pmr.print_topics(_model->_log_beta, index_file, 40, ofs_topic_word);
		cout << "Printing topic-corr_word distribution using " << corr_index_file << endl;
		pmr.print_topics(_model->_log_rho, corr_index_file, 40, ofs_topic_corrword);

		ofs.close();
		ofs_topic_word.close();
		ofs_topic_corrword.close();

	} else if ( _model->get_model_operation_mode() == "test" || _model->get_model_operation_mode() == "predict" ) {
		cout << "Performing final e-step() for document level assignments" << endl;
		final_e_step();
	}

	cout << endl << "done EM." << endl;
}

/**
 * @brief expectation step for MMLDA
 */
void CMMLDAEMFunctionoid::expectation_step() {
	_model_log_likelihood = 0;

	SMMLDADocument* doc;
	size_t d, k, n, m;
	double phi_nk, lambda_mk;

	// zero initialize sufficient statistics
	_model->zero_initialize_suff_stats();

	vector<double> alpha_ss_exp_temp(_model->_K);
	vector<double> alpha_ss_exp_square_temp(_model->_K);

	for ( d = 0; d < _model->_D; ++d ) {

		if ( d % 500 == 0 )
			cout << "Processing document " << d << endl;

		doc = _model->_corpus->_document_vec->at(d);
		_model_log_likelihood += doc_expectation_step(d, doc);

		// update sufficient statistics
		/*
		 * beta_ss
		 */
		for (k = 0; k < _model->_K; ++k) {

			for (m = 0; m < doc->_num_of_datum; ++m) {

				lambda_mk = (*(*_model->_var_lambda)[m])[k];

				(*(*_model->_beta_ss)[k])[ (*doc->_datum_id_vec)[m] ] +=
						(*doc->_datum_id_count_vec)[m] * lambda_mk;

				(*_model->_beta_total_ss)[k] +=
						(*doc->_datum_id_count_vec)[m] * lambda_mk;
			}
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


}

/**
 * @brief expectation step for CorrLDA
 */
double CMMLDAEMFunctionoid::doc_expectation_step(size_t doc_id, SMMLDADocument* doc) {
	double likelihood = 1, likelihood_old = 0.0, likelihood_temp, converged = 1.0;
	size_t k, n, m;

	double var_gamma_sum;
	double digamma_var_gamma_sum;

	double digamma_var_gamma[_model->_K];
	double old_var_phi[_model->_K];
	double old_var_lambda[_model->_K];
	double var_phi_sum, var_lambda_sum;

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
	digamma_var_gamma_sum = CMathUtilities::digamma(var_gamma_sum);	// Not used

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
		 * var_lambda --------------------------
		 */
		//cout << "computing var lambda" << endl;	//^^^^^^^^^^

		for (m = 0; m < doc->_num_of_datum; ++m) {

			for (k = 0; k < _model->_K; ++k) {

				old_var_lambda[k] = (*(*_model->_var_lambda)[m])[k];

				(*(*_model->_var_lambda)[m])[k] =
						(
								digamma_var_gamma[k]
								                  + (*(*_model->_log_beta)[k])[ (*doc->_datum_id_vec)[m] ]
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

				assert(!isnan(digamma_var_gamma[k]));

			}
			//digamma_var_gamma_sum = CMathUtilities::digamma(var_gamma_sum);

		}

		for ( k = 0; k < _model->_K; ++k )
			digamma_var_gamma[k] = CMathUtilities::digamma((*(*_model->_var_gamma)[doc_id])[k]);

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
void CMMLDAEMFunctionoid::maximization_step() {
	size_t k, j;

	// normalize the sufficient statistics
	/*
	 * beta
	 */
	for (k = 0; k < _model->_K; ++k) {

		for (j = 0; j < _model->_V; ++j) {

			if ( (*(*_model->_beta_ss)[k])[j] > 0 ) {

				(*(*_model->_log_beta)[k])[j] =
						log((*(*_model->_beta_ss)[k])[j])
						- log( (*_model->_beta_total_ss)[k] );
			} else {

				(*(*_model->_log_beta)[k])[j] = -100;
			}
		}
	}

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

	// estimate alpha and eta
	if ( _model->_estimate_hypparam ) {

		if ( _model->_estimate_hypparam ) {
			if ( b_verbose_global ) cout << "Estimating Alpha" << endl;
			if ( !_model->_b_use_asym_alpha )
				_model->_alpha = CDirichletOptimizer::opt_symmetric_dirichlet_hyper_param(_model->_alpha_ss, _model->_D, _model->_K);
			else	// using asymmetric alpha
				(*_model->_asym_alpha) = CDirichletOptimizer::opt_asymmetric_dirichlet_hyper_param( _model->_asym_alpha_ss, _model->_asym_alpha_exp_ss, _model->_asym_alpha_exp_square_ss, _model->_D, _model->_K );
			//(*_model->_asym_alpha) = CDirichletOptimizer::opt_asymmetric_dirichlet_hyper_param( _model->_asym_alpha_ss, _model->_D, _model->_K );
			// the first version takes less number of iterations
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
 * @note posterior inference over joint variables only
 */
void CMMLDAEMFunctionoid::final_e_step() {

	if ( _model->_operation_mode == "predict" ) {
		final_e_step_predict();
		return;
	}

	_model_log_likelihood = 0;

	SMMLDADocument* doc;
	size_t d, k, n, m;
	double phi_nk, lambda_mk;

	// zero initialize sufficient statistics
	_model->zero_initialize_suff_stats();

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

		if ( d % 500 == 0 )
			cout << "Processing document " << d << endl;

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
		for ( n = 0; n < phi->size(); ++n ) {
			vector<double>* phi_n = (*phi)[n];
			term_topic = CTemplateVectorMath<double>::get_max_index_from_vector(*phi_n);
			ofs_per_doc_assignments << " " << (*doc->_datum_id_vec)[n] << ":" << term_topic << ":";
			for ( size_t i = 0; i < _model->_K; ++i )
				ofs_per_doc_assignments << (*(*phi)[n])[i] << ",";
		}
		ofs_per_doc_assignments << endl;

		// update sufficient statistics
		//
		// beta_ss
		//
		for (k = 0; k < _model->_K; ++k) {

			for (m = 0; m < doc->_num_of_datum; ++m) {

				lambda_mk = (*(*_model->_var_lambda)[m])[k];

				(*(*_model->_beta_ss)[k])[ (*doc->_datum_id_vec)[m] ] +=
						(*doc->_datum_id_count_vec)[m] * lambda_mk;

				(*_model->_beta_total_ss)[k] +=
						(*doc->_datum_id_count_vec)[m] * lambda_mk;
			}
		}

		//
		// rho_ss
		//
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

/**
 * @brief compute final e step to obtain doc specific variational parameters
 * @note predictions - test input file has counts of the textual words to be 0
 */
void CMMLDAEMFunctionoid::final_e_step_predict() {

	_model_log_likelihood = 0;

	SMMLDADocument* doc;
	size_t d, k, n, m;
	double phi_nk, lambda_mk;

	// zero initialize sufficient statistics
	_model->zero_initialize_suff_stats();

	vector<double> alpha_ss_exp_temp(_model->_K);
	vector<double> alpha_ss_exp_square_temp(_model->_K);

	double doc_likelihood;
	string per_doc_elbo_filename = _model->get_model_root_dir() +
			CUtilities::_file_path_separator + "per_doc_likelihood.txt"; // deleted + CUtilities::number_to_string(_model->_K)
	ofstream ofs_per_doc_elbo(per_doc_elbo_filename.c_str());

	string per_doc_predictions_filename = _model->get_model_root_dir() + CUtilities::_file_path_separator +
			"per_doc_predictions-MMLDA-"+CUtilities::number_to_string<size_t>(_model->_K)+"-none-rewt.txt";
	ofstream ofs_per_doc_predictions(per_doc_predictions_filename.c_str());

	// this map is cleared and re-used for every document
	multimap<double, size_t, std::greater<double> > raw_pred_score_multimap;

	// iterate over documents
	for ( d = 0; d < _model->_D; ++d ) {

		if ( d % 50 == 0 )
			cout << "Processing document " << d << endl;

		doc = _model->_corpus->_document_vec->at(d);
		doc_likelihood = doc_expectation_step(d, doc);
		_model_log_likelihood += doc_likelihood;
		ofs_per_doc_elbo << setiosflags(ios::fixed) << setprecision(10) << doc_likelihood << endl;

		// get per doc predictions
		raw_pred_score_multimap.clear();

		size_t _V_train = global_training_topic_word_logprob_matrix->at(0)->size();
		for ( size_t v = 0; v < _V_train ; ++v ) {

			vector<double> topic_given_word_logprob_vec;
			// adjust the weight of the word v
			topic_given_word_logprob_vec.clear();
			for (k = 0; k < _model->_K; ++k)
				topic_given_word_logprob_vec.push_back(global_training_topic_word_logprob_matrix->at(k)->at(v));
			assert(topic_given_word_logprob_vec.size() == _model->_K);
			double topic_idf_normalizer = 0;
			for ( size_t i = 0; i < _model->_K; ++i )
				topic_idf_normalizer += topic_given_word_logprob_vec.at(i);	// the beta matrix is already in logspace
			topic_idf_normalizer *= (1.0/_model->_K);
			topic_idf_normalizer = exp(topic_idf_normalizer);

			double score_per_word = 0;

			for (n = 0; n < doc->_num_of_corr_datum; ++n) {
				for (k = 0; k < _model->_K; ++k) {

					phi_nk = (*(*_model->_var_phi)[n])[k];
					score_per_word += phi_nk * exp(global_training_topic_word_logprob_matrix->at(k)->at(v));
				}
			}

			raw_pred_score_multimap.insert(make_pair(score_per_word, v));
		}

		int c;
		multimap<double, size_t, std::greater<double> > pred_score_multimap;
		multimap<double, size_t, std::greater<double> >::iterator itr;

		for ( itr = raw_pred_score_multimap.begin(), c = 0; itr != raw_pred_score_multimap.end() ; ++itr, ++c ) {

			size_t termID = itr->second;
			double score = itr->first;
			// change score weight if necessary

			pred_score_multimap.insert(make_pair(score, termID));
		}

		//cout << "Doc " << d << " ------------" << endl;
		// final top 40 predicted words
		for ( itr = pred_score_multimap.begin(), c = 0; itr != pred_score_multimap.end() && c < 40; ++itr, ++c ) {
			string term = global_training_datum_id_label_map->find(itr->second)->second;

			ofs_per_doc_predictions << term << " ";

		}
		ofs_per_doc_predictions << endl;
		//cout << "-----------------" << endl;

		// update sufficient statistics
		//
		// beta_ss
		//
		for (k = 0; k < _model->_K; ++k) {

			for (m = 0; m < doc->_num_of_datum; ++m) {

				lambda_mk = (*(*_model->_var_lambda)[m])[k];

				(*(*_model->_beta_ss)[k])[ (*doc->_datum_id_vec)[m] ] +=
						(*doc->_datum_id_count_vec)[m] * lambda_mk;

				(*_model->_beta_total_ss)[k] +=
						(*doc->_datum_id_count_vec)[m] * lambda_mk;
			}
		}

		//
		// rho_ss
		//
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
	ofs_per_doc_predictions.close();

}
