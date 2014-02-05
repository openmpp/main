/**
 * @file    AgentFuncSymbol.cpp
 * Definitions for the AgentFuncSymbol class.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AgentFuncSymbol.h"
#include "AgentSymbol.h"

using namespace std;

void AgentFuncSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        // Add this agentfunc to the agent's list of agentfuncs
        pp_agent->pp_agent_funcs.push_back(this);
        break;
    default:
        break;
    }
}

CodeBlock AgentFuncSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    if (doc_block.size() == 0) {
        // use a default short doxygen comment
        h += doxygen_short(label());
    }
    else {
        // use documentation block of this symbol
        h += doc_block;
    }
    h += return_decl + " " + name + "(" + arg_list_decl + ");";
    h += "";

    return h;
}



CodeBlock AgentFuncSymbol::cxx_definition_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    if (!suppress_defn) {
        c += return_decl + " " + unique_name + "(" + arg_list_decl + ")";
        c += "{";
        c += func_body;
        c += "}";
    }

    return c;
}

