/**
* @file    BoolSymbol.cpp
* Definitions for the BoolSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "BoolSymbol.h"
#include "BoolEnumeratorSymbol.h"

using namespace std;

void BoolSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        // Create enumerators for false and true, associated with the bool enumeration symbol
        auto sym0 = new BoolEnumeratorSymbol(token_to_string(token::TK_false), this, 0);
        auto sym1 = new BoolEnumeratorSymbol(token_to_string(token::TK_true), this, 1);
    }
    break;
    default:
    break;
    }
}

bool BoolSymbol::is_valid_constant(const Constant &k) const
{
    // only named enumerators are valid for a bool
    if (!k.is_enumerator) return false;

    // check that the enumerator value is in the enumeration's map of enumerators to ordinals
    if (pp_name_to_int.count(k.value()) == 0) return false;

    return true;
}

string BoolSymbol::format_for_storage(const Constant &k) const
{
    // Get the ordinal corresponding to the enumerator name using 
    // the enumeration's map of enumerators to ordinals
    auto it = pp_name_to_int.find(k.value());
    assert(it != pp_name_to_int.end()); // validity of initializers was already verified
    auto ordinal = it->second;

    string result = to_string(ordinal);
    return result;
}


BoolSymbol *BoolSymbol::find()
{
    auto bs = dynamic_cast<BoolSymbol *>(get_symbol(token_to_string(token::TK_bool)));
    assert(bs); // only called when valid
    return bs;
}
