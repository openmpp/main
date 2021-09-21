/**
* @file    SimpleAttributeSymbol.cpp
* Definitions for the SimpleAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "SimpleAttributeEnumSymbol.h"

using namespace std;

void SimpleAttributeEnumSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer for use post-parse
        pp_enumerator = dynamic_cast<EnumeratorSymbol *> (pp_symbol(enumerator));
        //TODO Check that enumerator is valid for enumeration of attribute
        assert(pp_enumerator); // developer error
        break;
    }
    default:
    break;
    }
}

string SimpleAttributeEnumSymbol::initialization_value(bool type_default) const
{
    string result;

    if (!type_default) {
        result = pp_enumerator->name;
    }
    else {
        result = AttributeSymbol::initialization_value(type_default);
    }

    return result;
}

