/*
 * TemplateMath.h
 *
 *  Created on: Mar 1, 2010
 *      Author: pdas3
 */

#ifndef TEMPLATEMATH_H_
#define TEMPLATEMATH_H_

#include "includes.h"

template<typename T>
class CTemplateVectorMath {

public:

	static void normalize_vector(std::vector<T>& v);
	static void normalize_vector(std::vector<T>& v, size_t limit);
	static T dot_product(std::vector<T> const & v1, std::vector<T> const & v2);
	static T sum_vector(std::vector<T> const & v);
	static void hadamard_product(std::vector<T> const & v1, std::vector<T> const & v2, std::vector<T>& result);
	static int get_max_index_from_vector(std::vector<T> const & v);
};

template<typename T>
void CTemplateVectorMath<T>::normalize_vector(std::vector<T>& v) {
	double init = 0.0, sum;
	sum = std::accumulate(v.begin(), v.end(), init);
	size_t vec_length = v.size();
	for ( size_t i = 0; i < vec_length; ++i )
		v[i] = v[i]/sum;
}

template<typename T>
void CTemplateVectorMath<T>::normalize_vector(std::vector<T>& v, size_t limit) {
	double init = 0.0, sum;
	typename std::vector<T>::iterator itr = v.begin();
	std::advance(itr, limit-1);
	sum = std::accumulate(v.begin(), itr, init);
	size_t vec_length = v.size();
	for ( size_t i = 0; i < vec_length; ++i )
		v[i] = v[i]/sum;
}

template<typename T>
T CTemplateVectorMath<T>::dot_product(std::vector<T> const & v1, std::vector<T> const & v2) {
	size_t len_v1 = v1.size();
	size_t len_v2 = v2.size();

	assert( len_v1 == len_v2 );
	T dot_prod = 0;
	for ( size_t i = 0; i < len_v1; ++i )
		dot_prod += v1[i]*v2[i];

	return dot_prod;
}

/*
 * returns the max of the elements in the std::vector<T> v and
 * the index of the max element is kept in the reference index
 * complexity - O(N) where N is the # of elements in the std::vector
 */
template<typename T>
int CTemplateVectorMath<T>::get_max_index_from_vector(std::vector<T> const & v)	{
	T mx = 0;
	int index = -1;
	size_t length = v.size();
	for (size_t i = 0; i < length; ++i) {
		//if (b_verbose_global) std::cout << " " << v->at(i);
		if ( v.at(i) > mx ) {
			mx = v.at(i);
			index = i;
		}
	}

	return index;
}

template<typename T>
T CTemplateVectorMath<T>::sum_vector(std::vector<T> const & v) {
	size_t v_len = v.size();
	T sum_elements = 0;
	for ( size_t i = 0; i < v_len; ++i )
		sum_elements += v[i];

	return sum_elements;
}

template<typename T>
void CTemplateVectorMath<T>::hadamard_product(std::vector<T> const & v1, std::vector<T> const & v2, std::vector<T>& result) {
	size_t len_v1 = v1.size();
	size_t len_v2 = v2.size();
	size_t len_r = result.size();

	assert( len_v1 == len_v2 == len_r );

	for ( size_t i = 0; i < len_v1; ++i )
		result[i] = v1[i]*v2[i];
}

template<typename T>
class CTemplateMatrixMath {

public:

	static T max_three_numbers(const T& a, const T& b, const T& c);
	static void max_value_indices_in_array(T *v, size_t num_elements, int *index);

	// templatized matrix manipulation functions
	static void zero_initialize_matrix(std::vector<std::vector<T>* >* mat);
	static void zero_initialize_vector(std::vector<T>* vec);
	static void create_2D_matrix(std::vector<std::vector<T>* >** mat, size_t n_rows, size_t n_cols);
	static void delete_2D_matrix(std::vector<std::vector<T>* >* mat);

};

