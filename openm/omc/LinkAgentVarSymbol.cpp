/**
 * @file    AgentLinkSymbol.cpp
 * Definitions for the AgentLinkSymbol class.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "LinkAgentVarSymbol.h"
#include "AgentFuncSymbol.h"
#include "AgentSymbol.h"
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
        {
            // Add this link agentvar symbol to the agent's list of all such symbols
            pp_agent->pp_link_agentvars.push_back(this);
        }
        break;
    case ePopulateDependencies:
        {
            // Dependency on agentvars in expression
            CodeBlock& c = side_effects_fn->func_body;
            c += "// Maintain reciprocal link: " + reciprocal_link->name + " in " + reciprocal_link->agent->name + " - while avoiding stack overflow";
            c += "if (old_value.get() != nullptr && old_value->" + reciprocal_link->name + ".get().get() == this) {";
            c += "old_value->" + reciprocal_link->name + " = nullptr;";
            c += "}";
            c += "if (new_value.get() == nullptr && new_value->" + reciprocal_link->name + ".get().get() != this) {";
            c += "new_value->" + reciprocal_link->name + " = this;";
            c += "}";
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



