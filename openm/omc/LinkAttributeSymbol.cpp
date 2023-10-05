/**
 * @file    LinkAttributeSymbol.cpp
 * Definitions for the LinkAttributeSymbol class.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "LinkAttributeSymbol.h"
#include "EntityMultilinkSymbol.h"
#include "EntityFuncSymbol.h"
#include "EntitySymbol.h"
#include "TypeSymbol.h"
#include "CodeBlock.h"

using namespace std;

void LinkAttributeSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // Add this link attribute symbol to the entity's list of all such symbols
        pp_entity->pp_link_attributes.push_back(this);
        break;
    }
    case ePopulateDependencies:
    {
        // Maintain reciprocal link
        CodeBlock& c = side_effects_fn->func_body;
        if (reciprocal_link) {
            // reciprocal link is single
            auto reciprocal = reciprocal_link;
            c += "// Maintain reciprocal single link: " + reciprocal->name + " in " + reciprocal->entity->name;
            c += "if (om_old.get() != nullptr && om_old->" + reciprocal->name + ".get().get() == this) {";
            c += "om_old->" + reciprocal->name + " = nullptr;";
            c += "}";
            c += "if (om_new.get() != nullptr && om_new->" + reciprocal->name + ".get().get() != this) {";
            c += "om_new->" + reciprocal->name + " = this;";
            c += "}";
            c += "";
        }
        else {
            // reciprocal link is multi
            assert(reciprocal_multilink); // logic guarantee
            auto reciprocal = reciprocal_multilink;
            c += "// Maintain reciprocal multi-link: " + reciprocal->name + " in " + reciprocal->entity->name;
            c += "if (om_old.get() != nullptr) {";
            c += "om_old->" + reciprocal->name + ".erase(this);";
            c += "}";
            c += "if (om_new.get() != nullptr) {";
            c += "om_new->" + reciprocal->name + ".insert(this);";
            c += "}";
            c += "";
        }
        break;
    }
    default:
        break;
    }
}

CodeBlock LinkAttributeSymbol::cxx_declaration_entity()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_entity();

    // Perform operations specific to this level in the Symbol hierarchy.

    EntitySymbol *reciprocal_entity = nullptr;
    if (reciprocal_link) {
        reciprocal_entity = reciprocal_link->pp_entity;
    }
    else {
        assert(reciprocal_multilink); // grammar guarantee
        reciprocal_entity = reciprocal_multilink->pp_entity;
    }

    std::string member_type_name = name + "_om_type";
    h += "typedef LinkAttribute<"
        + pp_data_type->name + ", "
        + "bool, " // allow access to bool cast in wrapped link
        + entity->name + ", "
        + reciprocal_entity->name + ", "
        + "&om_name_" + name + ", "
        + (is_time_like ? "true" : "false") + ", "
        + "&" + side_effects_fn->unique_name + ", "
        + (!side_effects_fn->empty() ? "true" : "false") + ", "
        + "&" + notify_fn->unique_name + ", "
        + (!notify_fn->empty() ? "true" : "false")
        + "> " + member_type_name + ";";
    h += doxygen_short("attribute(link) " + pp_data_type->name + ": " + label());
    h += member_type_name + " " + name + ";";

    return h;
}



