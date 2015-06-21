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
#include "AgentInternalSymbol.h"
#include "AgentEventSymbol.h"
#include "TimeSymbol.h"
#include "RealSymbol.h"
#include "BoolSymbol.h"
#include "AggregationSymbol.h"
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
    string result;

    result = "om_" + token_to_string(tok);

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
                                              const ConstantSymbol *k3)
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
        sym = new DerivedAgentVarSymbol(agent, tok, av1, av2, prt, cls, k1, k2, k3);
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

void DerivedAgentVarSymbol::assign_sorting_group()
{

    // The sorting_group affects the order of injection of side-effect code 
    // for related derived agentvars with inter-dependencies (in a side-effect of the same agentvar).
    // Code injection is ordered by sorting group, then lexicographically by unique_name.
    // 
    // The ordering requirements are as follows:
    // continuously-updated durations occur first (side-effect of 'time')
    // undergone_* (2) is used by value_at_first_* (1) in side-effect of condition
    // active_spell_* (2) is used by completed_spell_* (1)in side-effect of condition
    // active_spell_delta (2) occurs before identity condition (9) in side-effect of time

    switch (tok) {
    case token::TK_trigger_entrances:
    case token::TK_trigger_exits:
    case token::TK_trigger_transitions:
    case token::TK_trigger_changes:
    {
        // trigger states are reset before anything else in the self-scheduling event.
        sorting_group = 1;
        break;
    }
    case token::TK_duration:
    case token::TK_weighted_duration:
    {
        // continuously-updated states occur next in 'time' side-effects.
        sorting_group = 2;
        break;
    }
    case token::TK_value_at_first_entrance:
    case token::TK_value_at_first_exit:
    case token::TK_value_at_first_transition:
    case token::TK_value_at_first_change:
    case token::TK_completed_spell_duration:
    case token::TK_completed_spell_weighted_duration:
    case token::TK_completed_spell_delta:
    {
        // Must occur before undergone_* or active_spell_* updates the value
        sorting_group = 3;
        break;
    }
    case token::TK_undergone_entrance:
    case token::TK_undergone_exit:
    case token::TK_undergone_transition:
    case token::TK_undergone_change:
    case token::TK_active_spell_duration:
    case token::TK_active_spell_weighted_duration:
    case token::TK_active_spell_delta:
    {
        sorting_group = 4;
        break;
    }
    default:
    {
        sorting_group = 5;
        break;
    }
    }
}

void DerivedAgentVarSymbol::create_auxiliary_symbols()
{
    // Create associated identity agentvar for expression "av1 == k1"
    switch (tok) {
    case token::TK_duration:
    case token::TK_weighted_duration:
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
        dav = DerivedAgentVarSymbol::create_symbol(agent, token::TK_undergone_entrance, *av1, k1);
        assert(dav);
        break;
    }
    case token::TK_value_at_first_exit:
    {
        assert(av1);
        assert(k1);
        dav = DerivedAgentVarSymbol::create_symbol(agent, token::TK_undergone_exit, *av1, k1);
        assert(dav);
        break;
    }
    case token::TK_value_at_first_transition:
    {
        assert(av1);
        assert(k1);
        assert(k2);
        dav = DerivedAgentVarSymbol::create_symbol(agent, token::TK_undergone_transition, *av1, k1, k2);
        assert(dav);
        break;
    }
    case token::TK_value_at_first_change:
    {
        assert(av1);
        dav = DerivedAgentVarSymbol::create_symbol(agent, token::TK_undergone_change, *av1);
        assert(dav);
        break;
    }
    case token::TK_completed_spell_duration:
    {
        assert(av1);
        assert(k1);
        dav = DerivedAgentVarSymbol::create_symbol(agent, token::TK_active_spell_duration, *av1, k1);
        assert(dav);
        break;
    }
    case token::TK_completed_spell_weighted_duration:
    {
        assert(av1);
        assert(k1);
        assert(av2); // weight
        dav = DerivedAgentVarSymbol::create_symbol(agent, token::TK_active_spell_weighted_duration, *av1, k1, *av2);
        assert(dav);
        break;
    }
    case token::TK_completed_spell_delta:
    {
        assert(av1);
        assert(k1);
        assert(av2); // observed
        dav = DerivedAgentVarSymbol::create_symbol(agent, token::TK_active_spell_delta, *av1, k1, *av2);
        assert(dav);
        break;
    }
    case token::TK_active_spell_delta:
    {
        // note that order of initialization of these statics is guaranteed by C++ standard
        static auto true_lit = new BooleanLiteral("true");
        static auto true_cnst = new ConstantSymbol(true_lit);

        assert(true_lit);
        assert(true_cnst);
        assert(av1);
        assert(k1);
        assert(av2); // observed
        assert(iav);
        // create derived agentvar to hold value of observed agentvar at beginning of spell
        dav = DerivedAgentVarSymbol::create_symbol(agent, token::TK_value_at_latest_entrance, iav, true_cnst, *av2);
        assert(dav);
        break;
    }
    default:
    break;
    }

    // Create the internal symbol for the event time of the self-scheduling derived agentvar.
    if (is_self_scheduling()) {
        // create the internal symbol for the event time of the self-scheduling derived agentvar
        ait = new AgentInternalSymbol("om_ss_time_" + name, agent, NumericSymbol::find(token::TK_Time), "time_infinite");
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
        assert(false); // A type for each kind of derived agentvar should have been assigned above.
    break;
    }

}

void DerivedAgentVarSymbol::assign_pp_members()
{
    // assign direct pointers for post-parse use
    if (av1) {
        pp_av1 = dynamic_cast<AgentVarSymbol *> (pp_symbol(av1));
        if (!pp_av1) {
            pp_error("Error - '" + (*av1)->name + "' is not an attribute of " + agent->name);
            throw HelperException("fatal error: stopping post parse processing");
        }
    }
    if (av2) {
        pp_av2 = dynamic_cast<AgentVarSymbol *> (pp_symbol(av2));
        if (!pp_av2) {
            pp_error("Error - '" + (*av2)->name + "' is not an attribute of " + agent->name);
            throw HelperException("fatal error: stopping post parse processing");
        }
    }
    if (prt) {
        pp_prt = dynamic_cast<PartitionSymbol *> (pp_symbol(prt));
        if (!pp_prt) {
            pp_error("Error - '" + (*prt)->name + "' is not a partition");
            throw HelperException("fatal error: stopping post parse processing");
        }
    }
    if (cls) {
        pp_cls = dynamic_cast<ClassificationSymbol *> (pp_symbol(cls));
        if (!pp_cls) {
            pp_error("Error - '" + (*cls)->name + "' is not a classification");
            throw HelperException("fatal error: stopping post parse processing");
        }
    }
}


