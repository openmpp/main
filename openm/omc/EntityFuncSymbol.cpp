/**
 * @file    EntityFuncSymbol.cpp
 * Definitions for the EntityFuncSymbol class.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "EntityFuncSymbol.h"
#include "EntitySymbol.h"

using namespace std;

void EntityFuncSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        // Allow model code to override generated member function body for specific member functions
        if (name == "get_entity_key" || name == "get_microdata_key") {
            auto search = memfunc_defn_loc.find(unique_name);
            if (search != memfunc_defn_loc.end()) {
                suppress_defn = true;
            }
        }

        // Process early to handle possible missing symbol creation
        if (suppress_defn) {
            // Is a developer-supplied entity member function.

            // Note the definition location
            auto search = memfunc_defn_loc.find(unique_name);
            if (search != memfunc_defn_loc.end()) {
                defn_loc = search->second;
            }
            else {
                pp_warning(LT("warning : entity member function '") + unique_name + LT("' was declared but has no definition."));
            }

            // Get the name of the generated hook function for this function.
            auto hook_fn_name = hook_implement_name(name);

            // See if it is called in the source code of this function
            bool hook_is_called = false;
            {
                // Determine if hook is called in function body.
                // Also, construct the set of all identifiers used in the function body.
                auto rng = memfunc_bodyids.equal_range(unique_name);
                for_each(
                    rng.first,
                    rng.second,
                    [&](unordered_multimap<string, string>::value_type& vt)
                    {
                        if (vt.second == hook_fn_name) {
                            hook_is_called = true;
                        }
                        body_identifiers.insert(vt.second);
                    }
                );
            }

            if (hook_is_called) {
                auto sym = get_symbol(hook_fn_name, agent);
                if (!sym) {
                    // The hook function for this function is called in model code,
                    // and the associated EntityFuncSymbol does not exist.
                    // Create it so that it will be declared and defined in generated source.
                    auto fn_sym = new EntityFuncSymbol(hook_fn_name, agent, "void", "");
                    fn_sym->doc_block = doxygen_short("Call the functions hooked to the function '" + name + "'");
                }
            }

            {
                // Construct the set of all RNG streams used in the function body.
                auto rng = memfunc_rng_streams.equal_range(unique_name);
                for_each(
                    rng.first,
                    rng.second,
                    [&](unordered_multimap<string, int>::value_type& vt)
                    {
                        rng_streams.insert(vt.second);
                    }
                );
            }

            {
                // Construct the set of all RNG Normal streams used in the function body.
                auto rng = memfunc_rng_normal_streams.equal_range(unique_name);
                for_each(
                    rng.first,
                    rng.second,
                    [&](unordered_multimap<string, int>::value_type& vt)
                    {
                        rng_normal_streams.insert(vt.second);
                    }
                );
            }
        }
        break;
    }
    case ePopulateCollections:
    {
        // Add this agentfunc to the agent's list of agentfuncs
        pp_agent->pp_agent_funcs.push_back(this);
        break;
    }
    default:
        break;
    }
}

CodeBlock EntityFuncSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    if (suppress_code_if_empty && empty()) {
        // Suppress declaration
        return CodeBlock();
    }

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

CodeBlock EntityFuncSymbol::cxx_definition_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    if (suppress_code_if_empty && empty()) {
        // Suppress definition
        return CodeBlock();
    }

    if (!suppress_defn) {
        c += return_decl + " " + unique_name + "(" + arg_list_decl + ")";
        c += "{";
        c += func_body;
        c += "}";
    }

    return c;
}

