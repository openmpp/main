/**
* @file    AgentHookSymbol.cpp
* Definitions for the AgentHookSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include <algorithm>
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

        // Create entry in agent multimap of all hooks.
        // The key is the name of the 'to' function.
        pp_agent->pp_hooks.emplace(pp_to->name, pp_from->name);

        // Create entry in agent multimap of all hooks (additionally distinguished by order).
        // The key is constructed using two parts to allow subsequent testing for 
        // ties in hook order.
        string key = pp_to->name + "_om_" + to_string(order);
        pp_agent->pp_hooks_with_order.emplace(key, pp_from->name);
        break;
    }
    case ePopulateDependencies:
    {
        assert(hook_fn); // logic guarantee
        string nm = hook_fn->name;
        // verify that hook_fn is actually used somewhere in the body of the 'to' hook function
        if (!any_of(
                    pp_to->body_identifiers.begin(),
                    pp_to->body_identifiers.end(),
                    [nm](string id){ return nm == id; })) {
            pp_error("error - the target function '" + pp_to->unique_name + "' of the hook contains no call to '" + hook_fn->name + "'");
        }

        // Test for ambiguous hook order and emit warning if found.
        // 
        // First test if the hook has no explicit order, but other hooks are present.
        if (order == 0 && pp_agent->pp_hooks.count(pp_to->name) > 1) {
            pp_warning("Warning: One or more functions hooking to '" + pp_to->name + "' are ordered ambiguously with respect to '" + pp_from->name + "'.");
        }
        else {
            // Second test is if the hook has an explicit order, but is tied to one or more other hooks.
            // See comment for 'key' above
            string key = pp_to->name + "_om_" + to_string(order);
            if (pp_agent->pp_hooks_with_order.count(key) > 1) {
                pp_warning("Warning: One or more functions hooking to '" + pp_to->name + "' are ordered ambiguously with respect to '" + pp_from->name + "'.");
            }
        }

        CodeBlock & c = hook_fn->func_body;
        c += injection_description();
        c += pp_from->name + "();";
        break;
    }
    default:
        break;
    }
}