void DerivedAgentVarSymbol::create_side_effects()
{
    // The local variable init_cxx is used to inject code into the function init_derived_attributes().
    // That function is called as part of the entity lifecycle, before model developer initialization code
    // for the entity, generally in a function named Start().
    // It is needed to handle situations where the initial value of a derived attribute is not zero.
    // For example, this can occur for aggregate() as well as for split().

    assert(pp_agent->initialize_derived_attributes_fn);
    CodeBlock& init_cxx = pp_agent->initialize_derived_attributes_fn->func_body;

    // The local variable reset_cxx is used to inject code into the function reset_derived_attributes().
    // That function is called after developer code in Start() to remove any side-effects which may have compromised
    // the initial value of a derived attribute.
    // For example, changes(attr) must record the number of changes in 'attr' as a result of events
    // in the simulation, and not side effects from assignment to 'attr' in developer code in Start() 
    // before the entity enters the simulation.  So value of changes(attr) needs to be reset to 0 after 
    // developer code in Start(), but before the entity enters the simulation.

    assert(pp_agent->reset_derived_attributes_fn);
    CodeBlock& reset_cxx = pp_agent->reset_derived_attributes_fn->func_body;

    switch (tok) {
    case token::TK_duration:
    {
        if (!av1) {
            // duration()
            // add side-effect to time
            auto *av = pp_agent->pp_time;
            assert(av);
            CodeBlock& c = av->side_effects_fn->func_body;
            c += injection_description();
            c += "// Advance time for " + pretty_name();
            c += "if (om_active) " + name + ".set(" + name + ".get() + om_delta);";
        }
        else {
            // duration(av, value)
            // add side-effect to time
            auto *av = pp_agent->pp_time;
            assert(av);
            assert(iav);
            CodeBlock& c = av->side_effects_fn->func_body;
            c += injection_description();
            c += "// Advance time for " + pretty_name();
            c += "if (om_active && " + iav->name + ") {";
            c += name + ".set(" + name + ".get() + om_delta);";
            c += "}";
        }
        break;
    }
    case token::TK_weighted_duration:
    {
        if (!av1) {
            // weighted_duration()
            // add side-effect to time
            auto *av = pp_agent->pp_time;
            auto *wgt = pp_av2;
            assert(av);
            assert(wgt);
            CodeBlock& c = av->side_effects_fn->func_body;
            c += injection_description();
            c += "// Advance time for " + pretty_name();
            c += "if (om_active) " + name + ".set(" + name + ".get() + om_delta * " + wgt->name + ".get());";
        }
        else {
            // weighted_duration(av, value)
            // add side-effect to time
            auto *av = pp_agent->pp_time;
            auto *wgt = pp_av2;
            assert(av);
            assert(iav);
            assert(wgt);
            CodeBlock& c = av->side_effects_fn->func_body;
            c += injection_description();
            c += "// Advance time for " + pretty_name();
            c += "if (om_active && " + iav->name + ") {";
            c += name + ".set(" + name + ".get() + om_delta * " + wgt->name + ".get());";
            c += "}";
        }
        break;
    }
    case token::TK_weighted_cumulation:
    {
        // TODO (or almost certainly not!)
        pp_warning("Warning - Not implemented (value never changes) - " + Symbol::token_to_string(tok) + "( ... )");
        break;
    }
    case token::TK_active_spell_duration:
    {
        // add side-effect to time
        auto *av = pp_agent->pp_time;
        assert(av);
        assert(iav);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Advance time for " + pretty_name();
        c += "if (om_active && " + iav->name + ") {";
        c += name + ".set(" + name + ".get() + om_delta);";
        c += "}";

        // add side-effect to identity agentvar (condition)
        CodeBlock& c2 = iav->side_effects_fn->func_body;
        c2 += injection_description();
        c2 += "if (om_new == false) {";
        c2 += "// Active spell is ending, set " + pretty_name() + " to zero.";
        c2 += name + ".set(0);";
        c2 += "}";
        break;
    }
    case token::TK_completed_spell_duration:
    {
        assert(iav);
        assert(dav); // active spell value
        // add side-effect to identity agentvar (condition)
        CodeBlock& c2 = iav->side_effects_fn->func_body;
        c2 += injection_description();
        c2 += "// If spell ending, move active spell value to " + pretty_name();
        c2 += "if (om_new == false) {";
        c2 += name + ".set(" + dav->name + ");";
        c2 += "}";
        break;
    }
    case token::TK_active_spell_weighted_duration:
    {
        // add side-effect to time
        auto *av = pp_agent->pp_time;
        auto *wgt = pp_av2;
        assert(av);
        assert(iav);
        assert(wgt);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Advance time for " + pretty_name();
        c += "if (om_active && " + iav->name + ") {";
        c += name + ".set(" + name + ".get() + om_delta * " + wgt->name + ".get());";
        c += "}";

        // add side-effect to identity agentvar (condition)
        CodeBlock& c2 = iav->side_effects_fn->func_body;
        c2 += injection_description();
        c2 += "// If spell ending, reset " + pretty_name();
        c2 += "if (om_new == false) {";
        c2 += name + ".set(0);";
        c2 += "}";
        break;
    }
    case token::TK_completed_spell_weighted_duration:
    {
        assert(iav);
        assert(dav); // active spell value
        // add side-effect to identity agentvar (condition)
        CodeBlock& c2 = iav->side_effects_fn->func_body;
        c2 += injection_description();
        c2 += "// If spell ending, move active spell value to " + pretty_name();
        c2 += "if (om_new == false) {";
        c2 += name + ".set(" + dav->name + ");";
        c2 += "}";
        break;
    }
    case token::TK_active_spell_delta:
    {
        // Cannot safely inject side-effect to the observed agentvar
        // since the observed agentvar might change and the spell condition go false in same event.
        // If the spell condition goes false before the observed agentvar changes value in the event
        // implementation, the active spell delta would be in error.
        // Instead, the side-effect is added to time, which is guaranteed to invoke side-effects before
        // the event is implemented.  But that doesn't handle all cases, since
        // some agentvars are updated when time advances, including possibly the spell condition.
        // So active_spell_delta is assigned a sorting_group which comes before identity agentvars.
        // OK, so much for background, now...
        // add side-effect to time
        auto *av = pp_agent->pp_time;
        assert(iav);
        assert(pp_av2); // observed agentvar
        assert(dav); // holds value of observed agentvar at beginning of spell
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain value for " + pretty_name();
        c += "if (om_active && " + iav->name + ") {";
        c += name + ".set(" + pp_av2->name + ".get() - " + dav->name + ".get());";
        c += "}";

        // add side-effect to identity agentvar (condition)
        CodeBlock& c2 = iav->side_effects_fn->func_body;
        c2 += injection_description();
        c2 += "// If spell ending, reset " + pretty_name();
        c2 += "if (om_new == false) {";
        c2 += name + ".set(0);";
        c2 += "}";
        break;
    }
    case token::TK_completed_spell_delta:
    {
        assert(iav);
        assert(dav); // active spell value
        // add side-effect to identity agentvar (condition)
        CodeBlock& c2 = iav->side_effects_fn->func_body;
        c2 += injection_description();
        c2 += "// If spell ending, move active spell value to " + pretty_name();
        c2 += "if (om_new == false) {";
        c2 += name + ".set(" + dav->name + ");";
        c2 += "}";
        break;
    }
    case token::TK_undergone_entrance:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        assert(av);
        assert(k1);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (!" + name + ".get() && om_new == " + k1->value() + ") {";
        c += name + ".set(true);";
        c += "}";
        break;
    }
    case token::TK_undergone_exit:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        assert(av);
        assert(k1);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (!" + name + ".get() && om_old == " + k1->value() + ") {";
        c += name + ".set(true);";
        c += "}";
        break;
    }
    case token::TK_undergone_transition:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        assert(av);
        assert(k1);
        assert(k2);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (!" + name + ".get() && om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        c += name + ".set(true);";
        c += "}";
        break;
    }
    case token::TK_undergone_change:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        assert(av);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (!" + name + ".get()) {";
        c += name + ".set(true);";
        c += "}";
        break;
    }
    case token::TK_entrances:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        assert(av);
        assert(k1);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (om_new == " + k1->value() + ") {";
        c += name + ".set(" + name + ".get() + 1);";
        c += "}";
        break;
    }
    case token::TK_exits:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        assert(av);
        assert(k1);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (om_old == " + k1->value() + ") {";
        c += name + ".set(" + name + ".get() + 1);";
        c += "}";
        break;
    }
    case token::TK_transitions:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        assert(av);
        assert(k1);
        assert(k2);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        c += name + ".set(" + name + ".get() + 1);";
        c += "}";
        break;
    }
    case token::TK_changes:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        assert(av);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += name + ".set(" + name + ".get() + 1);";
        break;
    }
    case token::TK_value_at_first_entrance:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        auto *noted = pp_av2;
        auto undergone = dav;
        assert(av);
        assert(k1);
        assert(noted);
        assert(undergone);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (!" + undergone->name + " && om_new == " + k1->value() + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        break;
    }
    case token::TK_value_at_latest_entrance:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(k1);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (om_new == " + k1->value() + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        break;
    }
    case token::TK_value_at_first_exit:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        auto *noted = pp_av2;
        auto undergone = dav;
        assert(av);
        assert(k1);
        assert(noted);
        assert(undergone);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (!" + undergone->name + " && om_old == " + k1->value() + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        break;
    }
    case token::TK_value_at_latest_exit:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(k1);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (om_old == " + k1->value() + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        break;
    }
    case token::TK_value_at_first_transition:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        auto *noted = pp_av2;
        auto undergone = dav;
        assert(av);
        assert(k1);
        assert(k2);
        assert(noted);
        assert(undergone);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (!" + undergone->name + " && om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        break;
    }
    case token::TK_value_at_latest_transition:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(k1);
        assert(k2);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        break;
    }
    case token::TK_value_at_first_change:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        auto *noted = pp_av2;
        auto undergone = dav;
        assert(av);
        assert(noted);
        assert(undergone);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (!" + undergone->name + ") {";
        c += name + ".set(" + noted->name + ".get());";
        c += "}";
        break;
    }
    case token::TK_value_at_latest_change:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += name + ".set(" + noted->name + ".get());";
        break;
    }
    case token::TK_value_at_entrances:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(k1);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (om_new == " + k1->value() + ") {";
        c += name + ".set(" + name + ".get() + " + noted->name + ".get());";
        c += "}";
        break;
    }
    case token::TK_value_at_exits:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(k1);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (om_old == " + k1->value() + ") {";
        c += name + ".set(" + name + ".get() + " + noted->name + ".get());";
        c += "}";
        break;
    }
    case token::TK_value_at_transitions:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(k1);
        assert(k2);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += "if (om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        c += name + ".set(" + name + ".get() + " + noted->name + ".get());";
        c += "}";
        break;
    }
    case token::TK_value_at_changes:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        auto *noted = pp_av2;
        assert(av);
        assert(noted);
        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += name + ".set(" + name + ".get() + " + noted->name + ".get());";
        break;
    }
    case token::TK_split:
    {
        auto *av = pp_av1;
        assert(av); // observed
        assert(pp_prt);

        init_cxx += injection_description();
        init_cxx += "// Set initial value based on dependent attribute";
        init_cxx += name + ".set(" + pp_prt->name + "::value_to_interval(" + av->name + "));";;

        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += name + ".set(" + pp_prt->name + "::value_to_interval(" + av->name + "));";
        break;
    }
    case token::TK_aggregate:
    {
        auto *av = pp_av1;
        assert(av); // observed
        assert(pp_cls);
        auto typ = av->pp_data_type;
        if (!typ->is_classification()) {
            pp_error("error - observed attribute '" + av->name + "' must be of type classification in " + pretty_name());
            break;
        }
        auto from = dynamic_cast<ClassificationSymbol *>(typ);
        assert(from); // logic guarantee
        auto to = pp_cls;
        // find the required aggregation
        string aggregation_name = AggregationSymbol::symbol_name(from, to);
        auto agg = dynamic_cast<AggregationSymbol *>(get_symbol(aggregation_name));
        if (!agg) {
            pp_error("error - required aggregation from '" + from->name + "' to '" + to->name + "' is missing for " + pretty_name());
            break;
        }

        init_cxx += injection_description();
        init_cxx += "// Set initial value based on dependent attribute";
        init_cxx += name + ".set(" + agg->name + ".find(" + av->name + ")->second);";

        CodeBlock& c = av->side_effects_fn->func_body;
        c += injection_description();
        c += "// Maintain " + pretty_name();
        c += name + ".set(" + agg->name + ".find(" + av->name + ")->second);";
        break;
    }
    case token::TK_trigger_entrances:
    case token::TK_trigger_exits:
    case token::TK_trigger_transitions:
    case token::TK_trigger_changes:
    case token::TK_duration_trigger:
    case token::TK_duration_counter:
    {
        auto *av = pp_av1; // the triggering attribute
        assert(av);
        assert(ait); // the previously-created agent internal symbol which holds the next time of occurrence of the self-scheduling agentvar

        {
            // Code injection into the om_reset_derived_attributes function.

            // This function runs after developer initialization just before the entity enters the simulation.
            // The local variable reset_cxx is declared at the beginning of this function.
            reset_cxx += injection_description();
            reset_cxx += "{";
            reset_cxx += "auto & ss_attr = " + name + ";";
            reset_cxx += "auto & ss_time = " + ait->name + ";";

            // attribute-specific code (begin)
            switch (tok) {

            case token::TK_trigger_entrances:
            case token::TK_trigger_exits:
            case token::TK_trigger_transitions:
            case token::TK_trigger_changes:
            case token::TK_duration_trigger:
            reset_cxx += "// There is no change in the triggering condition when the entity enters the simulation.";
            reset_cxx += "ss_time = time_infinite;";
            reset_cxx += "ss_attr.set(false);";
            break;

            case token::TK_duration_counter:
            reset_cxx += "// There is no change in the triggering condition when the entity enters the simulation.";
            reset_cxx += "ss_time = time_infinite;";
            reset_cxx += "ss_attr.set(0);";
            break;

            default:
            assert(false);
            }
            // attribute-specific code (end)

            reset_cxx += "}";
        }

        {
            // Code injection into the side-effect function of the triggering agentvar

            CodeBlock& cxx = av->side_effects_fn->func_body;
            assert(pp_agent->ss_event);
            cxx += injection_description();
            cxx += "if (om_active) {";
            cxx += "auto & ss_attr = " + name + ";";
            cxx += "auto & ss_time = " + ait->name + ";";
            cxx += "auto & ss_event = " + pp_agent->ss_event->name + ";";

            // attribute-specific code (begin)
            switch (tok) {

            case token::TK_trigger_entrances:
            cxx += "if (om_new == " + k1->value() + ") {";
            cxx += "// Activate trigger " + pretty_name();
            cxx += "ss_attr.set(true);";
            cxx += "// Set the self-scheduling event to reset the trigger immediately";
            cxx += "ss_time = time;";
            cxx += "// Mark the entity's self-scheduling event for recalculation";
            cxx += "ss_event.make_dirty();";
            cxx += "}";
            break;

            case token::TK_trigger_exits:
            cxx += "if (om_old == " + k1->value() + ") {";
            cxx += "// Activate trigger " + pretty_name();
            cxx += "ss_attr.set(true);";
            cxx += "// Set the self-scheduling event to reset the trigger immediately";
            cxx += "ss_time = time;";
            cxx += "// Mark the entity's self-scheduling event for recalculation";
            cxx += "ss_event.make_dirty();";
            cxx += "}";
            break;

            case token::TK_trigger_transitions:
            cxx += "if (om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
            cxx += "// Activate trigger " + pretty_name();
            cxx += "ss_attr.set(true);";
            cxx += "// Set the self-scheduling event to reset the trigger immediately";
            cxx += "ss_time = time;";
            cxx += "// Mark the entity's self-scheduling event for recalculation";
            cxx += "ss_event.make_dirty();";
            cxx += "}";
            break;

            case token::TK_trigger_changes:
            cxx += "{";
            cxx += "// Activate trigger " + pretty_name();
            cxx += "ss_attr.set(true);";
            cxx += "// Set the self-scheduling event to reset the trigger immediately";
            cxx += "ss_time = time;";
            cxx += "// Mark the entity's self-scheduling event for recalculation";
            cxx += "ss_event.make_dirty();";
            cxx += "}";
            break;

            case token::TK_duration_trigger:
            cxx += "if (om_new == " + k1->value() + ") {";
            cxx += "// Start the timer for " + pretty_name();
            cxx += "ss_time = time + " + k2->value() + ";";
            cxx += "// Mark the entity's self-scheduling event for recalculation";
            cxx += "ss_event.make_dirty();";
            cxx += "}";
            cxx += "else if (om_old == " + k1->value() + ") {";
            cxx += "// The condition is no longer satisfied for " + pretty_name();
            cxx += "ss_attr.set(false);";
            cxx += "ss_time = time_infinite;";
            cxx += "// Mark the entity's self-scheduling event for recalculation";
            cxx += "ss_event.make_dirty();";
            cxx += "}";
            break;

            case token::TK_duration_counter:
            cxx += "if (om_new == " + k1->value() + ") {";
            cxx += "// Start the timer for " + pretty_name();
            cxx += "ss_time = time + " + k2->value() + ";";
            cxx += "// Mark the entity's self-scheduling event for recalculation";
            cxx += "ss_event.make_dirty();";
            cxx += "}";
            cxx += "else if (om_old == " + k1->value() + ") {";
            cxx += "// The condition is no longer satisfied for " + pretty_name();
            cxx += "ss_attr.set(0);";
            cxx += "ss_time = time_infinite;";
            cxx += "// Mark the entity's self-scheduling event for recalculation";
            cxx += "ss_event.make_dirty();";
            cxx += "}";
            break;

            default:
            assert(false);
            }
            // attribute-specific code (end)

            cxx += "}";
        }

        {
            // Code injection into the implement function of self-scheduling events

            assert(pp_agent->ss_implement_fn); // the implement function of the event which manages all self-scheduling agentvars in the agent
            CodeBlock& cxx = pp_agent->ss_implement_fn->func_body; // body of the C++ event implement function of the self-scheduling event
            cxx += injection_description();
            cxx += "{";
            cxx += "// Maintain " + pretty_name();
            cxx += "auto & ss_attr = " + name + ";";
            cxx += "auto & ss_time = " + ait->name + ";";
            cxx += "if (current_time == ss_time) {";

            // attribute-specific code (begin)
            switch (tok) {

            case token::TK_trigger_entrances:
            case token::TK_trigger_exits:
            case token::TK_trigger_transitions:
            case token::TK_trigger_changes:
            cxx += "ss_attr.set(false);";
            cxx += "ss_time = time_infinite;";
            break;

            case token::TK_duration_trigger:
            cxx += "ss_attr.set(true);";
            cxx += "ss_time = time_infinite;";
            break;

            case token::TK_duration_counter:
            if (k3) {
                // variant with counter limit
                cxx += "ss_attr.set(1 + ss_attr.get());";
                cxx += "if (ss_attr.get() < " + k3->value() + ") {";
                cxx += "ss_time = time + " + k2->value() + ";";
                cxx += "}";
                cxx += "else {";
                cxx += "ss_time = time_infinite;";
                cxx += "}";
            }
            else {
                // variant with no counter limit
                cxx += "ss_attr.set(1 + ss_attr.get());";
                cxx += "ss_time = time + " + k2->value() + ";";
            }
            break;

            default:
            assert(false);
            }
            // attribute-specific code (end)

            if (Symbol::option_event_trace) {
                cxx += "// Dump event time information to trace log";
                string evt_name = "scheduled - " + to_string(numeric_id);
                cxx += "if (BaseEvent::trace_event_on) "
                    "BaseEvent::event_trace_msg("
                    "\"" + agent->name + "\", "
                    "(int)entity_id, "
                    "GetCaseSeed(), "
                    "\"" + evt_name + "\", "
                    " (double) time);"
                    ;
            }
            cxx += "}";  // end of the block started at "if (current_time == ..."
            cxx += "}";  // end of the block for this code injection
        }

        {
            // Code injection into the time function of self-scheduling events

            assert(pp_agent->ss_time_fn); // the time function of the event which manages all self-scheduling agentvars in the agent
            CodeBlock& cxx = pp_agent->ss_time_fn->func_body; // body of the C++ event time function of the self-scheduling event
            // The generated code minimizes the working variable 'et' with the next time of this self-scheduling agentvar
            // The required 'return et;' statement in the body of the event time function is generated elsewhere
            // after all code injection to this function is complete.
            cxx += injection_description();
            cxx += "{";
            cxx += "// Check scheduled time for " + pretty_name();
            cxx += "auto & ss_time = " + ait->name + ";";
            cxx += "if (ss_time < et) et = ss_time;";
            cxx += "}";
        }

        break;
    }
    case token::TK_self_scheduling_int:
    case token::TK_self_scheduling_split:
    {
        // Common variables and code for all self-scheduling agentvars
        auto *av = pp_av1;
        assert(av); // the agentvar being integerized or split, e.g. "age", "time", etc.
        assert(ait); // the previously-created agent internal symbol which holds the next time of occurrence of the self-scheduling agentvar
        assert(pp_agent->ss_event); // the event for self-scheduling attributes
        assert(pp_agent->ss_time_fn); // the time function of the event which manages all self-scheduling agentvars in the agent
        assert(pp_agent->ss_implement_fn); // the implement function of the event which manages all self-scheduling agentvars in the agent
        CodeBlock& ctf = pp_agent->ss_time_fn->func_body; // body of the C++ event time function of the self-scheduling event
        CodeBlock& cif = pp_agent->ss_implement_fn->func_body; // body of the C++ event implement function of the self-scheduling event

        // The generated code minimizes the working variable 'et' with the next time of this self-scheduling agentvar
        // The required 'return et;' statement in the body of the event time function is generated elsewhere
        // after all code injection to this function is complete.
        ctf += injection_description();
        // Inject code to the event time function which minimizes the working variable 'et' with the next time of this self-scheduling agentvar
        ctf += "{";
        ctf += "// Check scheduled time for " + pretty_name();
        ctf += "auto & ss_time = " + ait->name + ";";
        ctf += "if (ss_time < et) et = ss_time;";
        ctf += "}";

        // Code for specific variants of self_scheduling_int() and self_scheduling_split() agentvars
        if (av->name == "age" || av->name == "time") {

            // Code injection: age/time side-effects, for use in Start
            // get the side-effect call-back function for age / time
            assert(av->side_effects_fn);
            CodeBlock& cse = av->side_effects_fn->func_body; // body of the C++ side-effect function for the argument attribute (e.g. "age")
            // For self_scheduling_int(age), the side-effects for initial assignment need to be injected into
            // both age and time, in case time is assigned after age.  So we build the side effect code block separately
            // into a local CodeBlock named blk, and then inject it after it's build in the right place(s).
            // Note that if the block is for 'age', the code needs to use age.get() rather than om_new, since
            // it will be injected into the side-effect function for time as well as age.  In the side-effect function for time
            // om_new is the new value of time, not age, which is why we need to use age.get() instead in the following code.
            CodeBlock blk;
            blk += injection_description();
            blk += "if (!om_active) {";
            blk += "// Initial values are being assigned.";
            blk += "auto & ss_attr = " + name + ";";
            blk += "auto & ss_time = " + ait->name + ";";
            if (tok == token::TK_self_scheduling_split) {
                blk += "auto part = ss_attr.get(); // working copy of partition";
            }

            // There are 4 distinct cases to handle
            if (tok == token::TK_self_scheduling_int && av->name == "age") {
                blk += "// Initial value is the integer part of age.";
                blk += "ss_attr.set((int)age.get());";
                blk += "// Time to wait for next change is the fraction of time remaining to next integer boundary";
                blk += "ss_time = time + (1 - (age.get() - (int)age.get()));";
            }
            else if (tok == token::TK_self_scheduling_int && av->name == "time") {
                blk += "// Initial value is the integer part of time.";
                blk += "ss_attr.set((int)om_new);";
                blk += "// Time of next change is next integer time";
                blk += "ss_time = 1 + (int)om_new;";
            }
            else if (tok == token::TK_self_scheduling_split && av->name == "age") {
                blk += "// Initial value is the corresponding partition interval.";
                blk += "part.set_from_value(age.get());";
                blk += "ss_attr.set(part);";
                blk += "// Time to wait for next change is the remaining time to get to upper bound of current interval in partition.";
                blk += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                blk += "else ss_time = time + (part.upper() - age.get());";
            }
            else if (tok == token::TK_self_scheduling_split && av->name == "time") {
                blk += "// Initial value is the corresponding partition interval.";
                blk += "part.set_from_value(om_new);";
                blk += "ss_attr.set(part);";
                blk += "// Time to wait for next change is the remaining time to get to upper bound of current interval in partition.";
                blk += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                blk += "else ss_time = time + (part.upper() - om_new);";
            }
            else {
                assert(false);
            }
            blk += "}";
            // inject the side-effect block for self_scheduling_QQQ(time) into time, and self_scheduling_QQQ(age) into age
            cse += blk;
            // For self_scheduling_QQQ(age), inject the side-effect block into time as well.
            if (av->name == "age") {
                // find the attribute for time
                auto attr_time = av->pp_agent->pp_time;
                assert(attr_time);
                // get the body of the C++ side-effect function for "time"
                CodeBlock& cse_time = attr_time->side_effects_fn->func_body; 
                // inject the fragment into time
                cse_time += blk;
            }

            // Code injection: self-scheduling event implement function
            cif += injection_description();
            cif += "if (current_time == " + ait->name + ") {";
            cif += "auto & ss_attr = " + name + ";";
            cif += "auto & ss_time = " + ait->name + ";";
            if (tok == token::TK_self_scheduling_split) {
                cif += "auto part = ss_attr.get(); // working copy of partition";
            }
            cif += "";

            // There are 2 distinct cases to handle
            if (tok == token::TK_self_scheduling_int) {
                cif += "// Update the value";
                cif += "ss_attr.set(ss_attr.get() + 1);";
                cif += "// Update the time of next change";
                cif += "ss_time += 1;";
            }
            else if (tok == token::TK_self_scheduling_split) {
                cif += "// Update the value";
                cif += "part++; // Advance to the next interval in the partition.";
                cif += "ss_attr.set(part);";
                cif += "// Update the time of next change by the width of that new interval.";
                cif += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                cif += "else ss_time += part.width();";
            }
            else {
                assert(false);
            }

            if (Symbol::option_event_trace) {
                cif += "// Dump event time information to trace log";
                string evt_name = "scheduled - " + to_string(numeric_id);
                cif += "if (BaseEvent::trace_event_on) "
                    "BaseEvent::event_trace_msg("
                    "\"" + agent->name + "\", "
                    "(int)entity_id, "
                    "GetCaseSeed(), "
                    "\"" + evt_name + "\", "
                    " (double) time);"
                    ;
            }
            cif += "}";
        }
        else {
            // The argument of self_scheduling_XXX is a derived attribute, either active_spell_duration(attr,val), duration(), or duration(attr,val)
            auto dav = dynamic_cast<DerivedAgentVarSymbol *>(pp_av1);
            assert(dav); // the argument, e.g. active_spell_duration
            // TODO raise syntax error if following assert is not true
            assert(dav->tok == token::TK_active_spell_duration || dav->tok == token::TK_duration);

            // Inject code into the om_reset_derived_attributes function
            reset_cxx += injection_description();
            reset_cxx += "{";
            reset_cxx += "auto & ss_attr = " + name + ";";
            reset_cxx += "auto & ss_time = " + ait->name + ";";
            if (dav->tok == token::TK_duration && dav->iav) {
                reset_cxx += "auto & ss_cond = " + dav->iav->name + ";";
            }
            if (tok == token::TK_self_scheduling_split) {
                reset_cxx += "auto part = ss_attr.get(); // working copy of partition";
            }

            // There are 6 distinct cases to handle
            if (tok == token::TK_self_scheduling_int && dav->tok == token::TK_active_spell_duration) {
                reset_cxx += "// No spells are active at simulation entry";
                reset_cxx += "ss_time = time_infinite;";
                reset_cxx += "// Set the integerized duration to zero.";
                reset_cxx += "ss_attr.set(0);";
            }
            else if (tok == token::TK_self_scheduling_int && dav->tok == token::TK_duration && dav->iav) {
                reset_cxx += "// Initialize the integerized duration to zero.";
                reset_cxx += "ss_attr.set(0);";
                // condition is true
                reset_cxx += "if (ss_cond) {";
                reset_cxx += "// Condition is true at simulation entry";
                reset_cxx += "// The time to wait is one interval of time from the current time.";
                reset_cxx += "ss_time = time + 1;";
                reset_cxx += "}";
                // condition is false
                reset_cxx += "else {";
                reset_cxx += "// Condition is false at simulation entry";
                reset_cxx += "// There is no next change scheduled.";
                reset_cxx += "ss_time = time_infinite;";
                reset_cxx += "}";
            }
            else if (tok == token::TK_self_scheduling_int && dav->tok == token::TK_duration && !dav->iav) {
                reset_cxx += "// Initialize the integerized duration to zero.";
                reset_cxx += "ss_attr.set(0);";
                reset_cxx += "// The time to wait is one interval of time from the current time.";
                reset_cxx += "ss_time = time + 1;";
            }
            else if (tok == token::TK_self_scheduling_split && dav->tok == token::TK_active_spell_duration) {
                reset_cxx += "// No spells are active at simulation entry";
                reset_cxx += "ss_time = time_infinite;";
                reset_cxx += "// Set the partitioned duration to the interval containing zero.";
                reset_cxx += "part.set_from_value(0);";
                reset_cxx += "ss_attr.set(part);";
            }
            else if (tok == token::TK_self_scheduling_split && dav->tok == token::TK_duration && dav->iav) {
                reset_cxx += "// Set the partitioned duration to the interval containing zero.";
                reset_cxx += "part.set_from_value(0);";
                reset_cxx += "ss_attr.set(part);";
                // condition is true
                reset_cxx += "if (ss_cond) {";
                reset_cxx += "// Condition is true at simulation entry";
                reset_cxx += "// The time to wait is the upper bound of the current interval in the partition.";
                reset_cxx += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                reset_cxx += "else ss_time = time + part.upper();";
                reset_cxx += "}";
                // condition is false
                reset_cxx += "else {";
                reset_cxx += "// Condition is false at simulation entry";
                reset_cxx += "// There is no next change scheduled.";
                reset_cxx += "ss_time = time_infinite;";
                reset_cxx += "}";
            }
            else if (tok == token::TK_self_scheduling_split && dav->tok == token::TK_duration && !dav->iav) {
                reset_cxx += "// Set the partitioned duration to the interval containing zero.";
                reset_cxx += "part.set_from_value(0);";
                reset_cxx += "ss_attr.set(part);";
                reset_cxx += "// The time to wait is the upper bound of the current interval in the partition.";
                reset_cxx += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                reset_cxx += "else ss_time = time + part.upper();";
            }
            else {
                assert(false);
            }
            reset_cxx += "}";

            // Inject code into the spell condition side-effects function.
            if (dav->iav) {
                // We're here except if a self_scheduling of unconditioned duration
                assert(dav->iav); // the identity attribute which holds the spell condition
                assert(dav->iav->side_effects_fn); // the side-effects function of the identity attribute which holds the spell condition
                CodeBlock& cse = dav->iav->side_effects_fn->func_body; // the function body of that side-effects function
 
                // Inject code into the spell condition side-effects function.
                cse += injection_description();
                cse += "if (om_active) {";
                cse += "auto & ss_attr = " + name + ";";
                cse += "auto & ss_time = " + ait->name + ";";
                cse += "auto & ss_event = " + pp_agent->ss_event->name + ";";
                if (dav->tok == token::TK_duration) {
                    cse += "auto & ss_dur = " + dav->name + ";";
                }
                if (tok == token::TK_self_scheduling_split) {
                    cse += "auto part = ss_attr.get(); // working copy of partition";
                }
                cse += "// The self-scheduling event will require recalculation";
                cse += "ss_event.make_dirty();";
                cse += "";

                // There are 4 distinct cases to handle
                if (tok == token::TK_self_scheduling_int && dav->tok == token::TK_active_spell_duration) {
                    // spell start
                    cse += "if (om_new == true) {";
                    cse += "// Active spell is starting, initialize self-scheduling attribute";
                    cse += "// Set the integerized duration to zero.";
                    cse += "ss_attr.set(0);";
                    cse += "// The time to wait is one interval of time from current time.";
                    cse += "ss_time = time + 1;";
                    cse += "}";
                    // spell end
                    cse += "else { // om_new == false";
                    cse += "// The active spell is ending, so reset self-scheduling attribute.";
                    cse += "// Set the integerized duration to zero.";
                    cse += "ss_attr.set(0);";
                    cse += "// There is no next change scheduled.";
                    cse += "ss_time = time_infinite;";
                    cse += "}";
                }
                else if (tok == token::TK_self_scheduling_int && dav->tok == token::TK_duration) {
                    // spell start
                    cse += "if (om_new == true) {";
                    cse += "// Spell is starting.";
                    cse += "// The time to wait is the time remaining to the next integer boundary of duration.";
                    cse += "ss_time = time + (1 - (ss_dur - (int)ss_dur));";
                    cse += "}";
                    // spell end
                    cse += "else {";
                    cse += "// Spell is ending.";
                    cse += "// There is no next change scheduled.";
                    cse += "ss_time = time_infinite;";
                    cse += "}";
                }
                else if (tok == token::TK_self_scheduling_split && dav->tok == token::TK_active_spell_duration) {
                    // spell start
                    cse += "if (om_new == true) {";
                    cse += "// Active spell is starting, initialize self-scheduling attribute";
                    cse += "// Set the partitioned duration to the interval containing zero.";
                    cse += "part.set_from_value(0);";
                    cse += "ss_attr.set(part);";
                    cse += "// The time to wait is the upper bound of the current interval in the partition.";
                    cse += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                    cse += "else ss_time = time + part.upper();";
                    cse += "}";
                    // spell end
                    cse += "else {";
                    cse += "// The active spell is ending, so reset self-scheduling attribute.";
                    cse += "// Set the partitioned duration to the interval containing zero.";
                    cse += "part.set_from_value(0);";
                    cse += "ss_attr.set(part);";
                    cse += "// There is no next change scheduled.";
                    cse += "ss_time = time_infinite;";
                    cse += "}";
                }
                else if (tok == token::TK_self_scheduling_split && dav->tok == token::TK_duration) {
                    // spell start
                    cse += "if (om_new == true) {";
                    cse += "// Spell is starting";
                    cse += "// Time to wait for next change is the remaining time to get to upper bound of current interval in partition.";
                    cse += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                    cse += "else ss_time = time + part.upper() - ss_dur;";
                    cse += "}";
                    // spell end
                    cse += "else {";
                    cse += "// Spell is ending.";
                    cse += "// There is no next change scheduled.";
                    cse += "ss_time = time_infinite;";
                    cse += "}";
                }
                else {
                    assert(false);
                }
                cse += "} // if (om_active)";
            }

            // Code injection: self-scheduling event implement function
            cif += injection_description();
            cif += "if (current_time == " + ait->name + ") {";
            cif += "auto & ss_attr = " + name + ";";
            cif += "auto & ss_time = " + ait->name + ";";
            if (tok == token::TK_self_scheduling_split) {
                cif += "auto part = ss_attr.get(); // working copy of partition";
            }
            cif += "";

            // There are 2 distinct cases to handle
            if (tok == token::TK_self_scheduling_int) {
                cif += "// Increment the integerized duration.";
                cif += "ss_attr.set(ss_attr.get() + 1);";
                cif += "// The time to wait is one interval of time.";
                cif += "ss_time = time + 1;";
            }
            else if (tok == token::TK_self_scheduling_split) {
                cif += "// Set the partitioned duration to the next interval.";
                cif += "part++;";
                cif += "ss_attr.set(part);";
                cif += "// The time to wait is the width of the current interval in the partition.";
                cif += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                cif += "else ss_time = time + part.width();";
            }
            else {
                assert(false);
            }

            if (Symbol::option_event_trace) {
                cif += "// Dump event time information to trace log";
                string evt_name = "scheduled - " + to_string(numeric_id);
                cif += "if (BaseEvent::trace_event_on) "
                    "BaseEvent::event_trace_msg("
                    "\"" + agent->name + "\", "
                    "(int)entity_id, "
                    "GetCaseSeed(), "
                    "\"" + evt_name + "\", "
                    " (double) time);"
                    ;
            }
            cif += "}";
        }
        break;
    }

    default:
    break;
    }
}

