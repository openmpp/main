/**
* @file    DerivedAttributeSymbol.cpp
* Definitions for the DerivedAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

//#define DEPENDENCY_TEST 1

#include <cassert>
#include "libopenm/omError.h"
#include "DerivedAttributeSymbol.h"
#include "EntitySymbol.h"
#include "AttributeSymbol.h"
#include "ConstantSymbol.h"
#include "PartitionSymbol.h"
#include "ClassificationSymbol.h"
#include "IdentityAttributeSymbol.h"
#include "BuiltinAttributeSymbol.h"
#include "EntityInternalSymbol.h"
#include "EntityEventSymbol.h"
#include "TimeSymbol.h"
#include "RealSymbol.h"
#include "BoolSymbol.h"
#include "AggregationSymbol.h"
#include "ExprForAttribute.h"
#include "Literal.h"
#include "CodeBlock.h"

using namespace std;

// static
string DerivedAttributeSymbol::member_name(token_type tok,
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
DerivedAttributeSymbol * DerivedAttributeSymbol::create_symbol(const Symbol* agent,
                                              token_type tok,
                                              const Symbol *av1,
                                              const Symbol *av2,
                                              const Symbol *prt,
                                              const Symbol *cls,
                                              const ConstantSymbol *k1,
                                              const ConstantSymbol *k2,
                                              const ConstantSymbol *k3)
{
    DerivedAttributeSymbol *sym = nullptr;
    string mem_name = member_name(tok, av1, av2, prt, cls, k1, k2, k3);
    string nm = Symbol::symbol_name(mem_name, agent);
    auto it = symbols.find(nm);
    if (it != symbols.end()) {
        sym = dynamic_cast<DerivedAttributeSymbol *>(it->second);
        assert(sym);
    }
    else {
        sym = new DerivedAttributeSymbol(agent, tok, av1, av2, prt, cls, k1, k2, k3);
    }
    return sym;
}

void DerivedAttributeSymbol::validate()
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

void DerivedAttributeSymbol::assign_sorting_group()
{

    // The sorting_group affects the order of injection of side-effect code 
    // for related derived attributes with inter-dependencies (in a side-effect of the same attribute).
    // Code injection is ordered by sorting group, then lexicographically by unique_name.
    // 
    // The ordering requirements are as follows:
    // - continuously-updated durations occur first (side-effect of 'time')
    // - undergone_* (2) is used by value_at_first_* (1) in side-effect of condition
    // - active_spell_* (2) is used by completed_spell_* (1)in side-effect of condition
    // - active_spell_delta (2) occurs before identity condition (9) in side-effect of time

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
#if defined(DEPENDENCY_TEST)
    // Assign all derived attributes to sorting group 8
    // so that code_order resolves depdencies among them.
    sorting_group = 8;
#endif
}

void DerivedAttributeSymbol::create_auxiliary_symbols()
{
    // Create associated identity attribute for expression "av1 == k1"
    switch (tok) {
    case token::TK_duration:
    case token::TK_weighted_duration:
    {
        if (av1 && k1) {
            // is a conditioned duration
            iav = IdentityAttributeSymbol::create_equality_identity_attribute(agent, *av1, k1, decl_loc);
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
        iav = IdentityAttributeSymbol::create_equality_identity_attribute(agent, *av1, k1, decl_loc);
        assert(iav);
        break;
    }
    default:
    break;
    }

    // Create associated derived attribute
    switch (tok) {
    case token::TK_value_at_first_entrance:
    {
        assert(av1);
        assert(k1);
        dav = DerivedAttributeSymbol::create_symbol(agent, token::TK_undergone_entrance, *av1, k1);
        assert(dav);
        break;
    }
    case token::TK_value_at_first_exit:
    {
        assert(av1);
        assert(k1);
        dav = DerivedAttributeSymbol::create_symbol(agent, token::TK_undergone_exit, *av1, k1);
        assert(dav);
        break;
    }
    case token::TK_value_at_first_transition:
    {
        assert(av1);
        assert(k1);
        assert(k2);
        dav = DerivedAttributeSymbol::create_symbol(agent, token::TK_undergone_transition, *av1, k1, k2);
        assert(dav);
        break;
    }
    case token::TK_value_at_first_change:
    {
        assert(av1);
        dav = DerivedAttributeSymbol::create_symbol(agent, token::TK_undergone_change, *av1);
        assert(dav);
        break;
    }
    case token::TK_completed_spell_duration:
    {
        assert(av1);
        assert(k1);
        dav = DerivedAttributeSymbol::create_symbol(agent, token::TK_active_spell_duration, *av1, k1);
        assert(dav);
        break;
    }
    case token::TK_completed_spell_weighted_duration:
    {
        assert(av1);
        assert(k1);
        assert(av2); // weight
        dav = DerivedAttributeSymbol::create_symbol(agent, token::TK_active_spell_weighted_duration, *av1, k1, *av2);
        assert(dav);
        break;
    }
    case token::TK_completed_spell_delta:
    {
        assert(av1);
        assert(k1);
        assert(av2); // observed
        dav = DerivedAttributeSymbol::create_symbol(agent, token::TK_active_spell_delta, *av1, k1, *av2);
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
        // create derived attribute to hold value of observed attribute at beginning of spell
        dav = DerivedAttributeSymbol::create_symbol(agent, token::TK_value_at_latest_entrance, iav, true_cnst, *av2);
        assert(dav);
        break;
    }
    default:
    break;
    }

    // Create the internal symbol for the event time of the self-scheduling derived attribute.
    if (is_self_scheduling()) {
        // create the internal symbol for the event time of the self-scheduling derived attribute
        ait = new EntityInternalSymbol("om_ss_time_" + name, agent, NumericSymbol::find(token::TK_Time), "time_infinite");
    }
}


void DerivedAttributeSymbol::check_ss_arguments()
{
    // Some special conditions apply to first 'argument' of self_scheduling_int and self_scheduling_split
    if (tok == token::TK_self_scheduling_int || tok == token::TK_self_scheduling_split) {
         // av is the attribute being integerized or split, e.g. "age", "time", duration(), etc.
        auto *av = pp_av1;
        assert(av); // logic guarantee

        // first argument can be 'age' or 'time'
        if (av->name == "age" || av->name == "time") {
            return;
        }

        // first argument can be duration() or active_spell_duration()
        auto dav = dynamic_cast<DerivedAttributeSymbol *>(pp_av1);
        if (dav) {
            if (dav->tok == token::TK_active_spell_duration || dav->tok == token::TK_duration) {
                return;
            }
        }
        pp_fatal(LT("error: first argument of ") + token_to_string(tok) + LT(" must be age, time, duration, or active_spell_duration"));
    }
}

void DerivedAttributeSymbol::assign_data_type()
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
    {
        auto *sym = NumericSymbol::find(token::TK_counter);
        assert(sym);  // Initialization guarantee
        change_data_type(sym);
        break;
    }

    // type is integer
    case token::TK_self_scheduling_int:
    {
        // Note that self_scheduling_int(age) and self_scheduling_int(time) can be negative,
        // so the integer protean type is used.
        auto *sym = NumericSymbol::find(token::TK_integer);
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
        assert(false); // A type for each kind of derived attribute should have been assigned above.
    break;
    }

}

void DerivedAttributeSymbol::record_dependencies()
{
    // dependency on explicitly named attribute (first)
    if (pp_av1) {
        pp_dependent_attributes.emplace(pp_av1);
    }

    // dependency on explicitly named attribute (second)
    if (pp_av2) {
        pp_dependent_attributes.emplace(pp_av2);
    }

    // dependency on explicit condition (identity attribute)
    if (iav) {
        pp_dependent_attributes.emplace(iav);
    }

    // implicit dependencies on time and age
    switch (tok) {

    case token::TK_duration:
    case token::TK_weighted_duration:
    case token::TK_active_spell_duration:
    case token::TK_active_spell_weighted_duration:
    {
        // implicit dependency on time
        auto *sym = pp_agent->pp_time;
        assert(sym);
        pp_dependent_attributes.emplace(sym);
        break;
    }

    case token::TK_self_scheduling_int:
    case token::TK_self_scheduling_split:
    {
        // Dependency on time and age are already handled above through pp_av1.
        // which is time, age, durationl, or active_spell_duration
        // No need to go deeper into the composition of the duration or active_spell_duration,
        // (e.g the identity attribute holding a condition), since such dependencies
        // are already accounted for in the duration/active_spell_duration attribute itself.
        break;
    }

    case token::TK_value_at_first_entrance:
    case token::TK_value_at_first_exit:
    case token::TK_value_at_first_transition:
    case token::TK_value_at_first_change:
    {
        // These attributes make use of the corresponding undergone_* attribute
        // to record only the first change.  So, the undergone_* attribute
        // needs to be updated after the value_at* attribute
        // to record only the first change.
        assert(dav);
        dav->pp_dependent_attributes.emplace(this);
    }

    case token::TK_completed_spell_duration:
    case token::TK_completed_spell_weighted_duration:
    case token::TK_completed_spell_delta:
    {
        // These attributes make use of the corresponding active_spell_* attribute
        // to get the value before the spell becomes false.
        // So, the active_spell_* attribute
        // needs to be updated after the completed_spell_* attribute
        // to record its value before the spell becomse false.
        assert(dav);
        dav->pp_dependent_attributes.emplace(this);
    }

    case token::TK_weighted_cumulation:
    case token::TK_undergone_entrance:
    case token::TK_undergone_exit:
    case token::TK_undergone_transition:
    case token::TK_undergone_change:
    case token::TK_value_at_latest_entrance:
    case token::TK_value_at_latest_exit:
    case token::TK_value_at_latest_transition:
    case token::TK_value_at_latest_change:
    case token::TK_trigger_entrances:
    case token::TK_trigger_exits:
    case token::TK_trigger_transitions:
    case token::TK_trigger_changes:
    case token::TK_duration_trigger:
    case token::TK_entrances:
    case token::TK_exits:
    case token::TK_transitions:
    case token::TK_changes:
    case token::TK_duration_counter:
    case token::TK_active_spell_delta:
    case token::TK_value_at_entrances:
    case token::TK_value_at_exits:
    case token::TK_value_at_transitions:
    case token::TK_value_at_changes:
    case token::TK_split:
    case token::TK_aggregate:
    {
        // no implicit dependency
        break;
    }

    default:
        assert(false); // above cases should be exhaustive
    break;
    }


}

void DerivedAttributeSymbol::assign_pp_members()
{
    // assign direct pointers for post-parse use
    if (av1) {
        pp_av1 = dynamic_cast<AttributeSymbol *> (pp_symbol(av1));
        if (!pp_av1) {
            pp_error(LT("error : '") + (*av1)->name + LT("' is not an attribute of ") + agent->name);
            throw HelperException(LT("error : unsafe to continue, stopping post parse processing"));
        }
    }
    if (av2) {
        pp_av2 = dynamic_cast<AttributeSymbol *> (pp_symbol(av2));
        if (!pp_av2) {
            pp_error(LT("error : '") + (*av2)->name + LT("' is not an attribute of ") + agent->name);
            throw HelperException(LT("error : unsafe to continue, stopping post parse processing"));
        }
    }
    if (prt) {
        pp_prt = dynamic_cast<PartitionSymbol *> (pp_symbol(prt));
        if (!pp_prt) {
            pp_error(LT("error : '") + (*prt)->name + LT("' is not a partition"));
            throw HelperException(LT("error : unsafe to continue, stopping post parse processing"));
        }
    }
    if (cls) {
        pp_cls = dynamic_cast<ClassificationSymbol *> (pp_symbol(cls));
        if (!pp_cls) {
            pp_error(LT("error : '") + (*cls)->name + LT("' is not a classification"));
            throw HelperException(LT("error : unsafe to continue, stopping post parse processing"));
        }
    }
}


void DerivedAttributeSymbol::create_side_effects()
{
    // The local variable init_cxx is used to inject code into the function init_derived_attributes().
    // That function is called as part of the entity lifecycle, before model developer initialization code
    // for the entity, generally in a function named Start().
    // It is needed to handle situations where the initial value of a derived attribute is not zero.
    // For example, this can occur for aggregate() as well as for split().

    assert(pp_agent->initialize_derived_attributes_fn);
    CodeBlock& init_cxx = pp_agent->initialize_derived_attributes_fn->func_body;

    // The local variable reset_cxx is used to inject code into the function reset_derived_attributes().
    // That function is called before the entity enters the simulation (generally after developer code in Start),
    // to remove any side-effects which may have compromised the initial value of a derived attribute.
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
            CodeBlock& time_cxx = av->side_effects_fn->func_body;
            time_cxx += injection_description();

            time_cxx += "if (om_active) {";
            time_cxx += "// Advance time for " + pretty_name();
            time_cxx += name + ".set(" + name + ".get() + om_delta);";
            time_cxx += "}";
        }
        else {
            // duration(av, value)
            // add side-effect to time
            auto *av = pp_agent->pp_time;
            assert(av);
            assert(iav);
            CodeBlock& time_cxx = av->side_effects_fn->func_body;
            time_cxx += injection_description();
            time_cxx += "if (om_active) {";
            time_cxx += "if (" + iav->name + ") {";
            time_cxx += "// Advance time for " + pretty_name();
            time_cxx += name + ".set(" + name + ".get() + om_delta);";
            time_cxx += "}";
            time_cxx += "}";
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
            CodeBlock& time_cxx = av->side_effects_fn->func_body;
            time_cxx += injection_description();
            time_cxx += "// Advance time for " + pretty_name();
            time_cxx += "if (om_active) " + name + ".set(" + name + ".get() + om_delta * " + wgt->name + ".get());";
        }
        else {
            // weighted_duration(av, value)
            // add side-effect to time
            auto *av = pp_agent->pp_time;
            auto *wgt = pp_av2;
            assert(av);
            assert(iav);
            assert(wgt);
            CodeBlock& time_cxx = av->side_effects_fn->func_body;
            time_cxx += injection_description();
            time_cxx += "if (om_active) {";
            time_cxx += "if (" + iav->name + ") {";
            time_cxx += "// Advance time for " + pretty_name();
            time_cxx += name + ".set(" + name + ".get() + om_delta * " + wgt->name + ".get());";
            time_cxx += "}";
            time_cxx += "}";
        }
        break;
    }
    case token::TK_weighted_cumulation:
    {
        // weighted_cumulation(av, wgt)
        auto *av = pp_av1;
        auto *wgt = pp_av2;
        assert(av);
        assert(wgt);
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "auto delta = om_new - om_old;";
        observed_cxx += "auto wgt = " + wgt->name + ".get();";
        observed_cxx += "auto curr = " + name + ".get();";
        observed_cxx += "auto next = curr + (real) delta * wgt;";
        observed_cxx += "";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += name + ".set(next);";
        observed_cxx += "}";

        break;
    }
    case token::TK_active_spell_duration:
    {
        // add side-effect to time
        auto *av = pp_agent->pp_time;
        assert(av);
        assert(iav);
        CodeBlock& time_cxx = av->side_effects_fn->func_body;
        time_cxx += injection_description();
        time_cxx += "if (om_active) {";
        time_cxx += "if (" + iav->name + ") {";
        time_cxx += "// Advance time for " + pretty_name();
        time_cxx += name + ".set(" + name + ".get() + om_delta);";
        time_cxx += "}";
        time_cxx += "}";

        // add side-effect to identity attribute (condition)
        CodeBlock& identity_cxx = iav->side_effects_fn->func_body;
        identity_cxx += injection_description();
        identity_cxx += "if (om_new == false) {";
        identity_cxx += "// Active spell is ending, set " + pretty_name() + " to zero.";
        identity_cxx += name + ".set(0);";
        identity_cxx += "}";
        break;
    }
    case token::TK_completed_spell_duration:
    {
        assert(iav);
        assert(dav); // active spell value
        // add side-effect to identity attribute (condition)
        CodeBlock& identity_cxx = iav->side_effects_fn->func_body;
        identity_cxx += injection_description();
        identity_cxx += "if (om_new == false) {";
        identity_cxx += "// Spell ending, move active spell value to " + pretty_name();
        identity_cxx += name + ".set(" + dav->name + ");";
        identity_cxx += "}";
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
        CodeBlock& time_cxx = av->side_effects_fn->func_body;
        time_cxx += injection_description();
        time_cxx += "if (om_active) {";
        time_cxx += "if (" + iav->name + ") {";
        time_cxx += "// Advance time for " + pretty_name();
        time_cxx += name + ".set(" + name + ".get() + om_delta * " + wgt->name + ".get());";
        time_cxx += "}";
        time_cxx += "}";

        // add side-effect to identity attribute (condition)
        CodeBlock& identity_cxx = iav->side_effects_fn->func_body;
        identity_cxx += injection_description();
        identity_cxx += "if (om_new == false) {";
        identity_cxx += "// Spell ending, reset " + pretty_name();
        identity_cxx += name + ".set(0);";
        identity_cxx += "}";
        break;
    }
    case token::TK_completed_spell_weighted_duration:
    {
        assert(iav);
        assert(dav); // active spell value
        // add side-effect to identity attribute (condition)
        CodeBlock& identity_cxx = iav->side_effects_fn->func_body;
        identity_cxx += injection_description();
        identity_cxx += "if (om_new == false) {";
        identity_cxx += "// Spell ending, move active spell value to " + pretty_name();
        identity_cxx += name + ".set(" + dav->name + ");";
        identity_cxx += "}";
        break;
    }
    case token::TK_active_spell_delta:
    {
        assert(iav); // spell condition
        assert(pp_av2); // observed attribute
        assert(dav); // holds value of observed attribute at beginning of spell

        // add side-effect to observed attribute
        CodeBlock& observed_cxx = pp_av2->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "if (om_active) {";
        observed_cxx += "if (" + iav->name + ") {";
        observed_cxx += "// Maintain value for " + pretty_name();
        observed_cxx += name + ".set(" + pp_av2->name + ".get() - " + dav->name + ".get());";
        observed_cxx += "}";
        observed_cxx += "}";

        // add side-effect to identity attribute (condition)
        CodeBlock& identity_cxx = iav->side_effects_fn->func_body;
        identity_cxx += injection_description();
        identity_cxx += "if (om_new == false) {";
        identity_cxx += "// Spell ending, reset " + pretty_name();
        identity_cxx += name + ".set(0);";
        identity_cxx += "}";
        break;
    }
    case token::TK_completed_spell_delta:
    {
        assert(iav);
        assert(dav); // active spell value
        // add side-effect to identity attribute (condition)
        CodeBlock& identity_cxx = iav->side_effects_fn->func_body;
        identity_cxx += injection_description();
        identity_cxx += "if (om_new == false) {";
        identity_cxx += "// Spell ending, move active spell value to " + pretty_name();
        identity_cxx += name + ".set(" + dav->name + ");";
        identity_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (!" + name + ".get() && om_new == " + k1->value() + ") {";
        observed_cxx += name + ".set(true);";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (!" + name + ".get() && om_old == " + k1->value() + ") {";
        observed_cxx += name + ".set(true);";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (!" + name + ".get() && om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        observed_cxx += name + ".set(true);";
        observed_cxx += "}";
        observed_cxx += "}";
        break;
    }
    case token::TK_undergone_change:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        assert(av);
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (!" + name + ".get()) {";
        observed_cxx += name + ".set(true);";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (om_new == " + k1->value() + ") {";
        observed_cxx += name + ".set(" + name + ".get() + 1);";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (om_old == " + k1->value() + ") {";
        observed_cxx += name + ".set(" + name + ".get() + 1);";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        observed_cxx += name + ".set(" + name + ".get() + 1);";
        observed_cxx += "}";
        observed_cxx += "}";
        break;
    }
    case token::TK_changes:
    {
        reset_cxx += injection_description();
        reset_cxx += "// Re-assign starting value for simulation entry";
        reset_cxx += cxx_initialization_expression(false);

        auto *av = pp_av1;
        assert(av);
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += name + ".set(" + name + ".get() + 1);";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (!" + undergone->name + " && om_new == " + k1->value() + ") {";
        observed_cxx += name + ".set(" + noted->name + ".get());";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (om_new == " + k1->value() + ") {";
        observed_cxx += name + ".set(" + noted->name + ".get());";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (!" + undergone->name + " && om_old == " + k1->value() + ") {";
        observed_cxx += name + ".set(" + noted->name + ".get());";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (om_old == " + k1->value() + ") {";
        observed_cxx += name + ".set(" + noted->name + ".get());";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (!" + undergone->name + " && om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        observed_cxx += name + ".set(" + noted->name + ".get());";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        observed_cxx += name + ".set(" + noted->name + ".get());";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (!" + undergone->name + ") {";
        observed_cxx += name + ".set(" + noted->name + ".get());";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += name + ".set(" + noted->name + ".get());";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (om_new == " + k1->value() + ") {";
        observed_cxx += name + ".set(" + name + ".get() + " + noted->name + ".get());";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (om_old == " + k1->value() + ") {";
        observed_cxx += name + ".set(" + name + ".get() + " + noted->name + ".get());";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += "if (om_old == " + k1->value() + " && om_new == " + k2->value() + ") {";
        observed_cxx += name + ".set(" + name + ".get() + " + noted->name + ".get());";
        observed_cxx += "}";
        observed_cxx += "}";
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
        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += name + ".set(" + name + ".get() + " + noted->name + ".get());";
        observed_cxx += "}";
        break;
    }
    case token::TK_split:
    {
        auto *av = pp_av1;
        assert(av); // observed
        assert(pp_prt);

        init_cxx += injection_description();
        init_cxx += "{";
        init_cxx += "// Set initial value based on dependent attribute";
        init_cxx += name + ".set(" + pp_prt->name + "::value_to_interval((real)" + av->name + "));";;
        init_cxx += "}";

        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += name + ".set(" + pp_prt->name + "::value_to_interval((real)" + av->name + "));";
        observed_cxx += "}";
        break;
    }
    case token::TK_aggregate:
    {
        auto *av = pp_av1;
        assert(av); // observed
        assert(pp_cls);
        auto typ = av->pp_data_type;
        if (!typ->is_classification()) {
            pp_error(LT("error : observed attribute '") + av->name + LT("' must be of type classification in ") + pretty_name());
            break;
        }
        auto from = dynamic_cast<ClassificationSymbol *>(typ);
        assert(from); // logic guarantee
        auto to = pp_cls;
        // find the required aggregation
        string aggregation_name = AggregationSymbol::symbol_name(from, to);
        auto agg = dynamic_cast<AggregationSymbol *>(get_symbol(aggregation_name));
        if (!agg) {
            pp_error(LT("error : required aggregation from '") + from->name + LT("' to '") + to->name + LT("' is missing for ") + pretty_name());
            break;
        }

        init_cxx += injection_description();
        init_cxx += "{";
        init_cxx += "// Set initial value based on dependent attribute";
        init_cxx += name + ".set(" + agg->name + ".find(" + av->name + ")->second);";
        init_cxx += "}";

        CodeBlock& observed_cxx = av->side_effects_fn->func_body;
        observed_cxx += injection_description();
        observed_cxx += "{";
        observed_cxx += "// Maintain " + pretty_name();
        observed_cxx += name + ".set(" + agg->name + ".find(" + av->name + ")->second);";
        observed_cxx += "}";
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
        assert(ait); // the previously-created agent internal symbol which holds the next time of occurrence of the self-scheduling attribute

        {
            // Code injection into the om_reset_derived_attributes function.

            // This function runs after developer initialization just before the entity enters the simulation.
            // The local variable reset_cxx is declared above.
            reset_cxx += injection_description();
            reset_cxx += "{";
            reset_cxx += "auto & ss_attr = " + name + "; // the self-scheduling attribute being maintained";
            reset_cxx += "auto & ss_time = " + ait->name + "; // The scheduled time of maintenance of this self-scheduling attribute";
            reset_cxx += "";

            // attribute-specific code (begin)
            switch (tok) {

            case token::TK_trigger_entrances:
            case token::TK_trigger_exits:
            case token::TK_trigger_transitions:
            case token::TK_trigger_changes:
            reset_cxx += "// There is no change in the triggering condition when the entity enters the simulation.";
            reset_cxx += "ss_time = time_infinite;";
            reset_cxx += "ss_attr.set(false);";
            break;

            case token::TK_duration_trigger:
            assert(pp_av1);
            reset_cxx += "// Special case for duration_trigger - active if condition satisfied when entity enters the simulation.";
            reset_cxx += "auto & observed = " + pp_av1->name +"; // the observed attribute";
            reset_cxx += "const " + av->data_type->name + " k1 = " + k1->value() + "; // the first constant";
            reset_cxx += "const Time k2 = " + k2->value() + "; // the second constant";
            reset_cxx += "";
            reset_cxx += "if (observed == k1) {";
            reset_cxx += "// Start the timer for " + pretty_name();
            reset_cxx += "ss_time = time + k2;";
            reset_cxx += "ss_attr.set(false);";
            reset_cxx += "}";
            reset_cxx += "else {";
            reset_cxx += "ss_time = time_infinite;";
            reset_cxx += "ss_attr.set(false);";
            reset_cxx += "}";
            break;

            case token::TK_duration_counter:
            assert(pp_av1);
            reset_cxx += "// Special case for duration_counter - active if condition satisfied when entity enters the simulation.";
            reset_cxx += "auto & observed = " + pp_av1->name +"; // the observed attribute";
            reset_cxx += "const " + av->data_type->name + " k1 = " + k1->value() + "; // the first constant";
            reset_cxx += "const Time k2 = " + k2->value() + "; // the second constant";
            reset_cxx += "";
            reset_cxx += "if (observed == k1) {";
            reset_cxx += "// Start the timer for " + pretty_name();
            reset_cxx += "ss_time = time + k2;";
            reset_cxx += "ss_attr.set(0);";
            reset_cxx += "}";
            reset_cxx += "else {";
            reset_cxx += "ss_time = time_infinite;";
            reset_cxx += "ss_attr.set(0);";
            reset_cxx += "}";
            break;

            default:
            assert(false);
            }
            // attribute-specific code (end)

            reset_cxx += "}";
        }

        {
            // Code injection into the side-effect function of the triggering attribute

            CodeBlock& observed_cxx = av->side_effects_fn->func_body;
            assert(pp_agent->ss_event);
            observed_cxx += injection_description();
            observed_cxx += "if (om_active) {";
            observed_cxx += "auto & ss_attr = " + name + "; // the self-scheduling attribute being maintained";
            observed_cxx += "auto & ss_time = " + ait->name + "; // the scheduled time of maintenance of this self-scheduling attribute";
            observed_cxx += "auto & ss_event = " + pp_agent->ss_event->name + "; // the single event in the entity which maintains all self-scheduling attributes";

            // attribute-specific code (begin)
            switch (tok) {

            case token::TK_trigger_entrances:
            observed_cxx += "const " + av->data_type->name + " k1 = " + k1->value() + "; // the first constant";
            observed_cxx += "";
            observed_cxx += "if (om_new == k1) {";
            observed_cxx += "// Activate trigger " + pretty_name();
            observed_cxx += "ss_attr.set(true);";
            observed_cxx += "// Set the self-scheduling event to reset the trigger immediately";
            observed_cxx += "ss_time = time;";
            observed_cxx += "// Mark the entity's self-scheduling event for recalculation";
            observed_cxx += "ss_event.make_dirty();";
            observed_cxx += "}";
            break;

            case token::TK_trigger_exits:
            observed_cxx += "const " + av->data_type->name + " k1 = " + k1->value() + "; // the first constant";
            observed_cxx += "";
            observed_cxx += "if (om_old == k1) {";
            observed_cxx += "// Activate trigger " + pretty_name();
            observed_cxx += "ss_attr.set(true);";
            observed_cxx += "// Set the self-scheduling event to reset the trigger immediately";
            observed_cxx += "ss_time = time;";
            observed_cxx += "// Mark the entity's self-scheduling event for recalculation";
            observed_cxx += "ss_event.make_dirty();";
            observed_cxx += "}";
            break;

            case token::TK_trigger_transitions:
            observed_cxx += "const " + av->data_type->name + " k1 = " + k1->value() + "; // the first constant";
            observed_cxx += "const " + av->data_type->name + " k2 = " + k2->value() + "; // the second constant";
            observed_cxx += "";
            observed_cxx += "if (om_old == k1 && om_new == k2) {";
            observed_cxx += "// Activate trigger " + pretty_name();
            observed_cxx += "ss_attr.set(true);";
            observed_cxx += "// Set the self-scheduling event to reset the trigger immediately";
            observed_cxx += "ss_time = time;";
            observed_cxx += "// Mark the entity's self-scheduling event for recalculation";
            observed_cxx += "ss_event.make_dirty();";
            observed_cxx += "}";
            break;

            case token::TK_trigger_changes:
            observed_cxx += "";
            observed_cxx += "// Activate trigger " + pretty_name();
            observed_cxx += "ss_attr.set(true);";
            observed_cxx += "// Set the self-scheduling event to reset the trigger immediately";
            observed_cxx += "ss_time = time;";
            observed_cxx += "// Mark the entity's self-scheduling event for recalculation";
            observed_cxx += "ss_event.make_dirty();";
            break;

            case token::TK_duration_trigger:
            observed_cxx += "const " + av->data_type->name + " k1 = " + k1->value() + "; // the first constant";
            observed_cxx += "const Time k2 = " + k2->value() + "; // the second constant";
            observed_cxx += "";
            observed_cxx += "if (om_new == k1) {";
            observed_cxx += "// Start the timer for " + pretty_name();
            observed_cxx += "ss_time = time + k2;";
            observed_cxx += "// Mark the entity's self-scheduling event for recalculation";
            observed_cxx += "ss_event.make_dirty();";
            observed_cxx += "}";
            observed_cxx += "else if (om_old == k1) {";
            observed_cxx += "// The condition is no longer satisfied for " + pretty_name();
            observed_cxx += "ss_attr.set(false);";
            observed_cxx += "ss_time = time_infinite;";
            observed_cxx += "// Mark the entity's self-scheduling event for recalculation";
            observed_cxx += "ss_event.make_dirty();";
            observed_cxx += "}";
            break;

            case token::TK_duration_counter:
            observed_cxx += "const " + av->data_type->name + " k1 = " + k1->value() + "; // the first constant";
            observed_cxx += "const Time k2 = " + k2->value() + "; // the second constant";
            observed_cxx += "";
            observed_cxx += "if (om_new == k1) {";
            observed_cxx += "// Start the timer for " + pretty_name();
            observed_cxx += "ss_time = time + k2;";
            observed_cxx += "// Mark the entity's self-scheduling event for recalculation";
            observed_cxx += "ss_event.make_dirty();";
            observed_cxx += "}";
            observed_cxx += "else if (om_old == k1) {";
            observed_cxx += "// The condition is no longer satisfied for " + pretty_name();
            observed_cxx += "ss_attr.set(0);";
            observed_cxx += "ss_time = time_infinite;";
            observed_cxx += "// Mark the entity's self-scheduling event for recalculation";
            observed_cxx += "ss_event.make_dirty();";
            observed_cxx += "}";
            break;

            default:
            assert(false);
            }
            // attribute-specific code (end)

            if (Symbol::option_event_trace) {
                observed_cxx += "// Model is event trace capable, so generate event trace message";
                string evt_name = ""; // was never supported for Modgen-style event trace, so leave empty
                observed_cxx += "if (event_trace_on) "
                    "event_trace_msg("
                    "\"" + agent->name + "\", "
                    "(int)entity_id, "
                    "GetCaseSeed(), "
                    "\"" + pretty_name() + "\", "
                    + std::to_string(pp_agent->ss_event->pp_event_id) + ","
                    "\"" + evt_name + "\", "
                    " (double)ss_time, (double)age, (double)BaseEvent::get_global_time(), BaseEntity::et_msg_type::eQueuedSelfSchedulingEvent);"
                    ;
            }
            observed_cxx += "}";
        }

        {
            // Code injection into the self-scheduling implement function for these 6 self-scheduling attributes

            assert(pp_agent->ss_implement_fn); // the implement function of the event which manages all self-scheduling attributes in the agent
            CodeBlock& ss_implement_cxx = pp_agent->ss_implement_fn->func_body; // body of the C++ event implement function of the self-scheduling event
            ss_implement_cxx += injection_description();
            if (any_to_hooks) {
                ss_implement_cxx += "bool " + flag_name() + " = false;";
            }
            ss_implement_cxx += "{";
            ss_implement_cxx += "// Maintain " + pretty_name();
            ss_implement_cxx += "auto & ss_attr = " + name + ";";
            ss_implement_cxx += "auto & ss_time = " + ait->name + ";";
            if (any_to_hooks) {
                ss_implement_cxx += "auto & ss_flag = " + flag_name() + ";";
            }
            ss_implement_cxx += "if (current_time == ss_time) {";
            if (any_to_hooks) {
                ss_implement_cxx += "// Set local flag to call hooked function(s) below";
                ss_implement_cxx += "ss_flag = true;";
            }

            // attribute-specific code (begin)
            switch (tok) {

            case token::TK_trigger_entrances:
            case token::TK_trigger_exits:
            case token::TK_trigger_transitions:
            case token::TK_trigger_changes:
            ss_implement_cxx += "ss_attr.set(false);";
            ss_implement_cxx += "ss_time = time_infinite;";
            break;

            case token::TK_duration_trigger:
            ss_implement_cxx += "ss_attr.set(true);";
            ss_implement_cxx += "ss_time = time_infinite;";
            break;

            case token::TK_duration_counter:
            if (k3) {
                // variant with counter limit
                ss_implement_cxx += "ss_attr.set(1 + ss_attr.get());";
                ss_implement_cxx += "if (ss_attr.get() < " + k3->value() + ") {";
                ss_implement_cxx += "ss_time = time + " + k2->value() + ";";
                ss_implement_cxx += "}";
                ss_implement_cxx += "else {";
                ss_implement_cxx += "ss_time = time_infinite;";
                ss_implement_cxx += "}";
            }
            else {
                // variant with no counter limit
                ss_implement_cxx += "ss_attr.set(1 + ss_attr.get());";
                ss_implement_cxx += "ss_time = time + " + k2->value() + ";";
            }
            break;

            default:
            assert(false);
            }
            // attribute-specific code (end)

            if (Symbol::option_event_trace) {
                ss_implement_cxx += "// Model is event trace capable, so generate event trace message for event of self-scheduling attribute";
                string evt_name = "scheduled - " + to_string(numeric_id);
                ss_implement_cxx += "if (event_trace_on) "
                    "event_trace_msg("
                    "\"" + agent->name + "\", "
                    "(int)entity_id, "
                    "GetCaseSeed(), "
                    "\"" + pretty_name() + "\", "
                    + std::to_string(pp_agent->ss_event->pp_event_id) + ","
                    "\"" + evt_name + "\", "
                    " (double)time, (double)age, (double)BaseEvent::get_global_time(), BaseEntity::et_msg_type::eSelfSchedulingEventOccurrence);"
                    ;
            }
            if (Symbol::option_event_trace) {
                ss_implement_cxx += "// Model is event trace capable, so generate event trace message for queued time of self-scheduling attribute";
                string evt_name = ""; // was never supported for Modgen-style event trace, so leave empty
                ss_implement_cxx += "if (event_trace_on) "
                    "event_trace_msg("
                    "\"" + agent->name + "\", "
                    "(int)entity_id, "
                    "GetCaseSeed(), "
                    "\"" + pretty_name() + "\", "
                    + std::to_string(pp_agent->ss_event->pp_event_id) + ","
                    "\"" + evt_name + "\", "
                    " (double)ss_time, (double)age, (double)BaseEvent::get_global_time(), BaseEntity::et_msg_type::eQueuedSelfSchedulingEvent);"
                    ;
            }
            ss_implement_cxx += "}";  // end of the block started at "if (current_time == ..."
            ss_implement_cxx += "}";  // end of the block for this code injection
        }

        {
            // Code injection into the time function of these 6 self-scheduling attributes

            assert(pp_agent->ss_time_fn); // the time function of the event which manages all self-scheduling attributes in the agent
            CodeBlock& ss_time_cxx = pp_agent->ss_time_fn->func_body; // body of the C++ event time function of the self-scheduling event
            // The generated code minimizes the local working variable 'et' with the next time of this self-scheduling attribute
            // The required 'Time et = time_infinite;' statement at the start of the body of the event time function is generated elsewhere.
            // The required 'return et;' statement at the end of the body of the event time function is generated elsewhere.
            // after all code injection to the event time function is complete.
            ss_time_cxx += injection_description();
            ss_time_cxx += "{";
            ss_time_cxx += "auto & ss_time = " + ait->name + ";";
            ss_time_cxx += "";
            ss_time_cxx += "// Compare minimum time against scheduled time for " + pretty_name();
            ss_time_cxx += "if (ss_time < et) et = ss_time;";
            ss_time_cxx += "}";
        }

        break;
    }
    case token::TK_self_scheduling_int:
    case token::TK_self_scheduling_split:
    {
        // Common variables and code for these 2 self-scheduling attributes
        auto *av = pp_av1;
        assert(av); // the attribute being integerized or split, e.g. "age", "time", etc.
        assert(ait); // the previously-created agent internal symbol which holds the next time of occurrence of the self-scheduling attribute
        assert(pp_agent->ss_event); // the event for self-scheduling attributes
        assert(pp_agent->ss_time_fn); // the time function of the event which manages all self-scheduling attributes in the agent
        assert(pp_agent->ss_implement_fn); // the implement function of the event which manages all self-scheduling attributes in the agent
        CodeBlock& ss_time_cxx = pp_agent->ss_time_fn->func_body; // body of the C++ event time function of the self-scheduling event
        CodeBlock& ss_implement_cxx = pp_agent->ss_implement_fn->func_body; // body of the C++ event implement function of the self-scheduling event

        // The generated code minimizes the working variable 'et' with the next time of this self-scheduling attribute
        // The required 'return et;' statement in the body of the event time function is generated elsewhere
        // after all code injection to this function is complete.
        ss_time_cxx += injection_description();
        // Inject code to the event time function which minimizes the working variable 'et' with the next time of this self-scheduling attribute
        ss_time_cxx += "{";
        ss_time_cxx += "// Check scheduled time for " + pretty_name();
        ss_time_cxx += "auto & ss_time = " + ait->name + ";";
        ss_time_cxx += "if (ss_time < et) et = ss_time;";
        ss_time_cxx += "}";

        // Code for specific variants of self_scheduling_int() and self_scheduling_split() attributes
        if (av->name == "age" || av->name == "time") {

            // Code injection: age/time side-effects, for use in Start
            // get the side-effect call-back function for age / time
            assert(av->side_effects_fn);
            CodeBlock& time_or_age_cxx = av->side_effects_fn->func_body; // body of the C++ side-effect function for the argument attribute (e.g. "age")
            // For self_scheduling_int(age), the side-effects for initial assignment need to be injected into
            // both age and time, in case time is assigned after age.  So we build the side effect code block separately
            // into a local CodeBlock named blk_cxx, and then inject it after it's build in the right place(s).
            // Note that if the block is for 'age', the code needs to use age.get() rather than om_new, since
            // it will be injected into the side-effect function for time as well as age.  In the side-effect function for time
            // om_new is the new value of time, not age, which is why we need to use age.get() instead in the following code.
            CodeBlock blk_cxx;
            blk_cxx += injection_description();
            blk_cxx += "if (!om_active) {";
            blk_cxx += "// Initial values are being assigned.";
            blk_cxx += "auto & ss_attr = " + name + ";";
            blk_cxx += "auto & ss_time = " + ait->name + ";";
            if (tok == token::TK_self_scheduling_split) {
                blk_cxx += "auto part = ss_attr.get(); // working copy of partition";
            }

            // There are 4 distinct cases to handle
            if (tok == token::TK_self_scheduling_int && av->name == "age") {
                blk_cxx += "// Initial value is the largest integer less than or equal to age.";
                blk_cxx += "ss_attr.set((int)std::floor(age.get()));";
                blk_cxx += "// Time to wait for next change is the fraction of time remaining to the next integer boundary";
                blk_cxx += "ss_time = time + (1 - (age.get() - (int)std::floor(age.get())));";
            }
            else if (tok == token::TK_self_scheduling_int && av->name == "time") {
                blk_cxx += "// Initial value is the largest integer less than or equal to time.";
                blk_cxx += "ss_attr.set((int)std::floor(om_new));";
                blk_cxx += "// Time of next change is next integer time";
                blk_cxx += "ss_time = 1 + (int)std::floor(om_new);";
            }
            else if (tok == token::TK_self_scheduling_split && av->name == "age") {
                blk_cxx += "// Initial value is the corresponding partition interval.";
                blk_cxx += "part.set_from_value(age.get());";
                blk_cxx += "ss_attr.set(part);";
                blk_cxx += "// Time to wait for next change is the remaining time to get to upper bound of current interval in partition.";
                blk_cxx += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                blk_cxx += "else ss_time = time + (part.upper() - age.get());";
            }
            else if (tok == token::TK_self_scheduling_split && av->name == "time") {
                blk_cxx += "// Initial value is the corresponding partition interval.";
                blk_cxx += "part.set_from_value(om_new);";
                blk_cxx += "ss_attr.set(part);";
                blk_cxx += "// Time to wait for next change is the remaining time to get to upper bound of current interval in partition.";
                blk_cxx += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                blk_cxx += "else ss_time = time + (part.upper() - om_new);";
            }
            else {
                assert(false);
            }
            blk_cxx += "}";
            // inject the side-effect block for self_scheduling_QQQ(time) into time, and self_scheduling_QQQ(age) into age
            time_or_age_cxx += blk_cxx;
            // For self_scheduling_QQQ(age), inject the side-effect block into time as well.
            if (av->name == "age") {
                // find the attribute for time
                auto attr_time = av->pp_agent->pp_time;
                assert(attr_time);
                // get the body of the C++ side-effect function for "time"
                CodeBlock& time_cxx = attr_time->side_effects_fn->func_body; 
                // inject the fragment into time
                time_cxx += blk_cxx;
            }

            // Code injection: self-scheduling event implement function
            ss_implement_cxx += injection_description();
            if (any_to_hooks) {
                ss_implement_cxx += "bool " + flag_name() + " = false;";
            }
            ss_implement_cxx += "if (current_time == " + ait->name + ") {";
            ss_implement_cxx += "auto & ss_attr = " + name + ";";
            ss_implement_cxx += "auto & ss_time = " + ait->name + ";";
            if (any_to_hooks) {
                ss_implement_cxx += "auto & ss_flag = " + flag_name() + ";";
                ss_implement_cxx += "";
                ss_implement_cxx += "// Set local flag to call hooked function(s) below";
                ss_implement_cxx += "ss_flag = true;";
            }
            if (tok == token::TK_self_scheduling_split) {
                ss_implement_cxx += "auto part = ss_attr.get(); // working copy of partition";
            }
            ss_implement_cxx += "";

            // There are 2 distinct cases to handle
            if (tok == token::TK_self_scheduling_int) {
                ss_implement_cxx += "// Update the value";
                ss_implement_cxx += "ss_attr.set(ss_attr.get() + 1);";
                ss_implement_cxx += "// Update the time of next change";
                ss_implement_cxx += "ss_time += 1;";
            }
            else if (tok == token::TK_self_scheduling_split) {
                ss_implement_cxx += "// Update the value";
                ss_implement_cxx += "part++; // Advance to the next interval in the partition.";
                ss_implement_cxx += "ss_attr.set(part);";
                ss_implement_cxx += "// Update the time of next change by the width of that new interval.";
                ss_implement_cxx += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                ss_implement_cxx += "else ss_time += part.width();";
            }
            else {
                assert(false);
            }

            if (Symbol::option_event_trace) {
                ss_implement_cxx += "// Model is event trace capable, so generate event trace message for event of self-scheduling attribute";
                string evt_name = "scheduled - " + to_string(numeric_id);
                ss_implement_cxx += "if (event_trace_on) "
                    "event_trace_msg("
                    "\"" + agent->name + "\", "
                    "(int)entity_id, "
                    "GetCaseSeed(), "
                    "\"" + pretty_name() + "\", "
                    + std::to_string(pp_agent->ss_event->pp_event_id) + ","
                    "\"" + evt_name + "\", "
                    " (double)time, (double)age, (double)BaseEvent::get_global_time(), BaseEntity::et_msg_type::eSelfSchedulingEventOccurrence);"
                    ;
            }
            if (Symbol::option_event_trace) {
                ss_implement_cxx += "// Model is event trace capable, so generate event trace message for queued time of self-scheduling attribute";
                string evt_name = ""; // was never supported for Modgen-style event trace, so leave empty
                ss_implement_cxx += "if (event_trace_on) "
                    "event_trace_msg("
                    "\"" + agent->name + "\", "
                    "(int)entity_id, "
                    "GetCaseSeed(), "
                    "\"" + pretty_name() + "\", "
                    + std::to_string(pp_agent->ss_event->pp_event_id) + ","
                    "\"" + evt_name + "\", "
                    " (double)ss_time, (double)age, (double)BaseEvent::get_global_time(), BaseEntity::et_msg_type::eQueuedSelfSchedulingEvent);"
                    ;
            }
            ss_implement_cxx += "}";
        }
        else {
            // The argument of self_scheduling_XXX is a derived attribute, either active_spell_duration(attr,val), duration(), or duration(attr,val)
            auto dav = dynamic_cast<DerivedAttributeSymbol *>(pp_av1);
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
                CodeBlock& identity_cxx = dav->iav->side_effects_fn->func_body; // the function body of that side-effects function
 
                // Inject code into the spell condition side-effects function.
                identity_cxx += injection_description();
                identity_cxx += "if (om_active) {";
                identity_cxx += "auto & ss_attr = " + name + ";";
                identity_cxx += "auto & ss_time = " + ait->name + ";";
                identity_cxx += "auto & ss_event = " + pp_agent->ss_event->name + ";";
                if (dav->tok == token::TK_duration) {
                    identity_cxx += "auto & ss_dur = " + dav->name + ";";
                }
                if (tok == token::TK_self_scheduling_split) {
                    identity_cxx += "auto part = ss_attr.get(); // working copy of partition";
                }
                identity_cxx += "// The self-scheduling event will require recalculation";
                identity_cxx += "ss_event.make_dirty();";
                identity_cxx += "";

                // There are 4 distinct cases to handle
                if (tok == token::TK_self_scheduling_int && dav->tok == token::TK_active_spell_duration) {
                    // spell start
                    identity_cxx += "if (om_new == true) {";
                    identity_cxx += "// Active spell is starting, initialize self-scheduling attribute";
                    identity_cxx += "// Set the integerized duration to zero.";
                    identity_cxx += "ss_attr.set(0);";
                    identity_cxx += "// The time to wait is one interval of time from current time.";
                    identity_cxx += "ss_time = time + 1;";
                    identity_cxx += "}";
                    // spell end
                    identity_cxx += "else { // om_new == false";
                    identity_cxx += "// The active spell is ending, so reset self-scheduling attribute.";
                    identity_cxx += "// Set the integerized duration to zero.";
                    identity_cxx += "ss_attr.set(0);";
                    identity_cxx += "// There is no next change scheduled.";
                    identity_cxx += "ss_time = time_infinite;";
                    identity_cxx += "}";
                }
                else if (tok == token::TK_self_scheduling_int && dav->tok == token::TK_duration) {
                    // spell start
                    identity_cxx += "if (om_new == true) {";
                    identity_cxx += "// Spell is starting.";
                    identity_cxx += "// The time to wait is the time remaining to the next integer boundary of duration.";
                    identity_cxx += "ss_time = time + (1 - (ss_dur - (int)ss_dur));";
                    identity_cxx += "}";
                    // spell end
                    identity_cxx += "else {";
                    identity_cxx += "// Spell is ending.";
                    identity_cxx += "// There is no next change scheduled.";
                    identity_cxx += "ss_time = time_infinite;";
                    identity_cxx += "}";
                }
                else if (tok == token::TK_self_scheduling_split && dav->tok == token::TK_active_spell_duration) {
                    // spell start
                    identity_cxx += "if (om_new == true) {";
                    identity_cxx += "// Active spell is starting, initialize self-scheduling attribute";
                    identity_cxx += "// Set the partitioned duration to the interval containing zero.";
                    identity_cxx += "part.set_from_value(0);";
                    identity_cxx += "ss_attr.set(part);";
                    identity_cxx += "// The time to wait is the upper bound of the current interval in the partition.";
                    identity_cxx += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                    identity_cxx += "else ss_time = time + part.upper();";
                    identity_cxx += "}";
                    // spell end
                    identity_cxx += "else {";
                    identity_cxx += "// The active spell is ending, so reset self-scheduling attribute.";
                    identity_cxx += "// Set the partitioned duration to the interval containing zero.";
                    identity_cxx += "part.set_from_value(0);";
                    identity_cxx += "ss_attr.set(part);";
                    identity_cxx += "// There is no next change scheduled.";
                    identity_cxx += "ss_time = time_infinite;";
                    identity_cxx += "}";
                }
                else if (tok == token::TK_self_scheduling_split && dav->tok == token::TK_duration) {
                    // spell start
                    identity_cxx += "if (om_new == true) {";
                    identity_cxx += "// Spell is starting";
                    identity_cxx += "// Time to wait for next change is the remaining time to get to upper bound of current interval in partition.";
                    identity_cxx += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                    identity_cxx += "else ss_time = time + part.upper() - ss_dur;";
                    identity_cxx += "}";
                    // spell end
                    identity_cxx += "else {";
                    identity_cxx += "// Spell is ending.";
                    identity_cxx += "// There is no next change scheduled.";
                    identity_cxx += "ss_time = time_infinite;";
                    identity_cxx += "}";
                }
                else {
                    assert(false);
                }
                if (Symbol::option_event_trace) {
                    identity_cxx += "// Model is event trace capable, so generate event trace message";
                    string evt_name = ""; // was never supported for Modgen-style event trace, so leave empty
                    identity_cxx += "if (event_trace_on) "
                        "event_trace_msg("
                        "\"" + agent->name + "\", "
                        "(int)entity_id, "
                        "GetCaseSeed(), "
                        "\"" + pretty_name() + "\", "
                        + std::to_string(pp_agent->ss_event->pp_event_id) + ","
                        "\"" + evt_name + "\", "
                        " (double)ss_time, (double)age, (double)BaseEvent::get_global_time(), BaseEntity::et_msg_type::eQueuedSelfSchedulingEvent);"
                        ;
                }
                identity_cxx += "} // if (om_active)";
            }

            // Code injection: self-scheduling event implement function
            ss_implement_cxx += injection_description();
            if (any_to_hooks) {
                ss_implement_cxx += "bool " + flag_name() + " = false;";
            }
            ss_implement_cxx += "if (current_time == " + ait->name + ") {";
            ss_implement_cxx += "auto & ss_attr = " + name + ";";
            ss_implement_cxx += "auto & ss_time = " + ait->name + ";";
            if (any_to_hooks) {
                ss_implement_cxx += "auto & ss_flag = " + flag_name() + ";";
                ss_implement_cxx += "";
                ss_implement_cxx += "// Set local flag to call hooked function(s) below";
                ss_implement_cxx += "ss_flag = true;";
            }
            if (tok == token::TK_self_scheduling_split) {
                ss_implement_cxx += "auto part = ss_attr.get(); // working copy of partition";
            }
            ss_implement_cxx += "";

            // There are 2 distinct cases to handle
            if (tok == token::TK_self_scheduling_int) {
                ss_implement_cxx += "// Increment the integerized duration.";
                ss_implement_cxx += "ss_attr.set(ss_attr.get() + 1);";
                ss_implement_cxx += "// The time to wait is one interval of time.";
                ss_implement_cxx += "ss_time = time + 1;";
            }
            else if (tok == token::TK_self_scheduling_split) {
                ss_implement_cxx += "// Set the partitioned duration to the next interval.";
                ss_implement_cxx += "part++;";
                ss_implement_cxx += "ss_attr.set(part);";
                ss_implement_cxx += "// The time to wait is the width of the current interval in the partition.";
                ss_implement_cxx += "if (part.upper() == REAL_MAX) ss_time = time_infinite;";
                ss_implement_cxx += "else ss_time = time + part.width();";
            }
            else {
                assert(false);
            }

            if (Symbol::option_event_trace) {
                ss_implement_cxx += "// Model is event trace capable, so generate event trace message for event of self-scheduling attribute";
                string evt_name = "scheduled - " + to_string(numeric_id);
                ss_implement_cxx += "if (event_trace_on) "
                    "event_trace_msg("
                    "\"" + agent->name + "\", "
                    "(int)entity_id, "
                    "GetCaseSeed(), "
                    "\"" + pretty_name() + "\", "
                    + std::to_string(pp_agent->ss_event->pp_event_id) + ","
                    "\"" + evt_name + "\", "
                    " (double)time, (double)age, (double)BaseEvent::get_global_time(), BaseEntity::et_msg_type::eSelfSchedulingEventOccurrence);"
                    ;
            }
            if (Symbol::option_event_trace) {
                ss_implement_cxx += "// Model is event trace capable, so generate event trace message for queued time of self-scheduling attribute";
                string evt_name = ""; // was never supported for Modgen-style event trace, so leave empty
                ss_implement_cxx += "if (event_trace_on) "
                    "event_trace_msg("
                    "\"" + agent->name + "\", "
                    "(int)entity_id, "
                    "GetCaseSeed(), "
                    "\"" + pretty_name() + "\", "
                    + std::to_string(pp_agent->ss_event->pp_event_id) + ","
                    "\"" + evt_name + "\", "
                    " (double)ss_time, (double)age, (double)BaseEvent::get_global_time(), BaseEntity::et_msg_type::eQueuedSelfSchedulingEvent);"
                    ;
            }
            ss_implement_cxx += "}";
        }
        break;
    }

    default:
    break;
    }
}

string DerivedAttributeSymbol::pretty_name() const
{
    string result;
    constexpr const char* arg_sep = ","; // could be "," or ", "

    switch (tok) {
    case token::TK_duration:
    {
        if (!pp_av1) {
            result = token_to_string(tok) + "()";
        }
        else {
            result = token_to_string(tok) + "(" + pp_av1->pretty_name() + arg_sep + k1->value() + ")";
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
            result = token_to_string(tok) + "(" + pp_av1->pretty_name() + arg_sep + k1->value() + arg_sep + pp_av2->pretty_name() + ")";
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
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + arg_sep + k1->value() + ")";
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
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + arg_sep + k1->value() + arg_sep + pp_av2->pretty_name() + ")";
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
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + arg_sep + k1->value() + arg_sep + k2->value() + ")";
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
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + arg_sep + k1->value() + arg_sep + k2->value() + arg_sep + pp_av2->pretty_name() + ")";
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
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + arg_sep + pp_av2->pretty_name() + ")";
        break;
    }
    case token::TK_split:
    case token::TK_self_scheduling_split:
    {
        assert(pp_av1);
        assert(pp_prt);
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + arg_sep + pp_prt->name + ")";
        break;
    }
    case token::TK_aggregate:
    {
        assert(pp_av1);
        assert(pp_cls);
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + arg_sep + pp_cls->name + ")";
        break;
    }
    case token::TK_duration_counter:
    {
        assert(pp_av1);
        assert(k1);
        assert(k2);
        assert(k3 || !k3); // optional
        result = token_to_string(tok) + "(" + pp_av1->pretty_name() + arg_sep + k1->value() + arg_sep + k2->value() + (k3 ? (arg_sep + k3->value()) : "") + ")";
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

bool DerivedAttributeSymbol::is_self_scheduling() const
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

bool DerivedAttributeSymbol::is_trigger() const
{
    switch (tok) {
    case token::TK_trigger_entrances:
    case token::TK_trigger_exits:
    case token::TK_trigger_transitions:
    case token::TK_trigger_changes:
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

string DerivedAttributeSymbol::pp_modgen_name() const
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
            auto dav = dynamic_cast<DerivedAttributeSymbol *>(pp_av1);
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
            auto dav = dynamic_cast<DerivedAttributeSymbol *>(pp_av1);
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

void DerivedAttributeSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        if (is_self_scheduling()) {
            // create the event to support self-scheduling states in the containing agent
            auto agnt = dynamic_cast<EntitySymbol *>(agent);
            assert(agnt);
            agnt->create_ss_event();
            assert(agnt->ss_event);
            // Push the name into the post parse ignore hash for the current pass.
            pp_symbols_ignore.insert(agnt->ss_event->unique_name);
        }
        break;
    }
    case eAssignMembers:
    {
        assign_pp_members();
        if (is_self_scheduling()) {
            check_ss_arguments(); // will call fatal_error and throw exction on failure
        }
        break;
    }
    case eResolveDataTypes:
    {
        assign_data_type();
        break;
    }
    case ePopulateCollections:
    {
        record_dependencies();
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

CodeBlock DerivedAttributeSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    h += "AgentVar<"
        + pp_data_type->name + ", "
        + pp_data_type->exposed_type() + ", "
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