template<typename T>
T CTemplateMatrixMath<T>::max_three_numbers(const T& a, const T& b, const T& c) {
	T maxi = a;
	if ( b > a ) maxi = b;
	if ( c > maxi ) maxi = c;
	return maxi;
}

template<typename T>
void CTemplateMatrixMath<T>::max_value_indices_in_array(T* v, size_t num_elements, int *index) {
	T mx = std::numeric_limits<T>::min();
	for (size_t i = 0; i < num_elements; ++i)
		if ( v[i] > mx ) mx = v[i];

	for (size_t i = 0; i < num_elements; ++i) {
		index[i] = 0;
		if ( v[i] == mx ) index[i] = 1;
	}
	return;
}

/**
 * some common memory allocation routines for matrices represented as std::vectors
 */
template <typename T>
void CTemplateMatrixMath<T>::zero_initialize_matrix(std::vector<std::vector<T>* >* mat)
{
	for ( size_t i = 0; i < mat->size(); i++ )
		for ( size_t j = 0; j < mat->at(i)->size(); j++ )
			mat->at(i)->at(j) = 0;
}
template <typename T>
void CTemplateMatrixMath<T>::zero_initialize_vector(std::vector<T>* vec)
{
	for ( size_t i = 0; i < vec->size(); i++ )
		vec->at(i) = 0;
}
template <typename T>
void CTemplateMatrixMath<T>::create_2D_matrix(std::vector<std::vector<T>* >** mat, size_t n_rows, size_t n_cols)
{
	//if (b_verbose_global) std::cout << nRows << "x" << nCols << std::endl;
	*mat = new std::vector< std::vector<T>* >(n_rows);
	for ( size_t i = 0 ; i < n_rows ; i++ ) {
		std::vector<T>* row = new std::vector<T>(n_cols, 0);
		(*mat)->at(i) = row;
	}
}
template <typename T>
void CTemplateMatrixMath<T>::delete_2D_matrix(std::vector<std::vector<T>* >* mat)
{
	size_t n_rows = mat->size();
	for ( size_t i = 0 ; i < n_rows ; i++ ) {
		delete mat->at(i);
	}
	delete mat;
	mat = 0x0;
}

/**
 * Plan to use this class sometime
 */
template<typename T>
class CMatrix2D {
	/**
	 * The 2D matrix is a std::vector of std::vectors
	 */
	std::vector<std::vector<T>* >* _mat;
	size_t _rows;
	size_t _cols;

public:

	CMatrix2D () {
		_rows = 0;
		_cols = 0;
	}

	/**
	 * constructor
	 */
	CMatrix2D(size_t rows, size_t cols) {
		CTemplateMatrixMath<T>::create_2D_matrix(_mat, rows, cols);
		_rows = rows;
		_cols = cols;
	}

	/**
	 * destructor
	 */
	~CMatrix2D() {
		CTemplateMatrixMath<T>::delete_2D_matrix(_mat);
	}

	size_t get_rows() { return _rows; }
	size_t get_cols() { return _cols; }

	/**
	 * overloading operator () for matrix element access
	 */
	inline T& operator() (size_t r, size_t c) {
		if (r < 0 || r > _rows ) {
			if (b_verbose_global) std::cout<<"Illegal row index";
			return 0;
		} else if ( c < 0 || c > _cols) {
			if (b_verbose_global) std::cout<<"Illegal Column Index:";
			return 0;
		} else return (*(*_mat)[r])[c];
	}

	/**
	 * overloading << operator for stream insertion operations
	 */
	inline friend std::ostream& operator<< (std::ostream& os, CMatrix2D<T>& mat){
		size_t rows = mat.get_rows();
		size_t cols = mat.get_cols();
		for(size_t i = 0; i < rows; ++i) {
			for(size_t j=0; j < cols; ++j)
				os << mat(i,j) << " ";
			os << std::endl;
		}
		return os;
	}

