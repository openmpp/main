/**
 * @file   custom.h
 * Model-specific declarations and includes (late)
 *  
 * This file is included in all compiler-generated files for the model.
 * This file comes late in the include order, after entity declarations.
 */

#pragma once
#include "case_based.h"

#if defined(MODGEN)
namespace mm {
    // x-compatible API for random state storage and retrieval
    // (to support cloning).
    // Definitions in modgen_case_based.mpp
    extern void store_random_state();
    extern void retrieve_random_state();
};
#endif
