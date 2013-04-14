#ifndef _INCLUDES_H_
#define _INCLUDES_H_

#include <string>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <set>
#include <stack>
#include <bitset>
#include <algorithm>
#include <functional>
#include <numeric>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <ios>
#include <istream>
#include <fstream>

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <limits>
#include <cassert>

//#define CUSTOMIZED
#ifdef CUSTOMIZED
#include "customized_headers.h"
#endif


#define UNIX
#ifdef UNIX
#include <ctime>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#endif

extern bool b_verbose_global;

#define APPLICATIONEXIT 1
#define DATALOADEREXIT 2
#define PRINTEREXIT 6
#define UTILITIESEXIT 7

#endif
