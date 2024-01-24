/**
* @file    GlobalFuncSymbol.cpp
* Definitions for the GlobalFuncSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include <set>
#include <algorithm>
#include "GlobalFuncSymbol.h"

using namespace std;

void GlobalFuncSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        if (suppress_defn) {
            // Is a developer-supplied global function.

            // Construct the set of all identifiers used in the function body.
            auto rng = function_body_identifiers.equal_range(unique_name);
            for_each(
                rng.first,
                rng.second,
                [&](multimap<string, string>::value_type& vt)
                {
                    body_identifiers.insert(vt.second);
                }
            );
        }
        break;
    }
    case ePopulateCollections:
    {
        // add this to the complete list of global funcs
        pp_all_global_funcs.push_back(this);

        // Add this global function to xref of symbols used in its definition
        for (auto& identifier : body_identifiers) {
            // look for global identifiers (not entity members)
            auto s = get_symbol(identifier);
            if (s) {
                s->pp_global_funcs_using.insert(this);
            }
        }

        break;
    }
    default:
        break;
    }
}

CodeBlock GlobalFuncSymbol::cxx_declaration_global()
{
    CodeBlock h;

    // Emit declaration only if created by omc
    if (!suppress_defn) {

        // Hook into the hierarchical calling chain
        h = super::cxx_declaration_global();

        // Perform operations specific to this level in the Symbol hierarchy.

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
    }

    return h;
}

CodeBlock GlobalFuncSymbol::cxx_definition_global()
{
    CodeBlock c;

    // Emit declaration only if created by omc
    if (!suppress_defn) {
        // Hook into the hierarchical calling chain
        c = super::cxx_definition_global();

        // Perform operations specific to this level in the Symbol hierarchy.

        c += return_decl + " " + name + "(" + arg_list_decl + ")";
        c += "{";
        c += func_body;
        c += "}";
    }

    return c;
}

