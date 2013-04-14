readme.txt

 Created on: Dec 9, 2010
     Author: Pradipto Das
	email: pdas3@buffalo.edu

===== COMPILING =====
$> cd bin
$> make clean
$> make
$> cd ..

===== DATASET FORMAT =====
- Model input files and dictionary files can be found in _sample_data folder. See files in _sample_data/model-MMLDA-HOG+OB/index. 
- The file names and file contents are quite self-explanatory
- The model.input files reads like:
#tokens [token_id:token_count]+ | #corresponding_tokens [corresponding_token_id:corresponding_token_count]+

===== TRAINING =====
$> bin/MMLDA _config_files/sample-train-config-mmlda-asym-HOGOB-aladdin-events-30.txt train --verbose

===== INFERENCE =====
$> bin/MMLDA _config_files/sample-test-config-mmlda-asym-HOGOB-aladdin-events-30.txt test --verbose

===== PREDICTION ====
$> bin/MMLDA _config_files/sample-dev-predict-config-mmlda-asym-HOGOB-aladdin-events-30 predict --verbose

