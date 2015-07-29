/**
* @file    TableSymbol.cpp
* Definitions for the TableSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "TableSymbol.h"
#include "LanguageSymbol.h"
#include "EnumerationSymbol.h"
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
        // add this table to the complete list of tables
        pp_all_tables.push_back(this);

        break;
    }
    case ePopulateDependencies:
    {
        // Mark enumerations required for metadata support for this table
        // The enumeration of each dimension in the table is required
        for (auto dim : dimension_list) {
            auto es = dim->pp_enumeration;
            assert(es); // logic guarantee
            es->metadata_needed = true;
        }

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

    return h;
}

CodeBlock TableSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    return c;
}

size_t TableSymbol::cell_count() const
{
    size_t cells = 1;
    for (auto dim : dimension_list) {
        auto es = dim->pp_enumeration;
        assert(es); // integrity check guarantee
        cells *= es->pp_size();
    }
    return cells;
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
    cxx += "}";

    return cxx;
}

string TableSymbol::metadata_signature() const
{
    // Hook into the hierarchical calling chain
    string sig = super::metadata_signature();

    // Perform operations specific to this level in the Symbol hierarchy.
    sig += "dimensions: " + to_string(dimension_count()) + "\n";
    sig += "measures: " + to_string(measure_count()) + "\n";
    int dim_index = 0;
    for (auto dim : dimension_list) {
        sig += "dimension: " + to_string(dim_index) + "\n";
        sig += "has_margin: " + to_string(dim->has_margin) + "\n";
        auto enumeration = dim->pp_enumeration;
        sig += enumeration->metadata_signature();
        dim_index++;
    }

    return sig;
}

void TableSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    // The table
    {
        TableDicRow tableDic;

        tableDic.tableId = pp_table_id;
        tableDic.tableName = name;
        tableDic.isUser = true;
        tableDic.rank = dimension_count();
        tableDic.isSparse = true;   // do not store zeroes
        tableDic.isHidden = false;
        tableDic.exprPos = measures_position;
        metaRows.tableDic.push_back(tableDic);

        // Labels and notes for the table
        for (auto lang : Symbol::pp_all_languages) {

            TableDicTxtLangRow tableTxt;

            tableTxt.tableId = pp_table_id;
            tableTxt.langName = lang->name;
            tableTxt.descr = label(*lang);
            tableTxt.note = note(*lang);
            tableTxt.exprDescr = "Measures (" + lang->name + ")"; // TODO
            tableTxt.exprNote = "Measures Note (" + lang->name + ")"; // TODO
            metaRows.tableTxt.push_back(tableTxt);
        }
    }

    // The dimensions of the table
    for (auto dim : dimension_list ) {

        TableDimsRow tableDims;

        auto es = dim->pp_enumeration;
        assert(es); // logic guarantee
        tableDims.tableId = pp_table_id;
        tableDims.dimId = dim->index;
        //tableDims.name = "dim" + to_string(dim->index);
        tableDims.typeId = es->type_id;
        tableDims.isTotal = false;
        tableDims.dimSize = es->pp_size();
        metaRows.tableDims.push_back(tableDims);

        // Labels and notes for the dimensions of the table
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
}
