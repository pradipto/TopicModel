/*
 * CVanillaAsymLDAEMFunctionoid.cpp
 *
 *  Created on: Oct 14, 2010
 *      Author: data
 */

#include "CVanillaAsymLDAEMFunctionoid.h"
#include "../DataSet/CParameterMatrixReaderWriter.h"
#include "../Utilities/TemplateMath.h"
#include "../Utilities/CDirichletOptimizer.h"
using namespace std;

CVanillaAsymLDAEMFunctionoid::CVanillaAsymLDAEMFunctionoid() {
	_model = new CVanillaAsymLDA();
	_base_model = dynamic_cast<CVanillaAsymLDAGeneralModel*>(_model);
	_model_log_likelihood = 0;

}

CVanillaAsymLDAEMFunctionoid::~CVanillaAsymLDAEMFunctionoid() {
	delete _model;
}

/**
 * @brief operator() is the algorithm that operates on the model, thereby changing it's "state".\n
 * In this case the algorithm is a local minimizer called EM short for Expectation Maximization
 */
void CVanillaAsymLDAEMFunctionoid::operator ()() {
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
	 * while ( log_likelihood has converged or max_iter has been exceeded )
	 * do
	 * 	inference() // compute variational matrices and update suff stats
	 * 	mle();		// fill in the parameter matrices from the suff stats
	 * done
	 * write outputs to files
	 */
	if ( _model->_model_operation_mode == "train" ) {
		_model->corpus_initialize();
		// do an initial mle to transfer the contents of the suff stats to the model parameters
		_model->_estimate_hypparam = false;

		maximization_step();
	}

	// run expectation maximization
	_model->_estimate_hypparam = true;

	int i = 0;
	double log_likelihood_old = 0, converged = 1;
	double EM_CONVERGED = 0.0001;

	while (((converged < 0) || (converged > EM_CONVERGED) || (i <= 2))
			&& (i <= _model->_n_em_iter)) {

		++i;

		_model->_estimate_hypparam = true; // false is for debug mode only
		// e-step
		expectation_step();
		// m-step
		maximization_step();

		// compute likelihood - don't really have to do that since the e-step does that for you
		//_model_log_likelihood = _model->compute_log_likelihood();

		// check for convergence

		converged = (log_likelihood_old - _model_log_likelihood) / (log_likelihood_old);
		if (converged < 0) _model->_n_em_var_iter = _model->_n_em_var_iter + 50;
		log_likelihood_old = _model_log_likelihood;

		fprintf(stdout, "log_likelihood = % 14.8f  : converged = % 10.5f  num_em_steps = %- d (out of %- d) %d em_var_iter\n", _model_log_likelihood, converged, i, _model->_n_em_iter, _model->_n_em_var_iter);
		fflush(stdout);
		fprintf(fp_likelihood, "likelihood = % 14.8f  : converged = % 10.5f  num_em_steps = %- d (out of %- d) %d em_var_iter\n", _model_log_likelihood, converged, i, _model->_n_em_iter, _model->_n_em_var_iter);
		fflush(fp_likelihood);

	}

	// save model
	_model->save_model();
	fclose(fp_likelihood);

	// DIAGNOSTICS
	if ( _model->get_model_operation_mode() == "train" ) {
		string tw_filename = _model->get_model_root_dir() +
				CUtilities::_file_path_separator + "topic_word.txt"; // deleted + CUtilities::number_to_string(_model->_K)
		ofstream ofs(tw_filename.c_str());
		string index_file = _model->_datum_to_id_dict_filename;
		if (b_verbose_global) cout << "reading index file " << index_file << " and writing topics to file " << tw_filename << endl;
		CParameterMatrixReaderWriter pmr;
		pmr.print_topics(_model->_log_beta, index_file, 40, ofs);

		string topic_assignment_filename = _model->get_model_root_dir() +
				CUtilities::_file_path_separator + "topicassign.train.txt";
		//write_doc_topic_assignments(topic_assignment_filename);

	} else if ( _model->get_model_operation_mode() == "test" ) {

		string tw_filename = _model->get_model_root_dir() +
				CUtilities::_file_path_separator + "topic_word.txt"; // deleted + CUtilities::number_to_string(_model->_K)
		ofstream ofs(tw_filename.c_str());
		string index_file = _model->_datum_to_id_dict_filename;
		if (b_verbose_global) cout << "reading index file " << index_file << " and writing topics to file " << tw_filename << endl;
		CParameterMatrixReaderWriter pmr;
		pmr.print_topics(_model->_log_beta, index_file, 40, ofs);

		string topic_assignment_filename = _model->get_model_root_dir() +
				CUtilities::_file_path_separator + "topicassign.test.txt";

		// a final expectation step to write the word topic assignments
		write_doc_topic_assignments(topic_assignment_filename);
	}

	cout << endl << "done EM." << endl;
}

