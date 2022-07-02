/**
* @file    PartitionSymbol.cpp
* Definitions for the PartitionSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

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
                pp_error(LT("error : partition cut-points are not strictly increasing"));
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

    h += "";
    h += doxygen("Partition: " + name);

    // lower bounds of intervals in partition
    h += "extern const std::array<real, " + to_string(pp_size()) + "> om_" + name + "_lower;";
    // upper bounds of intervals in partition
    h += "extern const std::array<real, " + to_string(pp_size()) + "> om_" + name + "_upper;";
    // splitter map for partition
    h += "extern const std::map<real, " + token_to_string(storage_type) + "> om_" + name + "_splitter;";

    h += "extern const std::string om_name_" + name + ";";
    h += doxygen_short("Partition {0..." + to_string(pp_size() - 1) + "}: " + label());
    h += "typedef Partition<" + token_to_string(storage_type) + ", "
        + to_string(pp_size()) + ", "
        + "om_" + name + "_lower, "
        + "om_" + name + "_upper, "
        + "om_" + name + "_splitter, " 
        + "&om_name_" + name
        + "> "
        + name + ";" ;
    h += doxygen_short("C-type of " + name + " (" + exposed_type() + ")");
    h += "typedef " + exposed_type() + " " + name + "_t; // For use in model code";

    return h;
}

CodeBlock PartitionSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    c += "";
    c += doxygen_short(name);

    // Used below as argument to real_literal() to avoid compiler warnings if real is float
    bool real_is_float = RealSymbol::find()->is_float();

    int index;  // index of interval in partition

    c += "// lower bounds of intervals in partition";
    c += "const std::array<real, " + to_string(pp_size()) + "> om_" + name + "_lower = {";
    index = 0;  // index of interval in partition
    for (auto enumerator : pp_enumerators) {
        if (index > 0) {
            auto pes = dynamic_cast<PartitionEnumeratorSymbol *>(enumerator);
            assert(pes); // grammar guarantee
            c += real_literal(pes->lower_split_point, real_is_float) + ",";
        }
        else {
            // special case for upper limit of last partition interval
            c += "-REAL_MAX,";
        }
        ++index;
    }
    c += "};";

    c += "// upper bounds of intervals in partition";
    c += "const std::array<real, " + to_string(pp_size()) + "> om_" + name + "_upper = {";
    index = 0;
    for (auto enumerator : pp_enumerators) {
        if (index < (pp_size() - 1)) {
            auto pes = dynamic_cast<PartitionEnumeratorSymbol *>(enumerator);
            assert(pes); // grammar guarantee
            c += real_literal(pes->upper_split_point, real_is_float) + ",";
        }
        else {
            // special case for upper limit of last partition interval
            c += "REAL_MAX";
        }
        ++index;
    }
    c += "};";

    c += "// splitter map for partition";
    c += "const std::map<real, " + token_to_string(storage_type) + "> om_" + name + "_splitter = {";
    index = 0;  // index of interval in partition
    for (auto enumerator : pp_enumerators) {
        auto pes = dynamic_cast<PartitionEnumeratorSymbol *>(enumerator);
        assert(pes); // grammar guarantee
        c += "{ " + real_literal(pes->upper_split_point, real_is_float) + ", " + to_string(index) + " }," ;
        ++index;
        if (index == (pp_size() - 1)) break;
    }
    c += "};";

    c += "const std::string om_name_" + name + " = \"" + pretty_name() + "\";";

    return c;
}

bool PartitionSymbol::is_valid_constant(const Constant &k) const
{
    // named enumerators are invalid for a partition (only integers allowed)
    if (k.is_enumerator) return false;

    // floating point literals are invalid for partition
    if (is_floating()) return false;

    return is_valid_literal(k.value().c_str());
}

bool PartitionSymbol::is_valid_literal(const char * i_value) const
{
    if (i_value == nullptr) return false;

    // floating point literals are invalid for partition
    if (!IntegerLiteral::is_valid_literal(i_value)) return false;

    // value must fall with allowed ordinal values
    long v = stol(i_value);
    return 0 <= v && v < pp_size();
}

Constant * PartitionSymbol::make_constant(const string & i_value) const
{
    return (is_valid_literal(i_value.c_str())) ? new Constant(new Literal(i_value)) : nullptr;
}

//static
string PartitionSymbol::real_literal(string num, bool real_is_float)
{
    if (real_is_float) {
        // decimal point is required in value for a float literal
        // // e.g. 4.0f is valid but 4f is invalid.
        if (string::npos == num.find('.')) {
            return num + ".0f";
        }
        else {
            return num + "f";
        }
    }
    else {
        return num;
    }
}
