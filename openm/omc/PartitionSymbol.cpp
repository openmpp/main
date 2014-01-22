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

const string PartitionSymbol::get_initial_value() const
{
    // initial value is the first interval
    return "0";
};

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
    // Ex. extern real om_cutpoints_AGE_GROUP[];
    h += "extern real om_cutpoints_" + name + "[];";
    // Ex. typedef Partition<int, 5, om_cutpoints_AGE_GROUP> AGE_GROUP;
    h += "typedef Partition<int, " + to_string(pp_size()) + ", om_cutpoints_" + name + "> " + name + "; " ;

    return h;
}

CodeBlock PartitionSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    h += doxygen(name);
    h += "real om_cutpoints_" + name + "[] = { ";
    int j = 0;
    for (auto enumerator : pp_enumerators) {
        auto pes = dynamic_cast<PartitionEnumeratorSymbol *>(enumerator);
        assert(pes); // grammar guarantee
        h += pes->upper_split_point + ",";
        // omit last partition interval, since it has no upper break-point
        if ( (pp_size() - 1) == ++j ) break;
    }
    h += "};";

    return h;
}