string DerivedAgentVarSymbol::pretty_name() const
{
    string result;

    switch (tok) {
    case token::TK_duration:
    {
        if (!pp_av1) {
            result = token_to_string(tok) + "()";
        }
        else {
            result = token_to_string(tok) + "(" + pp_av1->pretty_name() + ", " + k1->value() + ")";
        }
        break;
    }
    case token::TK_weighted_duration:
    {
        assert(pp_av2);
        if (!pp_av1) {
            result = token_to_string(tok) + "(" + pp_av2->pretty_name() + ")";
        }
        else {
            result = token_to_string(tok) + "(" + pp_av1->pretty_name() + ", " + k1->value() + ", " + pp_av2->pretty_name() + ")";
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
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + ", " + k1->value() + ")";
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
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + ", " + k1->value() + ", " + pp_av2->pretty_name() + ")";
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
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + ", " + k1->value() + ", " + k2->value() + ")";
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
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + ", " + k1->value() + ", " + k2->value() + ", " + pp_av2->pretty_name() + ")";
        break;
    }
    case token::TK_undergone_change:
    case token::TK_changes:
    case token::TK_trigger_changes:
    {
        assert(pp_av1);
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + ")";
        break;
    }
    case token::TK_weighted_cumulation:
    case token::TK_value_at_first_change:
    case token::TK_value_at_latest_change:
    case token::TK_value_at_changes:
    {
        assert(pp_av1);
        assert(pp_av2);
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + ", " + pp_av2->pretty_name() + ")";
        break;
    }
    case token::TK_split:
    case token::TK_self_scheduling_split:
    {
        assert(pp_av1);
        assert(pp_prt);
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + ", " + pp_prt->name + ")";
        break;
    }
    case token::TK_aggregate:
    {
        assert(pp_av1);
        assert(pp_cls);
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + ", " + pp_cls->name + ")";
        break;
    }
    case token::TK_duration_counter:
    {
        assert(pp_av1);
        assert(k1);
        assert(k2);
        assert(k3 || !k3); // optional
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + ", " + k1->value() + ", " + k2->value() + (k3 ? (", " + k3->value()) : "") + ")";
        break;
    }
    case token::TK_self_scheduling_int:
    {
        assert(pp_av1);
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + ")";
        break;
    }
    default:
    {
        //TODO raise error once list is complete
        // assert(false);
        result = token_to_string(tok) + "( ... )";
        break;
    }

    }
    return result;
}

bool DerivedAgentVarSymbol::is_self_scheduling() const
{
    switch (tok) {
    case token::TK_trigger_entrances:
    case token::TK_trigger_exits:
    case token::TK_trigger_transitions:
    case token::TK_trigger_changes:
    case token::TK_duration_counter:
    case token::TK_duration_trigger:
    case token::TK_self_scheduling_int:
    case token::TK_self_scheduling_split:
    {
        return true;
        break;
    }
    default:
    {
        return false;
        break;
    }
    }
}

string DerivedAgentVarSymbol::pp_modgen_name() const
{
    if (!is_self_scheduling()) return unique_name;

    // Construct a string which has the same sorting characteristics as Modgen
    // names for self-scheduling attributes.  That ensures that the same numbering
    // for self-scheduling attributes is used in event trace outputs in Modgen and ompp.

    string result = pp_agent->name + "::";
    switch (tok) {
    case token::TK_trigger_entrances:
    {
        assert(pp_av1);
        assert(k1);
        result += "trigger_entrances_" + pp_av1->name + "_" + k1->value_as_name();
        break;
    }
    case token::TK_trigger_exits:
    {
        assert(pp_av1);
        assert(k1);
        result += "trigger_exits_" + pp_av1->name + "_" + k1->value_as_name();
        break;
    }
    case token::TK_trigger_transitions:
    {
        assert(pp_av1);
        assert(k1);
        assert(k2);
        result += "trigger_transitions_" + pp_av1->name + "_" + k1->value_as_name() + "_" + k2->value_as_name();
        break;
    }
    case token::TK_trigger_changes:
    {
        assert(pp_av1);
        result += "trigger_changes_" + pp_av1->name;
        break;
    }
    case token::TK_duration_counter:
    {
        assert(pp_av1);
        assert(k1);
        assert(k2);
        result += "duration_counter_" + pp_av1->name + "_" + k1->value_as_name() + "_" + k2->value_as_name();
        if (k3) result += "_" + k3->value_as_name();
        break;
    }
    case token::TK_duration_trigger:
    {
        assert(pp_av1);
        assert(k1);
        assert(k2);
        result += "duration_trigger_" + pp_av1->name + "_" + k1->value_as_name() + "_" + k2->value_as_name();
        break;
    }
    case token::TK_self_scheduling_int:
    {
        result += "ssint_";
        assert(pp_av1);
        if (pp_av1->name == "age" || pp_av1->name == "time") {
            result += pp_av1->name;
        }
        else {
            auto dav = dynamic_cast<DerivedAgentVarSymbol *>(pp_av1);
            assert(dav);
            result += token_to_string(dav->tok); // e.g. active_spell_duration
            result += "_";
            if (dav->pp_av1) result += dav->pp_av1->name;
            if (dav->k1) result += "_" + dav->k1->value();
        }
        break;
    }
    case token::TK_self_scheduling_split:
    {
        result += "sssplit_";
        assert(pp_av1);
        if (pp_av1->name == "age" || pp_av1->name == "time") {
            result += pp_av1->name;
        }
        else {
            auto dav = dynamic_cast<DerivedAgentVarSymbol *>(pp_av1);
            assert(dav);
            result += token_to_string(dav->tok); // e.g. active_spell_duration
            result += "_";
            if (dav->pp_av1) result += dav->pp_av1->name;
            if (dav->k1) result += "_" + dav->k1->value();
        }
        assert(pp_prt);
        result += "_" + pp_prt->name;
        break;
    }
    default:
    assert(false); // logic guarantee
    }

    // convert to lowercase to match MFC string comparison CompareNoCase.
    // in stl, '_' lies between lower and upper case, but is before any letter in CompareNoCase
    // which transforms letters to lowercase before performing the comparison using ASCII values.
    for (auto & ch : result) {
        ch = tolower(ch);
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
        if (is_self_scheduling()) {
            // create the event to support self-scheduling states in the containing agent
            auto agnt = dynamic_cast<AgentSymbol *>(agent);
            assert(agnt);
            agnt->create_ss_event();
        }
        break;
    }
    case eAssignMembers:
    {
        assign_pp_members();
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
        + "&om_name_" + name + ", "
        + "&" + side_effects_fn->unique_name + ", "
        + (!side_effects_fn->empty() ? "true" : "false") + ", "
        + "&" + notify_fn->unique_name + ", "
        + (!notify_fn->empty() ? "true" : "false")
        + ">";
    h += name + ";";

    return h;
}


