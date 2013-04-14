//============================================================================
// Name        : CorrTagSquaredLDA.cpp
// Author      : Pradipto Das
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#include <iostream>
using namespace std;

#include <iostream>
#include "CCorrTagSquaredLDATrainCPPUnit.h"
#include "CCorrTagSquaredLDATestCPPUnit.h"
#include "../Utilities/CUtilities.h"
using namespace std;

bool b_verbose_global;

int main(int argc, char **argv) {

	if (argc < 3) {
		CUtilities::print_command_help();
		return -1;
	}
	string config_file = argv[1];
	string model_operation_mode = argv[2];

	// setting the verbose option
	if ( (argc == 4) && ( (string("--verbose") == argv[3]) || (string("-v") == argv[3]) ) ) b_verbose_global = true;

	if ( model_operation_mode == "train" )
		CCorrTagSquaredLDATrainCPPUnit train_corr_mmtag2lda("", "", config_file);	// Note that the first two parameters are always set to empty string
	else if ( model_operation_mode == "test" )
		CCorrTagSquaredLDATestCPPUnit test_corr_mmtag2lda("", "", config_file);

	return 0;
}

