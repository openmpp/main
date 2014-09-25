/**
* @file    AgentHookSymbol.cpp
* Definitions for the AgentHookSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "Symbol.h"
#include "AgentHookSymbol.h"
#include "AgentSymbol.h"

using namespace std;

// static
string AgentHookSymbol::symbol_name(const Symbol* from, const Symbol* to)
{
    return "om_hook_" + from->name + "_om_" + to->name;
}

//static
bool AgentHookSymbol::exists(const Symbol* agent, const Symbol* from, const Symbol* to)
{
    return Symbol::exists(symbol_name(from, to), agent);
}


void AgentHookSymbol::create_auxiliary_symbols()
{
    // The hook_<func> member function
    {
        string fn_name = AgentFuncSymbol::hook_implement_name(to->name);
        auto sym = Symbol::get_symbol(fn_name, agent);

        AgentFuncSymbol *fn_sym = nullptr;
        if (sym) {
            // cast it to derived type
            fn_sym = dynamic_cast<AgentFuncSymbol *>(sym);
            if (!fn_sym) {
                pp_error("error - symbol '" + fn_name + "' is reserved for implementing hooks");
                return;
            }
        }
        else {
            // create it
            fn_sym = new AgentFuncSymbol(fn_name, agent, "void", "");
            fn_sym->doc_block = doxygen_short("Call functions hooked to the function '" + to->name + "'");
            CodeBlock& c = fn_sym->func_body;
        }
        // store it
        hook_fn = fn_sym;
    }
}

void AgentHookSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to agent for use post-parse
        pp_agent = dynamic_cast<AgentSymbol *> (pp_symbol(agent));
        assert(pp_agent); // parser guarantee
        // assign direct pointer to classifications for use post-parse
        pp_to = dynamic_cast<AgentFuncSymbol *> (pp_symbol(to));
        if (!pp_to) {
            pp_error("error: '" + to->name + "' must be a function member of agent '" + pp_agent->name + "'");
            break;
        }
        pp_from = dynamic_cast<AgentFuncSymbol *> (pp_symbol(from));
        if (!pp_from) {
            pp_error("error: '" + from->name + "' must be a function member of agent '" + pp_agent->name + "'");
            break;
        }
        break;
    }
    case ePopulateDependencies:
    {
        assert(hook_fn); // logic guarantee
        CodeBlock & c = hook_fn->func_body;
        c += injection_description();
        c += pp_from->name + "();";
        break;
    }
    default:
        break;
    }
}



