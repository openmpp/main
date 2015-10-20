/**
* @file    SimpleAttributeSymbol.cpp
* Definitions for the SimpleAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "SimpleAttributeSymbol.h"
#include "EntityFuncSymbol.h"
#include "TypeSymbol.h"
#include "CodeBlock.h"
#include "Literal.h"

using namespace std;

string SimpleAttributeSymbol::initialization_value(bool type_default) const
{
    string result;

    if (!type_default && initializer != nullptr) {
        result = initializer->value();
    }
    else {
        result = AttributeSymbol::initialization_value(type_default);
    }

    return result;
}

CodeBlock SimpleAttributeSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    // add declaration code specific to simple agentvars

    // example:         AssignableAgentVar<bool, Person, &Person::alive_side_effects> alive;
    h += "AssignableAgentVar<" 
        + pp_data_type->name + ", "
        + pp_data_type->exposed_type() + ", "
        + agent->name + ", "
        + "&om_name_" + name + ", "
        + "&" + side_effects_fn->unique_name + ", "
        + (!side_effects_fn->empty() ? "true" : "false") + ", "
        + "&" + notify_fn->unique_name + ", "
        + (!notify_fn->empty() ? "true" : "false")
        + ">";
    h += name + ";";

    return h;
}