/**
 * @brief expectation step for VanillaLDA
 */
double CVanillaAsymLDAEMFunctionoid::doc_expectation_step(size_t doc_id, SVanillaAsymLDADocument* doc) {
	double likelihood = 1, likelihood_old = 0.0, likelihood_temp = 0.0, converged = 1.0;
	size_t k, n;

	double digamma_var_gamma[_model->_K];
	double old_var_phi[_model->_K];
	double var_phi_sum;

	// Note doc->_total_num_of_datum is the total number of datum in the document
	// If this is not set properly, var_gamma will be less than 0
	// inference
	/*
	 * initialize vectors var_gamma (D x K)
	 */
	for ( k = 0 ; k < _model->_K; ++k ) {
		if ( !_model->_b_use_asym_alpha ) (*(*_model->_var_gamma)[doc_id])[k] = _model->_alpha + ((double)doc->_total_num_of_datum)/_model->_K;
		else (*(*_model->_var_gamma)[doc_id])[k] = (*_model->_asym_alpha)[k] + ((double)doc->_total_num_of_datum)/_model->_K;
		digamma_var_gamma[k] = CMathUtilities::digamma( (*(*_model->_var_gamma)[doc_id])[k]);
	}

	/*
	 * initialize matrix var_phi (N x K)
	 */
	for ( n = 0; n < doc->_num_of_datum; ++n )
		for ( k = 0 ; k < _model->_K; ++k )
			(*(*_model->_var_phi)[n])[k] = 1.0/_model->_K;

	int var_iter = 0;
	double VAR_CONVERGED = 0.00001;

	while ( (converged > VAR_CONVERGED) &&
			((var_iter < _model->_n_em_var_iter) || (_model->_n_em_var_iter == -1)) ) {

		++var_iter;


		/*
		 * var_phi --------------------------
		 * Note the way _gamma^{t} leads to updating _gamma^{t+1} due to the presence of \alpha from the original model.
		 * 				\gamma_k^{t+1} = \alpha_k + \phi_{nk}^{t+1} where ((*(*_model->_var_phi)[n])[k]) = \phi_{nk}
		 * 				\alpha_k = \gamma_k^{t} - \phi_{nk}^{t}
		 * \therefore, 	\gamma_k^{t+1} = \gamma_k^{t} + (\phi_{nk}^{t+1} - \phi_{nk}^{t})
		 */
		//cout << "computing var phi" << endl;	//^^^^^^^^^^

		for ( n = 0; n < doc->_num_of_datum; ++n ) {

			var_phi_sum = 0;
			for ( k = 0 ; k < _model->_K; ++k ) {

				old_var_phi[k] = (*(*_model->_var_phi)[n])[k];

				// update log phi
				(*(*_model->_var_phi)[n])[k] =
						(
								digamma_var_gamma[k]
								                  + (*(*_model->_log_beta)[k])[ (*doc->_datum_id_vec)[n] ]
						);
				// phi is initially in log space
				if ( k > 0 ) {
					var_phi_sum = CMathUtilities::log_sum(var_phi_sum, (*(*_model->_var_phi)[n])[k]);
				} else {
					var_phi_sum = (*(*_model->_var_phi)[n])[k];
				}

			}
			// phi is normalized to [0,1] through var_phi_sum

			// note that we are still continuing on a particular n at this point

			/*
			 * updating var_gamma ----------------------
			 */
			for ( k = 0 ; k < _model->_K; ++k ) {

				(*(*_model->_var_phi)[n])[k] =
						exp( (*(*_model->_var_phi)[n])[k] - var_phi_sum );		// phi normalizer

				// assert( (*(*_model->_var_phi)[n])[k] >= 0 && (*(*_model->_var_phi)[n])[k] <= 1 );

				// var_gamma[d][k] update (posterior dirichlet for topic proportions)

				(*(*_model->_var_gamma)[doc_id])[k] +=
						(*doc->_datum_id_count_vec)[n] *
						( (*(*_model->_var_phi)[n])[k] - old_var_phi[k] );

				assert( (*(*_model->_var_gamma)[doc_id])[k] > 0 );
				//digamma_var_gamma[k] = CMathUtilities::digamma((*(*_model->_var_gamma)[doc_id])[k]);

			}
			//cout << endl;	// ^^^^^
		}

		for ( k = 0 ; k < _model->_K; ++k )
			digamma_var_gamma[k] = CMathUtilities::digamma((*(*_model->_var_gamma)[doc_id])[k]);

		likelihood = _model->compute_doc_log_likelihood(doc_id, doc);

		assert(!isnan(likelihood));
		converged = (likelihood_old - likelihood) / likelihood_old;
		//cout << "likelihood for docID " << docID << " = " << likelihood << " at var_iter : " << var_iter
		//		<< " out of (" << VAR_MAX_ITER << ") and converged = " << converged << " with VAR_CONVERGED = " << VAR_CONVERGED << endl;
		likelihood_temp = likelihood_old;
		likelihood_old = likelihood;
	}


	//cout << "likelihood for docID " << doc_id << " = " << likelihood << " at var_iter : " << var_iter << endl;
	return likelihood_temp;
}

