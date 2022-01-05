/**
* @file    DimensionSymbol.h
* Definitions for the DimensionSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DimensionSymbol.h"
#include "AttributeSymbol.h"
#include "LinkToAttributeSymbol.h"
#include "EnumerationSymbol.h"
#include "libopenm/common/omHelper.h"

// static
string DimensionSymbol::symbol_name(const Symbol* symbol_with_dimensions, int index, bool after_analysis_dim)
{
    assert(symbol_with_dimensions);
    // For entity tables, the numeric identifier is based on all dimensions, including the analysis dimension
    int numeric_id = index + (after_analysis_dim ? 1 : 0);
    return symbol_with_dimensions->name + ".Dim" + to_string(numeric_id);
}

// Update dimension name to be suitable as database column name: it must be unique, alpanumeric and not longer than 255 chars
void DimensionSymbol::to_column_name(const string & i_ownerName, const list<DimensionSymbol *> i_dimLst, DimensionSymbol * io_dim)
{
    assert(io_dim);
    string colName = openm::toAlphaNumeric(io_dim->dim_name, OM_NAME_DB_MAX);  // make dimension name alpanumeric and truncate it to 255 chars

    for (auto pIt = i_dimLst.cbegin(); pIt != i_dimLst.cend() && *pIt != io_dim; ++pIt) {
        if (colName == (*pIt)->dim_name) {
            string sId = to_string(io_dim->index);
            colName = colName.replace(colName.length() - sId.length(), sId.length(), sId);
            break;
        }
    }
    if (io_dim->dim_name != colName) {
        // to do: use pp_warning() if we can apply LT without string + concatenation
        theLog->logFormatted(LT("warning: %s dimension [%d] name %s updated to %s"), i_ownerName.c_str(), io_dim->index, io_dim->dim_name.c_str(), colName.c_str());

        io_dim->dim_name = colName;      // change dimension name
    }
}

void DimensionSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignLabel:
    {
        // If an explicit short name was given by //NAME, use it
        auto search = explicit_names.find(unique_name);
        if (search != explicit_names.end()) {
            dim_name = search->second;
        }
        break;
    }
    case eAssignMembers:
    {
        if (attribute) {
            // Attribute was assigned during parsing.
            // The dimension symbol is in an entity table or entity set.
            // assign direct pointer to attribute for post-parse use
            pp_attribute = dynamic_cast<AttributeSymbol *> (pp_symbol(attribute));
            if (!pp_attribute) {
                pp_error(LT("error : '") + (*attribute)->name + LT("' must be an attribute to be used as a dimension"));
            }
        }
        else {
            assert(enumeration); // grammar guarnatee
            // Attribute was not assigned during parsing.
            // The dimension symbol is in a derived table.
            // There is no attribute associated with this dimension.
            // An enumeration was specified for this dimension.
            pp_enumeration = dynamic_cast<EnumerationSymbol *> (pp_symbol(enumeration));
            if (!pp_enumeration) {
                pp_error(LT("error : '") + (*enumeration)->name + LT("' must be an enumeration to be used as a dimension"));
            }
        }
        break;
    }
    case ePopulateCollections:
    {
        if (pp_attribute) {
            // Assign direct pointer to enumeration for post-parse use
            pp_enumeration = dynamic_cast<EnumerationSymbol *>(pp_attribute->pp_data_type);
            if (!pp_enumeration) {
                pp_error(LT("error : the data-type of '") + pp_attribute->name + LT("' must be an enumeration to be used as a dimension"));
            }
        }
        break;
    }
    default:
        break;
    }
}
