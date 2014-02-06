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



void AgentVarSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // Add this agentvar to the agent's list of all agentvars
            pp_agent->pp_agentvars.push_back(this);
        }
        break;
    default:
        break;
    }
}