/**
 * @brief expectation step for TagLDA
 */
void CVanillaAsymLDAEMFunctionoid::expectation_step() {
	_model_log_likelihood = 0;

	SVanillaAsymLDADocument* doc;
	size_t d, k, n;

	// zero initialize sufficient statistics
	_model->zero_initialize_suff_stats();

	_model_log_likelihood = 0;
	// iterate over all documents and compute the new _model_log_likelihood for the
	// new setting of parameters

	vector<double> alpha_ss_exp_temp(_model->_K);
	vector<double> alpha_ss_exp_square_temp(_model->_K);

	for ( d = 0; d < _model->_D; ++d ) {

		if ( d % 500 == 0 )
			cout << "Processing document " << d << endl;

		// compute doc expectation -----------------------------------
		doc = _model->_corpus->_document_vec->at(d);
		_model_log_likelihood += doc_expectation_step(d, doc);


		// update sufficient statistics ------------------------------
		/*
		 * beta_ss
		 */
		double phi_nk;
		for (n = 0; n < doc->_num_of_datum; ++n) {

			for (k = 0; k < _model->_K; ++k) {

				phi_nk = (*(*_model->_var_phi)[n])[k];

				(*(*_model->_beta_ss)[k])[ (*doc->_datum_id_vec)[n] ] +=
						(*doc->_datum_id_count_vec)[n] * phi_nk;

				(*_model->_beta_total_ss)[k] += (*doc->_datum_id_count_vec)[n] * phi_nk;
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

	// diagnostics -------
	//cout << _model->_D << " : " << _model->_K << endl;
	//if ( _model->_b_use_asym_alpha ) {
	//	for (k = 0; k < _model->_K; ++k)
	//		cout << (*_model->_asym_alpha)[k] << "  ";
	//	cout << endl;
	//	for (k = 0; k < _model->_K; ++k)
	//		cout << (*_model->_asym_alpha_ss)[k] << "  ";
	//	cout << endl;
	//} else
	//	cout << _model->_alpha_ss << endl;
	//exit(1);
	// -------------------
}

/**
 * @brief maximization step for TagLDA
 */
void CVanillaAsymLDAEMFunctionoid::maximization_step() {
	size_t k, j;

	// normalize the sufficient statistics
	/*
	 * beta
	 */
	for (k = 0; k < _model->_K; ++k) {

		for (j = 0; j < _model->_V; ++j) {

			if ( ( (*(*_model->_beta_ss)[k])[j] ) > 0 ) {

				(*(*_model->_log_beta)[k])[j] =
						log((*(*_model->_beta_ss)[k])[j])
						- log((*_model->_beta_total_ss)[k]);

			} else {
				(*(*_model->_log_beta)[k])[j] = -100;
			}
		}
	}


	// estimate alpha
	//cout << "m_model->m_estimate_hypparam = " << _model->_estimate_hypparam << endl;
	if ( _model->_estimate_hypparam ) {
		if ( b_verbose_global ) cout << "Estimating Alpha" << endl;
		if ( !_model->_b_use_asym_alpha )
			_model->_alpha = CDirichletOptimizer::opt_symmetric_dirichlet_hyper_param(_model->_alpha_ss, _model->_D, _model->_K);
		else	// using asymmetric alpha
			(*_model->_asym_alpha) = CDirichletOptimizer::opt_asymmetric_dirichlet_hyper_param( _model->_asym_alpha_ss, _model->_asym_alpha_exp_ss, _model->_asym_alpha_exp_square_ss, _model->_D, _model->_K );
			//(*_model->_asym_alpha) = CDirichletOptimizer::opt_asymmetric_dirichlet_hyper_param( _model->_asym_alpha_ss, _model->_D, _model->_K );
			// the first version takes less number of iterations and is more reliable
		if ( b_verbose_global && _model->_b_use_asym_alpha ) {
			cout << "Alpha: " ;
			for ( size_t k = 0; k < _model->_K; ++k )
				cout << (*_model->_asym_alpha)[k] << " ";
			cout << endl;
		}
	}

}

/*
 * @brief
 * @return bool
 */
bool CVanillaAsymLDAEMFunctionoid::write_doc_topic_assignments(string filename) {

	ofstream ofs(filename.c_str());
	if ( ofs.is_open() ) {
		// do a doc e-step and dump max of each phi_{d,n} for each n in doc d
		double model_log_likelihood = 0;

		SVanillaAsymLDADocument* doc;
		size_t d, n;

		// zero initialize sufficient statistics
		_model->zero_initialize_suff_stats();

		model_log_likelihood = 0;
		// iterate over all documents and compute the new _model_log_likelihood for the
		// new setting of parameters

		int topic;
		for ( d = 0; d < _model->_D; ++d ) {

			// compute doc expectation -----------------------------------
			doc = _model->_corpus->_document_vec->at(d);
			model_log_likelihood += doc_expectation_step(d, doc);

			// get the major topic of the document
			topic = CTemplateVectorMath<double>::get_max_index_from_vector( *(*_model->_var_gamma)[d] );
			ofs << "doc_topic:" << topic << " ";

			// write word assignments
			for (n = 0; n < doc->_num_of_datum; ++n) {

				topic = CTemplateVectorMath<double>::get_max_index_from_vector( *(*_model->_var_phi)[n] );
				ofs << " " << (*doc->_datum_id_vec)[n] << ":" << topic;
			}

			ofs << endl;

		}

		cout << "final e-step likelihood = " << model_log_likelihood << endl;
		cout << "written topic assigments to file " << filename << endl;

		ofs.close();
		return true;

	} else {
		cout << "Cannot open file " << filename << " for writing word topic assignments" << endl;
		return false;
	}
}


