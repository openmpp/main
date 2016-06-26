/**
* @file    EntityHookSymbol.cpp
* Definitions for the EntityHookSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include <algorithm>
#include "Symbol.h"
#include "EntityHookSymbol.h"
#include "EntitySymbol.h"
#include "DerivedAttributeSymbol.h"

using namespace std;

// static
string EntityHookSymbol::symbol_name(const Symbol* from, const Symbol* to)
{
    return "om_hook_" + from->name + "_TO_" + to->name;
}

//static
bool EntityHookSymbol::exists(const Symbol* agent, const Symbol* from, const Symbol* to)
{
    return Symbol::exists(symbol_name(from, to), agent);
}


void EntityHookSymbol::create_auxiliary_symbols()
{
    // The hook_<func> member function
    {
        auto ss = dynamic_cast<DerivedAttributeSymbol *>(to);
        if (ss) {
            assert(ss->is_self_scheduling()); // logic guarantee
            to_is_self_scheduling = true;
            ss->any_to_hooks = true;
            pp_to_ss = ss; // can do early, so do now rather than in post-parse phase
            // For hooks to ss, a single placeholder name is used for all hooks
            // to make them hook together.
            to_name = "self_scheduling"; 
        }
        else {
            to_is_self_scheduling = false;
            to_name = to->name; // name of the function being hooked to
            string fn_name = EntityFuncSymbol::hook_implement_name(to->name);
            auto sym = Symbol::get_symbol(fn_name, agent);

            EntityFuncSymbol *fn_sym = nullptr;
            if (sym) {
                // cast it to derived type
                fn_sym = dynamic_cast<EntityFuncSymbol *>(sym);
                if (!fn_sym) {
                    pp_error("error : symbol '" + fn_name + "' is reserved for implementing hooks");
                    return;
                }
            }
            else {
                // create it
                fn_sym = new EntityFuncSymbol(fn_name, agent, "void", "");
                fn_sym->doc_block = doxygen_short("Call the functions hooked to the function '" + to->name + "'");
            }
            // store it
            hook_fn = fn_sym;
        }
    }
}

void EntityHookSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to agent for use post-parse
        pp_agent = dynamic_cast<EntitySymbol *> (pp_symbol(agent));
        assert(pp_agent); // parser guarantee
        // assign direct pointer to the 'from' entity function symbol for use post-parse
        pp_from = dynamic_cast<EntityFuncSymbol *> (pp_symbol(from));
        if (!pp_from) {
            pp_error("error : '" + from->name + "' must be a function member of agent '" + pp_agent->name + "'");
            break;
        }
        if (!to_is_self_scheduling) {
            pp_to_fn = dynamic_cast<EntityFuncSymbol *> (pp_symbol(to));
            if (!pp_to_fn) {
                pp_error("error : '" + to->name + "' must be a function member of agent '" + pp_agent->name + "'");
                break;
            }
        }

        if (to_is_self_scheduling) {
            // hook function is the implement function of the self-scheduling event of the entity
            hook_fn = pp_agent->ss_implement_fn;
            assert(hook_fn);
        }

        // Create entry in agent multimap of all hooks.
        // The key is the name of the 'to' function
        // (or a placeholder string if the hook is to a self-scheduling attribute).
        pp_agent->pp_hooks.emplace(to_name, pp_from->name);

        // Create entry in agent multimap of all hooks (additionally distinguished by order).
        // The key is constructed using two parts to allow subsequent testing for 
        // ties in hook order.
        string key = to_name + "_om_" + to_string(order);
        pp_agent->pp_hooks_with_order.emplace(key, pp_from->name);
        break;
    }
    case ePopulateDependencies:
    {
        if (!to_is_self_scheduling) {
            assert(hook_fn); // logic guarantee
            string nm = hook_fn->name;
            // verify that hook_fn is actually used somewhere in the body of the 'to' hook function
            if (!any_of(
                        pp_to_fn->body_identifiers.begin(),
                        pp_to_fn->body_identifiers.end(),
                        [nm](string id){ return nm == id; })) {
                pp_error("error : the target function '" + pp_to_fn->unique_name + "' of the hook contains no call to '" + hook_fn->name + "'");
                Symbol::pp_logmsg(pp_to_fn->defn_loc, "note : see definition of target function '" + pp_to_fn->unique_name + "'");
            }
        }

        // Test for ambiguous hook order and emit warning if found.
        // 
        // First test if the hook has no explicit order, but other hooks are present.
        if (order == 0 && pp_agent->pp_hooks.count(to_name) > 1) {
            pp_warning("warning : one or more functions hooking to '" + to_name + "' are ordered ambiguously with respect to '" + pp_from->name + "'.");
        }
        else {
            // Second test is if the hook has an explicit order, but is tied to one or more other hooks.
            // See comment for 'key' above
            string key = to_name + "_om_" + to_string(order);
            if (pp_agent->pp_hooks_with_order.count(key) > 1) {
                pp_warning("warning : one or more functions hooking to '" + to_name + "' are ordered ambiguously with respect to '" + pp_from->name + "'.");
            }
        }

        CodeBlock & c = hook_fn->func_body;
        c += injection_description();
        if (to_is_self_scheduling) {
            assert(pp_to_ss); // logic guarantee
            c += "if (" + pp_to_ss->flag_name() + ") {";
            c += pp_from->name + "();";
            c += "}";
        }
        else {
            c += pp_from->name + "();";
        }
        break;
    }
    default:
        break;
    }
}



