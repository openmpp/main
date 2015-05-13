/**
* @file    DerivedTableSymbol.cpp
* Definitions for the DerivedTableSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DerivedTableSymbol.h"
#include "EnumerationSymbol.h"

using namespace std;

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

int DerivedTableSymbol::rank()
{
    return dimension_list.size();
}

int DerivedTableSymbol::cell_count()
{
    int cells = 1;
    for (auto dim : dimension_list) {
        auto es = dim->pp_enumeration;
        assert(es); // integrity check guarantee
        cells *= es->pp_size();
    }
    return cells;
}

void DerivedTableSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    //// Perform operations specific to this level in the Symbol hierarchy.
    //TableDicRow tableDic;

    //tableDic.tableId = pp_table_id;
    //tableDic.tableName = name;
    //tableDic.isUser = false;
    //tableDic.rank = rank();
    //tableDic.isSparse = true;   // do not store NULLs
    //tableDic.isHidden = false;
    //tableDic.exprPos = expr_dim_position;
    //metaRows.tableDic.push_back(tableDic);

    //for (auto lang : Symbol::pp_all_languages) {
    //    TableDicTxtLangRow tableTxt;
    //    tableTxt.tableId = pp_table_id;
    //    tableTxt.langName = lang->name;
    //    tableTxt.descr = label(*lang);
    //    tableTxt.note = note(*lang);
    //    tableTxt.exprDescr = "Expressions (" + lang->name + ")"; // TODO
    //    tableTxt.exprNote = "Expressions Note (" + lang->name + ")"; // TODO
    //    metaRows.tableTxt.push_back(tableTxt);
    //}

    //// dimensions for table
    //for (auto dim : dimension_list ) {
    //    auto av = dim->pp_attribute;
    //    assert(av); // logic guarantee
    //    auto es = dim->pp_enumeration;
    //    assert(es); // logic guarantee
    //    TableDimsRow tableDims;
    //    tableDims.tableId = pp_table_id;
    //    tableDims.dimId = dim->index;
    //    //tableDims.name = "dim" + to_string(dim->index);
    //    tableDims.typeId = es->type_id;
    //    tableDims.isTotal = false;
    //    tableDims.dimSize = es->pp_size();
    //    metaRows.tableDims.push_back(tableDims);

    //    for (auto lang : Symbol::pp_all_languages) {
    //        TableDimsTxtLangRow tableDimsTxt;
    //        tableDimsTxt.tableId = pp_table_id;
    //        tableDimsTxt.dimId = dim->index;
    //        tableDimsTxt.langName = lang->name;
    //        tableDimsTxt.descr = dim->label(*lang);
    //        tableDimsTxt.note = dim->note(*lang);
    //        metaRows.tableDimsTxt.push_back(tableDimsTxt);
    //    }
    //}

    //// accumulators for table
    //for (auto acc : pp_accumulators) {
    //    TableAccRow tableAcc;

    //    tableAcc.tableId = pp_table_id;
    //    tableAcc.accId = acc->index;
    //    // tableAcc.name = "acc" + to_string(acc->index);
    //    tableAcc.expr = acc->pretty_name();
    //    metaRows.tableAcc.push_back(tableAcc);

    //    for (auto lang : Symbol::pp_all_languages) {
    //        TableAccTxtLangRow tableAccTxt;
    //        tableAccTxt.tableId = pp_table_id;
    //        tableAccTxt.accId = acc->index;
    //        tableAccTxt.langName = lang->name;
    //        tableAccTxt.descr = acc->label(*lang);
    //        tableAccTxt.note = acc->note(*lang);
    //        metaRows.tableAccTxt.push_back(tableAccTxt);
    //    }
    //}

    //// expressions for table
    //for (auto expr : pp_expressions) {
    //    TableExprRow tableExpr;
    //    tableExpr.tableId = pp_table_id;
    //    tableExpr.exprId = expr->index;
    //    tableExpr.name = "expr" + to_string(expr->index);
    //    tableExpr.src = expr->get_expression(expr->root, TableExpressionSymbol::expression_style::sql);
    //    metaRows.tableExpr.push_back(tableExpr);

    //    for (auto lang : Symbol::pp_all_languages) {
    //        TableExprTxtLangRow tableExprTxt;
    //        tableExprTxt.tableId = pp_table_id;
    //        tableExprTxt.exprId = expr->index;

    //        tableExprTxt.langName = lang->name;

    //        // construct label by removing decimals=nnn from string if present
    //        tableExprTxt.descr = regexReplace(expr->label(*lang), "[[:space:]]+decimals=[[:digit:]]+", "");
    //        
    //        tableExprTxt.note = expr->note(*lang);
    //        metaRows.tableExprTxt.push_back(tableExprTxt);
    //    }
    //}
}




