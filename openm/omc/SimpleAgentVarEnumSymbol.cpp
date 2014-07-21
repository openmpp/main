/**
* @file    SimpleAgentVarSymbol.cpp
* Definitions for the SimpleAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "SimpleAgentVarEnumSymbol.h"

using namespace std;

void SimpleAgentVarEnumSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // assign direct pointer for use post-parse
        pp_enumerator = dynamic_cast<EnumeratorSymbol *> (pp_symbol(enumerator));
        //TODO Check that enumerator is valid for enumeration of agentvar
        assert(pp_enumerator); // developer error
        break;
    }
    default:
    break;
    }
}

string SimpleAgentVarEnumSymbol::initialization_value(bool type_default) const
{
    string result;

    if (!type_default) {
        result = pp_enumerator->name;
    }
    else {
        result = AgentVarSymbol::initialization_value(type_default);
    }

    return result;
}

