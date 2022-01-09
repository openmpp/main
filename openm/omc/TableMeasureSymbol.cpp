/**
* @file    TableMeasureSymbol.cpp
* Definitions for the TableMeasureSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DerivedTableSymbol.h"
#include "TableMeasureSymbol.h"
#include "libopenm/common/omHelper.h"
#include "../libopenm/include/dbExec.h" // for isSqlKeyword

using namespace std;
using namespace openm;

// static
string TableMeasureSymbol::symbol_name(const Symbol* table, int index, string* pname_default)
{
    assert(table);
    return table->name + "." + (pname_default ? *pname_default : "Expr" + to_string(index));
}

string TableMeasureSymbol::pretty_name() const
{
    // example:     measure 0: sum(delta(interval(duration)))
    string result = " measure " + to_string(index) + ": " + short_name;
    return result;
}

// Update measure name to be suitable as database column name: it must be unique, alphanumeric and not longer than 255 chars
void TableMeasureSymbol::to_column_name(const string & i_tableName, const list<TableMeasureSymbol *> i_measureLst, TableMeasureSymbol * io_me)
{
    assert(io_me);
    string colName = openm::toAlphaNumeric(io_me->short_name, OM_NAME_DB_MAX);   // make measure name alphanumeric and truncate it to 255 chars

    for (auto pIt = i_measureLst.cbegin(); pIt != i_measureLst.cend() && *pIt != io_me; ++pIt) {
        if (colName == (*pIt)->short_name) {
            string sId = to_string(io_me->index);
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
    if (io_me->short_name != colName) {
        // to do: use pp_warning() if we can apply LT without string + concatenation
        theLog->logFormatted(LT("warning: %s measure [%d] name %s updated to %s"), i_tableName.c_str(), io_me->index, io_me->short_name.c_str(), colName.c_str());

        io_me->short_name = colName;      // change measure name
    }
}

void TableMeasureSymbol::post_parse(int pass)
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
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<TableSymbol *> (pp_symbol(table));
        assert(pp_table); // parser guarantee

        // Add this measure to the table's list of measures
        pp_table->pp_measures.push_back(this);

        break;
    }
    case eResolveDataTypes:
    {
        // Process additional information in expression label
        // Symbols labels are only complete after pass eAssignMembers,
        // so do this in the next pass, which is eResolveDataTypes
        regex rm_decimals("[[:space:]]+decimals=[[:digit:]]+");
        regex rm_scale("(^|[[:space:]]+)scale=-?[[:digit:]]+");

        for (auto & lbl : pp_labels) {

            // process decimals= specifier
            {
                //TODO use regexp when possible
                string srch("decimals=");
                auto p = lbl.find(srch);
                if (p != std::string::npos) {
                    auto q = p + srch.length();
                    if (q < lbl.length()) {
                        string value;
                        for (auto i = q; q < lbl.length(); ++i) {
                            auto ch = lbl[i];
                            if (isdigit(ch)) value += ch;
                            else break;
                        }
                        if (value.length() > 0) {
                            decimals = stoi(value);
                        }
                    }
                    // remove decimals=nnn from label
                    lbl = regex_replace(lbl, rm_decimals, "");
                }
            }

            // process scale= specifier
            {
                //TODO use regexp when possible
                string srch("scale=");
                auto p = lbl.find(srch);
                if (p != std::string::npos) {
                    //pp_warning(LT("warning : scale= in label of table measure is deprecated"));
                    auto q = p + srch.length();
                    if (q < lbl.length()) {
                        string value;
                        for (auto i = q; q < lbl.length(); ++i) {
                            auto ch = lbl[i];
                            // handle leading - sign
                            if (i == q && ch == '-') {
                                value += ch;
                                continue;
                            }
                            if (isdigit(ch)) value += ch;
                            else break;
                        }
                        if (value.length() > 0) {
                            scale = stoi(value);
                        }
                    }
                    // remove scale=nnn from label
                    lbl = regex_replace(lbl, rm_scale, "");
                }
            }

            // label cannot be empty, so provide something.
            // This can occur if label consists only of decimals=, etc.
            // TODO Should perhaps call default_label() instead, but requires language code
            // and re-working containing loop.
            if (lbl.length() == 0) {
                lbl = name;
            }

        }
        
        break;
    }
    default:
        break;
    }
}

string TableMeasureSymbol::heuristic_short_name(void) const
{
    string hn; // heuristic name

    // Get the measure's label for the model's default language.
    string lbl = pp_labels[0];
    string unm = unique_name;
    if (lbl != unm) {
        // Use label from model source code, and truncate if necessary
        hn = lbl;
    }
    else {
        // no label in source code
        if (pp_table->measure_count() == 1) {
            hn = "Measure";
        }
        else {
            hn = short_name_default;
        }
    }

    // trim off leading "om_" prefix if present
    //if (hn.starts_with("om_")) {
    if (hn.length() >= 3 && hn.substr(0, 3) == "om_") {
        hn.erase(0, 3);
    }

    assert(hn.length() > 0); // logic guarantee
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
