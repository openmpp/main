/**
* @file    AgentVarSymbol.cpp
* Definitions for the AgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <string> // for stoi
#include "AgentVarSymbol.h"
#include "AgentSymbol.h"
#include "AgentFuncSymbol.h"
#include "TypeSymbol.h"
#include "NumericSymbol.h"
#include "AgentInternalSymbol.h"
#include "ConstantSymbol.h"
#include "Literal.h"
#include "EnumeratorSymbol.h"
#include "EnumerationWithEnumeratorsSymbol.h"
#include "RangeSymbol.h"
#include "PartitionSymbol.h"
#include "CodeBlock.h"

using namespace std;

void AgentVarSymbol::create_auxiliary_symbols()
{
    assert(!side_effects_fn); // logic guarantee
    side_effects_fn = new AgentFuncSymbol("om_side_effects_" + name,
                                          agent,
                                          "void",
                                          data_type->name + " om_old, " + data_type->name + " om_new");
    assert(side_effects_fn); // out of memory check
    side_effects_fn->doc_block = doxygen_short("Implement side effects of changing " + name + " in agent " + agent->name + ".");

    assert(!notify_fn); // logic guarantee
    notify_fn = new AgentFuncSymbol("om_notify_" + name,
                                          agent,
                                          "void",
                                          "");
    assert(notify_fn); // out of memory check
    notify_fn->doc_block = doxygen_short("Implement notification before changing " + name + " in agent " + agent->name + ".");
}

CodeBlock AgentVarSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += "static const string om_name_" + name + ";";

    return h;
}

CodeBlock AgentVarSymbol::cxx_definition_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "const string " + pp_agent->name + "::om_name_" + name + " = \"" + pretty_name() + "\";";

    return c;
}

void AgentVarSymbol::change_data_type(TypeSymbol *new_type)
{
    // TODO Pass it upwards to AgentDataMemberSymbol rather than fiddling directly with members.
    assert(new_type);
    if (pp_data_type != new_type) {
        pp_data_type = new_type;
        // Note that data_type remains as it was, since references cannot be reseated.
        assert(side_effects_fn); // logic guarantee
        side_effects_fn->arg_list_decl = pp_data_type->name + " om_old, " + pp_data_type->name + " om_new";
        // maintain global counter of type changes
        ++Symbol::type_changes;
    }
}

string AgentVarSymbol::get_lagged_name()
{
    return "om_lagged_" + name;
}

string AgentVarSymbol::get_lagged_event_counter_name()
{
    return "om_lagged_event_counter_" + name;
}

void AgentVarSymbol::create_lagged()
{
    string lagged_name = get_lagged_name();
    string lagged_counter_name = get_lagged_event_counter_name();

    // nothing to do if already created
    if (exists(lagged_name, this)) {
        assert(exists(lagged_counter_name, this)); // associated counter should exist too
        return;
    }

    // lagged stores in same type as this agentvar
    auto lagged = new AgentInternalSymbol(lagged_name, agent, data_type);

    // lagged event counter stores in same type as global event counter (big_counter)
    auto *typ = NumericSymbol::find(token::TK_big_counter);
    assert(typ); // initialization guarantee
    auto lagged_event_counter = new AgentInternalSymbol(lagged_counter_name, agent, typ);

    // Add side-effect code to maintain lagged value
    CodeBlock & c = side_effects_fn->func_body;
    c += "// maintain lagged value";
    c += "if (BaseEvent::global_event_counter > " + lagged_counter_name + ") {";
    c += lagged_name + " = om_old;";
    c += lagged_counter_name + " = BaseEvent::global_event_counter;";
    c += "}";
}

bool AgentVarSymbol::is_valid_comparison(const ConstantSymbol * constant, string &err_msg)
{
    assert(constant);
    assert(pp_data_type);
    if (pp_data_type->is_bool()) {
        if (constant->is_literal
            && (constant->value() == "true" || constant->value() == "false")) {
            // Valid
        }
        else {
            err_msg = "Error - '" + constant->value() + "' is not a boolean constant";
            return false;
        }
    }
    else if (pp_data_type->is_classification()) {
        if (constant->is_enumerator
            && constant->pp_enumerator->pp_enumeration->name == pp_data_type->name) {
            // Valid
        }
        else {
            err_msg = "Error - '" + constant->value() + "' is not valid for the classification '" + pp_data_type->name + "'";
            return false;
        }
    }
    else if (pp_data_type->is_range() ) {
        if (!constant->is_literal || !dynamic_cast<const IntegerLiteral *>(constant->literal)) {
            err_msg = "Error - '" + constant->value() + "' is not valid for the range '" + pp_data_type->name + "'";
            return false;
        }
        else {
            auto rs = dynamic_cast<RangeSymbol *>(pp_data_type);
            assert(rs);
            int val = stoi(constant->value());
            if (val < rs->lower_bound || val > rs->upper_bound) {
                err_msg = "Error - '" + constant->value() + "' is not within the range '" + pp_data_type->name + "'";
                return false;
            }
        }
    }
    else if (pp_data_type->is_partition() ) {
        if (!constant->is_literal || !dynamic_cast<const IntegerLiteral *>(constant)) {
            err_msg = "Error - '" + constant->value() + "' is not valid for the partition '" + pp_data_type->name + "'";
            return false;
        }
        else {
            auto ps = dynamic_cast<PartitionSymbol *>(pp_data_type);
            assert(ps);
            int val = stoi(constant->value());
            if (val < 0 || (size_t) val >= ps->pp_enumerators.size()) {
                err_msg = "Error - '" + constant->value() + "' is not within the partition '" + pp_data_type->name + "'";
                return false;
            }
        }
    }
    return true;
}


void AgentVarSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // Add this agentvar to the agent's list of all callback members
        pp_agent->pp_callback_members.push_back(this);
        break;
    }
    default:
        break;
    }
}


