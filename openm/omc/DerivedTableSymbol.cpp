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

    int maxAccIndex = 0;    // only to avoid acc->index increment later

    // Measures and accumulators are the same for derived tables.
    // For derived tables, information is stored in the measures collection
    // and the accumulators collection is empty.
    // For derived tables, the measures collection is iterated
    // to populate metadata for both accumulators and measures.
    // The working variable 'acc' below is actually a measure.
    // It is named 'acc' to maximize correspondence
    // with parallel code in EntityTableSymbol.
    for (auto acc : pp_measures) {

        TableAccRow tableAcc;

        tableAcc.tableId = pp_table_id;
        tableAcc.accId = acc->index;
        tableAcc.accSrc = acc->pretty_name();
        tableAcc.isDerived = false;
        metaRows.tableAcc.push_back(tableAcc);

        // Maintain maxAccIndex to use when creating derived accumulators below.
        if (maxAccIndex < tableAcc.accId) {
            maxAccIndex = tableAcc.accId;
        }

        // Labels and notes for accumulators
        for (const auto& langSym : Symbol::pp_all_languages) {
            TableAccTxtLangRow tableAccTxt;
            tableAccTxt.tableId = pp_table_id;
            tableAccTxt.accId = acc->index;
            tableAccTxt.langCode = langSym->name;
            tableAccTxt.descr = acc->label(*langSym);
            tableAccTxt.note = acc->note(*langSym);
            metaRows.tableAccTxt.push_back(tableAccTxt);
        }
    }

    // Measures for derived table.
    // There is an exact one-to-one relationship
    // between measures and accumulators for derived tables.
    for (auto mIt = pp_measures.begin(); mIt != pp_measures.end(); ++mIt) {

        TableMeasureSymbol * measure = *mIt;

        // make measure db column name: it must be unique, alphanumeric and not longer than 255 chars
        TableMeasureSymbol::to_column_name(name, pp_measures, measure);

        TableExprRow tableExpr;

        tableExpr.tableId = pp_table_id;
        tableExpr.exprId = measure->index;
        tableExpr.name = measure->short_name;
        tableExpr.decimals = measure->decimals;

        // construct scale part, e.g. "1.0E-3 * "
        string scale_part;
        if (measure->scale != 0) {
            scale_part = measure->scale_as_factor() + " * ";
        }

        // Construct the expression to compute the measure for a single simulation member.
        // For derived tables, this is just the corresponding 'accumulator'.
        string measure_expr = "acc" + to_string(measure->index);

        // Construct the expression used to compute the measure over simulation members.
        // For derived tables this is the average across simulation members.
        tableExpr.srcExpr = scale_part + "OM_AVG(acc" + to_string(measure->index) + ")";

        // override the table measure expression if a matching //EXPR was supplied
        auto search = explicit_exprs.find(measure->unique_name);
        if (search != explicit_exprs.end()) {
            auto text = (search->second).first;
            auto loc = (search->second).second;
            tableExpr.srcExpr = text;
        }

        // save table measure metadata
        metaRows.tableExpr.push_back(tableExpr);

        // Labels and notes for measures
        for (const auto& langSym : Symbol::pp_all_languages) {
            const string& lang = langSym->name; // e.g. "EN" or "FR"
            TableExprTxtLangRow tableExprTxt;
            tableExprTxt.tableId = pp_table_id;
            tableExprTxt.exprId = measure->index;

            tableExprTxt.langCode = lang;

            tableExprTxt.descr = measure->label(*langSym);
            
            tableExprTxt.note = measure->note(*langSym);
            metaRows.tableExprTxt.push_back(tableExprTxt);
        }

        // Create derived accumulators for the table, one for each measure.
        TableAccRow tableAcc;
        tableAcc.tableId = pp_table_id;
        tableAcc.accId = ++maxAccIndex;
        tableAcc.name = tableExpr.name;
        tableAcc.isDerived = true;
        tableAcc.accSrc = scale_part + measure_expr;
        metaRows.tableAcc.push_back(tableAcc);

        // important:
        // after this point we cannot any longer use pp_accumulators
        // specially acc->index, which is out of sync with reality
        // as result of: tableAcc.accId = ++maxAccIndex;

        // TODO: add description and notes for that accumulator
        // But, these would be identical to the description and notes
        // for the corresponding measure.  So unclear whether
        // there's any point to duplicating that info.
    }
}
