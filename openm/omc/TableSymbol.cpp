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
#include "EntityTableSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

bool TableSymbol::is_entity_table(void)
{
    return dynamic_cast<EntityTableSymbol*>(this);
}

void TableSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eResolveDataTypes:
    {
        if (any_tables_retain) { // model contains a tables_retain statement
            // Mark all tables as suppressed.
            // Those which are retained will be changed back in a subsequent pass.
            if (!is_internal) {
                is_suppressed_table = true;
            }
        }
        if (any_show) { // model contains a show statement
            // Mark all tables as hidden
            // Those which are shown will be changed back in a subsequent pass.
            is_hidden = true;

        }
        break;
    }
    case ePopulateCollections:
    {
        // add this table to the complete list of tables
        pp_all_tables.push_back(this);

        break;
    }
    default:
        break;
    }
}

void TableSymbol::post_parse_mark_enumerations(void)
{
    if (true) {
    //SFG if (!is_internal && !is_suppressed_table) {
        // Mark enumerations required for metadata support for this table
        // The enumeration of each dimension in the table is required
        for (auto dim : dimension_list) {
            auto es = dim->pp_enumeration;
            assert(es); // logic guarantee
            es->metadata_needed = true;
        }
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
        cells *= es->pp_size() + dim->has_margin;
    }
    return cells;
}

size_t TableSymbol::margin_count() const
{
    size_t result = 0;
    for (auto dim : dimension_list) {
        result += dim->has_margin;
    }
    return result;
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
        cxx += to_string(es->pp_size() + dim->has_margin);
    }
    cxx += "}";

    return cxx;
}

string TableSymbol::cxx_margin_dims_initializer_list() const
{
    string cxx = "{";
    bool first_value = true;
    size_t dim_index = 0;
    for (auto dim : dimension_list) {
        if (dim->has_margin) {
            if (first_value) {
                first_value = false;
            }
            else {
                cxx += ", ";
            }
            cxx += to_string(dim_index);
        }
        ++dim_index;
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

void TableSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    // The table
    {
         // The table

        TableDicRow tableDic;

        tableDic.tableId = pp_table_id;
        tableDic.tableName = name;
        tableDic.rank = dimension_count();
        tableDic.isSparse = false;          // sparse is no longer supported
        tableDic.exprPos = measures_position;
        tableDic.isHidden = is_hidden;
        metaRows.tableDic.push_back(tableDic);

        // Labels and notes for the table
        for (auto lang : Symbol::pp_all_languages) {

            TableDicTxtLangRow tableTxt;

            tableTxt.tableId = pp_table_id;
            tableTxt.langCode = lang->name;
            tableTxt.descr = label(*lang);
            tableTxt.note = note(*lang);

            // label and note for measure dimension
            assert(measure_dimension);
            tableTxt.exprDescr = measure_dimension->label(*lang);
            tableTxt.exprNote = measure_dimension->note(*lang);

            metaRows.tableTxt.push_back(tableTxt);
        }
    }

    // The dimensions of the table
    for (auto dimIt = dimension_list.begin(); dimIt != dimension_list.end(); ++dimIt) {

        DimensionSymbol * dim = *dimIt;

        // make dimension db column name: it must be unique, alpanumeric and not longer than 255 chars
        DimensionSymbol::to_column_name(name, dimension_list, dim);

        TableDimsRow tableDims;

        auto es = dim->pp_enumeration;
        assert(es); // logic guarantee
        tableDims.tableId = pp_table_id;
        tableDims.dimId = dim->index;
        tableDims.name = dim->short_name;     // Default is dim0, dim1, but can be named in model using =>
        tableDims.typeId = es->type_id;
        tableDims.isTotal = dim->has_margin;
        tableDims.dimSize = es->pp_size() + dim->has_margin;
        metaRows.tableDims.push_back(tableDims);

        // Labels and notes for the dimensions of the table
        for (auto lang : Symbol::pp_all_languages) {

            TableDimsTxtLangRow tableDimsTxt;

            tableDimsTxt.tableId = pp_table_id;
            tableDimsTxt.dimId = dim->index;
            tableDimsTxt.langCode = lang->name;
            tableDimsTxt.descr = dim->label(*lang);
            tableDimsTxt.note = dim->note(*lang);
            metaRows.tableDimsTxt.push_back(tableDimsTxt);
        }
    }
}
