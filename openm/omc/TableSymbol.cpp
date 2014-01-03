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

CodeBlock TableSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    return h;
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

void TableSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.
    TableDicRow tableDic;
    TableDicTxtLangRow tableTxt;

    tableDic.tableId = pp_numeric_id;
    tableDic.tableName = name;
    tableDic.isUser = false;
    tableDic.rank = 0;
    tableDic.isSparse = true;   // do not store NULLs
    tableDic.isHidden = false;
    metaRows.tableDic.push_back(tableDic);

    // TODO language entries are hard-coded bilingual for alpha
    tableTxt.tableId = pp_numeric_id;
    tableTxt.langName = "EN";
    tableTxt.descr = name + " short name (EN)";
    tableTxt.note = name + " note (EN)";
    tableTxt.unitDescr = "Expressions (EN)"; // TODO
    tableTxt.unitNote = "Expressions Note (EN)"; // TODO
    metaRows.tableTxt.push_back(tableTxt);

    tableTxt.langName = "FR";
    tableTxt.descr = name + " short name (FR)";
    tableTxt.note = name + " note (FR)";
    tableTxt.unitDescr = "Expressions (FR)"; // TODO
    tableTxt.unitNote = "Expressions Note (FR)"; // TODO
    metaRows.tableTxt.push_back(tableTxt);

    // TODO: rank 0 tables in alpha have no TableDimsRow entries
    // or TableDimsTxtLangRow entries

    // accumulators for table
    TableAccRow tableAcc;
    tableAcc.tableId = pp_numeric_id;
    TableAccTxtLangRow tableAccTxt;
    tableAccTxt.tableId = pp_numeric_id;
    for (auto acc : pp_accumulators) {
        tableAcc.accId = acc->index;
        tableAcc.name = "acc" + to_string(acc->index);
        tableAcc.expr = acc->pretty_name();
        metaRows.tableAcc.push_back(tableAcc);

        tableAccTxt.accId = acc->index;
        tableAccTxt.langName = "EN";
        tableAccTxt.descr = acc->name + " short name (EN)";
        tableAccTxt.note = acc->name + " note (EN)";
        metaRows.tableAccTxt.push_back(tableAccTxt);

        tableAccTxt.langName = "FR";
        tableAccTxt.descr = acc->name + " short name (FR)";
        tableAccTxt.note = acc->name + " note (FR)";
        metaRows.tableAccTxt.push_back(tableAccTxt);
    }

    // expressions for table
    TableUnitRow tableUnit;
    tableUnit.tableId = pp_numeric_id;
    TableUnitTxtLangRow tableUnitTxt;
    tableUnitTxt.tableId = pp_numeric_id;
    for (auto expr : pp_expressions) {
        tableUnit.unitId = expr->index;
        tableUnit.name = "expr" + to_string(expr->index);
        tableUnit.src = expr->get_expression(expr->root, TableExpressionSymbol::expression_style::sql);
        metaRows.tableUnit.push_back(tableUnit);

        tableUnitTxt.unitId = expr->index;
        tableUnitTxt.langName = "EN";
        tableUnitTxt.descr = expr->name + " short name (EN)";
        tableUnitTxt.note = expr->name + " note (EN)";
        metaRows.tableUnitTxt.push_back(tableUnitTxt);

        tableUnitTxt.langName = "FR";
        tableUnitTxt.descr = expr->name + " short name (FR)";
        tableUnitTxt.note = expr->name + " note (FR)";
        metaRows.tableUnitTxt.push_back(tableUnitTxt);
    }
}




