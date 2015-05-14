/**
* @file    TableSymbol.cpp
* Definitions for the TableSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <regex>
#include "TableSymbol.h"
#include "LanguageSymbol.h"
#include "EnumerationSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

// this function exist only because g++ below 4.9 does not support std::regex
static string regexReplace(const string & i_srcText, const char * i_pattern, const char * i_replaceWith);

void TableSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // add this derived table to the complete list of derived tables
        //TODO pp_all_derived_tables.push_back(this);

        break;
    }
    default:
        break;
    }
}

CodeBlock TableSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    size_t n_cells = cell_count();
    size_t n_measures = pp_placeholders.size();
    size_t n_rank = rank();
    //h += "typedef DerivedTable<"  + to_string(n_cells) + ", " + to_string(n_placeholders) + "> " + cxx_type + ";";
    h += "typedef BaseTable<"
        + to_string(n_measures) + ", "
        + to_string(n_rank) + ", "
        + to_string(n_cells)
        + "> " + cxx_type + ";";
    h += "extern thread_local "  + cxx_type + " * " + cxx_instance + ";";

    return h;
}

CodeBlock TableSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    size_t n_cells = cell_count();
    size_t n_placeholders = pp_placeholders.size();
    c += "thread_local "  + cxx_type + " * " + cxx_instance + " = nullptr;";

    return c;
}

int TableSymbol::rank() const
{
    return dimension_list.size();
}

int TableSymbol::cell_count() const
{
    int cells = 1;
    for (auto dim : dimension_list) {
        auto es = dim->pp_enumeration;
        assert(es); // integrity check guarantee
        cells *= es->pp_size();
    }
    return cells;
}

string TableSymbol::cxx_initializer() const
{
    string cxx;
    cxx = cxx_shape_initializer_list();
    return cxx;
}

string TableSymbol::cxx_shape_initializer_list() const
{
    string cxx = "{";
    bool first_dim = true;
    for (auto dim : dimension_list) {
        if (first_dim) {
            first_dim = false;
        }
        else {
            cxx += ", ";
        }
        auto es = dim->pp_enumeration;
        assert(es); // integrity check guarantee
        cxx += to_string(es->pp_size());
    }
    cxx += "}";

    return cxx;
}

string TableSymbol::cxx_measure_name_initializer_list() const
{
    string cxx = "{";
    //TODO
    bool first_dim = true;
    for (auto dim : dimension_list) {
        if (first_dim) {
            first_dim = false;
        }
        else {
            cxx += ", ";
        }
        auto es = dim->pp_enumeration;
        assert(es); // integrity check guarantee
        cxx += to_string(es->pp_size());
    }
    cxx += "}";

    return cxx;
}

void TableSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.
    TableDicRow tableDic;

    tableDic.tableId = pp_table_id;
    tableDic.tableName = name;
    tableDic.isUser = true;
    tableDic.rank = rank();
    tableDic.isSparse = true;   // do not store NULLs
    tableDic.isHidden = false;
    tableDic.exprPos = measures_position;
    metaRows.tableDic.push_back(tableDic);

    for (auto lang : Symbol::pp_all_languages) {
        TableDicTxtLangRow tableTxt;
        tableTxt.tableId = pp_table_id;
        tableTxt.langName = lang->name;
        tableTxt.descr = label(*lang);
        tableTxt.note = note(*lang);
        tableTxt.exprDescr = "Expressions (" + lang->name + ")"; // TODO
        tableTxt.exprNote = "Expressions Note (" + lang->name + ")"; // TODO
        metaRows.tableTxt.push_back(tableTxt);
    }

    // dimensions for table
    for (auto dim : dimension_list ) {
        auto es = dim->pp_enumeration;
        assert(es); // logic guarantee
        TableDimsRow tableDims;
        tableDims.tableId = pp_table_id;
        tableDims.dimId = dim->index;
        //tableDims.name = "dim" + to_string(dim->index);
        tableDims.typeId = es->type_id;
        tableDims.isTotal = false;
        tableDims.dimSize = es->pp_size();
        metaRows.tableDims.push_back(tableDims);

        for (auto lang : Symbol::pp_all_languages) {
            TableDimsTxtLangRow tableDimsTxt;
            tableDimsTxt.tableId = pp_table_id;
            tableDimsTxt.dimId = dim->index;
            tableDimsTxt.langName = lang->name;
            tableDimsTxt.descr = dim->label(*lang);
            tableDimsTxt.note = dim->note(*lang);
            metaRows.tableDimsTxt.push_back(tableDimsTxt);
        }
    }

    // 'accumulators' for derived table
    for (auto acc : pp_placeholders) {
        TableAccRow tableAcc;

        tableAcc.tableId = pp_table_id;
        tableAcc.accId = acc->index;
        // tableAcc.name = "acc" + to_string(acc->index);
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

    // 'expressions' for derived table
    // Just average the values across simulation members
    for (auto expr : pp_placeholders) {
        TableExprRow tableExpr;
        tableExpr.tableId = pp_table_id;
        tableExpr.exprId = expr->index;
        tableExpr.name = "expr" + to_string(expr->index);
        tableExpr.src = "OM_AVG(acc" + to_string(expr->index) + ")";
        metaRows.tableExpr.push_back(tableExpr);

        for (auto lang : Symbol::pp_all_languages) {
            TableExprTxtLangRow tableExprTxt;
            tableExprTxt.tableId = pp_table_id;
            tableExprTxt.exprId = expr->index;

            tableExprTxt.langName = lang->name;

            // construct label by removing decimals=nnn from string if present
            tableExprTxt.descr = regexReplace(expr->label(*lang), "[[:space:]]+decimals=[[:digit:]]+", "");
            
            tableExprTxt.note = expr->note(*lang);
            metaRows.tableExprTxt.push_back(tableExprTxt);
        }
    }
}

// this function exist only because g++ below 4.9 does not support std::regex
#ifdef _WIN32

string regexReplace(const string & i_srcText, const char * i_pattern, const char * i_replaceWith)
{
    regex pat(i_pattern);
    return regex_replace(i_srcText, pat, i_replaceWith);
}

#else

#include <regex.h>

#define MAX_RE_ERROR_MSG    1024

string regexReplace(const string & i_srcText, const char * i_pattern, const char * i_replaceWith)
{
    // prepare regex
    regex_t re;

    int nRet = regcomp(&re, i_pattern, REG_EXTENDED | REG_NEWLINE);
    if (nRet != 0) {
        char errMsg[MAX_RE_ERROR_MSG + 1];
        regerror(nRet, &re, errMsg, sizeof(errMsg));
        throw HelperException("Regular expression error: %s", errMsg);
    }

    // replace first occurrence of pattern in source text
    string sResult;
    regmatch_t matchPos;

    nRet = regexec(&re, i_srcText.c_str(), 1, &matchPos, 0);
    if (nRet == REG_NOMATCH) {
        regfree(&re);           // cleanup
        return i_srcText;       // pattern not found - return source text as is
    }
    if (nRet != 0) {            // error 
        regfree(&re);           // cleanup
        throw HelperException("Regular expression FAILED: %s", i_pattern);
    }
    // at this point nRet == 0 and we can replace first occurrence with replacement string
    if (matchPos.rm_so >= 0 && matchPos.rm_so < (int)i_srcText.length()) {

        string sResult =
            i_srcText.substr(0, matchPos.rm_so) +
            i_replaceWith +
            ((matchPos.rm_eo >= 0 && matchPos.rm_eo < (int)i_srcText.length()) ? i_srcText.substr(matchPos.rm_eo) : "");

        regfree(&re);       // cleanup
        return sResult;
    }
    else {                  // starting offset out of range - pattern not found
        regfree(&re);       // cleanup
        return i_srcText;   // return source text as is
    }
}

#endif // _WIN32



