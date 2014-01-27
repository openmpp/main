/**
* @file    RangeSymbol.cpp
* Definitions for the RangeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "RangeSymbol.h"
#include "RangeEnumeratorSymbol.h"
#include "CodeBlock.h"

#include "location.hh"
#include "libopenm/omCommon.h"

using namespace std;
using namespace openm;

const string RangeSymbol::default_initial_value() const {
    return to_string(lower_bound);
}

void RangeSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
        {
            // Create enumerators for each integer within range
            for (int i=lower_bound, j=0; i<=upper_bound; ++i, ++j) {
                string enumerator_name = name + "_";
                if (i < 0) enumerator_name += "_" + to_string(-i);
                else  enumerator_name += to_string(i);
                auto sym = new RangeEnumeratorSymbol(enumerator_name, this, j, lower_bound);
            }

            // Information to compute storage type is known in this pass
            storage_type = optimized_storage_type(lower_bound, upper_bound);

            // Semantic errors for range can be detected in this pass
            if (lower_bound > upper_bound) {
                pp_error("semantic error, minimum of range is greater than maximum");
            }
        }
        break;
    default:
        break;
    }
}




CodeBlock RangeSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    h += doxygen("Range: " + name);
    h += "typedef Range<" + token_to_string(storage_type) + ", " + to_string(lower_bound) + ", " + to_string(upper_bound) + "> " + name + "; " ;

    return h;
}

