/**
 * @file    AgentLinkSymbol.cpp
 * Definitions for the AgentLinkSymbol class.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "LinkAgentVarSymbol.h"
#include "AgentFuncSymbol.h"
#include "TypeSymbol.h"
#include "CodeBlock.h"

using namespace std;

void LinkAgentVarSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        break;
    case ePopulateDependencies:
        {
            // Dependency on agentvars in expression
            CodeBlock& c = side_effects_fn->func_body;
            c += "// Maintain reciprocal link: " + reciprocal_link->name + " in " + reciprocal_link->agent->name;
            c += "old_value->" + reciprocal_link->name + " = nullptr;";
            c += "new_value->" + reciprocal_link->name + " = this;";
            c += "";
        }
        break;
    default:
        break;
    }
}

CodeBlock LinkAgentVarSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    h += "LinkAgentVar<" + pp_data_type->name + ", "
        + agent->name + ", "
        + reciprocal_link->agent->name + ", "
        + "&" + side_effects_fn->unique_name + "> ";
    h += name + ";";

    return h;
}



