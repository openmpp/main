/**
* @file    RangeSymbol.cpp
* Definitions for the RangeSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "RangeSymbol.h"
#include "EnumeratorSymbol.h"
#include "CodeBlock.h"

using namespace std;
using namespace openm;

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
                auto sym = new EnumeratorSymbol(enumerator_name, this, j);
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

    h += doxygen(name);
    h += "enum " + name + " {";
    for (auto enumerator : pp_enumerators) {
        h += enumerator->name + ",";
    }
    h += "};";

    return h;
}

