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
string EntityTableAccumulatorSymbol::symbol_name(const Symbol *table, token_type accumulator, token_type increment, token_type table_op, const Symbol *agentvar)
{
    string result;
    result = "om_" + table->name + "_ta_" + token_to_string(accumulator);
    if (accumulator != token::TK_unit) {
        assert(agentvar); // grammar guarantee
        result += "_" + token_to_string(increment) + "_" + token_to_string(table_op) + "_" + agentvar->name;
    }
    return result;
}

// static
bool EntityTableAccumulatorSymbol::exists(const Symbol *table, token_type accumulator, token_type increment, token_type table_op, const Symbol *agentvar)
{
    string unm = symbol_name(table, accumulator, increment, table_op, agentvar);
    return symbols.count(unm) == 0 ? false : true;
}

string EntityTableAccumulatorSymbol::pretty_name() const
{
    // example:     accumulator 0: sum(delta(interval(duration)))
    string result = " accumulator " + to_string(index) + ": " + token_to_string(accumulator);
    if (accumulator != token::TK_unit) {
        assert(agentvar); // grammar guarantee
        result += "(" + token_to_string(increment) + "(" + token_to_string(table_op) + "(" + (*agentvar)->pretty_name() + ")))";
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
        // If agentvar is subject to event() tabulation operator create lagged version.
        if (table_op == token::TK_event) {
            auto av = dynamic_cast<AttributeSymbol *>(pp_symbol(agentvar));
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

        if (accumulator != token::TK_unit) {
            // assign direct pointer to attribute for post-parse use
            pp_agentvar = dynamic_cast<AttributeSymbol *> (pp_symbol(agentvar));
            if (pp_agentvar == nullptr) {
                throw HelperException(LT("error : %s is not an attribute in table %s"), (*agentvar)->name.c_str(), table->name.c_str());
            }

            // assign direct pointer to EntityTableMeasureAttributeSymbol for post-parse use
            pp_analysis_agentvar = dynamic_cast<EntityTableMeasureAttributeSymbol *> (pp_symbol(analysis_agentvar));
            assert(pp_analysis_agentvar); // parser guarantee
        }
        else {
            pp_agentvar = nullptr;
            pp_analysis_agentvar = nullptr;
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


