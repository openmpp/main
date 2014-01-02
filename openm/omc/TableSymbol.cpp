/**
* @file    TableSymbol.cpp
* Definitions for the TableSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "TableSymbol.h"
#include "AgentSymbol.h"
#include "TableAccumulatorSymbol.h"
#include "TableAnalysisAgentVarSymbol.h"
#include "CodeBlock.h"

void TableSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case 1:
        // assign direct pointer to agent for use post-parse
        pp_agent = dynamic_cast<AgentSymbol *> (agent);
        // add this table to the complete list of tables
        pp_tables.push_back(this);
        // Add this table to the agent's list of tables
        pp_agent->pp_agent_tables.push_back(this);
        break;
    default:
        break;
    }
}

const string TableSymbol::do_increments_func()
{
    // Eg. DurationOfLife_do_increments
    return name + "_do_increments";
}

const string TableSymbol::do_increments_decl()
{
    // E.g. void DurationOfLife_do_increments(bool prepare = true, bool process = true)
    return "void " + do_increments_func() + "( bool prepare = true, bool process = true );";
}

const string TableSymbol::do_increments_defn()
{
    // E.g. void Person::DurationOfLife_do_increments(bool prepare = true, bool process = true)
    return "void " + agent->name + "::" + do_increments_func() + "( bool prepare, bool process )";
}

CodeBlock TableSymbol::cxx_declaration_agent_scope()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent_scope();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += "";
    // example:         // DurationOfLife
    h += "// " + name;
    // example:        void DurationOfLife_do_increments( bool prepare = 0, bool process = 0 );
    h += do_increments_decl();
    return h;
}

CodeBlock TableSymbol::cxx_definition()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "";

    // example:         // DurationOfLife
    c += "// " + name;

    // example:        void DurationOfLife_do_increments( bool prepare = 0, bool process = 0 )
    c += do_increments_defn();
    c += "{";
    c += "if ( process ) {";
    for (auto acc : pp_accumulators) {
        // name of agentvar
        string agentvar_name = acc->agentvar->name;
        // name of 'in' for agentvar
        string in_agentvar_name = acc->pp_analysis_agentvar->in_agentvar_name();
        // index of accumulator as string
        string accumulator_index = to_string(acc->index);
        // expression for the accumulator as string
        string accumulator_expr = "the" + name + ".accumulators[" + accumulator_index + "][0]";

        // expression evaluating to value of increment
        string increment_expr;
        switch (acc->increment) {
        case token::TK_value_in:
            increment_expr = in_agentvar_name;
            break;
        case token::TK_value_out:
            increment_expr = agentvar_name;
            break;
        case token::TK_delta:
            increment_expr = "( " + agentvar_name + " - " + in_agentvar_name + " )";
            break;
            // TODO - all other increment operators
        default:
            assert(0); // grammar guarantee
        }

        c += "{";
        c += "// " + acc->pretty_name();
        c += "double dIncrement = " + increment_expr + ";";
        switch (acc->accumulator) {
        case token::TK_sum:
            c += accumulator_expr + " += dIncrement;";
            break;
        case token::TK_min:
            c += "double dAccumulator = " + accumulator_expr + ";";
            c += "if ( dIncrement < dAccumulator ) " + accumulator_expr + " = dIncrement;";
            break;
        case token::TK_max:
            c += "double dAccumulator = " + accumulator_expr + ";";
            c += "if ( dIncrement > dAccumulator ) " + accumulator_expr + " = dIncrement;";
            break;
        default:
            assert(0); // grammar guarantee
        }
        c += "}";
    }
    c += "}";
    c += "if ( prepare ) {";
    for (auto table_agentvar : pp_table_agentvars) {
        if (table_agentvar->need_value_in)
            c += table_agentvar->cxx_prepare_increment();
    }
    c += "}";
    c += "}";

    return c;
}



