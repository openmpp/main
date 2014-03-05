/**
* @file    AgentVarSymbol.cpp
* Definitions for the AgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AgentVarSymbol.h"
#include "AgentSymbol.h"
#include "AgentFuncSymbol.h"
#include "TypeSymbol.h"
#include "CodeBlock.h"

using namespace std;

void AgentVarSymbol::create_auxiliary_symbols()
{
    assert(!side_effects_fn); // logic guarantee

    side_effects_fn = new AgentFuncSymbol("om_side_effects_" + name,
                                          agent,
                                          "void",
                                          data_type->name + " old_value, " + data_type->name + " new_value");
    assert(side_effects_fn); // out of memory check

    side_effects_fn->doc_block = doxygen_short("Implement side effects of setting " + name + " in agent " + agent->name + ".");
}

void AgentVarSymbol::change_data_type(TypeSymbol *new_type)
{
    // TODO Pass it upwards to AgentDataMemberSymbol rather than fiddling directly with members.
    assert(new_type);
    pp_data_type = new_type;
    // Note that data_type remains as it was, since references cannot be reseated.
    assert(side_effects_fn); // logic guarantee
    side_effects_fn->arg_list_decl = pp_data_type->name + " old_value, " + pp_data_type->name + " new_value";
}


void AgentVarSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // Add this agentvar to the agent's list of all callback members
            pp_agent->pp_callback_members.push_back(this);
        }
        break;
    default:
        break;
    }
}


