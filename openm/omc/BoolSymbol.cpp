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

BoolSymbol *BoolSymbol::find()
{
    auto bs = dynamic_cast<BoolSymbol *>(get_symbol(token_to_string(token::TK_bool)));
    assert(bs); // only called when valid
    return bs;
}
