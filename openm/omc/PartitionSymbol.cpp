/**
* @file    PartitionSymbol.cpp
* Definitions for the PartitionSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "PartitionSymbol.h"
#include "PartitionEnumeratorSymbol.h"
#include "RealSymbol.h"
#include "CodeBlock.h"

using namespace std;
using namespace openm;

const string PartitionSymbol::default_initial_value() const
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
            // Information to compute storage type is known in this pass
            storage_type = optimized_storage_type(0, pp_size() - 1);
            //SFG storage_type = token::TK_int;

            // Iterate the enumerators (now sorted)
            // to provide the lower limit of each enumerator.
            // Also check that enumerators are strictly increasing.
            string lower_bound = "min";
            double lower_bound_value = -DBL_MAX;
            string upper_bound = ""; // initial value doesn't matter
            double upper_bound_value = 0; // initial value doesn't matter
            size_t j = 0; // enumerator counter
            bool order_ok = true;
            for ( auto e : pp_enumerators ) {
                auto pe = dynamic_cast<PartitionEnumeratorSymbol *>(e);
                assert(pe); // parser guarantee

                // upper bound of interval is already known
                upper_bound = pe->upper_split_point;
                if (j <= pp_enumerators.size() - 2) {
                    upper_bound_value = stod(upper_bound);
                }
                else {
                    // upper bound of final interval is something like 'max'
                    upper_bound_value = DBL_MAX;
                }

                // assign lower bound to interval
                pe->lower_split_point = lower_bound;

                // check that partition cutpoints are strictly increasing
                order_ok = order_ok && upper_bound_value > lower_bound_value;

                // prepare for next interval in iteration
                ++j;
                lower_bound = upper_bound;
                lower_bound_value = upper_bound_value;
            }

            if (!order_ok) {
                pp_error("semantic error, partition cutpoints are not strictly increasing");
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

    h += doxygen("Partition: " + name);

    // lower bounds of intervals in partition
    h += "extern const array<real, " + to_string(pp_size()) + "> om_" + name + "_lower;";
    // upper bounds of intervals in partition
    h += "extern const array<real, " + to_string(pp_size()) + "> om_" + name + "_upper;";
    // splitter map for partition
    h += "extern const map<real, " + token_to_string(storage_type) + "> om_" + name + "_splitter;";

    h += "typedef Partition<" + token_to_string(storage_type) + ", "
        + to_string(pp_size()) + ", "
        + "om_" + name + "_lower, "
        + "om_" + name + "_upper, "
        + "om_" + name + "_splitter> " + name + ";" ;

    return h;
}

CodeBlock PartitionSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    h += doxygen_short(name);

    // Determine floating point literal suffix
    // (required to avoid compiler warnings if real is float)
    string literal_suffix = RealSymbol::find()->is_float() ? "f" : "";

    int index;  // index of interval in partition

    h += "// lower bounds of intervals in partition";
    h += "const array<real, " + to_string(pp_size()) + "> om_" + name + "_lower = {";
    index = 0;  // index of interval in partition
    for (auto enumerator : pp_enumerators) {
        if (index > 0) {
            auto pes = dynamic_cast<PartitionEnumeratorSymbol *>(enumerator);
            assert(pes); // grammar guarantee
            h += pes->lower_split_point + literal_suffix + ",";
        }
        else {
            // special case for upper limit of last partition interval
            h += "-REAL_MAX,";
        }
        ++index;
    }
    h += "};";

    h += "// upper bounds of intervals in partition";
    h += "const array<real, " + to_string(pp_size()) + "> om_" + name + "_upper = {";
    index = 0;
    for (auto enumerator : pp_enumerators) {
        if (index < (pp_size() - 1)) {
            auto pes = dynamic_cast<PartitionEnumeratorSymbol *>(enumerator);
            assert(pes); // grammar guarantee
            h += pes->upper_split_point + literal_suffix + ",";
        }
        else {
            // special case for upper limit of last partition interval
            h += "REAL_MAX";
        }
        ++index;
    }
    h += "};";

    h += "// splitter map for partition";
    h += "extern const map<real, " + token_to_string(storage_type) + "> om_" + name + "_splitter = {";
    index = 0;  // index of interval in partition
    for (auto enumerator : pp_enumerators) {
        auto pes = dynamic_cast<PartitionEnumeratorSymbol *>(enumerator);
        assert(pes); // grammar guarantee
        h += "{ " + pes->upper_split_point + literal_suffix + ", " + to_string(index) + " }," ;
        ++index;
        if (index == (pp_size() - 1)) break;
    }
    h += "};";


    return h;
}

