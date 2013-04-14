/*
 * TemplatePrintContainer.h
 *
 *  Created on: Mar 1, 2010
 *      Author: pdas3
 */

#ifndef TEMPLATEPRINT_H_
#define TEMPLATEPRINT_H_

namespace PrintUtilities{

	// print a set
	template<typename T>
	static void print_set(std::set<T>* s) {
		typename std::set<T>::iterator itr;
		for (itr = s->begin(); itr != s->end(); itr++ )
			if (b_verbose_global) std::cout << *itr << " ";
		if (b_verbose_global) std::cout << std::endl;
	}
}

#endif /* TEMPLATEPRINT_H_ */
