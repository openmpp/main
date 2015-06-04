/**
* @file    SimpleAgentVarSymbol.cpp
* Definitions for the SimpleAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "SimpleAgentVarSymbol.h"
#include "AgentFuncSymbol.h"
#include "TypeSymbol.h"
#include "CodeBlock.h"
#include "Literal.h"

using namespace std;

string SimpleAgentVarSymbol::initialization_value(bool type_default) const
{
    string result;

    if (!type_default && initializer != nullptr) {
        result = initializer->value();
    }
    else {
        result = AgentVarSymbol::initialization_value(type_default);
    }

    return result;
}

CodeBlock SimpleAgentVarSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    // add declaration code specific to simple agentvars

    // example:         AssignableAgentVar<bool, Person, &Person::alive_side_effects> alive;
    h += "AssignableAgentVar<" 
        + pp_data_type->name + ", "
        + pp_data_type->wrapped_type() + ", "
        + agent->name + ", "
        + "&om_name_" + name + ", "
        + "&" + side_effects_fn->unique_name + ", "
        + (!side_effects_fn->empty() ? "true" : "false")
        + ">";
    h += name + ";";

    return h;
}

