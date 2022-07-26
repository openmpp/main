/**
* @file    AttributeSymbol.cpp
* Definitions for the AttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include <string> // for stoi
#include "AttributeSymbol.h"
#include "EntitySymbol.h"
#include "EntityFuncSymbol.h"
#include "TypeSymbol.h"
#include "NumericSymbol.h"
#include "EntityInternalSymbol.h"
#include "ConstantSymbol.h"
#include "Literal.h"
#include "EnumeratorSymbol.h"
#include "EnumerationWithEnumeratorsSymbol.h"
#include "RangeSymbol.h"
#include "PartitionSymbol.h"
#include "CodeBlock.h"

#include "MaintainedAttributeSymbol.h" // for check based on dynmaic cast below

using namespace std;

void AttributeSymbol::create_auxiliary_symbols()
{
    assert(!side_effects_fn); // logic guarantee
    side_effects_fn = new EntityFuncSymbol("om_side_effects_" + name,
                                          agent,
                                          "void",
                                          data_type->name + " om_old, " + data_type->name + " om_new");
    assert(side_effects_fn); // out of memory check
    side_effects_fn->doc_block = doxygen_short("Implement side effects of changing " + name + " in entity " + agent->name + ".");

    assert(!notify_fn); // logic guarantee
    notify_fn = new EntityFuncSymbol("om_notify_" + name,
                                          agent,
                                          "void",
                                          "");
    assert(notify_fn); // out of memory check
    notify_fn->doc_block = doxygen_short("Implement notification before changing " + name + " in entity " + agent->name + ".");
}

CodeBlock AttributeSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += "static const std::string om_name_" + name + ";";

    return h;
}

CodeBlock AttributeSymbol::cxx_definition_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "const std::string " + pp_agent->name + "::om_name_" + name + " = \"" + pretty_name() + "\";";

    return c;
}

void AttributeSymbol::change_data_type(TypeSymbol *new_type)
{
    assert(new_type);
    if (pp_data_type != new_type) {
        pp_data_type = new_type;
        assert(side_effects_fn); // logic guarantee
        side_effects_fn->arg_list_decl = pp_data_type->name + " om_old, " + pp_data_type->name + " om_new";
        // maintain global counter of type changes
        ++Symbol::type_changes;
    }
}

string AttributeSymbol::get_lagged_name()
{
    return "om_lagged_" + name;
}

string AttributeSymbol::get_lagged_event_counter_name()
{
    return "om_lagged_event_counter_" + name;
}

void AttributeSymbol::create_lagged()
{
    string lagged_name = get_lagged_name();
    string lagged_counter_name = get_lagged_event_counter_name();

    // nothing to do if already created
    if (exists(lagged_name, this)) {
        assert(exists(lagged_counter_name, this)); // associated counter should exist too
        return;
    }

    // lagged stores in same type as this attribute
    lagged = new EntityInternalSymbol(lagged_name, agent, data_type);
    lagged->provenance = name + " (lagged)";
    // note parent attribute for post-parse type resolution in case data_type is unknown
    lagged->parent = this->stable_pp();

    // lagged event counter stores in same type as global event counter (big_counter)
    auto *typ = NumericSymbol::find(token::TK_big_counter);
    assert(typ); // initialization guarantee
    lagged_event_counter = new EntityInternalSymbol(lagged_counter_name, agent, typ);
    lagged_event_counter->provenance = name + " (counter at lagged)";

    // Add side-effect code to maintain lagged value
    CodeBlock & c = side_effects_fn->func_body;
    c += "// maintain lagged value";
    c += "if (BaseEvent::global_event_counter > " + lagged_counter_name + ") {";
    c += lagged_name + " = om_old;";
    c += lagged_counter_name + " = BaseEvent::global_event_counter;";
    c += "}";
}

bool AttributeSymbol::is_valid_comparison(const ConstantSymbol * constant, string &err_msg)
{
    assert(constant);
    assert(pp_data_type);
    if (pp_data_type->is_bool()) {
        if (constant->is_literal
            && (constant->value() == "true" || constant->value() == "false")) {
            // Valid
        }
        else {
            err_msg = LT("error : '") + constant->value() + LT("' is not a boolean constant");
            return false;
        }
    }
    else if (pp_data_type->is_classification()) {
        if (constant->is_enumerator
            && constant->pp_enumerator->pp_enumeration->name == pp_data_type->name) {
            // Valid
        }
        else {
            err_msg = LT("error : '") + constant->value() + LT("' is not valid for the classification '") + pp_data_type->name + LT("'");
            return false;
        }
    }
    else if (pp_data_type->is_range() ) {
        if (!constant->is_literal || !dynamic_cast<const IntegerLiteral *>(constant->literal)) {
            err_msg = LT("error : '") + constant->value() + LT("' is not valid for the range '") + pp_data_type->name + LT("'");
            return false;
        }
        else {
            auto rs = dynamic_cast<RangeSymbol *>(pp_data_type);
            assert(rs);
            int val = stoi(constant->value());
            if (val < rs->lower_bound || val > rs->upper_bound) {
                err_msg = LT("error : '") + constant->value() + LT("' is not within the range '") + pp_data_type->name + LT("'");
                return false;
            }
        }
    }
    else if (pp_data_type->is_partition() ) {
        if (!constant->is_literal || !dynamic_cast<const IntegerLiteral *>(constant)) {
            err_msg = LT("error : '") + constant->value() + LT("' is not valid for the partition '") + pp_data_type->name + LT("'");
            return false;
        }
        else {
            auto ps = dynamic_cast<PartitionSymbol *>(pp_data_type);
            assert(ps);
            int val = stoi(constant->value());
            if (val < 0 || (size_t) val >= ps->pp_enumerators.size()) {
                err_msg = LT("error : '") + constant->value() + LT("' is not within the partition '") + pp_data_type->name + LT("'");
                return false;
            }
        }
    }
    return true;
}

bool AttributeSymbol::is_maintained() const
{
    return nullptr != dynamic_cast<const MaintainedAttributeSymbol *>(this);
}


void AttributeSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // Add this attribute to the agent's list of all callback members
        pp_agent->pp_callback_members.push_back(this);
        break;
    }
    case ePopulateDependencies:
    {
        if (Symbol::option_event_trace) {
            // inject event trace code into side_effects function
            CodeBlock& c = side_effects_fn->func_body;
            c += "";
            c += "// Code Injection: event trace";
            c += "if (event_trace_on && om_active) {";
            if (!is_link_attribute()) {
                c += "double old_value = (double)om_old;";
                c += "double new_value = (double)om_new;";
                c += "event_trace_msg("
                    "\"" + agent->name + "\", "
                    "(int)entity_id, "
                    "(double)age.direct_get(), "
                    "GetCaseSeed(), "
                    "\"\", " // event_name (empty)
                    + to_string(pp_member_id) + ", " // id (member_id)
                    "\"" + name + "\", " // other_name (attribute_name)
                    "old_value, "
                    "new_value, "
                    "(double)BaseEvent::get_global_time(), "
                    "BaseEntity::et_msg_type::eAttributeChange);"
                    ;
            }
            else {
                // if link is nullptr, pass -1, else use entity_id
                c += "auto old_ptr = om_old.get();";
                c += "double old_value = old_ptr ? (double)old_ptr->entity_id : -1.0;";
                c += "auto new_ptr = om_new.get();";
                c += "double new_value = new_ptr ? (double)new_ptr->entity_id : -1.0;";
                c += "event_trace_msg("
                        "\"" + agent->name + "\", "
                        "(int)entity_id, "
                        "(double)age.direct_get(), "
                        "GetCaseSeed(), "
                        "\"\", " // event_name (empty)
                        + to_string(pp_member_id) + ", " // id (member_id)
                        "\"" + name + "\", " // other_name (attribute_name)
                        "old_value, "
                        "new_value, "
                        "(double)BaseEvent::get_global_time(), "
                        "BaseEntity::et_msg_type::eLinkAttributeChange);"
                    ;
                // if requested, add the linked entity to entities selected for tracing
                c += "if (new_ptr && BaseEntity::event_trace_show_linked_entities && BaseEntity::event_trace_selected_entities.count(entity_id) > 0 && BaseEntity::event_trace_selected_entities.count(new_ptr->entity_id) == 0) {";
                c +=      "BaseEntity::event_trace_selected_entities.insert(new_ptr->entity_id);";
                c +=      "event_trace_msg("
                            "\"" + agent->name + "\", "
                            "(int)entity_id, "
                            "(double)age.direct_get(), "
                            "GetCaseSeed(), "
                            "\"\", " // cstr1
                            "new_ptr->entity_id, " // other_id
                            "\"" + name + "\", " // cstr2
                            "0, " // dbl1
                            "0, " // dbl2
                            "(double)BaseEvent::get_global_time(), "
                            "BaseEntity::et_msg_type::eSnowball);"
                        ;
                c += "}";
            }
            c += "}";
        }
        break;
    }
    default:
        break;
    }
}


