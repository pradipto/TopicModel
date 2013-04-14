readme.txt

 Created on: Dec 9, 2010
     Author: data

===== COMPILING =====
$> cd bin
$> make clean
$> make
$> cd ..

===== DATASET FORMAT =====
Sample data and resource files can be found in _sample_data folder
Each row of model.input file is a document and reads like:

num_of_positions term1_id:term1_count:term1_tag_id .... termM_id:termM_count:termM_tag_id | num_of_unique_corresponding_terms corr_term1:corr_term1_count ... corr_termN:corr_termN_count

- where term1_count is always 1

===== TRAINING =====

$> bin/MMTagSquaredLDA _config_files/sample-train-config-cikm11AR-20.txt train

===== INFERENCE =====
$> bin/MMTagSquaredLDA _config_files/sample-test-config-cikm11AR-20.txt test


