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
Sample data and resource files can be found in _sample_data folder
Each row of model.input file looks like:
#num_of_terms term1_id:term1_count .... termN_id:termN_count

Note - The model.input files where termM_count is 1 indicates that the terms of a document follow a sequential order

===== TRAINING =====

$> bin/VanillaLDA _config_files/sample-train-config.txt train

The fields in the sample-train-config.txt are very self explanatory and are reproduced here:
file sample-train-config.txt:
#+-------------------------------------------------------------------------+

input file: _sample_data/YA09/index/model.input
input dict file: _sample_data/YA09/index/term_to_id.txt
num topics: 10

# Number of iterations 
# -------------------- # 
em iter: 40
em var iter: 50

# the model root directory where the training model will be stored
# ---------------------------------------------------------------- #
model root dir: _sample_data/YA09/lda_store/train/10

# the symmetric dirichlet prior for topic proportions
# --------------------------------------------------- #
topic proportion prior : 0.1

# use of asymmetric alpha	- if this is 1, then the "asummetric alpha file" needs to be present. If 0, then symmetric dirichlet is assumed 
# -----------------------
use asym alpha : 1

# asymmetric alpha concentration parameter	- this is the concentration parameter which is multiplied with the base measure
# -----------------------
asym alpha concentration parameter : 10

# asymmetric alpha file		- This is the file which stores the initial base-measure values for the K-dimensional dirichlet
#							- each line stores the \m_{k} value corresponding to the k^{th} dimension of \alpha
#							- If there are K topics, we can also just specify the largest M < K measures. 
#							- The rest of the measures are set to 0.1 and the full vector of base measures is then
#							- normalized between 0 and 1 and multiplied with the concentration parameter.
#				- If the file is commented out, all alphas are initialized to 0.1 and then the data optimizes the parameters
-----------------------
#asym alpha file : _sample_data/YA09/index/asym_alpha.txt						

#+-------------------------------------------------------------------------+

===== OUTPUT ======
The model files are stored in the directory corresponding to the "model root dir" in the training config file
The files are:
likelihood.train.txt	- The likelihoods calculated at each iteration
model.asymalpha			- The optimized values of asymmetric alphas
model.beta				- topic word parameter matrix
model.theta				- expected counts of topic proportions in documents
topic_word.txt			- top 40 words per topic

===== INFERENCE =====

$> bin/VanillaLDA _config_files/sample-test-config.txt test
The fields in the sample-test-config.txt are very self explanatory and are reproduced here:
file sample-test-config.txt:

#+-------------------------------------------------------------------------+

input file: _sample_data/YA09/index/model.input
input dict file: _sample_data/YA09/index/term_to_id.txt
#num topics: 10		// not needed - read from training files

# Number of iterations 
# -------------------- # 
em iter: 50
em var iter: 30

# the model root directory where the training model will be stored
# ---------------------------------------------------------------- #
model root dir: _sample_data/YA09/lda_store/test/10

model train root dir: _sample_data/YA09/lda_store/train/10

# use of asymmetric alpha	- Not needed since the model.other file in the model training directory has the necessary information
# -----------------------
#use asym alpha : 1

#+-------------------------------------------------------------------------+

