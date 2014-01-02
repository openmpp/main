/**
* @file    AgentInternalSymbol.cpp
* Definitions for the AgentInternalSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AgentInternalSymbol.h"
#include "CodeBlock.h"

using namespace std;

void AgentInternalSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case 1:
        // Nothing
        break;
    default:
        break;
    }
}

CodeBlock AgentInternalSymbol::cxx_initialize_expression() const
{
    CodeBlock c;
    // example:              om_in_DurationOfLife_alive = false;\n
    c += name + " = " + initial_value() + ";";
    return c;
}

CodeBlock AgentInternalSymbol::cxx_declaration_agent_scope()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent_scope();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += token_to_string(type) + " " + name + ";";
    return h;
}

CodeBlock AgentInternalSymbol::cxx_definition()
{
    // First get definition code at next level up in the Symbol hierarchy
    CodeBlock c = super::cxx_definition();
    return c;
}


