/**
* @file    DerivedAgentVarSymbol.cpp
* Definitions for the DerivedAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "libopenm/common/omHelper.h"
#include "DerivedAgentVarSymbol.h"
#include "AgentSymbol.h"
#include "AgentVarSymbol.h"
#include "ConstantSymbol.h"
#include "PartitionSymbol.h"
#include "ClassificationSymbol.h"
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
string DerivedAgentVarSymbol::member_name(token_type tok,
                                          const Symbol *av1,
                                          const Symbol *av2,
                                          const Symbol *prt,
                                          const Symbol *cls,
                                          const ConstantSymbol *k1,
                                          const ConstantSymbol *k2,
                                          const ConstantSymbol *k3)
{
    string result = "om_";

    // Numeric prefix controls order of injection of side-effect code 
    // for related derived agentvars with inter-dependencies.
    // This works because post-parse passes respect lexicographical order of symbol names
    switch (tok) {
    case token::TK_undergone_entrance:
    case token::TK_undergone_exit:
    case token::TK_undergone_transition:
    case token::TK_undergone_change:
    {
        // undergone_* are used by value_at_first_*
        // so need to be updated after for the logic of 'first' detection to work.
        result += "2_";
        break;
    }
    case token::TK_value_at_first_entrance:
    case token::TK_value_at_first_exit:
    case token::TK_value_at_first_transition:
    case token::TK_value_at_first_change:
    {
        // will inject side-effect code after other derived agentvars
        result += "1_";
    }
    default:
    {
        break;
    }
    }

    result += token_to_string(tok);


    if (av1 != nullptr) result += "_FOR_" + av1->name;
    if (k1 != nullptr)  result += "_X_" + k1->value_as_name();
    if (av2 != nullptr) result += "_X_" + av2->name;
    if (prt != nullptr) result += "_X_" + prt->name;
    if (cls != nullptr) result += "_X_" + cls->name;
    if (k2 != nullptr)  result += "_X_" + k2->value_as_name();
    if (k3 != nullptr)  result += "_X_" + k3->value_as_name();

    return result;
}

// static
DerivedAgentVarSymbol * DerivedAgentVarSymbol::create_symbol(const Symbol* agent,
                                              token_type tok,
                                              const Symbol *av1,
                                              const Symbol *av2,
                                              const Symbol *prt,
                                              const Symbol *cls,
                                              const ConstantSymbol *k1,
                                              const ConstantSymbol *k2,
                                              const ConstantSymbol *k3,
                                              yy::location decl_loc)
{
    DerivedAgentVarSymbol *sym = nullptr;
    string mem_name = member_name(tok, av1, av2, prt, cls, k1, k2, k3);
    string nm = Symbol::symbol_name(mem_name, agent);
    auto it = symbols.find(nm);
    if (it != symbols.end()) {
        sym = dynamic_cast<DerivedAgentVarSymbol *>(it->second);
        assert(sym);
    }
    else {
        sym = new DerivedAgentVarSymbol(agent, tok, av1, av2, prt, cls, k1, k2, k3, decl_loc);
    }
    return sym;
}

void DerivedAgentVarSymbol::validate()
{
    // Check argument/member signature
    switch (tok) {

    case token::TK_duration:
    {
        assert(av1 || !av1); // observed (optional)
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(av1 ? (k1 != nullptr) : true); // constant (required for observed)
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_weighted_duration:
    {
        assert(av1 || !av1); // observed (optional)
        assert(av2); // weight
        assert(!prt);
        assert(!cls);
        assert(av1 ? (k1 != nullptr) : true); // constant (required for observed)
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_weighted_cumulation:
    {
        assert(av1); // observed
        assert(av2); // weight
        assert(!prt);
        assert(!cls);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_active_spell_duration:
    {
        assert(av1); // spell
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_completed_spell_duration:
    {
        assert(av1); // spell
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_active_spell_weighted_duration:
    {
        assert(av1); // spell
        assert(av2); // weight
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_completed_spell_weighted_duration:
    {
        assert(av1); // spell
        assert(av2); // weight
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_active_spell_delta:
    {
        assert(av1); // spell
        assert(av2); // delta
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_completed_spell_delta:
    {
        assert(av1); // spell
        assert(av2); // delta
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_undergone_entrance:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_undergone_exit:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_undergone_transition:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(k1); // from
        assert(k2); // to
        assert(!k3);
        break;
    }
    case token::TK_undergone_change:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_entrances:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_exits:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_transitions:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(k1); // from
        assert(k2); // to
        assert(!k3);
        break;
    }
    case token::TK_changes:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_first_entrance:
    {
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_latest_entrance:
    {
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_first_exit:
    {
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_latest_exit:
    {
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_first_transition:
    {
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!cls);
        assert(k1); // from
        assert(k2); // to
        assert(!k3);
        break;
    }
    case token::TK_value_at_latest_transition:
    {
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!cls);
        assert(k1); // from
        assert(k2); // to
        assert(!k3);
        break;
    }
    case token::TK_value_at_first_change:
    {
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!cls);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_latest_change:
    {
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!cls);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_entrances:
    {
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_exits:
    {
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_value_at_transitions:
    {
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!cls);
        assert(k1); // from
        assert(k2); // to
        assert(!k3);
        break;
    }
    case token::TK_value_at_changes:
    {
        assert(av1); // observed
        assert(av2); // value
        assert(!prt);
        assert(!cls);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_split:
    {
        assert(av1); // observed
        assert(!av2);
        assert(prt);
        assert(!cls);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_aggregate:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(cls);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_trigger_entrances:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_trigger_exits:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_trigger_transitions:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(k1); // from
        assert(k2); // to
        assert(!k3);
        break;
    }
    case token::TK_trigger_changes:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_duration_counter:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(k2); // interval
        assert(k3 || !k3); // maxcount (optional)
        break;
    }
    case token::TK_duration_trigger:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(k1); // constant
        assert(k2); // delay
        assert(!k3);
        break;
    }
    case token::TK_self_scheduling_int:
    {
        assert(av1); // observed
        assert(!av2);
        assert(!prt);
        assert(!cls);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    case token::TK_self_scheduling_split:
    {
        assert(av1); // observed
        assert(!av2);
        assert(prt);
        assert(!cls);
        assert(!k1);
        assert(!k2);
        assert(!k3);
        break;
    }
    default:
        assert(false);
    }
}

void DerivedAgentVarSymbol::create_auxiliary_symbols()
{
    // Create identity agentvar monitoring av1 == k1
    switch (tok) {
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

    // Create associated derived agentvar
    switch (tok) {
    case token::TK_value_at_first_entrance:
    {
        assert(av1);
        assert(k1);
        dav = DerivedAgentVarSymbol::create_symbol(agent, token::TK_undergone_entrance, *av1, k1, decl_loc);
        assert(dav);
        break;
    }
    case token::TK_value_at_first_exit:
    {
        assert(av1);
        assert(k1);
        dav = DerivedAgentVarSymbol::create_symbol(agent, token::TK_undergone_exit, *av1, k1, decl_loc);
        assert(dav);
        break;
    }
    case token::TK_value_at_first_transition:
    {
        assert(av1);
        assert(k1);
        assert(k2);
        dav = DerivedAgentVarSymbol::create_symbol(agent, token::TK_undergone_transition, *av1, k1, k2, decl_loc);
        assert(dav);
        break;
    }
    case token::TK_value_at_first_change:
    {
        assert(av1);
        dav = DerivedAgentVarSymbol::create_symbol(agent, token::TK_undergone_change, *av1, decl_loc);
        assert(dav);
        break;
    }
    default:
    break;
    }

}

void DerivedAgentVarSymbol::assign_data_type()
{
    switch (tok) {

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
    case token::TK_trigger_entrances:
    case token::TK_trigger_exits:
    case token::TK_trigger_transitions:
    case token::TK_trigger_changes:
    case token::TK_duration_trigger:
    {
        change_data_type(BoolSymbol::find());
        break;
    }

    // type is counter
    case token::TK_entrances:
    case token::TK_exits:
    case token::TK_transitions:
    case token::TK_changes:
    case token::TK_duration_counter:
    case token::TK_self_scheduling_int:
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

    // type is the appropriate summing type for av2 (real or integer)
    case token::TK_active_spell_delta:
    case token::TK_completed_spell_delta:
    case token::TK_value_at_entrances:
    case token::TK_value_at_exits:
    case token::TK_value_at_transitions:
    case token::TK_value_at_changes:
    {
        assert(pp_av2);
        auto typ = pp_av2->pp_data_type->summing_type();
        assert(typ);
        change_data_type(typ);
        break;
    }

    // type is the partition
    case token::TK_split:
    case token::TK_self_scheduling_split:
    {
        assert(pp_prt);
        change_data_type(pp_prt);
        break;
    }

    // type is the classification
    case token::TK_aggregate:
    {
        assert(pp_cls);
        change_data_type(pp_cls);
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
    switch (tok) {
    case token::TK_duration:
    {
        if (!av1) {
            // simple duration()
            // add side-effect to time
            auto *av = pp_agent->pp_time;
            assert(av);
            CodeBlock& c = av->side_effects_fn->func_body;
            c += "// Advance time for " + pretty_name();
            c += name + ".set(" + name + ".get() + om_delta);";
            c += "";
        }
        else {
            // conditioned duration(av, value)
            // add side-effect to time
            auto *av = pp_agent->pp_time;
            assert(av);
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
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_weighted_cumulation:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_active_spell_duration:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_completed_spell_duration:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_active_spell_weighted_duration:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_completed_spell_weighted_duration:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_active_spell_delta:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_completed_spell_delta:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_undergone_entrance:
    {
        auto *av = pp_av1;
        assert(av);
        assert(k1);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (!" + name + ".get() && om_new == " + k1->value() + ") {";
        c += name + ".set(true);";
        c += "}";
        c += "";
        break;
    }
    case token::TK_undergone_exit:
    {
        auto *av = pp_av1;
        assert(av);
        assert(k1);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (!" + name + ".get() && om_old == " + k1->value() + ") {";
        c += name + ".set(true);";
        c += "}";
        c += "";
        break;
    }
    case token::TK_undergone_transition:
    {
        auto *av = pp_av1;
        assert(av);
        assert(k1);
        assert(k2);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (!" + name + ".get() && om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        c += name + ".set(true);";
        c += "}";
        c += "";
        break;
    }
    case token::TK_undergone_change:
    {
        auto *av = pp_av1;
        assert(av);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (!" + name + ".get()) {";
        c += name + ".set(true);";
        c += "}";
        c += "";
        break;
    }
    case token::TK_entrances:
    {
        auto *av = pp_av1;
        assert(av);
        assert(k1);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (om_new == " + k1->value() + ") {";
        c += name + ".set(" + name + ".get() + 1);";
        c += "}";
        c += "";
        break;
    }
    case token::TK_exits:
    {
        auto *av = pp_av1;
        assert(av);
        assert(k1);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (om_old == " + k1->value() + ") {";
        c += name + ".set(" + name + ".get() + 1);";
        c += "}";
        c += "";
        break;
    }
    case token::TK_transitions:
    {
        auto *av = pp_av1;
        assert(av);
        assert(k1);
        assert(k2);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        c += name + ".set(" + name + ".get() + 1);";
        c += "}";
        c += "";
        break;
    }
    case token::TK_changes:
    {
        auto *av = pp_av1;
        assert(av);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += name + ".set(" + name + ".get() + 1);";
        c += "";
        break;
    }
    case token::TK_value_at_first_entrance:
    {
        auto *av = pp_av1;
        auto *noted = pp_av2;
        auto undergone = dav;
        assert(av);
        assert(k1);
        assert(noted);
        assert(undergone);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (!" + undergone->name + " && om_new == " + k1->value() + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        c += "";
        break;
    }
    case token::TK_value_at_latest_entrance:
    {
        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(k1);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (om_new == " + k1->value() + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        c += "";
        break;
    }
    case token::TK_value_at_first_exit:
    {
        auto *av = pp_av1;
        auto *noted = pp_av2;
        auto undergone = dav;
        assert(av);
        assert(k1);
        assert(noted);
        assert(undergone);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (!" + undergone->name + " && om_old == " + k1->value() + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        c += "";
        break;
    }
    case token::TK_value_at_latest_exit:
    {
        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(k1);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (om_old == " + k1->value() + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        c += "";
        break;
    }
    case token::TK_value_at_first_transition:
    {
        auto *av = pp_av1;
        auto *noted = pp_av2;
        auto undergone = dav;
        assert(av);
        assert(k1);
        assert(k2);
        assert(noted);
        assert(undergone);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (!" + undergone->name + " && om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        c += "";
        break;
    }
    case token::TK_value_at_latest_transition:
    {
        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(k1);
        assert(k2);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        c += "";
        break;
    }
    case token::TK_value_at_first_change:
    {
        auto *av = pp_av1;
        auto *noted = pp_av2;
        auto undergone = dav;
        assert(av);
        assert(noted);
        assert(undergone);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (!" + undergone->name + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        c += "";
        break;
    }
    case token::TK_value_at_latest_change:
    {
        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += name + ".set(" + noted->name + ".get());";
        c += "";
        break;
    }
    case token::TK_value_at_entrances:
    {
        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(k1);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (om_new == " + k1->value() + ") {";
        c += name + ".set(" + name + ".get() + " + noted->name + ".get());";
        c += "}";
        c += "";
        break;
    }
    case token::TK_value_at_exits:
    {
        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(k1);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (om_old == " + k1->value() + ") {";
        c += name + ".set(" + name + ".get() + " + noted->name + ".get());";
        c += "}";
        c += "";
        break;
    }
    case token::TK_value_at_transitions:
    {
        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(k1);
        assert(k2);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += "if (om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        c += name + ".set(" + name + ".get() + " + noted->name + ".get());";
        c += "}";
        c += "";
        break;
    }
    case token::TK_value_at_changes:
    {
        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += "// Maintain " + pretty_name();
        c += name + ".set(" + name + ".get() + " + noted->name + ".get());";
        c += "";
        break;
    }
    case token::TK_split:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_aggregate:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_trigger_entrances:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_trigger_exits:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_trigger_transitions:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_trigger_changes:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_duration_counter:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_duration_trigger:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_self_scheduling_int:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_self_scheduling_split:
    {
        // TODO
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }

    default:
    break;
    }
}

string DerivedAgentVarSymbol::pretty_name()
{
    string result;

    switch (tok) {
    case token::TK_duration:
    {
        if (!pp_av1) {
            result = token_to_string(tok) + "()";
        }
        else {
            result = token_to_string(tok) + "(" + pp_av1->name + ", " + k1->value() + ")";
        }
        break;
    }
    case token::TK_weighted_duration:
    {
        assert(pp_av2);
        if (!pp_av1) {
            result = token_to_string(tok) + "(" + pp_av2->name + ")";
        }
        else {
            result = token_to_string(tok) + "(" + pp_av1->name + ", " + k1->value() + ", " + pp_av2->name + ")";
        }
        break;
    }
    case token::TK_active_spell_duration:
    case token::TK_completed_spell_duration:
    case token::TK_undergone_entrance:
    case token::TK_undergone_exit:
    case token::TK_entrances:
    case token::TK_exits:
    case token::TK_trigger_entrances:
    case token::TK_trigger_exits:
    {
        assert(pp_av1);
        assert(k1);
        result = token_to_string(tok) + "(" + pp_av1->name + ", " + k1->value() + ")";
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
    case token::TK_value_at_entrances:
    case token::TK_value_at_exits:
    {
        assert(pp_av1);
        assert(k1);
        assert(pp_av2);
        result = token_to_string(tok) + "(" + pp_av1->name + ", " + k1->value() + ", " + pp_av2->name + ")";
        break;
    }
    case token::TK_undergone_transition:
    case token::TK_transitions:
    case token::TK_trigger_transitions:
    case token::TK_duration_trigger:
    {
        assert(pp_av1);
        assert(k1);
        assert(k2);
        result = token_to_string(tok) + "(" + pp_av1->name + ", " + k1->value() + ", " + k2->value() + ")";
        break;
    }
    case token::TK_value_at_first_transition:
    case token::TK_value_at_latest_transition:
    case token::TK_value_at_transitions:
    {
        assert(pp_av1);
        assert(k1);
        assert(k2);
        assert(pp_av2);
        result = token_to_string(tok) + "(" + pp_av1->name + ", " + k1->value() + ", " + k2->value() + ", " + pp_av2->name + ")";
        break;
    }
    case token::TK_undergone_change:
    case token::TK_changes:
    case token::TK_trigger_changes:
    {
        assert(pp_av1);
        result = token_to_string(tok) + "(" + pp_av1->name + ")";
        break;
    }
    case token::TK_weighted_cumulation:
    case token::TK_value_at_first_change:
    case token::TK_value_at_latest_change:
    case token::TK_value_at_changes:
    {
        assert(pp_av1);
        assert(pp_av2);
        result = token_to_string(tok) + "(" + pp_av1->name + ", " + pp_av2->name + ")";
        break;
    }
    case token::TK_split:
    case token::TK_self_scheduling_split:
    {
        assert(pp_av1);
        assert(pp_prt);
        result = token_to_string(tok) + "(" + pp_av1->name + ", " + pp_prt->name + ")";
        break;
    }
    case token::TK_aggregate:
    {
        assert(pp_av1);
        assert(pp_cls);
        result = token_to_string(tok) + "(" + pp_av1->name + ", " + pp_cls->name + ")";
        break;
    }
    case token::TK_duration_counter:
    {
        assert(pp_av1);
        assert(k1);
        assert(k2);
        assert(k3 || !k3); // optional
        result = token_to_string(tok) + "(" + pp_av1->name + ", " + k1->value() + ", " + k2->value() + (k3 ? (", " + k3->value()) : "") + ")";
        break;
    }
    case token::TK_self_scheduling_int:
    {
        assert(pp_av1);
        result = token_to_string(tok) + "(" + pp_av1->name + ")";
        break;
    }
    default:
    {
        result = token_to_string(tok) + "( ... )";
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
        if (av1) {
            pp_av1 = dynamic_cast<AgentVarSymbol *> (pp_symbol(av1));
            if (!pp_av1) {
                pp_error("Error - '" + (*av1)->name + "' is not an attribute of " + agent->name);
                throw HelperException("Stopping post parse processing");
            }
        }
        if (av2) {
            pp_av2 = dynamic_cast<AgentVarSymbol *> (pp_symbol(av2));
            if (!pp_av2) {
                pp_error("Error - '" + (*av2)->name + "' is not an attribute of " + agent->name);
                throw HelperException("Stopping post parse processing");
            }
        }
        if (prt) {
            pp_prt = dynamic_cast<PartitionSymbol *> (pp_symbol(prt));
            if (!pp_prt) {
                pp_error("Error - '" + (*prt)->name + "' is not a partition");
                throw HelperException("Stopping post parse processing");
            }
        }
        if (cls) {
            pp_cls = dynamic_cast<ClassificationSymbol *> (pp_symbol(cls));
            if (!pp_cls) {
                pp_error("Error - '" + (*cls)->name + "' is not a classification");
                throw HelperException("Stopping post parse processing");
            }
        }
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