	/**
	 * overloading = operator
	 */
	inline CMatrix2D<T>& operator= (CMatrix2D<T>& mat){
		size_t rows = mat.get_rows();
		size_t cols = mat.get_cols();
		CTemplateMatrixMath<T>::create_2D_matrix(this->_mat, rows, cols);
		for (size_t i = 0; i < rows; ++i )
			for(size_t j = 0; j < cols; ++j )
				this->_mat(i,j) = mat(i,j);

		return *this;
	}

	/**
	 * copy constructor
	 */
	CMatrix2D(CMatrix2D<T>& mat){
		size_t rows = mat.get_rows();
		size_t cols = mat.get_cols();
		CTemplateMatrixMath<T>::create_2D_matrix(this->_mat, rows, cols);
		for (size_t i = 0; i < rows; ++i )
			for(size_t j = 0; j < cols; ++j )
				this->_mat(i,j) = mat(i,j);
		return;
	}

	/**
	 * reset elements in the matrix to 0
	 */
	inline void reset() {
		CTemplateMatrixMath<T>::zero_initialize_matrix(this->_mat);
	}

	/**
	 * print the matrix
	 */
	inline void print() {
		for (size_t i = 0; i < _rows; ++i ) {
			for(size_t j = 0; j < _cols; ++j )
				if (b_verbose_global) std::cout << _mat(i,j) << " ";
			if (b_verbose_global) std::cout << std::endl;
		}
	}
};

/**
 * declaring a namespace for utility functions
 */
//namespace math_utilities_ns{
class CMathUtilities{

public:
	/*
	 * given log(a) and log(b), return log(a + b)
	 *
	 */
	static double log_sum(double log_a, double log_b)
	{
		double v;

		if (log_a < log_b) {
			v = log_b+log(1 + exp(log_a-log_b));
		} else {
			v = log_a+log(1 + exp(log_b-log_a));
		}
		return(v);
	}

	/**
	 * Proc to calculate the value of the trigamma, the second
	 * derivative of the loggamma function. Accepts positive matrices.
	 * From Abromowitz and Stegun.  Uses formulas 6.4.11 and 6.4.12 with
	 * recurrence formula 6.4.6.  Each requires workspace at least 5
	 * times the size of X.
	 *
	 **/

	static double trigamma(double x)
	{
		double p;
		int i;

		x=x+6;
		p=1/(x*x);
		p=(((((0.075757575757576*p-0.033333333333333)*p+0.0238095238095238)
				*p-0.033333333333333)*p+0.166666666666667)*p+1)/x+0.5*p;
		for (i=0; i<6 ;i++)
		{
			x=x-1;
			p=1/(x*x)+p;
		}
		return(p);
	}


	/*
	 * taylor approximation of first derivative of the log gamma function
	 *
	 */

	static double digamma(double x)
	{
		double p;
		x=x+6;
		p=1/(x*x);
		p=(((0.004166666666667*p-0.003968253986254)*p+
				0.008333333333333)*p-0.083333333333333)*p;
		if ( x < 0 ) printf("in digamma, x = %f ", x);
		assert(x>0);
		p=p+log(x)-0.5/x-1/(x-1)-1/(x-2)-1/(x-3)-1/(x-4)-1/(x-5)-1/(x-6);
		return p;
	}


	static double log_gamma(double x)
	{
		double z=1/(x*x);

		x=x+6;
		z=(((-0.000595238095238*z+0.000793650793651)
				*z-0.002777777777778)*z+0.083333333333333)/x;
		z=(x-0.5)*log(x)-x+0.918938533204673+z-log(x-1)-
		log(x-2)-log(x-3)-log(x-4)-log(x-5)-log(x-6);
		return z;
	}


	/*
	 * argmax
	 *
	 */
	static int argmax(double* x, int n)
	{
		int i;
		double max = x[0];
		int argmax = 0;
		for (i = 1; i < n; ++i)
		{
			if (x[i] > max)
			{
				max = x[i];
				argmax = i;
			}
		}
		return(argmax);
	}

};

//}	// end namespace MathUtilities

#endif /* TEMPLATEMATH_H_ */
