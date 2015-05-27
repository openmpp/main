/**
* @file    RangeSymbol.cpp
* Definitions for the RangeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "RangeSymbol.h"
#include "LanguageSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

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
            //for (int i=lower_bound, j=0; i<=upper_bound; ++i, ++j) {
            //    string enumerator_name = name + "_";
            //    if (i < 0) enumerator_name += "_" + to_string(-i);
            //    else  enumerator_name += to_string(i);
            //    auto sym = new RangeEnumeratorSymbol(enumerator_name, this, j, lower_bound);
            //}

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

string RangeSymbol::metadata_signature() const
{
    // Hook into the hierarchical calling chain
    string sig = super::metadata_signature();

    // Perform operations specific to this level in the Symbol hierarchy.
    sig += "range: " + to_string(lower_bound) + " to " + to_string(upper_bound) + "\n";

    return sig;
}

void RangeSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    // Only generate metadata if the enumeration has been marked
    // as being used in a table or external parameter.

    if (!metadata_needed) return;

    // Range enumerations do not populate an associated collection of enumerators.
    // Enumerator metadata for ranges is instead created directly at this level (RangeSymbol) 
    // of the hierarchical calling chain.

    for (int i = lower_bound, ordinal = 0; i <= upper_bound; ++i, ++ordinal) {
        string enumerator_name = name + "_";
        if (i < 0) enumerator_name += "_" + to_string(-i);
        else  enumerator_name += to_string(i);

        {
            TypeEnumLstRow typeEnum;
            typeEnum.typeId = type_id;
            typeEnum.enumId = ordinal;
            typeEnum.name = enumerator_name;
            metaRows.typeEnum.push_back(typeEnum);
        }

        for (auto lang : Symbol::pp_all_languages) {
            TypeEnumTxtLangRow typeEnumTxt;
            typeEnumTxt.typeId = type_id;
            typeEnumTxt.enumId = ordinal;
            typeEnumTxt.langName = lang->name;
            typeEnumTxt.descr = to_string(lower_bound + ordinal);
            typeEnumTxt.note = "";
            metaRows.typeEnumTxt.push_back(typeEnumTxt);
        }
    }
}

bool RangeSymbol::is_valid_constant(const Constant &k) const
{
    // named enumerators are invalid for a range (only integers allowed)
    if (k.is_enumerator) return false;

    // floating point literals are invalid for range
    if (is_floating()) return false;

    long value = stol(k.value());
    // value must fall with range bounds
    if (value < lower_bound || value > upper_bound) return false;

    return true;
}

string RangeSymbol::format_for_storage(const Constant &k) const
{
    long value = stol(k.value());

    // For storage, normalize range value to ordinal by subtracting lower bound
    value -= lower_bound;

    string result = to_string(value);
    return result;
}
