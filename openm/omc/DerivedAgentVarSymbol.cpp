/**
* @file    DerivedAgentVarSymbol.cpp
* Definitions for the DerivedAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DerivedAgentVarSymbol.h"
#include "AgentSymbol.h"
#include "AgentVarSymbol.h"
#include "ConstantSymbol.h"
#include "PartitionSymbol.h"
#include "IdentityAgentVarSymbol.h"
#include "BuiltinAgentVarSymbol.h"
#include "TimeSymbol.h"
#include "RealSymbol.h"
#include "BoolSymbol.h"
#include "ExprForAgentVar.h"
#include "Literal.h"
#include "CodeBlock.h"

using namespace std;

// static
string DerivedAgentVarSymbol::member_name(token_type tk1,
                                          token_type tk2,
                                          const Symbol *av1,
                                          const Symbol *av2,
                                          const Symbol *prt,
                                          const ConstantSymbol *k1,
                                          const ConstantSymbol *k2,
                                          const ConstantSymbol *k3)
{
    string result = "om";
    if (tk1 != token::TK_unused) result += "_" + token_to_string(tk1);
    if (tk2 != token::TK_unused) result += "_" + token_to_string(tk2);
    if (av1 != nullptr) result += "_" + av1->name;
    if (k1 != nullptr)  result += "_" + k1->value_as_name();
    if (av2 != nullptr) result += "_" + av2->name;
    if (prt != nullptr) result += "_" + prt->name;
    if (k2 != nullptr)  result += "_" + k2->value_as_name();
    if (k3 != nullptr)  result += "_" + k3->value_as_name();

    return result;
}

// static
Symbol * DerivedAgentVarSymbol::create_symbol(const Symbol* agent,
                                              token_type tk1,
                                              token_type tk2,
                                              const Symbol *av1,
                                              const Symbol *av2,
                                              const Symbol *prt,
                                              const ConstantSymbol *k1,
                                              const ConstantSymbol *k2,
                                              const ConstantSymbol *k3,
                                              yy::location decl_loc)
{
    Symbol *sym = nullptr;
    string mem_name = member_name(tk1, tk2, av1, av2, prt, k1, k2, k3);
    string nm = Symbol::symbol_name(mem_name, agent);
    auto it = symbols.find(nm);
    if (it != symbols.end()) {
        sym = it->second;
    }
    else {
        sym = new DerivedAgentVarSymbol(agent, tk1, tk2, av1, av2, prt, k1, k2, k3, decl_loc);
    }
    return sym;
}

void DerivedAgentVarSymbol::validate()
{
    // Check if implemented, and issue warning if not
    switch (tk1) {

    // implemented
    case token::TK_duration:
    break;

    // not implemented
    case token::TK_weighted_duration:
    case token::TK_weighted_cumulation:
    case token::TK_active_spell_duration:
    case token::TK_completed_spell_duration:
    case token::TK_active_spell_weighted_duration:
    case token::TK_completed_spell_weighted_duration:
    case token::TK_active_spell_delta:
    case token::TK_completed_spell_delta:
    case token::TK_undergone_entrance:
    case token::TK_undergone_exit:
    case token::TK_undergone_transition:
    case token::TK_undergone_change:
    case token::TK_entrances:
    case token::TK_exits:
    case token::TK_transitions:
    case token::TK_changes:
    case token::TK_value_at_first_entrance:
    case token::TK_value_at_latest_entrance:
    case token::TK_value_at_first_exit:
    case token::TK_value_at_latest_exit:
    case token::TK_value_at_first_transition:
    case token::TK_value_at_latest_transition:
    case token::TK_value_at_first_change:
    case token::TK_value_at_latest_change:
    case token::TK_value_at_entrances:
    case token::TK_value_at_exits:
    case token::TK_value_at_transitions:
    case token::TK_value_at_changes:
    case token::TK_split:
    case token::TK_aggregate:
    case token::TK_trigger_entrances:
    case token::TK_trigger_exits:
    case token::TK_trigger_transitions:
    case token::TK_trigger_changes:
    case token::TK_duration_counter:
    case token::TK_duration_trigger:
    case token::TK_self_scheduling_int:
    case token::TK_self_scheduling_split:
    default:
        pp_warning("Warning - Not implemented (will always be 0) - " + Symbol::token_to_string(tk1) + "( ... )");
    }

    // Check argument/member signature
    switch (tk1) {

    case token::TK_duration:
    {
        assert(tk2 == token::TK_unused);
        assert(av1 || !av1); // observed
        assert(!av2);
        assert(!prt);
        assert(av1 ? (k1 != nullptr) : true); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_weighted_duration:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // weight
        assert(!av2);
        assert(!prt);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_weighted_cumulation:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(av2); // weight
        assert(!prt);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_active_spell_duration:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // spell
        assert(!av2);
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_completed_spell_duration:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // spell
        assert(!av2);
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_active_spell_weighted_duration:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // spell
        assert(av2); // weight
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_completed_spell_weighted_duration:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // spell
        assert(av2); // weight
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_active_spell_delta:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // spell
        assert(av2); // delta
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_completed_spell_delta:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // spell
        assert(av2); // delta
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_undergone_entrance:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_undergone_exit:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_undergone_transition:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(k1); // from
        assert(k2); // to
        assert(!k3);
        break;
    }
    case token::TK_undergone_change:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_entrances:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_exits:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_transitions:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(k1); // from
        assert(k2); // to
        assert(!k3);
        break;
    }
    case token::TK_changes:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_first_entrance:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_latest_entrance:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_first_exit:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_latest_exit:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_first_transition:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(k1); // from
        assert(k2); // to
        assert(!k3);
        break;
    }
    case token::TK_value_at_latest_transition:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(k1); // from
        assert(k2); // to
        assert(!k3);
        break;
    }
    case token::TK_value_at_first_change:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_latest_change:
    {
        assert(tk2 == token::TK_unused);
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_entrances:
    case token::TK_value_at_exits:
    case token::TK_value_at_transitions:
    case token::TK_value_at_changes:
    case token::TK_split:
    case token::TK_aggregate:
    case token::TK_trigger_entrances:
    case token::TK_trigger_exits:
    case token::TK_trigger_transitions:
    case token::TK_trigger_changes:
    case token::TK_duration_counter:
    case token::TK_duration_trigger:
    case token::TK_self_scheduling_int:
    case token::TK_self_scheduling_split:
    break; // TODO temporary remove after completion of list
    default:
        assert(false);
    }

}

void DerivedAgentVarSymbol::create_auxiliary_symbols()
{
    // Create identity agentvar monitoring av1 == k1
    switch (tk1) {
    case token::TK_duration:
    {
        if (av1 && k1) {
            // is a conditioned duration
            iav = IdentityAgentVarSymbol::CreateEqualityIdentitySymbol(agent, *av1, k1, decl_loc);
            assert(iav);
        }
        break;
    }
    case token::TK_active_spell_duration:
    case token::TK_completed_spell_duration:
    case token::TK_active_spell_weighted_duration:
    case token::TK_completed_spell_weighted_duration:
    case token::TK_active_spell_delta:
    case token::TK_completed_spell_delta:
    {
        iav = IdentityAgentVarSymbol::CreateEqualityIdentitySymbol(agent, *av1, k1, decl_loc);
        assert(iav);
        break;
    }
    default:
    break;
    }
}

void DerivedAgentVarSymbol::assign_data_type()
{
    switch (tk1) {

    // type is Time
    case token::TK_duration:
    case token::TK_active_spell_duration:
    case token::TK_completed_spell_duration:
    {
        change_data_type(TimeSymbol::find());
        break;
    }

    // type is real
    case token::TK_weighted_duration:
    case token::TK_weighted_cumulation:
    case token::TK_active_spell_weighted_duration:
    case token::TK_completed_spell_weighted_duration:
    {
        change_data_type(RealSymbol::find());
        break;
    }

    // type is bool
    case token::TK_undergone_entrance:
    case token::TK_undergone_exit:
    case token::TK_undergone_transition:
    case token::TK_undergone_change:
    {
        change_data_type(BoolSymbol::find());
        break;
    }

    // type is counter
    case token::TK_entrances:
    case token::TK_exits:
    case token::TK_transitions:
    case token::TK_changes:
    {
        auto *sym = NumericSymbol::find(token::TK_counter);
        assert(sym);  // Initialization guarantee
        change_data_type(sym);
        break;
    }

    // type is same as av2
    case token::TK_value_at_first_entrance:
    case token::TK_value_at_latest_entrance:
    case token::TK_value_at_first_exit:
    case token::TK_value_at_latest_exit:
    case token::TK_value_at_first_transition:
    case token::TK_value_at_latest_transition:
    case token::TK_value_at_first_change:
    case token::TK_value_at_latest_change:
    {
        assert(pp_av2);
        auto typ = pp_av2->pp_data_type;
        assert(typ);
        change_data_type(typ);
        break;
    }

    case token::TK_active_spell_delta:
    case token::TK_completed_spell_delta:
    {
        if (pp_av2->pp_data_type->is_floating()) {
            // set type to real
            change_data_type(RealSymbol::find());
        }
        else {
            // set type to integer
            auto *sym = NumericSymbol::find(token::TK_integer);
            assert(sym);  // Initialization guarantee
            change_data_type(sym);
        }
        break;
    }

    default:
    // TODO Leave at double, but once all are implemented,
    //  assert(false);
    break;
    }

}

void DerivedAgentVarSymbol::create_side_effects()
{
    switch (tk1) {
    case token::TK_duration:
    {
        if (!av1) {
            // simple duration()
            // add side-effect to time
            auto *av = pp_agent->pp_time;
            CodeBlock& c = av->side_effects_fn->func_body;
            c += "// Advance time for " + pretty_name();
            c += name + ".set(" + name + ".get() + om_delta);";
            c += "";
        }
        else {
            // conditioned duration(av, value)
            // add side-effect to time
            auto *av = pp_agent->pp_time;
            CodeBlock& c = av->side_effects_fn->func_body;
            c += "// Advance time for " + pretty_name();
            c += "if (" + iav->name + ") {";
            c += name + ".set(" + name + ".get() + om_delta);";
            c += "}";
            c += "";
        }
        break;
    }
    case token::TK_weighted_duration:
    {
        // TODO
        break;
    }
    case token::TK_weighted_cumulation:
    {
        // TODO
        break;
    }
    case token::TK_active_spell_duration:
    {
        // TODO
        break;
    }
    case token::TK_completed_spell_duration:
    {
        // TODO
        break;
    }
    case token::TK_active_spell_weighted_duration:
    {
        // TODO
        break;
    }
    case token::TK_completed_spell_weighted_duration:
    {
        // TODO
        break;
    }
    case token::TK_active_spell_delta:
    {
        // TODO
        break;
    }
    case token::TK_completed_spell_delta:
    {
        // TODO
        break;
    }
    case token::TK_undergone_entrance:
    {
        // TODO
        break;
    }
    case token::TK_undergone_exit:
    {
        // TODO
        break;
    }
    case token::TK_undergone_transition:
    {
        // TODO
        break;
    }
    case token::TK_undergone_change:
    {
        // TODO
        break;
    }
    case token::TK_entrances:
    {
        // TODO
        break;
    }
    case token::TK_exits:
    {
        // TODO
        break;
    }
    case token::TK_transitions:
    {
        // TODO
        break;
    }
    case token::TK_changes:
    {
        // TODO
        break;
    }
    case token::TK_value_at_first_entrance:
    {
        // TODO
        break;
    }
    case token::TK_value_at_latest_entrance:
    {
        // TODO
        break;
    }
    case token::TK_value_at_first_exit:
    {
        // TODO
        break;
    }
    case token::TK_value_at_latest_exit:
    {
        // TODO
        break;
    }
    case token::TK_value_at_first_transition:
    {
        // TODO
        break;
    }
    case token::TK_value_at_latest_transition:
    {
        // TODO
        break;
    }
    case token::TK_value_at_first_change:
    {
        // TODO
        break;
    }
    case token::TK_value_at_latest_change:
    {
        // TODO
        break;
    }

    default:
    break;
    }
}

string DerivedAgentVarSymbol::pretty_name()
{
    string result;

    switch (tk1) {
    case token::TK_duration:
    {
        if (!av1) {
            result = token_to_string(tk1) + "()";
        }
        else {
            result = token_to_string(tk1) + "(" + pp_av1->name + ", " + k1->value() + ")";
        }
        break;
    }
    case token::TK_active_spell_duration:
    case token::TK_completed_spell_duration:
    case token::TK_undergone_entrance:
    case token::TK_undergone_exit:
    case token::TK_entrances:
    case token::TK_exits:
    {
        assert(av1);
        assert(k1);
        result = token_to_string(tk1) + "(" + pp_av1->name + ", " + k1->value() + ")";
        break;
    }
    case token::TK_active_spell_weighted_duration:
    case token::TK_completed_spell_weighted_duration:
    case token::TK_active_spell_delta:
    case token::TK_completed_spell_delta:
    case token::TK_value_at_first_entrance:
    case token::TK_value_at_latest_entrance:
    case token::TK_value_at_first_exit:
    case token::TK_value_at_latest_exit:
    {
        assert(av1);
        assert(k1);
        assert(av2);
        result = token_to_string(tk1) + "(" + pp_av1->name + ", " + k1->value() + ", " + pp_av2->name + ")";
        break;
    }
    case token::TK_undergone_transition:
    case token::TK_transitions:
    {
        assert(av1);
        assert(k1);
        assert(k2);
        result = token_to_string(tk1) + "(" + pp_av1->name + ", " + k1->value() + ", " + k2->value() + ")";
        break;
    }
    case token::TK_value_at_first_transition:
    case token::TK_value_at_latest_transition:
    {
        assert(av1);
        assert(k1);
        assert(k2);
        assert(av2);
        result = token_to_string(tk1) + "(" + pp_av1->name + ", " + k1->value() + ", " + k2->value() + ", " + pp_av2->name + ")";
        break;
    }
    case token::TK_weighted_duration:
    case token::TK_undergone_change:
    case token::TK_changes:
    {
        assert(av1);
        result = token_to_string(tk1) + "(" + pp_av1->name + ")";
        break;
    }
    case token::TK_weighted_cumulation:
    case token::TK_value_at_first_change:
    case token::TK_value_at_latest_change:
    {
        assert(av1);
        assert(av2);
        result = token_to_string(tk1) + "(" + pp_av1->name + ", " + pp_av2->name + ")";
        break;
    }
    default:
    {
        if (tk2 == token::TK_unused) {
            result = token_to_string(tk1) + "( ... )";
        }
        else {
            result = token_to_string(tk1) + "(" + token_to_string(tk1) + "( ... ))";
        }
        break;
    }

    }
    return result;
}

void DerivedAgentVarSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        break;
    }
    case ePopulateCollections:
    {
        // assign direct pointers for post-parse use
        if (av1) pp_av1 = dynamic_cast<AgentVarSymbol *> (pp_symbol(av1));
        if (av1) assert(pp_av1);
        if (av2) pp_av2 = dynamic_cast<AgentVarSymbol *> (pp_symbol(av2));
        if (av2) assert(pp_av2);
        if (prt) pp_prt = dynamic_cast<PartitionSymbol *> (pp_symbol(prt));
        if (prt) assert(pp_prt);
        break;
    }
    case eResolveDataTypes:
    {
        assign_data_type();
        break;
    }
    case ePopulateDependencies:
    {
        create_side_effects();
        break;
    }
    default:
        break;
    }
}

CodeBlock DerivedAgentVarSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    h += "AgentVar<"
        + pp_data_type->name + ", "
        + pp_data_type->wrapped_type() + ", "
        + pp_agent->name + ", "
        + "&" + side_effects_fn->unique_name + ">";
    h += name + ";";

    return h;
}


