#pragma once

#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>

#include <string>
#include <array>
#include <vector>
#include <stack>
#include <queue>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <regex>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cfloat>
#include <cmath>
#include <cassert>
#include <random>

#include <stdio.h>

#include <boost/functional/hash.hpp>

#include "log.h"
#include <omp.h>
#include "munkres.h"
#include "misc.h"
#include "geo.h"
#include "draw.h"

#define HUGE_FLOAT 1.e30
#define EPSILON 1.e-7

#define INTRA_MIA_ABUTT_WEIGHT (10.0)
#define MIA_WEIGHT (1.0)
#define FIXED_HARD_FILLER_WEIGHT (5.6)
#define ABS_DISPLACEMENT_WEIGHT (9.0)
#define INTRA_CELL_RIPUP_TIMES (1)
#define INTRA_CELL_RIPUP_FREQ (200)


// lef Reader modules
#include "lefrReader.hpp"
// def Reader modules
#include "defrReader.hpp"
#include "defiAlias.hpp"

#include "lefrReader.hpp"
#include "lefwWriter.hpp"
#include "lefiDebug.hpp"
#include "lefiEncryptInt.hpp"
#include "lefiUtil.hpp"


