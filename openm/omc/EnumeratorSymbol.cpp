/**
* @file    EnumeratorSymbol.cpp
* Definitions for the EnumeratorSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "EnumeratorSymbol.h"
#include "EnumerationWithEnumeratorsSymbol.h"

using namespace std;

void EnumeratorSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to enumeration for use post-parse
        pp_enumeration = dynamic_cast<EnumerationWithEnumeratorsSymbol *> (pp_symbol(enumeration));
        assert(pp_enumeration); // parser guarantee
        // Add this enumerator to the enumeration's list of enumerators
        pp_enumeration->pp_enumerators.push_back(this);
        // Add this enumerator to the enumeration's map of names to ordinal values
        pp_enumeration->pp_name_to_int.emplace(name, ordinal);
        break;
    }
    default:
        break;
    }
}



