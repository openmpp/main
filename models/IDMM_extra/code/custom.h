/**
 * @file   custom.h
 * Custom include file for the model
 *  
 * This file is included in all compiler-generated files for the model.
 * Place any required globals declarations in this file, or include other
 * files in this file, for correct placement in compiler-generated files.
 */

#pragma once
#include "omc/fixed_modgen_api.h"

#if defined(MODGEN)
namespace mm {

    // Functions related to random state.
    // Definitions in modgen_random_state.mpp
    extern void store_random_state();
    extern void retrieve_random_state();
    typedef std::vector<std_string> random_state;
    random_state serialize_random_state();
    void deserialize_random_state(const random_state& rs);
};
#endif
