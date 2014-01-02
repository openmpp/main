/**
* @file    TableAccumulatorSymbol.cpp
* Definitions for the TableAccumulatorSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "TableAccumulatorSymbol.h"
#include "TableSymbol.h"
#include "AgentVarSymbol.h"
#include "TableAnalysisAgentVarSymbol.h"
#include "CodeBlock.h"


// static
string TableAccumulatorSymbol::symbol_name(const Symbol *table, token_type accumulator, token_type increment, const Symbol *agentvar)
{
    string result;
    result = "om_ta_" + table->name + "_" + token_to_string(accumulator) + "_" + token_to_string(increment) + "_" + agentvar->name;
    return result;
}

// static
bool TableAccumulatorSymbol::exists(const Symbol *table, token_type accumulator, token_type increment, const Symbol *agentvar)
{
    string unm = symbol_name(table, accumulator, increment, agentvar);
    return symbols.count(unm) == 0 ? false : true;
}

string TableAccumulatorSymbol::pretty_name()
{
    // example:     accumulator 0: sum(delta(duration))
    string result = " accumulator " + to_string(index) + ": " + token_to_string(accumulator) + "(" + token_to_string(increment) + "(" + agentvar->pretty_name() + "))";
    return result;
}

void TableAccumulatorSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case 1:
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<TableSymbol *> (table);
        assert(pp_table); // grammar guarantee

        // assign direct pointer to agentvar for post-parse use
        pp_agentvar = dynamic_cast<AgentVarSymbol *> (agentvar);
        assert(pp_agentvar); // grammar guarantee

        // assign direct pointer to TableAnalysisAgentVarSymbol for post-parse use
        pp_analysis_agentvar = dynamic_cast<TableAnalysisAgentVarSymbol *> (analysis_agentvar);
        assert(pp_analysis_agentvar); // grammar guarantee

        // Add this table accumulator to the table's list of accumulators
        pp_table->pp_accumulators.push_back(this);

        break;
    default:
        break;
    }
}


