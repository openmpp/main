/**
* @file    EntityTableAccumulatorSymbol.cpp
* Definitions for the EntityTableAccumulatorSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/omError.h"
#include "EntityTableAccumulatorSymbol.h"
#include "EntityTableSymbol.h"
#include "AttributeSymbol.h"
#include "EntityInternalSymbol.h"
#include "EntityTableMeasureAttributeSymbol.h"
#include "CodeBlock.h"

using namespace std;
using namespace openm;


// static
string EntityTableAccumulatorSymbol::symbol_name(const Symbol *table, token_type accumulator, token_type increment, token_type table_op, const Symbol *attribute)
{
    string result;
    result = "om_" + table->name + "_ta_" + token_to_string(accumulator);
    if (accumulator != token::TK_unit) {
        assert(attribute); // grammar guarantee
        result += "_" + token_to_string(increment) + "_" + token_to_string(table_op) + "_" + attribute->name;
    }
    return result;
}

// static
bool EntityTableAccumulatorSymbol::exists(const Symbol *table, token_type accumulator, token_type increment, token_type table_op, const Symbol *attribute)
{
    string unm = symbol_name(table, accumulator, increment, table_op, attribute);
    return symbols.count(unm) == 0 ? false : true;
}

string EntityTableAccumulatorSymbol::pretty_name() const
{
    // example:     accumulator 0: sum(delta(interval(duration)))
    string result = " accumulator " + to_string(index) + ": " + token_to_string(statistic);
    if (statistic != token::TK_unit) {
        assert(attribute); // grammar guarantee
        result += "(" + token_to_string(increment) + "(" + token_to_string(timing) + "(" + (*attribute)->pretty_name() + ")))";
    }
    return result;
}

void EntityTableAccumulatorSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        // If attribute is subject to event() tabulation operator create lagged version.
        if (timing == token::TK_event) {
            auto av = dynamic_cast<AttributeSymbol *>(pp_symbol(attribute));
            assert(av);
            av->create_lagged();
            assert(av->lagged);
            assert(av->lagged_event_counter);
			// Push the names into the post parse ignore hash for the current pass.
			pp_symbols_ignore.insert(av->lagged->unique_name);
            pp_symbols_ignore.insert(av->lagged_event_counter->unique_name);
        }
        break;
    }
    case eAssignMembers:
    {
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<EntityTableSymbol *> (pp_symbol(table));
        assert(pp_table); // parser guarantee

        if (statistic != token::TK_unit) {
            // assign direct pointer to attribute for post-parse use
            pp_attribute = dynamic_cast<AttributeSymbol *> (pp_symbol(attribute));
            if (pp_attribute == nullptr) {
                throw HelperException(LT("error : %s is not an attribute in table %s"), (*attribute)->name.c_str(), table->name.c_str());
            }

            // assign direct pointer to EntityTableMeasureAttributeSymbol for post-parse use
            pp_analysis_attribute = dynamic_cast<EntityTableMeasureAttributeSymbol *> (pp_symbol(analysis_attribute));
            assert(pp_analysis_attribute); // parser guarantee
        }
        else {
            pp_attribute = nullptr;
            pp_analysis_attribute = nullptr;
        }

        // emit warning if weighting enabled and ordinal statistic used
        if (option_weighted_tabulation && has_obs_collection) {
            auto ordinal_name = token_to_string(statistic);
            pp_warning(LT("warning : weighting is not supported for ordinal statistic '") + ordinal_name + LT("' in table '") + pp_table->name +"'");
        }
        break;
    }
    case ePopulateCollections:
    {
        // Add this table accumulator to the table's list of accumulators
        pp_table->pp_accumulators.push_back(this);

        break;
    }
    default:
    break;
    }
}


