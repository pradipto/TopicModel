/*
 * AModelEMFunctionoid.h
 *
 *  Created on: May 20, 2010
 *      Author: pdas
 */

#ifndef AMODELEMFUNCTIONOID_H_
#define AMODELEMFUNCTIONOID_H_

class AModelEMFunctionoid {

protected:
	virtual ~AModelEMFunctionoid() {}		// this is provided to override a compiler warning

	virtual void operator()() = 0;

};

#endif /* AMODELEMFUNCTIONOID_H_ */
