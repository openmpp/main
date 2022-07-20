/**
* @file    EntityMultilinkSymbol.cpp
* Definitions for the EntityMultilinkSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "EntityMultilinkSymbol.h"
#include "LinkAttributeSymbol.h"
#include "EntitySymbol.h"
#include "EntityFuncSymbol.h"
#include "TypeSymbol.h"
#include "CodeBlock.h"

using namespace std;

void EntityMultilinkSymbol::create_auxiliary_symbols()
{
    assert(!side_effects_fn); // logic guarantee
    side_effects_fn = new EntityFuncSymbol("om_" + name + "_side_effects",
                                          agent,
                                          "void",
                                          "");
    side_effects_fn->doc_block = doxygen_short("Implement side effects of changes in multilink " + name + " in entity " + agent->name + ".");

    assert(!insert_fn); // logic guarantee
    insert_fn = new EntityFuncSymbol("om_" + name + "_insert",
                                          agent,
                                          "void",
                                          data_type->name + " lnk");
    insert_fn->doc_block = doxygen_short("Maintain reciprocal link on insert in multilink " + name + " in entity " + agent->name + ".");

    assert(!erase_fn); // logic guarantee
    erase_fn = new EntityFuncSymbol("om_" + name + "_erase",
                                          agent,
                                          "void",
                                          data_type->name + " lnk" );
    erase_fn->doc_block = doxygen_short("Maintain reciprocal link on erase in multilink " + name + " in entity " + agent->name + ".");
}

void EntityMultilinkSymbol::post_parse(int pass)
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
        CodeBlock& c_insert = insert_fn->func_body;
        CodeBlock& c_erase = erase_fn->func_body;

        if (Symbol::option_event_trace) {
            // EventTrace insert
            c_insert += "// Code Injection: event trace";
            c_insert += "if (event_trace_on && om_active) {";
            c_insert +=     "auto ent_ptr = lnk.get();";
            c_insert +=     "auto lst = " + name + ".contents();";
            c_insert +=     "event_trace_msg("
                "\"" + agent->name + "\", "
                "(int)entity_id, "
                "(double)age.direct_get(), "
                "GetCaseSeed(), "
                "lst.c_str(), " // multilink contents as comma-separated entity_id's
                + to_string(pp_member_id) + ", " // id (member_id)
                "\"" + name + "\", " // other_name (attribute_name)
                "(double)ent_ptr->entity_id, "
                "0, " // unused
                "(double)BaseEvent::get_global_time(), "
                "BaseEntity::et_msg_type::eMultilinkInsert);"
                ;
            // if requested, add the linked entity to traced entities
            c_insert +=     "if (BaseEntity::event_trace_show_linked_entities) {";
            c_insert +=         "BaseEntity::event_trace_selected_entities.insert(ent_ptr->entity_id);";
            c_insert +=     "}";
            c_insert += "}";
            c_insert += "";

            // EventTrace erase
            c_erase += "// Code Injection: event trace";
            c_erase += "if (event_trace_on && om_active) {";
            c_erase +=     "auto ent_ptr = lnk.get();";
            c_erase +=     "auto lst = " + name + ".contents();";
            c_erase +=     "event_trace_msg("
                "\"" + agent->name + "\", "
                "(int)entity_id, "
                "(double)age.direct_get(), "
                "GetCaseSeed(), "
                "lst.c_str(), " // multilink contents as comma-separated entity_id's
                + to_string(pp_member_id) + ", " // id (member_id)
                "\"" + name + "\", " // other_name (attribute_name)
                "(double)ent_ptr->entity_id, "
                "0, " // unused
                "(double)BaseEvent::get_global_time(), "
                "BaseEntity::et_msg_type::eMultilinkErase);"
                ;
            c_erase += "}";
            c_erase += "";
        }
        
        // Maintain reciprocal links
        if (reciprocal_link) {
            // reciprocal link is single
            auto reciprocal = reciprocal_link;

            c_insert += "// Maintain reciprocal link";
            c_insert += "if (lnk.get() != nullptr) lnk->" + reciprocal->name + " = this;";

            c_erase += "// Maintain reciprocal link";
            c_erase += "if (lnk->" + reciprocal->name + ".get().get() == this) lnk->" + reciprocal->name + " = nullptr;";
        }
        else {
            // reciprocal link is multi
            assert(reciprocal_multilink); // grammar guarantee
            auto reciprocal = reciprocal_multilink;

            c_insert += "// Maintain reciprocal multilink";
            c_insert += "lnk->" + reciprocal->name + ".insert(this);";

            c_erase += "// Maintain reciprocal multilink";
            c_erase +=  "lnk->" + reciprocal->name + ".erase(this);";
        }
        break;
    }
    default:
        break;
    }
}

CodeBlock EntityMultilinkSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    EntitySymbol *reciprocal_agent = nullptr;
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



