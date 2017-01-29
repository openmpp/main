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
    // Functions to save/restore random state
    // to support cloning in Modgen models.
    // Definitions in modgen_random_state.mpp
    extern void store_random_state();
    extern void retrieve_random_state();
};
#endif
