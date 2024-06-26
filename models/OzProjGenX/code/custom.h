/**
 * @file   custom.h
 * Model-specific declarations and includes (late)
 *  
 * This file is included in all compiler-generated files for the model.
 * This file comes late in the include order, after entity declarations.
 */

#pragma once
#include <cassert>
#include "case_based.h"
#include "omc/fixed_modgen_api.h"

extern std::string piece_it(std::string path, int subs, int sub);