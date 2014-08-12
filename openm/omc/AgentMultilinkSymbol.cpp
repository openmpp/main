/**
* @file    AgentMultilinkSymbol.cpp
* Definitions for the AgentMultilinkSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AgentMultilinkSymbol.h"
#include "LinkAgentVarSymbol.h"
#include "AgentSymbol.h"
#include "AgentFuncSymbol.h"
#include "TypeSymbol.h"
#include "CodeBlock.h"

using namespace std;

void AgentMultilinkSymbol::create_auxiliary_symbols()
{
    assert(!side_effects_fn); // logic guarantee
    side_effects_fn = new AgentFuncSymbol("om_" + name + "_side_effects",
                                          agent,
                                          "void",
                                          "");
    side_effects_fn->doc_block = doxygen_short("Implement side effects of changes in multilink " + name + " in agent " + agent->name + ".");

    assert(!insert_fn); // logic guarantee
    insert_fn = new AgentFuncSymbol("om_" + name + "_insert",
                                          agent,
                                          "void",
                                          data_type->name + " lnk");
    insert_fn->doc_block = doxygen_short("Maintain reciprocal link on insert in multilink " + name + " in agent " + agent->name + ".");

    assert(!erase_fn); // logic guarantee
    erase_fn = new AgentFuncSymbol("om_" + name + "_erase",
                                          agent,
                                          "void",
                                          data_type->name + " lnk" );
    erase_fn->doc_block = doxygen_short("Maintain reciprocal link on erase in multilink " + name + " in agent " + agent->name + ".");
}

void AgentMultilinkSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // Add this multilink member symbol to the agent's list of all such symbols
        pp_agent->pp_multilink_members.push_back(this);

        // Add this multilink member symbol to the agent's list of all callback members
        pp_agent->pp_callback_members.push_back(this);
        break;
    }
    case ePopulateDependencies:
    {
        // Create function bodies which maintain reciprocal link
        CodeBlock& c_insert = insert_fn->func_body;
        CodeBlock& c_erase = erase_fn->func_body;
        if (reciprocal_link) {
            // reciprocal link is single
            auto reciprocal = reciprocal_link;
            c_insert += "if (lnk.get() != nullptr) lnk->" + reciprocal->name + " = this;";
            c_erase += "if (lnk->" + reciprocal->name + ".get().get() == this) lnk->" + reciprocal->name + " = nullptr;";
        }
        else {
            // reciprocal link is multi
            assert(reciprocal_multilink); // grammar guarantee
            auto reciprocal = reciprocal_multilink;
            c_insert += "lnk->" + reciprocal->name + ".insert(this);";
            c_erase +=  "lnk->" + reciprocal->name + ".erase(this);";
        }
        break;
    }
    default:
        break;
    }
}

CodeBlock AgentMultilinkSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    AgentSymbol *reciprocal_agent = nullptr;
    if (reciprocal_link) {
        reciprocal_agent = reciprocal_link->pp_agent;
    }
    else {
        assert(reciprocal_multilink); // grammar guarantee
        reciprocal_agent = reciprocal_multilink->pp_agent;
    }

    h += "Multilink<"
        + data_type->name + ", "
        + agent->name + ", "
        + reciprocal_agent->name + ", "
        + "&" + side_effects_fn->unique_name + ", "
        + "&" + insert_fn->unique_name + ", "
        + "&" + erase_fn->unique_name
        + "> ";
    h += name + ";";

    return h;
}



