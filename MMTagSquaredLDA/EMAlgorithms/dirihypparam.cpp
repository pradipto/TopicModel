
#include "dirihypparam.h"
#include "../include/TemplateMath.h"


/*
 * objective function and its derivatives
 *
 */

double dirichlet_hyper_param_lhood(double a, double ss, int D, int K)
{ return(D * (lgamma(K * a) - K * lgamma(a)) + (a - 1) * ss); }

double d_dirichlet_hyper_param_lhood(double a, double ss, int D, int K)
{ return(D * (K * CMathUtilities::digamma(K * a) - K * CMathUtilities::digamma(a)) + ss); }

double d2_dirichlet_hyper_param_lhood(double a, int D, int K)
{ return(D * (K * K * CMathUtilities::trigamma(K * a) - K * CMathUtilities::trigamma(a))); }


/*
 * newtons method
 *
 */
double opt_dirichlet_hyper_param(double ss, int D, int K)
{
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
        f = dirichlet_hyper_param_lhood(a, ss, D, K);
        //cout << "** " << a << ":" << ss << ":" << D << ":" << K << endl;
        df = d_dirichlet_hyper_param_lhood(a, ss, D, K);
        //if ( isnan(df) ) {
        //	cout << "HypParam derivative assertion failed " << endl;
        //}
        d2f = d2_dirichlet_hyper_param_lhood(a, D, K);
        //assert( !isnan(df) );
        log_a = log_a - df/(d2f * a + df);
        if (isnan(log_a))
        {
        	init_a = init_a * 10;
            printf("warning : Dirichlet hyperparameter is nan; new init = %5.5f\n", init_a);
            a = init_a;
            log_a = log(a);

        }
        //printf("Dirichlet hyperparameter maximization : %5.5f   %5.5f   %5.5f\n", f, df, d2f);
    }
    while ((fabs(df) > 1e-5) && (iter < 1000));
    assert( !isnan(log_a) );
    return(exp(log_a));
}
