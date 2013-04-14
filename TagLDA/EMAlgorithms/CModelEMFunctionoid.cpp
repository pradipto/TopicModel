/*
 * CModelEMFunctionoid.cpp
 *
 *  Created on: Oct 13, 2010
 *      Author: data
 */

#include "CModelEMFunctionoid.h"
using namespace std;

CModelEMFunctionoid::CModelEMFunctionoid() {
	// TODO Auto-generated constructor stub

}

CModelEMFunctionoid::~CModelEMFunctionoid() {
	// TODO Auto-generated destructor stub
}

/**
 * @brief This function normalizes a vector of doubles
 *
 * @param vec is an stl::vector representing a vector of double
 */
void CModelEMFunctionoid::normalize_vector_double(vector<double>* vec) {
	vector<double>& v = *vec;
	double sum = std::accumulate(v.begin(), v.end(), 0.0);

	for_each(v.begin(), v.end(), SNormFunctionObj(sum));
}
