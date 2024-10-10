/**
 * @file    BuiltinAttributeSymbol.cpp
 * Definitions for the BuiltinAttributeSymbol class.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "BuiltinAttributeSymbol.h"
#include "EntitySymbol.h"
#include "EntityFuncSymbol.h"
#include "TypeSymbol.h"
#include "EntitySymbol.h"
#include "LanguageSymbol.h"
#include "CodeBlock.h"

using namespace std;

void BuiltinAttributeSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignLabel:
    {
        for (const auto& langSym : Symbol::pp_all_languages) {
            int lang_index = langSym->language_id; // 0-based
            pp_labels_explicit[lang_index] = true;
        }
        break;
    }
    case ePopulateDependencies:
    {
        if (name == "age") {
            // add side-effect to attribute 'time'
            AttributeSymbol *av = pp_entity->pp_time;
            assert(av);
            CodeBlock& time_cxx = av->side_effects_fn->func_body;
            time_cxx += injection_description();
            time_cxx += "if (om_active) {";
            time_cxx += "// Advance time for the attribute 'age'";
            time_cxx += "age.set(age.get() + om_delta);";
            time_cxx += "}";
        }
        break;
    }
    default:
        break;
    }
}

CodeBlock BuiltinAttributeSymbol::cxx_declaration_entity()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_entity();

    // Perform operations specific to this level in the Symbol hierarchy.

    std::string member_type_name = name + "_om_type";
    // example: typedef Attribute<Time, Person, &Person::time_side_effects> time_om_type;
    h += "typedef AssignableAttribute<"
        + pp_data_type->name + ", "
        + pp_data_type->exposed_type() + ", "
        + entity->name + ", "
        + "&om_name_" + name + ", "
        + (is_time_like ? "true" : "false") + ", "
        + "&" + side_effects_fn->unique_name + ", "
        + (!side_effects_fn->empty() ? "true" : "false") + ", "
        + "&" + notify_fn->unique_name + ", "
        + (!notify_fn->empty() ? "true" : "false")
        + "> " + member_type_name + ";";
    h += doxygen_short("attribute(built-in) " + pp_data_type->name + ": " + label());
    h += member_type_name + " " + name + ";";

    return h;
}


