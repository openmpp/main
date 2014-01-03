/**
* @file    SimpleAgentVarSymbol.cpp
* Definitions for the SimpleAgentVarSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "SimpleAgentVarSymbol.h"
#include "CodeBlock.h"
#include "Literal.h"

using namespace std;

string SimpleAgentVarSymbol::initial_value() const
{
    string result;
    if (initializer != nullptr)
        result = initializer->cxx_token;
    else
        result = AgentVarSymbol::initial_value();

    return result;
}

CodeBlock SimpleAgentVarSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    // add declaration code specific to simple agentvars

    // example:         SimpleAgentVar<bool, Person, &Person::alive_side_effects> alive;
    h += "SimpleAgentVar<" + token_to_string(type) + ", "
        + agent->name + ", "
        + "&" + agent->name + "::" + side_effects_func() + "> "
        + name + ";";

    return h;
}

