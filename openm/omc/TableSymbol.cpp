/**
* @file    TableSymbol.cpp
* Definitions for the TableSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "TableSymbol.h"
#include "LanguageSymbol.h"
#include "AgentSymbol.h"
#include "TableAccumulatorSymbol.h"
#include "TableAnalysisAgentVarSymbol.h"
#include "TableExpressionSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

void TableSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // assign direct pointer to agent for use post-parse
            pp_agent = dynamic_cast<AgentSymbol *> (agent);
            assert(pp_agent); // parser guarantee

            // add this table to the complete list of tables
            pp_all_tables.push_back(this);

            // Add this table to the agent's list of tables
            pp_agent->pp_agent_tables.push_back(this);
        }
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

CodeBlock TableSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    int n_accumulators = pp_accumulators.size();
    int n_expressions = pp_expressions.size();
    int n_cells = 1;

    h += "";
    h += "class " + name + " {";
    h += "public:";
    h += "void initialize_accumulators();";
    h += "void compute_expressions();";
    // constructor
    // E.g. DurationOfLife() { initialize_accumulators();}
    h += name + "() { initialize_accumulators();}";
    h += "";
    h += "// constants";
    h += "static const int n_cells = " + to_string(n_cells) + ";";
    h += "static const int n_accumulators = " + to_string(n_accumulators) + ";";
    h += "static const int n_expressions = " + to_string(n_expressions) + ";";

    h += "";
    h += "// expression storage";
    for (int j = 0; j < n_expressions; j++) {
        h += "double expr" + to_string(j) + "[" + to_string(n_cells) + "];";
    }

    h += "";
    h += "// accumulator storage";
    for (int j = 0; j < n_accumulators; j++) {
        h += "double acc" + to_string(j) + "[" + to_string(n_cells) + "];";
    }

    h += "";
    h += "// accumulator array of pointers";
    h += "double *accumulators[n_accumulators];";
    h += "";
    h += "// expression array of pointers";
    h += "double *expressions[n_expressions];";

    h += "};";
    h += "extern " + name + " the" + name + ";";

    return h;
}

CodeBlock TableSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "";
    // table definition
    // E.g. DurationOfLife theDurationOfLife;
    c += name + " the" + name + ";";

    // definition of initialize_accumulators()
    c += "void " + name + "::initialize_accumulators()";
    c += "{";
    for (auto acc : pp_accumulators) {
        string initial_value = "";
        switch (acc->accumulator) {
        case token::TK_sum:
            initial_value = "  0.0";
            break;
        case token::TK_min:
            initial_value = " DBL_MAX";
            break;
        case token::TK_max:
            initial_value = "-DBL_MAX";
            break;
        default:
            // not reached
            assert(0);
        }
        // e.g.  sum(value_in(alive))
        c += "// " + Symbol::token_to_string(acc->accumulator) + "(" + Symbol::token_to_string(acc->increment) + "(" + acc->agentvar->name + "))";
        // e.g. for ( int cell = 0; cell < n_cells; cell++ ) acc0[cell] =   0.0;
        c += "for ( int cell = 0; cell < n_cells; cell++ ) acc" + to_string(acc->index) + "[cell] = " + initial_value + ";";
        // e.g. accumulators[0] = acc0;
        c += "accumulators[" + to_string(acc->index) + "] = acc" + to_string(acc->index) + ";";
        c += "";

    }
    c += "}";
    c += "";

    // definition of compute_expressions()
    // E.g. void DurationOfLife::compute_expressions()
    c += "void " + name + "::compute_expressions()";
    c += "{";
    for (auto table_expr : pp_expressions) {
        // E.g.  // SUM_BEFORE( acc0 )
        c += "// " + table_expr->get_expression(table_expr->root, TableExpressionSymbol::expression_style::sql);
        // E.g. expressions[0] = expr0;
        c += "expressions[" + to_string(table_expr->index) + "] = expr" + to_string(table_expr->index) + ";";
        // E.g. for ( int cell = 0; cell < n_cells; cell++ ) expressions[0][cell] = accumulators[0][cell] ;
        c += "for ( int cell = 0; cell < n_cells; cell++ ) "
            "expressions[" + to_string(table_expr->index) + "][cell] = " + table_expr->get_expression(table_expr->root, TableExpressionSymbol::expression_style::cxx) + " ;";
        c += "";
    }
    c += "}";
    c += "";
    return c;
}

CodeBlock TableSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += "";
    // example:         // DurationOfLife
    h += "// " + name;
    // example:        void DurationOfLife_do_increments( bool prepare = 0, bool process = 0 );
    h += do_increments_decl();
    return h;
}

CodeBlock TableSymbol::cxx_definition_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_agent();

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
            assert(0); // parser guarantee
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
            assert(0); // parser guarantee
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

void TableSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.
    TableDicRow tableDic;

    tableDic.tableId = pp_table_id;
    tableDic.tableName = name;
    tableDic.isUser = false;
    tableDic.rank = 0;
    tableDic.isSparse = true;   // do not store NULLs
    tableDic.isHidden = false;
    metaRows.tableDic.push_back(tableDic);

    for (auto lang : Symbol::pp_all_languages) {
        TableDicTxtLangRow tableTxt;
        tableTxt.tableId = pp_table_id;
        tableTxt.langName = lang->name;
        tableTxt.descr = label(*lang);
        tableTxt.note = note(*lang);
        tableTxt.unitDescr = "Expressions (" + lang->name + ")"; // TODO
        tableTxt.unitNote = "Expressions Note (" + lang->name + ")"; // TODO
        metaRows.tableTxt.push_back(tableTxt);
    }

    // TODO: rank 0 tables in alpha have no TableDimsRow entries
    // or TableDimsTxtLangRow entries

    // accumulators for table
    for (auto acc : pp_accumulators) {
        TableAccRow tableAcc;

        tableAcc.tableId = pp_table_id;
        tableAcc.accId = acc->index;
        tableAcc.name = "acc" + to_string(acc->index);
        tableAcc.expr = acc->pretty_name();
        metaRows.tableAcc.push_back(tableAcc);

        for (auto lang : Symbol::pp_all_languages) {
            TableAccTxtLangRow tableAccTxt;
            tableAccTxt.tableId = pp_table_id;
            tableAccTxt.accId = acc->index;
            tableAccTxt.langName = lang->name;
            tableAccTxt.descr = acc->label(*lang);
            tableAccTxt.note = acc->note(*lang);
            metaRows.tableAccTxt.push_back(tableAccTxt);
        }
    }

    // expressions for table
    for (auto expr : pp_expressions) {
        TableUnitRow tableUnit;
        tableUnit.tableId = pp_table_id;
        tableUnit.unitId = expr->index;
        tableUnit.name = "expr" + to_string(expr->index);
        tableUnit.src = expr->get_expression(expr->root, TableExpressionSymbol::expression_style::sql);
        metaRows.tableUnit.push_back(tableUnit);

        for (auto lang : Symbol::pp_all_languages) {
            TableUnitTxtLangRow tableUnitTxt;
            tableUnitTxt.tableId = pp_table_id;
            tableUnitTxt.unitId = expr->index;

            tableUnitTxt.langName = lang->name;
            tableUnitTxt.descr = expr->label(*lang);
            tableUnitTxt.note = expr->note(*lang);
            metaRows.tableUnitTxt.push_back(tableUnitTxt);
        }
    }
}




