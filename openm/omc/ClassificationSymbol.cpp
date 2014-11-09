/**
* @file    ClassificationSymbol.cpp
* Definitions for the ClassificationSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <algorithm>
#include <list>
#include <cctype>
#include "ClassificationSymbol.h"
#include "EnumeratorSymbol.h"
#include "CodeBlock.h"

using namespace std;
using namespace openm;

void ClassificationSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateDependencies:
        {
            // Information to compute storage type is known in this pass
            storage_type = optimized_storage_type(0, pp_size() - 1);
        }
        break;
    default:
        break;
    }
}

CodeBlock ClassificationSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    h += "";
    h += doxygen("Classification: " + name);
    h += "enum om_enum_" + name + " : " + token_to_string(storage_type) + " {";
    bool first = true;
    for (auto enumerator : pp_enumerators) {
        h += doxygen_short(enumerator->name);
        if (first) {
            h += enumerator->name + " = 0," ;
            first = false;
        }
        else {
            h += enumerator->name + "," ;
        }
        h += "";
    }
    h += "};";
    h += "typedef Classification<om_enum_" + name + ", " + to_string(pp_size()) + "> " + name + ";" ;

    if (candidate_for_integer_transforms()) {
        h += name + " IntTo_" + name + "(int number);";
    }

    return h;
}

CodeBlock ClassificationSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "";

    return c;
}

bool ClassificationSymbol::is_valid_constant(const Constant &k) const
{
    // only named enumerators are valid for a classification
    if (!k.is_enumerator) return false;

    // check that the enumerator value is in the enumeration's map of enumerators to ordinals
    if (pp_name_to_int.count(k.value()) == 0) return false;

    return true;
}

string ClassificationSymbol::format_for_storage(const Constant &k) const
{
    // Get the ordinal corresponding to the enumerator name using 
    // the enumeration's map of enumerators to ordinals
    auto it = pp_name_to_int.find(k.value());
    assert(it != pp_name_to_int.end()); // validity of initializers was already verified
    auto ordinal = it->second;

    string result = to_string(ordinal);
    return result;
}

bool ClassificationSymbol::candidate_for_integer_transforms()
{
    return any_of(pp_enumerators.cbegin(), pp_enumerators.cend(), [](EnumeratorSymbol *es){ return isdigit(es->name.back()); });
}

