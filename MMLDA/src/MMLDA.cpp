//============================================================================
// Name        : MMLDA.cpp
// Author      : Pradipto Das
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "CMMLDATrainCPPUnit.h"
#include "CMMLDATestCPPUnit.h"
#include "CMMLDAPredictCPPUnit.h"
#include "../Utilities/CUtilities.h"
using namespace std;

bool b_verbose_global;

int main(int argc, char **argv) {

	if (argc < 3) {
		CUtilities::print_command_help();
		return -1;
	}
	string model_operation_mode = argv[2];
	string config_file = argv[1];

	// setting the verbose option
	if ( (argc == 4) && ( (string("--verbose") == argv[3]) || (string("-v") == argv[3]) ) ) b_verbose_global = true;

	if ( model_operation_mode == "train" )
		CMMLDATrainCPPUnit train_mmlda("", "", config_file);	// Note that the first two parameters are always set to empty string
	else if ( model_operation_mode == "test" )
		CMMLDATestCPPUnit test_mmlda("", "", config_file);
	else if ( model_operation_mode == "predict" )
		CMMLDAPredictCPPUnit predict_mmlda("", "", config_file);

	return 0;
}

