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
#include "../libopenm/include/dbExec.h" // for isSqlKeyword

using namespace openm;

// static
string DimensionSymbol::symbol_name(const Symbol* symbol_with_dimensions, int index, bool after_analysis_dim)
{
    assert(symbol_with_dimensions);
    //// For entity tables, the numeric identifier is based on all dimensions, including the analysis dimension
    //int numeric_id = index + (after_analysis_dim ? 1 : 0);
    //return symbol_with_dimensions->name + ".Dim" + to_string(numeric_id);
    return symbol_with_dimensions->name + ".Dim" + to_string(index);
}

// Update dimension name to be suitable as database column name: it must be unique, alphanumeric and not longer than 32 chars
void DimensionSymbol::to_column_name(const string & i_ownerName, const list<DimensionSymbol *> i_dimLst, DimensionSymbol * io_dim)
{
    assert(io_dim);
    string colName = openm::toAlphaNumeric(io_dim->short_name, OM_CODE_DB_MAX);  // make dimension name alphanumeric and truncate it to 32 chars

    for (auto pIt = i_dimLst.cbegin(); pIt != i_dimLst.cend() && *pIt != io_dim; ++pIt) {
        if (colName == (*pIt)->short_name) {
            string sId = to_string(io_dim->index);
            if (colName.length() + sId.length() <= OM_CODE_DB_MAX) {
                // append disambiguating unique numeric suffix
                colName += sId;
            }
            else {
                // replace trailing characters of name with numeric suffix
                colName = colName.replace(colName.length() - sId.length(), sId.length(), sId);
            }
            break;
        }
    }
    if (io_dim->short_name != colName) {
        // to do: use pp_warning() if we can apply LT without string + concatenation
        theLog->logFormatted(LT("warning: %s dimension [%d] name %s updated to %s"), i_ownerName.c_str(), io_dim->index, io_dim->short_name.c_str(), colName.c_str());

        io_dim->short_name = colName;      // change dimension short name
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
            short_name_explicit = search->second;
            short_name = short_name_explicit;
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

string DimensionSymbol::heuristic_short_name(void) const
{
    string hn; // heuristic name

    // Get the dimension's label for the model's default language.
    string lbl = pp_labels[0];
    string unm = unique_name;
    if (lbl != unm && lbl.length() <= OM_CODE_DB_MAX) {
        // Use label from model source code, if not too long.
        hn = lbl;
    }
    else {
        // Get the dimension's attribute, if present (if dimension is in an entity table).
        auto attr = pp_attribute;
        if (attr) {
            // Dimension has an attribute.
            // Base the heuristic name on the dimension's attribute.
            string lbl = attr->pp_labels[0];
            string unm = attr->unique_name;
            if (lbl != unm && lbl.length() <= OM_CODE_DB_MAX) {
                // Use explicit label from model source code.
                hn = lbl;
            }
            else {
                // No explicit label in source code, or is too long.
                // Use the same name as model source (unqualified by entity name).
                hn = attr->name;
            }
        }
        else {
            // Base the name on the dimension's underlying enumeration.
            assert(pp_enumeration); // logic guarantee
            string lbl = pp_enumeration->pp_labels[0];
            string unm = pp_enumeration->unique_name;
            if (lbl != unm && lbl.length() <= OM_CODE_DB_MAX) {
                // Use explicit label from model source code.
                hn = lbl;
            }
            else {
                // No explicit label in source code, or is too long
                // Use the same name as model source code.
                hn = unm;
            }
        }
    }

    assert(hn.length() > 0); // logic guarantee

    // trim off leading "om_" prefix if present
    //if (hn.starts_with("om_")) { // c++20
    if (hn.length() >= 3 && hn.substr(0, 3) == "om_") {
        hn.erase(0,3);
    }

    if (!std::isalpha(hn[0], locale::classic())) {
        // First character is not alphabetic.
        // Prepend X_ to make valid name
        hn = "X_" + hn;
    }

    if (IDbExec::isSqlKeyword(hn.c_str())) {
        // Name clashes with a SQL reserved word.
        // Prepend X_ to make valid name
        hn = "X_" + hn;
    }

    // trim off trailing "_" if present
    //if (hn.ends_with("_")) { // c++20
    if (hn[hn.length() - 1] == '_') {
        hn.erase(hn.length() - 1, 1);
    }

    // if name is subject to truncation, remove characters from middle rather than truncating from end,
    // because long descriptions often disambiguate at both beginning and end.
    if (hn.length() > OM_CODE_DB_MAX) {
        // replacement string for snipped section
        string r = "_x_";
        // number of characters to snip from middle (excess plus length of replacement)
        size_t n = hn.length() - OM_CODE_DB_MAX + r.length();
        // start of snipped section (middle section of original string)
        size_t p = (hn.length() - n) / 2;
        // replace snipped section with _
        if (p > 0 && p < hn.length()) {
            hn.replace(p, n, r);
        }
    }

    // Make name alphanumeric and truncate it to 32 chars.
    hn = openm::toAlphaNumeric(hn, OM_CODE_DB_MAX);

    // trim off trailing "_" if present
    //if (hn.ends_with("_")) { // c++20
    if (hn[hn.length() - 1] == '_') {
        hn.erase(hn.length() - 1, 1);
    }

    return hn;
}
