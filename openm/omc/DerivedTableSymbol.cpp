/**
* @file    DerivedTableSymbol.cpp
* Definitions for the DerivedTableSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DerivedTableSymbol.h"
#include "LanguageSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

void DerivedTableSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // add this derived table to the complete list of derived tables
        pp_all_derived_tables.push_back(this);

        break;
    }
    default:
        break;
    }
}

CodeBlock DerivedTableSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    size_t n_cells = cell_count();
    size_t n_measures = measure_count();
    size_t n_rank = dimension_count();
    //h += "typedef DerivedTable<"  + to_string(n_cells) + ", " + to_string(n_placeholders) + "> " + cxx_type + ";";
    h += "typedef DerivedTable<"
        + to_string(n_rank) + ", "
        + to_string(n_cells) + ", "
        + to_string(n_measures)
        + "> " + cxx_type + ";";
    h += "extern thread_local "  + cxx_type + " * " + cxx_instance + ";";
    h += "";

    return h;
}

CodeBlock DerivedTableSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "thread_local "  + cxx_type + " * " + cxx_instance + " = nullptr;";

    return c;
}

string DerivedTableSymbol::cxx_initializer() const
{
    string cxx;
    cxx = cxx_shape_initializer_list();
    return cxx;
}

void DerivedTableSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    // 'accumulators' for derived table
    for (auto acc : pp_measures) {

        TableAccRow tableAcc;

        tableAcc.tableId = pp_table_id;
        tableAcc.accId = acc->index;
        // tableAcc.name = "acc" + to_string(acc->index);
        tableAcc.expr = acc->pretty_name();
        metaRows.tableAcc.push_back(tableAcc);

        // Labels and notes for accumulators
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

    // measures for derived table
    // Just average the values across simulation members
    for (auto expr : pp_measures) {

        TableExprRow tableExpr;

        tableExpr.tableId = pp_table_id;
        tableExpr.exprId = expr->index;
        tableExpr.name = "meas" + to_string(expr->index);
        tableExpr.decimals = expr->decimals;
        // construct scale part, e.g. "1.0E-3 * "
        string scale_part;
        if (expr->scale != 0) {
            scale_part = expr->scale_as_factor() + " * ";
        }
        tableExpr.srcExpr = scale_part + "OM_AVG(acc" + to_string(expr->index) + ")";
        metaRows.tableExpr.push_back(tableExpr);

        // Labels and notes for measures
        for (auto lang : Symbol::pp_all_languages) {

            TableExprTxtLangRow tableExprTxt;

            tableExprTxt.tableId = pp_table_id;
            tableExprTxt.exprId = expr->index;

            tableExprTxt.langName = lang->name;

            tableExprTxt.descr = expr->label(*lang);
            
            tableExprTxt.note = expr->note(*lang);
            metaRows.tableExprTxt.push_back(tableExprTxt);
        }
    }
}
