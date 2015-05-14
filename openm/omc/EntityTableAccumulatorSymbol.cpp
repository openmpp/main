/**
* @file    EntityTableAccumulatorSymbol.cpp
* Definitions for the EntityTableAccumulatorSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "EntityTableAccumulatorSymbol.h"
#include "EntityTableSymbol.h"
#include "AgentVarSymbol.h"
#include "EntityTableMeasureAttributeSymbol.h"
#include "CodeBlock.h"


// static
string EntityTableAccumulatorSymbol::symbol_name(const Symbol *table, token_type accumulator, token_type increment, token_type table_op, const Symbol *agentvar)
{
    string result;
    result = "om_" + table->name + "_ta_" + token_to_string(accumulator) + "_" + token_to_string(increment) + "_" + token_to_string(table_op) + "_" + agentvar->name;
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
    string result = " accumulator " + to_string(index) + ": " + token_to_string(accumulator) + "(" + token_to_string(increment) + "(" + token_to_string(table_op) + "(" + agentvar->pretty_name() + ")))";
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
            auto av = dynamic_cast<AgentVarSymbol *>(pp_symbol(agentvar));
            assert(av);
            av->create_lagged();
        }
        break;
    }
    case eAssignMembers:
    {
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<EntityTableSymbol *> (pp_symbol(table));
        assert(pp_table); // parser guarantee

        // assign direct pointer to agentvar for post-parse use
        pp_agentvar = dynamic_cast<AgentVarSymbol *> (pp_symbol(agentvar));
        assert(pp_agentvar); // parser guarantee

        // assign direct pointer to EntityTableMeasureAttributeSymbol for post-parse use
        pp_analysis_agentvar = dynamic_cast<EntityTableMeasureAttributeSymbol *> (pp_symbol(analysis_agentvar));
        assert(pp_analysis_agentvar); // parser guarantee

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


