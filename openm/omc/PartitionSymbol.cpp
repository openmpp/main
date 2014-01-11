/**
* @file    PartitionSymbol.cpp
* Definitions for the PartitionSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "PartitionSymbol.h"
#include "PartitionEnumeratorSymbol.h"
#include "CodeBlock.h"

using namespace std;
using namespace openm;

void PartitionSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.

    switch ( pass ) {
    case ePopulateDependencies:
        {
            // Iterate the enumerators (now sorted)
            // to provide the lower limit of each enumerator.
            string lower_split_point = "min";
            for ( auto enumerator : pp_enumerators ) {
                auto partition_enumerator = dynamic_cast<PartitionEnumeratorSymbol *>(enumerator);
                assert(partition_enumerator); // parser guarantee
                partition_enumerator->lower_split_point = lower_split_point;
                lower_split_point = partition_enumerator->upper_split_point;
            }
        }
        break;
    default:
        break;
    }
}


CodeBlock PartitionSymbol::cxx_declaration_global()
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

