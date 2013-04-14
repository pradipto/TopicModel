/*
 * Dirichlet optimization
 */

#ifndef DIRI_HYPPARAM_H
#define DIRI_HYPPARAM_H

#include "../include/includes.h"
using namespace std;

double dirichlet_hyper_param_lhood(double a, double ss, int D, int K);
double d_dirichlet_hyper_param_lhood(double a, double ss, int D, int K);
double d2_dirichlet_hyper_param_lhood(double a, int D, int K);
double opt_dirichlet_hyper_param(double ss, int D, int K);

#endif
