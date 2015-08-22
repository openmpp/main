/**
 * @file    BuiltinAgentVarSymbol.cpp
 * Definitions for the BuiltinAgentVarSymbol class.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "BuiltinAgentVarSymbol.h"
#include "AgentSymbol.h"
#include "AgentFuncSymbol.h"
#include "TypeSymbol.h"
#include "CodeBlock.h"

using namespace std;

void BuiltinAgentVarSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateDependencies:
        {
            if (name == "age") {
                // add side-effect to time agentvar
                AgentVarSymbol *av = pp_agent->pp_time;
                assert(av);
                CodeBlock& c = av->side_effects_fn->func_body;
                c += injection_description();
                c += "// Advance time for the age agentvar";
                c += "if (om_active) age.set(age.get() + om_delta);";
            }
        }
        break;
    default:
        break;
    }
}

CodeBlock BuiltinAgentVarSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    // example:         AgentVar<Time, Person, &Person::time_side_effects> time;
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


